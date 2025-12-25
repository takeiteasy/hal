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

#ifndef HAL_DEVICE_NAME_HEAD
#define HAL_DEVICE_NAME_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_DEVICE_NAME
#include "hal.h"

/*!
 @function hal_device_name_available
 @return Returns true if device name query is available
 @brief Check if device name query is available
*/
bool hal_device_name_available(void);
/*!
 @function hal_device_name_get
 @return Returns device name (static buffer), NULL on failure
 @brief Get the device name/hostname
*/
const char* hal_device_name_get(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_DEVICE_NAME_HEAD