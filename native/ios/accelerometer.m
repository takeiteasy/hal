/* https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

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

#define PAUL_ONLY_ACCELEROMETER
#include "../../paul.h"
#include "../accelerometer.h"
#import <Foundation/Foundation.h>
#import <CoreMotion/CoreMotion.h>

static CMMotionManager* _manager(void) {
    return [[[UIApplication sharedApplication] delegate] sharedManager];
}

bool accelerometer_available(void) {
    CMMotionManager *manager = _manager();
    return manager && [manager isAccelerometerAvailable];
}

void accelerometer_enable(void) {
    CMMotionManager *manager = _manager();
    if (!manager || ![manager isAccelerometerAvailable])
        return;
    if (![manager isAccelerometerActive])
        [manager startAccelerometerUpdates];
}

void accelerometer_disable(void) {
    CMMotionManager *manager = _manager();
    if (!manager || ![manager isAccelerometerAvailable])
        return;
    if ([manager isAccelerometerActive])
        [manager stopAccelerometerUpdates];
}

bool accelerometer_enabled(void) {
    CMMotionManager *manager = _manager();
    return manager && [manager isAccelerometerAvailable] && [manager isAccelerometerActive];
}

bool accelerometer_disabled(void) {
    CMMotionManager *manager = _manager();
    return !manager || ![manager isAccelerometerAvailable] || ![manager isAccelerometerActive];
}

bool accelerometer_toggle(void) {
    bool state = accelerometer_enabled();
    if (state)
        accelerometer_disable();
    else
        accelerometer_enable();
    return !state;
}

bool accelerometer_acceleration(float *x, float *y, float *z)  {
    CMMotionManager *manager = _manager();
    if (accelerometer_enabled()) {
        [manager setAccelerometerUpdateInterval:.1];
        [manager startAccelerometerUpdatesToQueue:[NSOperationQueue mainQueue]
                                      withHandler:^(CMAccelerometerData *accelerometerData, NSError *error) {
            if (x)
                *x = accelerometerData.acceleration.x;
            if (y)
                *y = accelerometerData.acceleration.y;
            if (z)
                *z = accelerometerData.acceleration.z;
            return true;
        }];
    } else {
        if (x)
            *x = -1;
        if (y)
            *y = -1;
        if (z)
            *z = -1;
        return false;
    }
}
