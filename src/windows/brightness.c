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

// Windows brightness using Monitor Configuration API
// Note: Requires Dxva2.lib and works only with monitors supporting MCCS

#ifndef HAL_NO_BRIGHTNESS
#include "hal/brightness.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>

#pragma comment(lib, "Dxva2.lib")

static HANDLE get_physical_monitor(void) {
    HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    if (!hMonitor)
        return NULL;
    
    DWORD numMonitors = 0;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numMonitors) || numMonitors == 0)
        return NULL;
    
    PHYSICAL_MONITOR *monitors = (PHYSICAL_MONITOR*)malloc(sizeof(PHYSICAL_MONITOR) * numMonitors);
    if (!monitors)
        return NULL;
    
    if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, numMonitors, monitors)) {
        free(monitors);
        return NULL;
    }
    
    HANDLE physicalMonitor = monitors[0].hPhysicalMonitor;
    free(monitors);
    
    return physicalMonitor;
}

bool hal_brightness_available(void) {
    HANDLE monitor = get_physical_monitor();
    if (!monitor)
        return false;
    
    DWORD caps = 0, colorTemps = 0;
    BOOL result = GetMonitorCapabilities(monitor, &caps, &colorTemps);
    DestroyPhysicalMonitor(monitor);
    
    return result && (caps & MC_CAPS_BRIGHTNESS);
}

float hal_brightness_get(void) {
    HANDLE monitor = get_physical_monitor();
    if (!monitor)
        return -1.0f;
    
    DWORD min = 0, current = 0, max = 0;
    BOOL result = GetMonitorBrightness(monitor, &min, &current, &max);
    DestroyPhysicalMonitor(monitor);
    
    if (!result || max == min)
        return -1.0f;
    
    return (float)(current - min) / (float)(max - min);
}

bool hal_brightness_set(float level) {
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    
    HANDLE monitor = get_physical_monitor();
    if (!monitor)
        return false;
    
    DWORD min = 0, current = 0, max = 0;
    if (!GetMonitorBrightness(monitor, &min, &current, &max)) {
        DestroyPhysicalMonitor(monitor);
        return false;
    }
    
    DWORD newValue = min + (DWORD)(level * (float)(max - min));
    BOOL result = SetMonitorBrightness(monitor, newValue);
    DestroyPhysicalMonitor(monitor);
    
    return result != 0;
}

#endif // HAL_NO_BRIGHTNESS
