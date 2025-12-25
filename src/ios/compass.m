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

// iOS compass using CoreMotion magnetometer

#ifndef HAL_NO_COMPASS
#include "hal/compass.h"
#import <CoreMotion/CoreMotion.h>

static CMMotionManager *motion_manager = nil;
static bool is_enabled = false;
static float mag_x = 0, mag_y = 0, mag_z = 0;
static bool mag_valid = false;

static void init_motion_manager(void) {
    if (!motion_manager) {
        motion_manager = [[CMMotionManager alloc] init];
        motion_manager.magnetometerUpdateInterval = 1.0 / 60.0;
    }
}

bool hal_compass_available(void) {
    init_motion_manager();
    return motion_manager.magnetometerAvailable;
}

void hal_compass_enable(void) {
    if (is_enabled) return;
    init_motion_manager();
    if (!motion_manager.magnetometerAvailable) return;
    
    [motion_manager startMagnetometerUpdatesToQueue:[NSOperationQueue mainQueue]
        withHandler:^(CMMagnetometerData *data, NSError *error) {
            if (!error && data) {
                mag_x = data.magneticField.x;
                mag_y = data.magneticField.y;
                mag_z = data.magneticField.z;
                mag_valid = true;
            }
        }];
    is_enabled = true;
}

void hal_compass_disable(void) {
    if (!is_enabled) return;
    [motion_manager stopMagnetometerUpdates];
    is_enabled = false;
    mag_valid = false;
}

bool hal_compass_enabled(void) { return is_enabled; }

bool hal_compass_get(float *x, float *y, float *z) {
    if (!mag_valid) return false;
    if (x) *x = mag_x;
    if (y) *y = mag_y;
    if (z) *z = mag_z;
    return true;
}
#endif
