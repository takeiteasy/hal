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

#ifndef HAL_BRIGHTNESS_HEAD
#define HAL_BRIGHTNESS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_BRIGHTNESS
#include "hal.h"

/*!
 @function hal_brightness_available
 @return Returns true if brightness control is available
 @brief Check if brightness control is available
*/
bool hal_brightness_available(void);
/*!
 @function hal_brightness_get
 @return Returns brightness level (0.0-1.0), -1.0 on error
 @brief Get brightness level
*/
float hal_brightness_get(void);
/*!
 @function hal_brightness_set
 @param level Brightness level (0.0-1.0)
 @return Returns true on success
 @brief Set brightness level
*/
bool hal_brightness_set(float level);

#ifdef __cplusplus
}
#endif
#endif // HAL_BRIGHTNESS_HEAD