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

// iOS gravity using CoreMotion deviceMotion

#ifndef HAL_NO_GRAVITY
#include "hal/gravity.h"
#import <CoreMotion/CoreMotion.h>

static CMMotionManager *motion_manager = nil;
static bool is_enabled = false;
static float grav_x = 0, grav_y = 0, grav_z = 0;
static bool grav_valid = false;

static void init_motion_manager(void) {
    if (!motion_manager) {
        motion_manager = [[CMMotionManager alloc] init];
        motion_manager.deviceMotionUpdateInterval = 1.0 / 60.0;
    }
}

bool hal_gravity_available(void) {
    init_motion_manager();
    return motion_manager.deviceMotionAvailable;
}

void hal_gravity_enable(void) {
    if (is_enabled) return;
    init_motion_manager();
    if (!motion_manager.deviceMotionAvailable) return;
    
    [motion_manager startDeviceMotionUpdatesToQueue:[NSOperationQueue mainQueue]
        withHandler:^(CMDeviceMotion *motion, NSError *error) {
            if (!error && motion) {
                // Gravity in g's, convert to m/s^2
                grav_x = motion.gravity.x * 9.81f;
                grav_y = motion.gravity.y * 9.81f;
                grav_z = motion.gravity.z * 9.81f;
                grav_valid = true;
            }
        }];
    is_enabled = true;
}

void hal_gravity_disable(void) {
    if (!is_enabled) return;
    [motion_manager stopDeviceMotionUpdates];
    is_enabled = false;
    grav_valid = false;
}

bool hal_gravity_enabled(void) { return is_enabled; }

bool hal_gravity_get(float *x, float *y, float *z) {
    if (!grav_valid) return false;
    if (x) *x = grav_x;
    if (y) *y = grav_y;
    if (z) *z = grav_z;
    return true;
}
#endif
