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

#ifndef HAL_CALL_HEAD
#define HAL_CALL_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_CALL
#include "hal.h"

/*!
 @function hal_call_available
 @return Returns true if call functionality is available
 @brief Check if calling is available on this platform
*/
bool hal_call_available(void);
/*!
 @function hal_call_dial
 @param tel Phone number to dial
 @return Returns true if dialer was opened successfully
 @brief Open the phone dialer with the specified number pre-filled
*/
bool hal_call_dial(const char *tel);
/*!
 @function hal_call_make
 @param tel Phone number to call
 @return Returns true if call was initiated successfully
 @brief Make a direct phone call (requires appropriate permissions)
*/
bool hal_call_make(const char *tel);

#ifdef __cplusplus
}
#endif
#endif // HAL_CALL_HEAD
