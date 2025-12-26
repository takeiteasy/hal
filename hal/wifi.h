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

#ifndef HAL_WIFI_HEAD
#define HAL_WIFI_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_WIFI
#include "hal.h"

/*!
 @struct hal_wifi_network_t
 @field ssid Network SSID (name)
 @field bssid Network BSSID (MAC address)
 @field signal_strength Signal strength in dBm
 @field is_secure Whether network requires password
 @discussion Information about a wifi network
*/
typedef struct {
    char ssid[64];
    char bssid[18];
    int signal_strength;
    bool is_secure;
} hal_wifi_network_t;

/*!
 @function hal_wifi_available
 @return Returns true if wifi functionality is available
 @brief Check if wifi is available on this platform
*/
bool hal_wifi_available(void);
/*!
 @function hal_wifi_is_enabled
 @return Returns true if wifi is enabled
 @brief Check if wifi hardware is enabled
*/
bool hal_wifi_is_enabled(void);
/*!
 @function hal_wifi_enable
 @return Returns true if wifi was enabled successfully
 @brief Enable wifi hardware
*/
bool hal_wifi_enable(void);
/*!
 @function hal_wifi_disable
 @return Returns true if wifi was disabled successfully
 @brief Disable wifi hardware
*/
bool hal_wifi_disable(void);
/*!
 @function hal_wifi_start_scan
 @return Returns true if scan was started successfully
 @brief Start scanning for available wifi networks
*/
bool hal_wifi_start_scan(void);
/*!
 @function hal_wifi_get_networks
 @param networks Array to receive network information
 @param max_count Maximum number of networks to return
 @return Returns number of networks found
 @brief Get list of available wifi networks
*/
int hal_wifi_get_networks(hal_wifi_network_t *networks, int max_count);
/*!
 @function hal_wifi_connect
 @param ssid Network SSID to connect to
 @param password Network password (NULL for open networks)
 @return Returns true if connection was initiated
 @brief Connect to a wifi network
*/
bool hal_wifi_connect(const char *ssid, const char *password);
/*!
 @function hal_wifi_disconnect
 @return Returns true if disconnection was successful
 @brief Disconnect from current wifi network
*/
bool hal_wifi_disconnect(void);
/*!
 @function hal_wifi_is_connected
 @return Returns true if connected to a wifi network
 @brief Check if connected to a wifi network
*/
bool hal_wifi_is_connected(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_WIFI_HEAD
