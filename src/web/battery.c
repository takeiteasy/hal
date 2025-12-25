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

// Web/Emscripten battery implementation
// Uses Battery Status API via JavaScript interop

#ifndef HAL_NO_BATTERY
#include "hal/battery.h"
#include <emscripten.h>

EM_JS(int, js_battery_available, (), {
    return ('getBattery' in navigator) ? 1 : 0;
});

EM_JS(int, js_battery_level, (), {
    if (!Module._batteryManager) return -1;
    return Math.round(Module._batteryManager.level * 100);
});

EM_JS(int, js_battery_charging, (), {
    if (!Module._batteryManager) return 0;
    return Module._batteryManager.charging ? 1 : 0;
});

EM_ASYNC_JS(void, js_init_battery, (), {
    if ('getBattery' in navigator) {
        try {
            Module._batteryManager = await navigator.getBattery();
        } catch (e) {
            Module._batteryManager = null;
        }
    }
});

static bool battery_initialized = false;

static void ensure_battery_init(void) {
    if (!battery_initialized) {
        js_init_battery();
        battery_initialized = true;
    }
}

bool hal_battery_available(void) {
    return js_battery_available() == 1;
}

int hal_battery_level(void) {
    ensure_battery_init();
    return js_battery_level();
}

hal_battery_status_t hal_battery_status(void) {
    ensure_battery_init();
    int level = js_battery_level();
    int charging = js_battery_charging();
    
    if (level < 0)
        return HAL_BATTERY_STATUS_UNKNOWN;
    
    if (charging) {
        if (level >= 100)
            return HAL_BATTERY_STATUS_FULL;
        return HAL_BATTERY_STATUS_CHARGING;
    }
    
    return HAL_BATTERY_STATUS_DISCHARGING;
}

bool hal_battery_is_charging(void) {
    ensure_battery_init();
    return js_battery_charging() == 1;
}

bool hal_battery_is_plugged(void) {
    // Same as charging for web
    return hal_battery_is_charging();
}

#endif // HAL_NO_BATTERY