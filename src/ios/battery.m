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
#import <UIKit/UIKit.h>

bool hal_battery_available(void) {
    return true;
}

int hal_battery_level(void) {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
    float level = [[UIDevice currentDevice] batteryLevel];
    if (level < 0)
        return -1;
    return (int)(level * 100.0f);
}

hal_battery_status_t hal_battery_status(void) {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
    UIDeviceBatteryState state = [[UIDevice currentDevice] batteryState];
    
    switch (state) {
        case UIDeviceBatteryStateCharging:
            return HAL_BATTERY_STATUS_CHARGING;
        case UIDeviceBatteryStateFull:
            return HAL_BATTERY_STATUS_FULL;
        case UIDeviceBatteryStateUnplugged:
            return HAL_BATTERY_STATUS_DISCHARGING;
        case UIDeviceBatteryStateUnknown:
        default:
            return HAL_BATTERY_STATUS_UNKNOWN;
    }
}

bool hal_battery_is_charging(void) {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
    UIDeviceBatteryState state = [[UIDevice currentDevice] batteryState];
    return state == UIDeviceBatteryStateCharging;
}

bool hal_battery_is_plugged(void) {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
    UIDeviceBatteryState state = [[UIDevice currentDevice] batteryState];
    return state == UIDeviceBatteryStateCharging || state == UIDeviceBatteryStateFull;
}

#endif // HAL_NO_BATTERY
