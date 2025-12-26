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

// Windows wifi using WLAN API

#ifndef HAL_NO_WIFI
#include "hal/wifi.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wlanapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "wlanapi.lib")

static HANDLE g_wlan_handle = NULL;

static bool init_wlan(void) {
    if (g_wlan_handle) return true;
    
    DWORD version;
    DWORD result = WlanOpenHandle(2, NULL, &version, &g_wlan_handle);
    return result == ERROR_SUCCESS;
}

static GUID *get_interface_guid(void) {
    static GUID guid;
    PWLAN_INTERFACE_INFO_LIST list = NULL;
    
    if (WlanEnumInterfaces(g_wlan_handle, NULL, &list) != ERROR_SUCCESS)
        return NULL;
    
    if (list->dwNumberOfItems > 0) {
        guid = list->InterfaceInfo[0].InterfaceGuid;
        WlanFreeMemory(list);
        return &guid;
    }
    
    WlanFreeMemory(list);
    return NULL;
}

bool hal_wifi_available(void) {
    return init_wlan() && get_interface_guid() != NULL;
}

bool hal_wifi_is_enabled(void) {
    if (!init_wlan()) return false;
    
    GUID *guid = get_interface_guid();
    if (!guid) return false;
    
    PWLAN_INTERFACE_INFO_LIST list = NULL;
    if (WlanEnumInterfaces(g_wlan_handle, NULL, &list) != ERROR_SUCCESS)
        return false;
    
    bool enabled = list->dwNumberOfItems > 0 &&
                   list->InterfaceInfo[0].isState != wlan_interface_state_not_ready;
    
    WlanFreeMemory(list);
    return enabled;
}

bool hal_wifi_enable(void) {
    // Windows doesn't have a simple API to enable/disable wifi adapter
    return false;
}

bool hal_wifi_disable(void) {
    return false;
}

bool hal_wifi_start_scan(void) {
    if (!init_wlan()) return false;
    
    GUID *guid = get_interface_guid();
    if (!guid) return false;
    
    return WlanScan(g_wlan_handle, guid, NULL, NULL, NULL) == ERROR_SUCCESS;
}

int hal_wifi_get_networks(hal_wifi_network_t *networks, int max_count) {
    if (!networks || max_count <= 0 || !init_wlan()) return 0;
    
    GUID *guid = get_interface_guid();
    if (!guid) return 0;
    
    PWLAN_AVAILABLE_NETWORK_LIST list = NULL;
    if (WlanGetAvailableNetworkList(g_wlan_handle, guid, 0, NULL, &list) != ERROR_SUCCESS)
        return 0;
    
    int count = 0;
    for (DWORD i = 0; i < list->dwNumberOfItems && count < max_count; i++) {
        WLAN_AVAILABLE_NETWORK *net = &list->Network[i];
        
        // Convert SSID
        int len = net->dot11Ssid.uSSIDLength;
        if (len > 63) len = 63;
        memcpy(networks[count].ssid, net->dot11Ssid.ucSSID, len);
        networks[count].ssid[len] = '\0';
        
        // BSSID not available in this structure
        networks[count].bssid[0] = '\0';
        
        networks[count].signal_strength = net->wlanSignalQuality - 100;
        networks[count].is_secure = net->bSecurityEnabled;
        
        count++;
    }
    
    WlanFreeMemory(list);
    return count;
}

bool hal_wifi_connect(const char *ssid, const char *password) {
    if (!ssid || !init_wlan()) return false;
    
    GUID *guid = get_interface_guid();
    if (!guid) return false;
    
    // Create connection parameters
    WLAN_CONNECTION_PARAMETERS params;
    memset(&params, 0, sizeof(params));
    params.wlanConnectionMode = wlan_connection_mode_profile;
    
    // Convert SSID to wide string
    wchar_t wssid[64];
    MultiByteToWideChar(CP_UTF8, 0, ssid, -1, wssid, 64);
    params.strProfile = wssid;
    
    params.pDot11Ssid = NULL;
    params.pDesiredBssidList = NULL;
    params.dot11BssType = dot11_BSS_type_infrastructure;
    params.dwFlags = 0;
    
    return WlanConnect(g_wlan_handle, guid, &params, NULL) == ERROR_SUCCESS;
}

bool hal_wifi_disconnect(void) {
    if (!init_wlan()) return false;
    
    GUID *guid = get_interface_guid();
    if (!guid) return false;
    
    return WlanDisconnect(g_wlan_handle, guid, NULL) == ERROR_SUCCESS;
}

bool hal_wifi_is_connected(void) {
    if (!init_wlan()) return false;
    
    GUID *guid = get_interface_guid();
    if (!guid) return false;
    
    PWLAN_CONNECTION_ATTRIBUTES attrs = NULL;
    DWORD size = sizeof(WLAN_CONNECTION_ATTRIBUTES);
    WLAN_OPCODE_VALUE_TYPE type;
    
    if (WlanQueryInterface(g_wlan_handle, guid, wlan_intf_opcode_current_connection,
                           NULL, &size, (PVOID *)&attrs, &type) != ERROR_SUCCESS)
        return false;
    
    bool connected = attrs->isState == wlan_interface_state_connected;
    WlanFreeMemory(attrs);
    
    return connected;
}

#endif // HAL_NO_WIFI
