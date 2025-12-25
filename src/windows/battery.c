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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool hal_battery_available(void) {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status))
        return false;
    
    // BatteryFlag 128 means no system battery
    return (status.BatteryFlag != 128);
}

int hal_battery_level(void) {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status))
        return -1;
    
    // BatteryLifePercent 255 means unknown
    if (status.BatteryLifePercent == 255)
        return -1;
    
    return status.BatteryLifePercent;
}

hal_battery_status_t hal_battery_status(void) {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status))
        return HAL_BATTERY_STATUS_UNKNOWN;
    
    // Check if no battery
    if (status.BatteryFlag == 128)
        return HAL_BATTERY_STATUS_NO_BATTERY;
    
    // Check charging first (bit 8)
    if (status.BatteryFlag & 8)
        return HAL_BATTERY_STATUS_CHARGING;
    
    // Check if on AC power
    if (status.ACLineStatus == 1) {
        if (status.BatteryLifePercent == 100)
            return HAL_BATTERY_STATUS_FULL;
        return HAL_BATTERY_STATUS_NOT_CHARGING;
    }
    
    return HAL_BATTERY_STATUS_DISCHARGING;
}

bool hal_battery_is_charging(void) {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status))
        return false;
    
    return (status.BatteryFlag & 8) != 0;
}

bool hal_battery_is_plugged(void) {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status))
        return false;
    
    return status.ACLineStatus == 1;
}

#endif // HAL_NO_BATTERY
