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

// TODO

bool paul_accelerometer_available(void) {
    return false;
}

void paul_accelerometer_enable(void) {}

void paul_accelerometer_disable(void) {}

bool paul_accelerometer_enabled(void) {
    return false;
}

bool paul_accelerometer_disabled(void) {
    return true;
}

bool paul_accelerometer_toggle(void) {
    return false;
}

bool paul_accelerometer_acceleration(float *x, float *y, float *z) {
    if (x)
        *x = -1;
    if (y)
        *y = -1;
    if (z)
        *z = -1;
    return false
}
