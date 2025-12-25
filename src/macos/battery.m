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

#ifndef HAL_NO_BATTERY
#include "hal/battery.h"
#import <Foundation/Foundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

static CFDictionaryRef get_power_source_info(void) {
    CFTypeRef power_info = IOPSCopyPowerSourcesInfo();
    if (!power_info)
        return NULL;
    
    CFArrayRef power_sources = IOPSCopyPowerSourcesList(power_info);
    if (!power_sources) {
        CFRelease(power_info);
        return NULL;
    }
    
    CFIndex count = CFArrayGetCount(power_sources);
    if (count == 0) {
        CFRelease(power_sources);
        CFRelease(power_info);
        return NULL;
    }
    
    // Get the first power source (typically the internal battery)
    CFDictionaryRef source = IOPSGetPowerSourceDescription(power_info, CFArrayGetValueAtIndex(power_sources, 0));
    
    CFRelease(power_sources);
    CFRelease(power_info);
    
    return source;
}

bool hal_battery_available(void) {
    CFDictionaryRef source = get_power_source_info();
    if (!source)
        return false;
    
    CFStringRef type = CFDictionaryGetValue(source, CFSTR(kIOPSTypeKey));
    return type && CFEqual(type, CFSTR(kIOPSInternalBatteryType));
}

int hal_battery_level(void) {
    CFDictionaryRef source = get_power_source_info();
    if (!source)
        return -1;
    
    CFNumberRef current_capacity = CFDictionaryGetValue(source, CFSTR(kIOPSCurrentCapacityKey));
    CFNumberRef max_capacity = CFDictionaryGetValue(source, CFSTR(kIOPSMaxCapacityKey));
    
    if (!current_capacity || !max_capacity)
        return -1;
    
    int current = 0, max = 0;
    CFNumberGetValue(current_capacity, kCFNumberIntType, &current);
    CFNumberGetValue(max_capacity, kCFNumberIntType, &max);
    
    if (max <= 0)
        return -1;
    
    return (current * 100) / max;
}

hal_battery_status_t hal_battery_status(void) {
    CFDictionaryRef source = get_power_source_info();
    if (!source)
        return HAL_BATTERY_STATUS_NO_BATTERY;
    
    CFBooleanRef is_charging = CFDictionaryGetValue(source, CFSTR(kIOPSIsChargingKey));
    CFBooleanRef is_charged = CFDictionaryGetValue(source, CFSTR(kIOPSIsChargedKey));
    CFStringRef power_source = CFDictionaryGetValue(source, CFSTR(kIOPSPowerSourceStateKey));
    
    if (is_charged && CFBooleanGetValue(is_charged))
        return HAL_BATTERY_STATUS_FULL;
    
    if (is_charging && CFBooleanGetValue(is_charging))
        return HAL_BATTERY_STATUS_CHARGING;
    
    if (power_source && CFEqual(power_source, CFSTR(kIOPSBatteryPowerValue)))
        return HAL_BATTERY_STATUS_DISCHARGING;
    
    return HAL_BATTERY_STATUS_NOT_CHARGING;
}

bool hal_battery_is_charging(void) {
    CFDictionaryRef source = get_power_source_info();
    if (!source)
        return false;
    
    CFBooleanRef is_charging = CFDictionaryGetValue(source, CFSTR(kIOPSIsChargingKey));
    return is_charging && CFBooleanGetValue(is_charging);
}

bool hal_battery_is_plugged(void) {
    CFDictionaryRef source = get_power_source_info();
    if (!source)
        return false;
    
    CFStringRef power_source = CFDictionaryGetValue(source, CFSTR(kIOPSPowerSourceStateKey));
    return power_source && CFEqual(power_source, CFSTR(kIOPSACPowerValue));
}

#endif // HAL_NO_BATTERY
