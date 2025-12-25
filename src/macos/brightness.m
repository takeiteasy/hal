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
#import <CoreGraphics/CoreGraphics.h>
#import <IOKit/graphics/IOGraphicsLib.h>

// Private API declarations for display brightness
extern int DisplayServicesGetBrightness(CGDirectDisplayID display, float *brightness);
extern int DisplayServicesSetBrightness(CGDirectDisplayID display, float brightness);

static CGDirectDisplayID get_main_display(void) {
    return CGMainDisplayID();
}

bool hal_brightness_available(void) {
    // Check if we can get the brightness
    float brightness = 0.0f;
    int result = DisplayServicesGetBrightness(get_main_display(), &brightness);
    return result == 0;
}

float hal_brightness_get(void) {
    float brightness = -1.0f;
    int result = DisplayServicesGetBrightness(get_main_display(), &brightness);
    if (result != 0)
        return -1.0f;
    return brightness;
}

bool hal_brightness_set(float level) {
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    
    int result = DisplayServicesSetBrightness(get_main_display(), level);
    return result == 0;
}

#endif // HAL_NO_BRIGHTNESS
