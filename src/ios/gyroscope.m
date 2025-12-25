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

// iOS gyroscope using CoreMotion

#ifndef HAL_NO_GYROSCOPE
#include "hal/gyroscope.h"
#import <CoreMotion/CoreMotion.h>

static CMMotionManager *motion_manager = nil;
static bool is_enabled = false;
static float gyro_x = 0, gyro_y = 0, gyro_z = 0;
static bool gyro_valid = false;

static void init_motion_manager(void) {
    if (!motion_manager) {
        motion_manager = [[CMMotionManager alloc] init];
        motion_manager.gyroUpdateInterval = 1.0 / 60.0;
    }
}

bool hal_gyroscope_available(void) {
    init_motion_manager();
    return motion_manager.gyroAvailable;
}

void hal_gyroscope_enable(void) {
    if (is_enabled) return;
    init_motion_manager();
    if (!motion_manager.gyroAvailable) return;
    
    [motion_manager startGyroUpdatesToQueue:[NSOperationQueue mainQueue]
        withHandler:^(CMGyroData *data, NSError *error) {
            if (!error && data) {
                gyro_x = data.rotationRate.x;
                gyro_y = data.rotationRate.y;
                gyro_z = data.rotationRate.z;
                gyro_valid = true;
            }
        }];
    is_enabled = true;
}

void hal_gyroscope_disable(void) {
    if (!is_enabled) return;
    [motion_manager stopGyroUpdates];
    is_enabled = false;
    gyro_valid = false;
}

bool hal_gyroscope_enabled(void) { return is_enabled; }

bool hal_gyroscope_get(float *x, float *y, float *z) {
    if (!gyro_valid) return false;
    if (x) *x = gyro_x;
    if (y) *y = gyro_y;
    if (z) *z = gyro_z;
    return true;
}
#endif
