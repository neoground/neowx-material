#
# Copyright (c) 2025 OnlineCheck Extension for NeoWX Material
#
# Distributed under the terms of the GNU GENERAL PUBLIC LICENSE
#

"""Provides a simple online/offline check for weewx skins.

To use, add the following to the [CheetahGenerator] section of skin.conf:

[CheetahGenerator]
    search_list_extensions = user.onlinecheck.OnlineCheck

Then, in your template, you can get the status like this:

#set $online_check = $onlinecheck.get_status()
#if $online_check.offline
    ... we are offline ...
#end if

Configuration options can be placed in an [[OnlineCheck]] section:

[[OnlineCheck]]
    # URL to check for connectivity. Default: http://www.google.com
    check_url = http://www.google.com

    # Timeout for the check in seconds. Default: 10
    timeout = 10

    # Interval for the check in minutes. Default: 3
    interval = 3

    # Always show online status icon. Default: false
    always_show_status = false
"""

import time
import urllib.request
import urllib.error
import logging

try:
    # weewx-specific imports
    from weewx.cheetahgenerator import SearchList
except ImportError:
    # Fallback for running outside of weewx
    SearchList = object

VERSION = "1.0.7"

log = logging.getLogger(__name__)

# Global cache for online status
_online_cache = {
    "last_check": 0,
    "data": None,
}

class OnlineCheck(SearchList):
    """
    SearchList extension that provides an online status check.
    """
    def __init__(self, generator):
        self.skin_dict = generator.skin_dict
        self.generator = generator

        # Get config from skin.conf, with defaults
        self.config = self.skin_dict.get('Extras', {}).get('OnlineCheck', {})
        self.check_url = self.config.get('check_url', 'http://www.google.com')
        self.timeout = int(self.config.get('timeout', 10))
        self.cache_duration = int(self.config.get('interval', 5)) * 60  # in seconds

        # Determine if an online check is needed
        self.perform_check = self._is_check_required()

        log.info("OnlineCheck version %s", VERSION)
        if self.perform_check:
            log.info("Initialized OnlineCheck: URL='%s', Timeout=%ds, Interval=%dmin",
                     self.check_url, self.timeout, self.cache_duration / 60)
        else:
            log.info("OnlineCheck: No online features enabled, check is disabled.")

    def get_extension_list(self, timespan, db_lookup):
        """
        Returns a search list extension with the online check object.
        """
        return [{'onlinecheck': self}]

    def finalize(self):
        """
        Called at the end of the generation process.
        """
        pass

    def _is_check_required(self):
        """
        Checks if either update check or forecast is enabled.
        """
        # Check for update check setting
        update_check_mode = self.skin_dict.get('Extras', {}).get('Footer', {}).get('update_check', 'off')
        if update_check_mode != 'off':
            return True

        # Check if forecast is in search_list_extensions
        search_list = self.generator.config_dict.get('CheetahGenerator', {}).get('search_list_extensions', '')
        if 'user.openmeteo.Forecast' in search_list:
            return True

        return False

    def get_status(self):
        """
        Performs the online check and returns a dictionary with the status.
        Caches the result to avoid frequent checks.
        """
        global _online_cache

        # If no online features are enabled, return a default online status
        if not self.perform_check:
            return {'offline': False}

        current_time = time.time()

        # Check cache first
        if (current_time - _online_cache["last_check"]) < self.cache_duration and _online_cache["data"] is not None:
            log.debug("OnlineCheck: Using cached online status.")
            return _online_cache["data"]

        # If cache is stale, perform the check
        offline_status = False
        try:
            log.debug("OnlineCheck: Performing online check using URL: %s", self.check_url)
            # A HEAD request is more lightweight than GET
            request = urllib.request.Request(self.check_url, method='HEAD')
            with urllib.request.urlopen(request, timeout=self.timeout):
                # Any successful response means we are online
                pass
            log.debug("OnlineCheck: Check successful.")
        except (urllib.error.URLError, OSError) as e:
            log.info("OnlineCheck: Check failed, assuming offline. Error: %s", e)
            offline_status = True

        result = {'offline': offline_status}

        # Update cache
        _online_cache["last_check"] = current_time
        _online_cache["data"] = result

        log.debug("OnlineCheck: result: %s", result)
        return result
