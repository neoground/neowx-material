# WeeWX & Skin Update Check + Dismissible Banner Plan

## 1. Goals & Requirements

1. **Check both NeoWX Material skin and WeeWX core versions**:
   - Extend `bin/user/updatecheck.py` (bump extension `VERSION` to `1.1.0`) to check the installed WeeWX version (`weewx.__version__`) against the latest stable WeeWX release in addition to the existing NeoWX Material skin check.
   - Display separate lines in the update banner for **NeoWX Material skin** and **WeeWX** whenever an update is available for either or both, each with its respective version details and GitHub link.
2. **Dismissible update banner with version-aware `localStorage` persistence**:
   - Add a close (`×`) button in the top-right corner of the update banner.
   - Store the dismissed update version state in `localStorage` (similar to how UI panel/footer state is persisted), so dismissed banners stay hidden across page reloads and auto-refreshes without visual flash.
   - Automatically remove the stored dismissal state and show the banner again whenever a **newer** skin or WeeWX release becomes available (different from the dismissed version), or when no updates remain on the live page.

---

## 2. Architecture & Design

### 2.1 WeeWX Release Source (`bin/user/updatecheck.py`)

- **Primary source (default)**: `https://github.com/weewx/weewx/releases/latest`
  - Uses a lightweight, non-redirecting HTTP `HEAD` request and inspects the `302` `Location` header (e.g., `https://github.com/weewx/weewx/releases/tag/v5.5.2`).
  - **Why this is preferred over the GitHub REST API**:
    - No unauthenticated 60 req/hour GitHub API rate limit.
    - Transfers zero response body (~100 bytes).
    - Automatically returns only the latest **stable** GitHub Release (skipping beta/pre-release tags such as `v5.6.0b2`).
    - Maps cleanly to the exact GitHub release page (`https://github.com/weewx/weewx/releases/tag/vX.Y.Z`).
- **Automatic fallback & multi-format support**:
  - If the default GitHub redirect check fails after retries, automatically fall back to the official PyPI JSON endpoint (`https://pypi.org/pypi/weewx/json`, reading `info.version`, which also excludes pre-releases and has no strict rate limit).
  - If the user overrides `weewx_update_check_url` in `skin.conf`, automatically support:
    1. GitHub `/releases/latest` redirect URL
    2. GitHub Releases REST API JSON (`https://api.github.com/repos/weewx/weewx/releases/latest` -> `tag_name`)
    3. PyPI JSON API (`https://pypi.org/pypi/weewx/json` -> `info.version`)

### 2.2 Backend Changes (`bin/user/updatecheck.py` & `skins/neowx-material/skin.conf`)

1. **Extension version bump**: Update `VERSION = "1.1.0"` in `bin/user/updatecheck.py`.
2. **Current WeeWX version detection**:
   - Read `weewx.__version__` (with safe fallback when running outside WeeWX for tests).
3. **Configuration under `[Extras] -> [[Footer]]` in `skin.conf`**:
   - Existing `update_check` (`off` / `minor` / `patch`) governs both skin and WeeWX checks by default so existing installations get WeeWX update checks automatically.
   - Optional `weewx_update_check` override (defaults to `update_check` when omitted/empty) to allow disabling or tuning WeeWX update notifications independently.
   - `update_interval = 1440` (24 hours in minutes — aligned as default in both `skin.conf` and `bin/user/updatecheck.py`).
   - `weewx_update_check_url = https://github.com/weewx/weewx/releases/latest` (configurable override).
4. **Independent Caching & Failure Cooldown**:
   - Split `_update_cache` into independent `"skin"` and `"weewx"` entries (`last_check`, `data`, `last_failure`).
   - Add a short failure cooldown (`min(cache_duration, 300)` seconds) so an offline/unreachable endpoint does not repeatedly stall every template in a report cycle with 3 retries x 5s delay.
5. **Version Parsing & Safe Links**:
   - Normalize leading `v`/`V`, validate fetched release versions strictly against `^[0-9]+(?:\.[0-9]+){1,3}$`.
   - Support both `packaging.version` and an improved fallback parser (extracting leading numeric dotted segments so pre-release local versions like `5.6.0b1` compare cleanly).
   - Construct outbound URLs deterministically from trusted HTTPS base URLs and validated versions:
     - Skin: `https://github.com/seehase/neowx-material`
     - WeeWX: `https://github.com/weewx/weewx/releases/tag/v<latest_version>` (or `https://github.com/weewx/weewx/releases` if fetched from a non-GitHub custom URL).
6. **Backwards-Compatible Return Payload**:
   `update_check()` returns `None` if neither skin nor WeeWX has an eligible update; otherwise:
   ```python
   {
       "available": True,
       "skin": {
           "available": True,
           "current_version": "2.1.0",
           "latest_version": "2.2.0",
           "update_type": "minor",
           "github_url": "https://github.com/seehase/neowx-material",
       },  # or None
       "weewx": {
           "available": True,
           "current_version": "5.5.1",
           "latest_version": "5.5.2",
           "update_type": "patch",
           "github_url": "https://github.com/weewx/weewx/releases/tag/v5.5.2",
       },  # or None
       # Legacy top-level keys kept for backwards compatibility (from skin if present, else weewx):
       "current_version": "...",
       "latest_version": "...",
       "update_type": "...",
       "github_url": "...",
   }
   ```

### 2.3 Update Banner UI & Dismissal Logic (`skins/neowx-material/inc/footer.inc`)

1. **Multi-line Markup**:
   - Capture `#set $uc = $update_check` once per template render.
   - Render `#nwm-update-banner` with `data-skin-latest` and `data-weewx-latest` attributes (all attributes quoted).
   - Add a top-right dismiss button:
     ```html
     <button type="button" class="close nwm-update-close" id="nwm-update-banner-close"
             aria-label="$gettext('close')" title="$gettext('close')">
         <span aria-hidden="true">&times;</span>
     </button>
     ```
   - Inside `.nwm-update-banner-body`, render:
     - Skin line (when `$uc.skin` is present):
       `$gettext("update_available") <strong>v$uc.skin.latest_version</strong> ($gettext("current"): v$uc.skin.current_version)` + link to `$uc.skin.github_url` (`rel="noopener noreferrer"`).
     - WeeWX line (when `$uc.weewx` is present):
       `$gettext("update_available_weewx") <strong>v$uc.weewx.latest_version</strong> ($gettext("current"): v$uc.weewx.current_version)` + link to `$uc.weewx.github_url` (`rel="noopener noreferrer"`).
2. **`localStorage` Dismissal & Auto-Reset Logic** (key: `nwm-update-dismissed`):
   - Stored JSON format: `{"skin": "<latest_skin_ver>", "weewx": "<latest_weewx_ver>"}` (empty string for any component that had no update when dismissed).
   - **Immediate anti-flash check** (synchronous IIFE immediately after the banner so auto-refreshes never flash a dismissed banner, while keeping the banner visible when JS is disabled):
     - Read and validate `localStorage.getItem('nwm-update-dismissed')` inside `try/catch`.
     - Compute coverage:
       - `skinCovered = !skinLatest || stored.skin === skinLatest`
       - `weewxCovered = !weewxLatest || stored.weewx === weewxLatest`
     - If `skinCovered && weewxCovered`: hide `#nwm-update-banner` immediately (`banner.style.display = 'none'`).
     - If **not** covered (meaning a newer skin release or a newer WeeWX release is now available compared to the dismissed state): call `localStorage.removeItem('nwm-update-dismissed')` and show the banner.
   - **Cleanup when up to date**:
     - On the live/current page (`$active_nav == 'current'`), if no `#nwm-update-banner` is rendered, remove `nwm-update-dismissed` from `localStorage` so old dismissal records do not linger after upgrading. (Skipped on static archive pages so browsing old archive HTML never clears an active dismissal.)
   - **Dismiss click handler**:
     - Clicking `#nwm-update-banner-close` persists `{ skin: skinLatest, weewx: weewxLatest }` into `localStorage` under `nwm-update-dismissed` and hides/removes the banner using safe DOM operations (no `innerHTML`).
3. **Reset View Button in Footer About Card** (previously "Panels" / "Reset panels"):
   - Update the reset button (`#nwm-reset-view-btn`, keeping the existing click flow) in the footer About card so it clears:
     - `sessionStorage.removeItem('nwm-panels')`
     - `localStorage.removeItem('nwm-panels')`
     - `localStorage.removeItem('nwm-update-dismissed')`
     before reloading the page, restoring both collapsed panel states and any dismissed update banner.
   - Rename the UI labels from `Panels` / `Reset panels` to broader, clearer keys across all 12 language files:
     - Row label (`view`): e.g. `View` (DE: `Ansicht`)
     - Button text (`reset_view`): e.g. `Reset view` (DE: `Ansicht zurücksetzen`)
     - Tooltip (`reset_view_tooltip`): e.g. `Reset panel states and dismissed notifications to default` (DE: `Panel-Zustände und ausgeblendete Hinweise auf Standard zurücksetzen`)
     - Keep legacy `panels` / `reset_panels` / `reset_panels_tooltip` keys in `lang/*.conf` for backwards compatibility.

### 2.4 Styling (`skins/neowx-material/scss/_neowx-material.scss`)

- Add `.nwm-update-banner` styling inside `footer.page-footer`:
  - `position: relative` with balanced horizontal padding (`padding: 0.65rem 2.75rem 0.65rem 1.25rem;`) so text stays visually centered and never collides with the top-right close button.
  - `.nwm-update-banner-body`: vertical stack (`display: flex; flex-direction: column; gap: 0.3rem; text-align: center;`).
  - `.nwm-update-close`: anchored top-right (`position: absolute; top: 0.35rem; right: 0.55rem;`), clean opacity/hover/focus styling.
  - Explicitly style `.nwm-update-banner a.alert-link` (`color: #062c33; text-decoration: underline;`) so `footer.page-footer a { color: #fff; }` from MDB does not wash out link contrast inside the light-blue `.alert-info` box.
- Recompile CSS via `cd skins/neowx-material && yarn run build`.

### 2.5 Localization (`skins/neowx-material/lang/*.conf`)

- Preserve the existing `update_available` key for the NeoWX Material skin line across all 12 language files.
- Add `update_available_weewx` to all 12 shipped language files (`ca`, `da`, `de`, `en`, `es`, `fi`, `fr`, `it`, `nl`, `pl`, `se`, `sk`).
- Add `view`, `reset_view`, and `reset_view_tooltip` to all 12 language files (while preserving legacy `panels`, `reset_panels`, `reset_panels_tooltip`).

---

## 3. Files to Modify

| File | Change |
| :--- | :--- |
| `bin/user/updatecheck.py` | Bump `VERSION = "1.1.0"`; add WeeWX release check (GitHub `/releases/latest` redirect + PyPI fallback + GitHub API support), separate caches with failure cooldown, strict version/URL validation, combined return dict |
| `skins/neowx-material/skin.conf` | Document and add `weewx_update_check_url` (and optional `weewx_update_check`) under `[Extras] -> [[Footer]]` |
| `skins/neowx-material/inc/footer.inc` | Render per-component lines for Skin and WeeWX, add top-right close `(×)` button, implement anti-flash `localStorage` dismiss & auto-clear logic, update the About-card reset button to also clear `nwm-update-dismissed` and use the new `view` / `reset_view` labels |
| `skins/neowx-material/scss/_neowx-material.scss` | Add `.nwm-update-banner`, `.nwm-update-banner-body`, `.nwm-update-line`, `.nwm-update-close`, and high-contrast `.alert-link` rules; compile `css/style.css` & `css/style.min.css` |
| `skins/neowx-material/lang/*.conf` (12 files) | Add `update_available_weewx`, `view`, `reset_view`, and `reset_view_tooltip` translations for all supported locales |

---

## 4. Verification Plan

### 4.1 Functional & Unit Verification
1. **Python unit tests for `bin/user/updatecheck.py`**:
   - Verify skin-only, WeeWX-only, both-available, and up-to-date responses.
   - Verify `update_check` modes (`off`, `minor`, `patch`) and `weewx_update_check` override.
   - Verify GitHub `/releases/latest` 302 redirect parsing, GitHub API JSON parsing, PyPI JSON parsing, and automatic fallback when GitHub redirect fails.
   - Verify cache hits and failure cooldown behavior (no repeated blocking retries within a cycle).
   - Verify live query against `https://github.com/weewx/weewx/releases/latest` returns the current stable WeeWX version (excluding beta tags).
2. **Build & Template Verification**:
   - Compile SCSS via `cd skins/neowx-material && yarn run build` and verify zero Sass errors.
   - Verify Cheetah template syntax of `skins/neowx-material/inc/footer.inc` with `python3` / Cheetah parser (if installed) or balanced directive check.
3. **Frontend Dismissal & Persistence Verification**:
   - Test dismissing banner stores `nwm-update-dismissed` in `localStorage` and hides the banner immediately.
   - Test reloading with identical `data-skin-latest` / `data-weewx-latest` keeps the banner hidden without flash.
   - Test incrementing either `data-skin-latest` or `data-weewx-latest` removes `nwm-update-dismissed` from `localStorage` and reveals the banner again.
   - Test loading the live page (`$active_nav == 'current'`) with no banner removes any leftover `nwm-update-dismissed` entry.
   - Test clicking **Reset view** in the footer About drawer clears both `nwm-panels` and `nwm-update-dismissed` and reloads the page with the update banner visible again.

### 4.2 Security Verification
- **Input Validation & Allow-Listing**: Confirm remote version strings are validated against a strict numeric semver regex (`^[0-9]+(?:\.[0-9]+){1,3}$`) before being returned or rendered; malformed payloads are rejected.
- **URL & Scheme Enforcement**: Confirm configured update check URLs must use `https://` before `urllib.request` is invoked; outbound `<a href>` links are constructed deterministically from hardcoded `https://github.com/...` bases and validated version numbers (never passing arbitrary redirect targets into HTML).
- **XSS & Safe DOM Handling**: Confirm all Cheetah template attributes (`data-skin-latest`, `data-weewx-latest`, `href`, `aria-label`, `title`) are double-quoted, external links use `rel="noopener noreferrer"`, and client-side JS avoids `innerHTML`/`outerHTML`/`document.write`/`insertAdjacentHTML` completely.
- **Safe `localStorage` Usage**: Confirm only non-sensitive UI version strings are stored in `localStorage` (no credentials/tokens/PII), all storage access is wrapped in `try/catch`, and parsed JSON is validated before property access.
