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

// iOS spatial orientation using CoreMotion device motion

#ifndef HAL_NO_SPATIAL_ORIENTATION
#include "hal/spatial_orientation.h"
#import <CoreMotion/CoreMotion.h>

static CMMotionManager *motion_manager = nil;
static bool is_enabled = false;
static float sp_yaw = 0, sp_pitch = 0, sp_roll = 0;
static bool sp_valid = false;

static void init_motion_manager(void) {
    if (!motion_manager) {
        motion_manager = [[CMMotionManager alloc] init];
        motion_manager.deviceMotionUpdateInterval = 1.0 / 60.0;
    }
}

bool hal_spatial_orientation_available(void) {
    init_motion_manager();
    return motion_manager.deviceMotionAvailable;
}

void hal_spatial_orientation_enable(void) {
    if (is_enabled) return;
    init_motion_manager();
    if (!motion_manager.deviceMotionAvailable) return;
    
    [motion_manager startDeviceMotionUpdatesToQueue:[NSOperationQueue mainQueue]
        withHandler:^(CMDeviceMotion *motion, NSError *error) {
            if (!error && motion) {
                sp_yaw = motion.attitude.yaw;
                sp_pitch = motion.attitude.pitch;
                sp_roll = motion.attitude.roll;
                sp_valid = true;
            }
        }];
    is_enabled = true;
}

void hal_spatial_orientation_disable(void) {
    if (!is_enabled) return;
    [motion_manager stopDeviceMotionUpdates];
    is_enabled = false;
    sp_valid = false;
}

bool hal_spatial_orientation_enabled(void) { return is_enabled; }

bool hal_spatial_orientation_get(float *yaw, float *pitch, float *roll) {
    if (!sp_valid) return false;
    if (yaw) *yaw = sp_yaw;
    if (pitch) *pitch = sp_pitch;
    if (roll) *roll = sp_roll;
    return true;
}
#endif
