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

/*!
 @header environment.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief Environment variable access
*/

#ifndef HAL_ENVIRONMENT_HEAD
#define HAL_ENVIRONMENT_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_ENVIRONMENT
#include "hal.h"

/*!
 @function hal_environment_available
 @return Returns true if environment access is available
 @brief Check if environment variable access is supported
*/
bool hal_environment_available(void);

/*!
 @function hal_environment_get
 @param name Environment variable name
 @return Returns value (must be freed by caller), NULL if not found
 @brief Get an environment variable
*/
char* hal_environment_get(const char *name);

/*!
 @function hal_environment_set
 @param name Environment variable name
 @param value Value to set (NULL to unset)
 @return Returns true on success
 @brief Set an environment variable
*/
bool hal_environment_set(const char *name, const char *value);

#ifdef __cplusplus
}
#endif
#endif // HAL_ENVIRONMENT_HEAD
