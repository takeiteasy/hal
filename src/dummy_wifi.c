/* https://github.com/takeiteasy/hal

 hal Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef HAL_NO_WIFI
#include "hal/wifi.h"

bool hal_wifi_available(void) { return false; }
bool hal_wifi_is_enabled(void) { return false; }
bool hal_wifi_enable(void) { return false; }
bool hal_wifi_disable(void) { return false; }
bool hal_wifi_start_scan(void) { return false; }
int hal_wifi_get_networks(hal_wifi_network_t *networks, int max_count) {
    (void)networks; (void)max_count; return 0;
}
bool hal_wifi_connect(const char *ssid, const char *password) {
    (void)ssid; (void)password; return false;
}
bool hal_wifi_disconnect(void) { return false; }
bool hal_wifi_is_connected(void) { return false; }

#endif // HAL_NO_WIFI
