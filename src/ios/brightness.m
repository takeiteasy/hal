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

#ifndef HAL_NO_BRIGHTNESS
#include "hal/brightness.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

bool hal_brightness_available(void) {
    return true;
}

float hal_brightness_get(void) {
    return (float)[[UIScreen mainScreen] brightness];
}

bool hal_brightness_set(float level) {
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    
    [[UIScreen mainScreen] setBrightness:(CGFloat)level];
    return true;
}

#endif // HAL_NO_BRIGHTNESS
