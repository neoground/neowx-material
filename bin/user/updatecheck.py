#
# Copyright (c) 2025-2026 UpdateCheck Extension for NeoWX Material
#
# Distributed under the terms of the GNU GENERAL PUBLIC LICENSE
#

"""Extends the Cheetah generator search list to check for skin and WeeWX updates.

[CheetahGenerator]
    search_list_extensions = user.updatecheck.UpdateCheck

# Update Check behavior
# -------------------------------------------------------------------------
# Configure under [Extras] -> [[Footer]] in skin.conf:
#
[[Footer]]
    # Update check modes:
    # off - No update checking performed
    # minor - Only show notifications for minor/major version updates (e.g., 2.1.x -> 2.2.x)
    # patch - Show notifications for all version updates including patches (e.g., 2.2.0 -> 2.2.1)
    update_check = minor

    # Optional independent mode for WeeWX core updates (defaults to update_check if omitted/empty)
    # weewx_update_check = minor

    # Update check interval in minutes (default: 1440 minutes = 24 hours = 1 day)
    update_interval = 1440

    # URL to check for the latest skin version
    update_check_url = https://raw.githubusercontent.com/seehase/neowx-material/master/skins/neowx-material/skin.conf

    # URL to check for the latest WeeWX release (default uses GitHub latest release redirect)
    weewx_update_check_url = https://github.com/weewx/weewx/releases/latest

"""

import json
import logging
import re
import time
import urllib.error
import urllib.parse
import urllib.request

try:
    from packaging import version
    HAVE_PACKAGING = True
except ImportError:
    HAVE_PACKAGING = False
    version = None

try:
    import weewx
except ImportError:
    weewx = None

try:
    from weewx.cheetahgenerator import SearchList
except ImportError:
    # Fallback when imported outside of WeeWX (e.g. unit tests)
    class SearchList:
        def __init__(self, generator):
            self.generator = generator

VERSION = "1.1.0"

log = logging.getLogger(__name__)

DEFAULT_SKIN_CHECK_URL = (
    "https://raw.githubusercontent.com/seehase/neowx-material/master/skins/neowx-material/skin.conf"
)
DEFAULT_SKIN_GITHUB_URL = "https://github.com/seehase/neowx-material"
DEFAULT_WEEWX_CHECK_URL = "https://github.com/weewx/weewx/releases/latest"
DEFAULT_WEEWX_FALLBACK_URL = "https://pypi.org/pypi/weewx/json"
DEFAULT_WEEWX_RELEASES_BASE_URL = "https://github.com/weewx/weewx/releases/tag/v"

DEFAULT_UPDATE_INTERVAL = 1440  # 24 hours in minutes
CACHE_DURATION = 24 * 60 * 60  # 24 hours in seconds
FAILURE_COOLDOWN = 300  # 5 minutes cooldown after failed fetches
TIMEOUT = 15  # seconds
RETRIES = 3
DELAY = 5  # seconds between retries
MAX_RESPONSE_BYTES = 512 * 1024  # 512 KiB safety cap
USER_AGENT = "neowx-material-updatecheck/%s" % VERSION

_STRICT_VERSION_RE = re.compile(r"^[0-9]+(?:\.[0-9]+){1,3}$")
_VALID_MODES = ("off", "minor", "patch")

# Global cache dictionary - independent entries for skin and WeeWX
_update_cache = {
    "skin": {"last_check": 0, "data": None, "last_failure": 0, "url": None},
    "weewx": {"last_check": 0, "data": None, "last_failure": 0, "url": None},
}


class _NoRedirectHandler(urllib.request.HTTPRedirectHandler):
    """HTTP redirect handler that stops at 3xx responses so Location can be inspected."""

    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


def _normalize_version(ver):
    """Strip whitespace and any leading 'v'/'V' prefix."""
    if ver is None:
        return None
    cleaned = str(ver).strip()
    if cleaned.startswith(("v", "V")):
        cleaned = cleaned[1:]
    return cleaned or None


def _validate_remote_version(ver):
    """Strictly validate a remotely fetched stable version string."""
    cleaned = _normalize_version(ver)
    if not cleaned or not _STRICT_VERSION_RE.match(cleaned):
        return None
    return cleaned


def _is_valid_https_url(url):
    """Ensure update check URLs strictly use HTTPS with a non-empty host."""
    if not url or not isinstance(url, str):
        return False
    parsed = urllib.parse.urlparse(url.strip())
    return parsed.scheme == "https" and bool(parsed.netloc)


def _parse_version(ver):
    """Fallback version parser when packaging.version is unavailable.

    Returns ((major, minor, patch, ...), stability_flag) where stability_flag is
    0 for pre-releases (e.g. 5.6.0b1) and 1 for final releases (e.g. 5.6.0).
    """
    cleaned = _normalize_version(ver) or ""
    match = re.match(r"^([0-9]+(?:\.[0-9]+)*)(.*)$", cleaned)
    if not match:
        return ((), 1)
    nums = tuple(int(x) for x in match.group(1).split("."))
    suffix = match.group(2).strip().lower()
    is_prerelease = bool(suffix and re.match(r"^(?:a|alpha|b|beta|rc|dev|\.dev)", suffix))
    return (nums, 0 if is_prerelease else 1)


def _compare_fallback_versions(current_ver, latest_ver):
    """Compare two version strings using fallback tuple logic.

    Returns:
        -1 if current < latest
         0 if current == latest
         1 if current > latest
    """
    cur_nums, cur_stable = _parse_version(current_ver)
    lat_nums, lat_stable = _parse_version(latest_ver)
    max_len = max(len(cur_nums), len(lat_nums))
    cur_padded = cur_nums + (0,) * (max_len - len(cur_nums))
    lat_padded = lat_nums + (0,) * (max_len - len(lat_nums))
    cur_key = (cur_padded, cur_stable)
    lat_key = (lat_padded, lat_stable)
    if cur_key < lat_key:
        return -1
    if cur_key > lat_key:
        return 1
    return 0


def _get_current_weewx_version(generator=None):
    """Determine the currently running WeeWX version."""
    if weewx is not None:
        ver = getattr(weewx, "__version__", None)
        if ver:
            return _normalize_version(ver)
    if generator is not None:
        stn_info = getattr(generator, "stn_info", None)
        if stn_info is not None:
            ver = getattr(stn_info, "version", None)
            if ver:
                return _normalize_version(ver)
    return None


class UpdateCheck(SearchList):

    def __init__(self, generator):
        super().__init__(generator)
        log.info("UpdateCheck version: %s", VERSION)

        self.generator = generator
        extras = generator.skin_dict.get("Extras", {})
        self.footer_dict = extras.get("Footer", {})
        self.current_version = _normalize_version(extras.get("version", "0.0.0"))
        self.current_weewx_version = _get_current_weewx_version(generator)

        # Configuration
        raw_mode = str(self.footer_dict.get("update_check", "off")).strip().lower()
        self.update_check_mode = raw_mode if raw_mode in _VALID_MODES else "off"

        raw_weewx_mode = str(self.footer_dict.get("weewx_update_check", "")).strip().lower()
        if raw_weewx_mode in _VALID_MODES:
            self.weewx_update_check_mode = raw_weewx_mode
        else:
            self.weewx_update_check_mode = self.update_check_mode

        try:
            self.update_interval = max(
                1, int(self.footer_dict.get("update_interval", str(DEFAULT_UPDATE_INTERVAL)))
            )
        except (ValueError, TypeError):
            self.update_interval = DEFAULT_UPDATE_INTERVAL

        self.update_check_url = str(
            self.footer_dict.get("update_check_url", DEFAULT_SKIN_CHECK_URL)
        ).strip() or DEFAULT_SKIN_CHECK_URL

        self.weewx_update_check_url = str(
            self.footer_dict.get("weewx_update_check_url", DEFAULT_WEEWX_CHECK_URL)
        ).strip() or DEFAULT_WEEWX_CHECK_URL

        # Convert interval from minutes to seconds
        self.cache_duration = self.update_interval * 60

    def update_check(self):
        """Main function that returns update information for skin and WeeWX."""
        if self.update_check_mode == "off" and self.weewx_update_check_mode == "off":
            log.debug("UpdateCheck: Update check is disabled")
            return None

        skin_update = None
        weewx_update = None

        if self.update_check_mode != "off" and self.current_version:
            try:
                latest_skin = self._get_latest_skin_version()
                if latest_skin:
                    skin_update = self._compare_versions(
                        self.current_version,
                        latest_skin,
                        mode=self.update_check_mode,
                        github_url=DEFAULT_SKIN_GITHUB_URL,
                    )
            except Exception as e:
                log.debug("UpdateCheck: Error checking skin update: %s", e)

        if self.weewx_update_check_mode != "off" and self.current_weewx_version:
            try:
                latest_weewx = self._get_latest_weewx_version()
                if latest_weewx:
                    weewx_url = DEFAULT_WEEWX_RELEASES_BASE_URL + latest_weewx
                    weewx_update = self._compare_versions(
                        self.current_weewx_version,
                        latest_weewx,
                        mode=self.weewx_update_check_mode,
                        github_url=weewx_url,
                    )
            except Exception as e:
                log.debug("UpdateCheck: Error checking WeeWX update: %s", e)

        if not skin_update and not weewx_update:
            return None

        # Keep legacy top-level fields populated for backwards compatibility
        primary = skin_update if skin_update else weewx_update
        return {
            "available": True,
            "skin": skin_update,
            "weewx": weewx_update,
            "current_version": primary["current_version"],
            "latest_version": primary["latest_version"],
            "update_type": primary["update_type"],
            "github_url": primary["github_url"],
        }

    def _get_cached_or_cooldown(self, cache_key, target_url, now):
        """Return (use_cache, cached_value) based on cache TTL and failure cooldown."""
        entry = _update_cache[cache_key]
        if entry["url"] != target_url:
            return (False, None)

        if entry["data"] and (now - entry["last_check"]) < self.cache_duration:
            log.debug(
                "UpdateCheck: Using cached %s version %s (interval: %d minutes)",
                cache_key,
                entry["data"],
                self.update_interval,
            )
            return (True, entry["data"])

        cooldown = min(self.cache_duration, FAILURE_COOLDOWN)
        if entry["last_failure"] and (now - entry["last_failure"]) < cooldown:
            log.debug(
                "UpdateCheck: Skipping %s fetch during failure cooldown (%ds)",
                cache_key,
                cooldown,
            )
            return (True, None)

        return (False, None)

    def _store_cache_success(self, cache_key, target_url, ver, now):
        _update_cache[cache_key] = {
            "last_check": int(now),
            "data": ver,
            "last_failure": 0,
            "url": target_url,
        }

    def _store_cache_failure(self, cache_key, target_url, now):
        entry = _update_cache[cache_key]
        entry["last_failure"] = int(now)
        entry["url"] = target_url

    def _get_latest_version(self):
        """Legacy alias for fetching the latest skin version."""
        return self._get_latest_skin_version()

    def _get_latest_skin_version(self):
        """Get the latest skin version from GitHub with caching."""
        now = time.time()
        github_url = self.update_check_url

        if not _is_valid_https_url(github_url):
            log.warning("UpdateCheck: Refusing non-HTTPS skin update_check_url: %s", github_url)
            return None

        use_cache, cached = self._get_cached_or_cooldown("skin", github_url, now)
        if use_cache:
            return cached

        for attempt in range(1, RETRIES + 1):
            try:
                log.debug("UpdateCheck: Fetching skin version from %s (attempt %d)", github_url, attempt)
                req = urllib.request.Request(github_url, headers={"User-Agent": USER_AGENT})
                with urllib.request.urlopen(req, timeout=TIMEOUT) as response:
                    if response.status != 200:
                        raise Exception("HTTP error: %s" % response.status)

                    content = response.read(MAX_RESPONSE_BYTES).decode("utf-8", errors="replace")
                    version_match = re.search(
                        r"^\s*version\s*=\s*([0-9]+(?:\.[0-9]+){1,3})\b",
                        content,
                        re.MULTILINE,
                    )
                    if not version_match:
                        raise Exception("Version not found in remote skin.conf")

                    latest_version = _validate_remote_version(version_match.group(1))
                    if not latest_version:
                        raise Exception("Invalid version format in remote skin.conf")

                    log.debug("UpdateCheck: Found latest skin version: %s", latest_version)
                    self._store_cache_success("skin", github_url, latest_version, now)
                    return latest_version

            except (urllib.error.URLError, urllib.error.HTTPError, OSError, Exception) as e:
                if attempt < RETRIES:
                    log.debug("UpdateCheck: Skin fetch retry in %d seconds... (%s)", DELAY, e)
                    time.sleep(DELAY)
                else:
                    log.info("UpdateCheck: Failed to fetch latest skin version after %d attempts", RETRIES)
                    log.debug("UpdateCheck: Last skin fetch error: %s", e)

        self._store_cache_failure("skin", github_url, now)
        return None

    def _get_latest_weewx_version(self):
        """Get the latest stable WeeWX release version with caching and fallback."""
        now = time.time()
        primary_url = self.weewx_update_check_url

        if not _is_valid_https_url(primary_url):
            log.warning("UpdateCheck: Refusing non-HTTPS weewx_update_check_url: %s", primary_url)
            return None

        use_cache, cached = self._get_cached_or_cooldown("weewx", primary_url, now)
        if use_cache:
            return cached

        urls_to_try = [primary_url]
        if primary_url == DEFAULT_WEEWX_CHECK_URL:
            urls_to_try.append(DEFAULT_WEEWX_FALLBACK_URL)

        for url in urls_to_try:
            for attempt in range(1, RETRIES + 1):
                try:
                    log.debug(
                        "UpdateCheck: Fetching WeeWX version from %s (attempt %d)",
                        url,
                        attempt,
                    )
                    latest_version = self._fetch_weewx_version_from_url(url)
                    if latest_version:
                        log.debug("UpdateCheck: Found latest WeeWX version: %s", latest_version)
                        self._store_cache_success("weewx", primary_url, latest_version, now)
                        return latest_version
                    raise Exception("WeeWX version not found in response")
                except (urllib.error.URLError, urllib.error.HTTPError, OSError, ValueError, Exception) as e:
                    if attempt < RETRIES:
                        log.debug("UpdateCheck: WeeWX fetch retry in %d seconds... (%s)", DELAY, e)
                        time.sleep(DELAY)
                    else:
                        log.info(
                            "UpdateCheck: Failed to fetch latest WeeWX version from %s after %d attempts",
                            url,
                            RETRIES,
                        )
                        log.debug("UpdateCheck: Last WeeWX fetch error: %s", e)

        self._store_cache_failure("weewx", primary_url, now)
        return None

    def _fetch_weewx_version_from_url(self, url):
        """Fetch and parse a validated WeeWX version from GitHub redirect, GitHub API, or PyPI JSON."""
        if not _is_valid_https_url(url):
            raise ValueError("URL must use https")

        parsed = urllib.parse.urlparse(url)
        host = (parsed.hostname or "").lower()
        path = parsed.path.rstrip("/")

        # 1. GitHub web release redirect: https://github.com/<owner>/<repo>/releases/latest
        if host == "github.com" and path.endswith("/releases/latest"):
            opener = urllib.request.build_opener(_NoRedirectHandler)
            req = urllib.request.Request(
                url,
                method="HEAD",
                headers={"User-Agent": USER_AGENT},
            )
            location = None
            try:
                with opener.open(req, timeout=TIMEOUT) as response:
                    location = response.geturl()
            except urllib.error.HTTPError as e:
                if e.code in (301, 302, 303, 307, 308):
                    location = e.headers.get("Location")
                else:
                    raise

            if not location:
                return None

            tag_match = re.search(
                r"/releases/tag/v?([0-9]+(?:\.[0-9]+){1,3})(?:[/?#]|$)",
                location,
            )
            if not tag_match:
                return None
            return _validate_remote_version(tag_match.group(1))

        # 2. JSON API endpoints (PyPI JSON or GitHub Releases REST API)
        req = urllib.request.Request(
            url,
            headers={
                "User-Agent": USER_AGENT,
                "Accept": "application/json",
            },
        )
        with urllib.request.urlopen(req, timeout=TIMEOUT) as response:
            if response.status != 200:
                raise Exception("HTTP error: %s" % response.status)

            raw = response.read(MAX_RESPONSE_BYTES).decode("utf-8", errors="replace")
            payload = json.loads(raw)
            if not isinstance(payload, dict):
                return None

            # GitHub API (/releases/latest): {"tag_name": "v5.5.2", ...}
            if "tag_name" in payload:
                return _validate_remote_version(payload.get("tag_name"))

            # PyPI JSON (/pypi/weewx/json): {"info": {"version": "5.5.2", ...}}
            info = payload.get("info")
            if isinstance(info, dict) and "version" in info:
                return _validate_remote_version(info.get("version"))

            return None

    def _compare_versions(self, current_ver, latest_ver, mode=None, github_url=DEFAULT_SKIN_GITHUB_URL):
        """Compare versions and return update information based on check mode."""
        check_mode = mode if mode is not None else self.update_check_mode
        try:
            norm_current = _normalize_version(current_ver)
            norm_latest = _validate_remote_version(latest_ver)
            if not norm_current or not norm_latest:
                return None

            if HAVE_PACKAGING:
                current = version.parse(norm_current)
                latest = version.parse(norm_latest)
                is_newer = latest > current
                major_minor_changed = (current.major != latest.major) or (current.minor != latest.minor)
            else:
                is_newer = _compare_fallback_versions(norm_current, norm_latest) < 0
                cur_nums = _parse_version(norm_current)[0]
                lat_nums = _parse_version(norm_latest)[0]
                cur_major = cur_nums[0] if len(cur_nums) >= 1 else 0
                cur_minor = cur_nums[1] if len(cur_nums) >= 2 else 0
                lat_major = lat_nums[0] if len(lat_nums) >= 1 else 0
                lat_minor = lat_nums[1] if len(lat_nums) >= 2 else 0
                major_minor_changed = (cur_major != lat_major) or (cur_minor != lat_minor)

            if not is_newer:
                log.debug(
                    "UpdateCheck: Current version %s is up to date (latest: %s)",
                    norm_current,
                    norm_latest,
                )
                return None

            log.debug("UpdateCheck: Update available %s -> %s", norm_current, norm_latest)

            if check_mode == "minor":
                if major_minor_changed:
                    return self._create_update_info(norm_current, norm_latest, "minor", github_url)
            elif check_mode == "patch":
                return self._create_update_info(norm_current, norm_latest, "patch", github_url)

            return None

        except Exception as e:
            log.debug("UpdateCheck: Error comparing versions %s vs %s: %s", current_ver, latest_ver, e)
            return None

    def _create_update_info(self, current_ver, latest_ver, update_type, github_url=DEFAULT_SKIN_GITHUB_URL):
        """Create update information dictionary."""
        return {
            "available": True,
            "current_version": current_ver,
            "latest_version": latest_ver,
            "update_type": update_type,
            "github_url": github_url,
        }
