#
# Copyright (c) 2025 UpdateCheck Extension for NeoWX Material
#
# Distributed under the terms of the GNU GENERAL PUBLIC LICENSE
#

"""Extends the Cheetah generator search list to check for updates from GitHub

[CheetahGenerator]
    search_list_extensions = user.updatecheck.UpdateCheck

# Update Check behavior
# -------------------------------------------------------------------------
# Here you can configure update check behavior
#
[[UpdateCheck]]
    # Update check modes:
    # off - No update checking performed
    # minor - Only show notifications for minor version updates (e.g., 1.50.x -> 1.51.x)
    # patch - Show notifications for all version updates including patches (e.g., 1.51.1 -> 1.51.2)
    update_check = patch

    # Update check interval in minutes (default: 1440 minutes = 24 hours = 1 day)
    update_interval = 1440

    # URL to check for the latest version (default points to the skin.conf in the GitHub repository)
    update_check_url = https://raw.githubusercontent.com/seehase/neowx-material/master/skins/neowx-material/skin.conf

"""

import time
import urllib.request
import urllib.parse
import urllib.error
import logging
import re

try:
    from packaging import version
    HAVE_PACKAGING = True
except ImportError:
    HAVE_PACKAGING = False
    version = None

def _parse_version(ver):
    return tuple(int(x) for x in ver.split('.') if x.isdigit())

try:
    from weewx.cheetahgenerator import SearchList
except ImportError:
    # This will be available when running in weewx context
    SearchList = object

VERSION = "1.0.0"

log = logging.getLogger(__name__)

# Global cache variable - check only every 4 hours
_update_cache = {
    "last_check": 0,
    "data": None,
}

CACHE_DURATION = 4 * 60 * 60  # 4 hours in seconds
TIMEOUT = 15  # seconds
RETRIES = 3
DELAY = 5  # seconds between retries


class UpdateCheck(SearchList):

    def __init__(self, generator):
        super().__init__(generator)
        log.info("version: %s" % VERSION)

        self.generator = generator
        self.footer_dict = generator.skin_dict.get("Extras", {}).get("Footer", {})
        self.current_version = generator.skin_dict.get("Extras", {}).get("version", "0.0.0")

        # Configuration
        self.update_check_mode = self.footer_dict.get("update_check", "off")
        self.update_interval = int(self.footer_dict.get("update_interval", "240"))  # minutes
        self.update_check_url = self.footer_dict.get("update_check_url", "https://raw.githubusercontent.com/seehase/neowx-material/master/skins/neowx-material/skin.conf")

        # Convert interval from minutes to seconds
        self.cache_duration = self.update_interval * 60

    def update_check(self):
        """Main function that returns update information"""

        if self.update_check_mode == "off":
            log.debug("Update check is disabled")
            return None

        if not self.current_version:
            log.debug("No current version found in skin.conf")
            return None

        try:
            latest_version = self._get_latest_version()
            if not latest_version:
                return None

            return self._compare_versions(self.current_version, latest_version)

        except Exception as e:
            log.debug("Error during update check: %s", e)
            return None

    def _get_latest_version(self):
        """Get the latest version from GitHub with caching"""
        global _update_cache

        current_time = time.time()

        # Check if we have cached data that's still valid
        if (current_time - _update_cache["last_check"]) < self.cache_duration and _update_cache["data"]:
            log.debug("Using cached version data (interval: %d minutes)", self.update_interval)
            return _update_cache["data"]

        # Fetch new version from the configured URL
        github_url = self.update_check_url

        for attempt in range(1, RETRIES + 1):
            try:
                log.debug("Fetching version from: %s (attempt %d)", github_url, attempt)

                with urllib.request.urlopen(github_url, timeout=TIMEOUT) as response:
                    if response.status != 200:
                        raise Exception(f"HTTP error: {response.status}")

                    content = response.read().decode('utf-8')
                    version_match = re.search(r'version\s*=\s*([0-9]+\.[0-9]+\.[0-9]+)', content)

                    if version_match:
                        latest_version = version_match.group(1)
                        log.debug("Found latest version: %s", latest_version)

                        # Update cache
                        _update_cache["last_check"] = int(current_time)
                        _update_cache["data"] = latest_version

                        return latest_version
                    else:
                        raise Exception("Version not found in remote skin.conf")

            except (urllib.error.URLError, urllib.error.HTTPError, Exception) as e:
                if attempt < RETRIES:
                    log.debug("Retry in %d seconds... (%s)", DELAY, str(e))
                    time.sleep(DELAY)
                else:
                    log.info("Failed to fetch latest version from GitHub after %d attempts", RETRIES)
                    log.debug("Last error: %s", e)
                    return None

        return None

    def _compare_versions(self, current_ver, latest_ver):
        """Compare versions and return update information based on check mode"""
        try:
            if HAVE_PACKAGING:
                current = version.parse(current_ver)
                latest = version.parse(latest_ver)
            else:
                current = _parse_version(current_ver)
                latest = _parse_version(latest_ver)

            if latest <= current:
                log.debug("Current version %s is up to date (latest: %s)", current_ver, latest_ver)
                return None

            log.debug("Current version %s latest version: %s)", current_ver, latest_ver)

            # Determine if we should show this update based on the check mode
            if self.update_check_mode == "minor":
                # Only show if major or minor version changed
                if HAVE_PACKAGING:
                    if current.major != latest.major or current.minor != latest.minor:
                        return self._create_update_info(current_ver, latest_ver, "minor")
                else:
                    # Fallback: parse version manually for major.minor comparison
                    current_parts = current_ver.split('.')
                    latest_parts = latest_ver.split('.')
                    if (len(current_parts) >= 2 and len(latest_parts) >= 2 and
                            (current_parts[0] != latest_parts[0] or current_parts[1] != latest_parts[1])):
                        return self._create_update_info(current_ver, latest_ver, "minor")
            elif self.update_check_mode == "patch":
                # Show all updates including patches
                return self._create_update_info(current_ver, latest_ver, "patch")

            return None

        except Exception as e:
            log.debug("Error comparing versions %s vs %s: %s", current_ver, latest_ver, e)
            return None

    def _create_update_info(self, current_ver, latest_ver, update_type):
        """Create update information dictionary"""
        return {
            "available": True,
            "current_version": current_ver,
            "latest_version": latest_ver,
            "update_type": update_type,
            "github_url": "https://github.com/seehase/neowx-material"
        }
