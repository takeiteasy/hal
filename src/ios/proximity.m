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

// iOS proximity sensor using UIDevice proximity monitoring

#ifndef HAL_NO_PROXIMITY
#include "hal/proximity.h"
#import <UIKit/UIKit.h>

static bool is_enabled = false;
static bool is_near = false;

bool hal_proximity_available(void) {
    return true; // iOS devices have proximity sensors
}

void hal_proximity_enable(void) {
    if (is_enabled) return;
    
    [[UIDevice currentDevice] setProximityMonitoringEnabled:YES];
    
    // Register for proximity state changes
    [[NSNotificationCenter defaultCenter] addObserverForName:UIDeviceProximityStateDidChangeNotification
        object:nil
        queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification *note) {
            is_near = [[UIDevice currentDevice] proximityState];
        }];
    
    is_enabled = true;
}

void hal_proximity_disable(void) {
    if (!is_enabled) return;
    
    [[UIDevice currentDevice] setProximityMonitoringEnabled:NO];
    [[NSNotificationCenter defaultCenter] removeObserver:nil name:UIDeviceProximityStateDidChangeNotification object:nil];
    
    is_enabled = false;
    is_near = false;
}

bool hal_proximity_enabled(void) { return is_enabled; }

float hal_proximity_get(void) {
    // iOS doesn't provide distance, only near/far state
    return is_near ? 0.0f : 5.0f;
}

bool hal_proximity_is_near(void) {
    return is_near;
}
#endif
