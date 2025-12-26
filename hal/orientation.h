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

#ifndef HAL_ORIENTATION_HEAD
#define HAL_ORIENTATION_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_ORIENTATION
#include "hal.h"

/*!
 @function hal_orientation_available
 @return Returns true if orientation control is available
 @brief Check if screen orientation control is available
*/
bool hal_orientation_available(void);
/*!
 @function hal_orientation_set_landscape
 @param reverse If true, use reverse landscape orientation
 @return Returns true if orientation was set successfully
 @brief Set screen to landscape orientation
*/
bool hal_orientation_set_landscape(bool reverse);
/*!
 @function hal_orientation_set_portrait
 @param reverse If true, use reverse portrait orientation  
 @return Returns true if orientation was set successfully
 @brief Set screen to portrait orientation
*/
bool hal_orientation_set_portrait(bool reverse);
/*!
 @function hal_orientation_set_sensor
 @return Returns true if orientation was set successfully
 @brief Allow screen to rotate freely based on device sensors
*/
bool hal_orientation_set_sensor(void);
/*!
 @function hal_orientation_lock
 @return Returns true if orientation was locked successfully
 @brief Lock screen to current orientation
*/
bool hal_orientation_lock(void);
/*!
 @function hal_orientation_unlock
 @return Returns true if orientation was unlocked successfully
 @brief Unlock screen orientation
*/
bool hal_orientation_unlock(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_ORIENTATION_HEAD
