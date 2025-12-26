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

#ifndef HAL_BLUETOOTH_HEAD
#define HAL_BLUETOOTH_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_BLUETOOTH
#include "hal.h"

/*!
 @function hal_bluetooth_available
 @return Returns true if bluetooth hardware is available
 @brief Check if bluetooth is available on this device
*/
bool hal_bluetooth_available(void);
/*!
 @function hal_bluetooth_is_enabled
 @return Returns true if bluetooth is enabled
 @brief Check if bluetooth is currently enabled
*/
bool hal_bluetooth_is_enabled(void);
/*!
 @function hal_bluetooth_enable
 @return Returns true if bluetooth was enabled successfully
 @brief Enable bluetooth hardware
*/
bool hal_bluetooth_enable(void);
/*!
 @function hal_bluetooth_disable
 @return Returns true if bluetooth was disabled successfully
 @brief Disable bluetooth hardware
*/
bool hal_bluetooth_disable(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_BLUETOOTH_HEAD
