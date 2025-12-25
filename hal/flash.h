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

#ifndef HAL_FLASH_HEAD
#define HAL_FLASH_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_FLASH
#include "hal.h"

/*!
 @function hal_flash_available
 @return Returns true if flash is available
 @brief Check if camera flash (torch) is available
*/
bool hal_flash_available(void);
/*!
 @function hal_flash_on
 @brief Turn on the flash/torch
*/
void hal_flash_on(void);
/*!
 @function hal_flash_off
 @brief Turn off the flash/torch
*/
void hal_flash_off(void);
/*!
 @function hal_flash_toggle
 @return Returns the new state of the flash
 @brief Toggle the flash state
*/
bool hal_flash_toggle(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_FLASH_HEAD