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

#ifndef HAL_NO_BAROMETER
#include "hal/barometer.h"
#import <Foundation/Foundation.h>
#import <CoreMotion/CoreMotion.h>

static CMAltimeter *altimeter = nil;
static float current_pressure = 0.0f;
static bool is_enabled = false;
static bool data_valid = false;

static CMAltimeter* _altimeter(void) {
    if (!altimeter) {
        altimeter = [[CMAltimeter alloc] init];
    }
    return altimeter;
}

bool hal_barometer_available(void) {
    return [CMAltimeter isRelativeAltitudeAvailable];
}

void hal_barometer_enable(void) {
    if (is_enabled)
        return;
    
    if (![CMAltimeter isRelativeAltitudeAvailable])
        return;
    
    CMAltimeter *alt = _altimeter();
    if (!alt)
        return;
    
    [alt startRelativeAltitudeUpdatesToQueue:[NSOperationQueue mainQueue]
                                 withHandler:^(CMAltitudeData *altitudeData, NSError *error) {
        if (error) {
            data_valid = false;
            return;
        }
        // CMAltitudeData.pressure is in kilopascals, convert to hectopascals (hPa)
        current_pressure = [altitudeData.pressure floatValue] * 10.0f;
        data_valid = true;
    }];
    
    is_enabled = true;
}

void hal_barometer_disable(void) {
    if (!is_enabled)
        return;
    
    CMAltimeter *alt = _altimeter();
    if (alt) {
        [alt stopRelativeAltitudeUpdates];
    }
    
    is_enabled = false;
    data_valid = false;
}

bool hal_barometer_enabled(void) {
    return is_enabled;
}

bool hal_barometer_disabled(void) {
    return !is_enabled;
}

bool hal_barometer_toggle(void) {
    if (is_enabled) {
        hal_barometer_disable();
        return false;
    } else {
        hal_barometer_enable();
        return is_enabled;
    }
}

bool hal_barometer_pressure(float *pressure) {
    if (!is_enabled || !data_valid) {
        if (pressure)
            *pressure = 0.0f;
        return false;
    }
    
    if (pressure)
        *pressure = current_pressure;
    
    return true;
}

#endif // HAL_NO_BAROMETER
