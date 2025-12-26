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

#ifndef HAL_NO_CAMERA
#include "hal/camera.h"

bool hal_camera_available(void) { return false; }

bool hal_camera_take_picture(const char *filename, hal_camera_cb on_complete, void *ctx) {
    (void)filename; (void)on_complete; (void)ctx;
    return false;
}

bool hal_camera_take_video(const char *filename, hal_camera_cb on_complete, void *ctx) {
    (void)filename; (void)on_complete; (void)ctx;
    return false;
}

#endif // HAL_NO_CAMERA
