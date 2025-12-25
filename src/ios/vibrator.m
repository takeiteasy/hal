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

// iOS vibrator using AudioServicesPlaySystemSound

#ifndef HAL_NO_VIBRATOR
#include "hal/vibrator.h"
#import <AudioToolbox/AudioToolbox.h>

// kSystemSoundID_Vibrate
#define SYSTEM_SOUND_VIBRATE 0x00000FFF

bool hal_vibrator_available(void) {
    return true;
}

void hal_vibrator_vibrate(int duration_ms) {
    (void)duration_ms; // iOS doesn't support custom duration
    AudioServicesPlaySystemSound(SYSTEM_SOUND_VIBRATE);
}

void hal_vibrator_cancel(void) {
    // iOS doesn't support cancel
}
#endif // HAL_NO_VIBRATOR
