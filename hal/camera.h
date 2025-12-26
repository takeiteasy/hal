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

#ifndef HAL_CAMERA_HEAD
#define HAL_CAMERA_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_CAMERA
#include "hal.h"

/*!
 @typedef hal_camera_cb
 @param filepath Path to the captured file, or NULL on failure
 @param ctx User context pointer
 @discussion Callback for camera capture completion
*/
typedef void (*hal_camera_cb)(const char *filepath, void *ctx);

/*!
 @function hal_camera_available
 @return Returns true if camera is available
 @brief Check if camera functionality is available
*/
bool hal_camera_available(void);
/*!
 @function hal_camera_take_picture
 @param filename Desired filename for the captured image
 @param on_complete Callback when capture completes
 @param ctx User context passed to callback
 @return Returns true if capture was initiated
 @brief Take a picture using the system camera UI
*/
bool hal_camera_take_picture(const char *filename, hal_camera_cb on_complete, void *ctx);
/*!
 @function hal_camera_take_video
 @param filename Desired filename for the captured video
 @param on_complete Callback when capture completes
 @param ctx User context passed to callback
 @return Returns true if capture was initiated
 @brief Record a video using the system camera UI
*/
bool hal_camera_take_video(const char *filename, hal_camera_cb on_complete, void *ctx);

#ifdef __cplusplus
}
#endif
#endif // HAL_CAMERA_HEAD
