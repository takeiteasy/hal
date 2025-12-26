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

#ifndef HAL_KEYSTORE_HEAD
#define HAL_KEYSTORE_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_KEYSTORE
#include "hal.h"

/*!
 @function hal_keystore_available
 @return Returns true if keystore is available
 @brief Check if secure keystore is available
*/
bool hal_keystore_available(void);
/*!
 @function hal_keystore_set
 @param service Service name (namespace)
 @param key Key identifier
 @param value Value to store
 @return Returns true if value was stored successfully
 @brief Store a value in the secure keystore
*/
bool hal_keystore_set(const char *service, const char *key, const char *value);
/*!
 @function hal_keystore_get
 @param service Service name (namespace)
 @param key Key identifier
 @return Returns the stored value, or NULL if not found. Caller must free.
 @brief Retrieve a value from the secure keystore
*/
char *hal_keystore_get(const char *service, const char *key);
/*!
 @function hal_keystore_delete
 @param service Service name (namespace)
 @param key Key identifier
 @return Returns true if value was deleted successfully
 @brief Delete a value from the secure keystore
*/
bool hal_keystore_delete(const char *service, const char *key);

#ifdef __cplusplus
}
#endif
#endif // HAL_KEYSTORE_HEAD
