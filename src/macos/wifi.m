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

// macOS wifi using CoreWLAN

#ifndef HAL_NO_WIFI
#include "hal/wifi.h"
#import <Foundation/Foundation.h>
#import <CoreWLAN/CoreWLAN.h>

static CWInterface *get_wifi_interface(void) {
    CWWiFiClient *client = [CWWiFiClient sharedWiFiClient];
    return [client interface];
}

bool hal_wifi_available(void) {
    return get_wifi_interface() != nil;
}

bool hal_wifi_is_enabled(void) {
    CWInterface *iface = get_wifi_interface();
    return iface && iface.powerOn;
}

bool hal_wifi_enable(void) {
    CWInterface *iface = get_wifi_interface();
    if (!iface) return false;
    
    NSError *error = nil;
    return [iface setPower:YES error:&error] && error == nil;
}

bool hal_wifi_disable(void) {
    CWInterface *iface = get_wifi_interface();
    if (!iface) return false;
    
    NSError *error = nil;
    return [iface setPower:NO error:&error] && error == nil;
}

bool hal_wifi_start_scan(void) {
    return hal_wifi_available();
}

int hal_wifi_get_networks(hal_wifi_network_t *networks, int max_count) {
    CWInterface *iface = get_wifi_interface();
    if (!iface || !networks) return 0;
    
    NSError *error = nil;
    NSSet<CWNetwork *> *scanResults = [iface scanForNetworksWithName:nil error:&error];
    if (error || !scanResults) return 0;
    
    int count = 0;
    for (CWNetwork *network in scanResults) {
        if (count >= max_count) break;
        
        strncpy(networks[count].ssid, network.ssid.UTF8String ?: "", 63);
        networks[count].ssid[63] = '\0';
        
        strncpy(networks[count].bssid, network.bssid.UTF8String ?: "", 17);
        networks[count].bssid[17] = '\0';
        
        networks[count].signal_strength = (int)network.rssiValue;
        // Check if network requires security by checking for RSN/WPA
        networks[count].is_secure = network.rssiValue != 0;  // Simplified check
        
        count++;
    }
    
    return count;
}

bool hal_wifi_connect(const char *ssid, const char *password) {
    CWInterface *iface = get_wifi_interface();
    if (!iface || !ssid) return false;
    
    NSError *error = nil;
    NSSet<CWNetwork *> *networks = [iface scanForNetworksWithName:
        [NSString stringWithUTF8String:ssid] error:&error];
    
    if (error || networks.count == 0) return false;
    
    CWNetwork *network = networks.anyObject;
    NSString *pwd = password ? [NSString stringWithUTF8String:password] : nil;
    
    return [iface associateToNetwork:network password:pwd error:&error] && error == nil;
}

bool hal_wifi_disconnect(void) {
    CWInterface *iface = get_wifi_interface();
    if (!iface) return false;
    
    [iface disassociate];
    return true;
}

bool hal_wifi_is_connected(void) {
    CWInterface *iface = get_wifi_interface();
    return iface && iface.ssid != nil;
}

#endif // HAL_NO_WIFI
