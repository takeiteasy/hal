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

// Linux wifi using nmcli (NetworkManager CLI)

#ifndef HAL_NO_WIFI
#include "hal/wifi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool command_exists(const char *cmd) {
    char buf[256];
    snprintf(buf, sizeof(buf), "command -v %s >/dev/null 2>&1", cmd);
    return system(buf) == 0;
}

bool hal_wifi_available(void) {
    return command_exists("nmcli");
}

bool hal_wifi_is_enabled(void) {
    FILE *fp = popen("nmcli radio wifi 2>/dev/null", "r");
    if (!fp) return false;
    
    char buf[32];
    bool enabled = false;
    if (fgets(buf, sizeof(buf), fp)) {
        enabled = strstr(buf, "enabled") != NULL;
    }
    pclose(fp);
    return enabled;
}

bool hal_wifi_enable(void) {
    return system("nmcli radio wifi on 2>/dev/null") == 0;
}

bool hal_wifi_disable(void) {
    return system("nmcli radio wifi off 2>/dev/null") == 0;
}

bool hal_wifi_start_scan(void) {
    return system("nmcli device wifi rescan 2>/dev/null") == 0;
}

int hal_wifi_get_networks(hal_wifi_network_t *networks, int max_count) {
    if (!networks || max_count <= 0) return 0;
    
    FILE *fp = popen("nmcli -t -f SSID,BSSID,SIGNAL,SECURITY device wifi list 2>/dev/null", "r");
    if (!fp) return 0;
    
    int count = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        
        char *ssid = strtok(line, ":");
        char *bssid = strtok(NULL, ":");
        char *signal = strtok(NULL, ":");
        char *security = strtok(NULL, ":");
        
        if (ssid && strlen(ssid) > 0) {
            strncpy(networks[count].ssid, ssid, 63);
            networks[count].ssid[63] = '\0';
            
            if (bssid) {
                strncpy(networks[count].bssid, bssid, 17);
                networks[count].bssid[17] = '\0';
            } else {
                networks[count].bssid[0] = '\0';
            }
            
            networks[count].signal_strength = signal ? atoi(signal) - 100 : -100;
            networks[count].is_secure = security && strlen(security) > 0 && 
                                        strcmp(security, "--") != 0;
            count++;
        }
    }
    
    pclose(fp);
    return count;
}

bool hal_wifi_connect(const char *ssid, const char *password) {
    if (!ssid) return false;
    
    char cmd[512];
    if (password) {
        snprintf(cmd, sizeof(cmd), "nmcli device wifi connect '%s' password '%s' 2>/dev/null",
                 ssid, password);
    } else {
        snprintf(cmd, sizeof(cmd), "nmcli device wifi connect '%s' 2>/dev/null", ssid);
    }
    
    return system(cmd) == 0;
}

bool hal_wifi_disconnect(void) {
    return system("nmcli device disconnect wlan0 2>/dev/null; "
                  "nmcli device disconnect wlo1 2>/dev/null; "
                  "nmcli device disconnect wifi0 2>/dev/null") == 0;
}

bool hal_wifi_is_connected(void) {
    FILE *fp = popen("nmcli -t -f TYPE,STATE connection show --active 2>/dev/null", "r");
    if (!fp) return false;
    
    char line[256];
    bool connected = false;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "wifi") || strstr(line, "802-11-wireless")) {
            connected = true;
            break;
        }
    }
    
    pclose(fp);
    return connected;
}

#endif // HAL_NO_WIFI
