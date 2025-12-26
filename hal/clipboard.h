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

#ifndef HAL_CLIPBOARD_HEAD
#define HAL_CLIPBOARD_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_CLIPBOARD
#include "hal.h"

/*!
 @function hal_clipboard_available
 @return Returns true if clipboard functionality is available
 @brief Check if clipboard is available on this platform
*/
bool hal_clipboard_available(void);
/*!
 @function hal_clipboard_has_text
 @return Returns true if clipboard contains text
 @brief Check if clipboard currently contains text data
*/
bool hal_clipboard_has_text(void);
/*!
 @function hal_clipboard_get_text
 @return Returns clipboard text content as newly allocated string, or NULL on error. Caller must free the returned string.
 @brief Get text from clipboard
*/
char *hal_clipboard_get_text(void);
/*!
 @function hal_clipboard_set_text
 @param text Text to copy to clipboard
 @return Returns true on success
 @brief Copy text to clipboard
*/
bool hal_clipboard_set_text(const char *text);
/*!
 @function hal_clipboard_clear
 @brief Clear clipboard contents
*/
void hal_clipboard_clear(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_CLIPBOARD_HEAD
