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
#include "../hal.h"

bool hal_clipboard_available(void);
// WARNING: This must be released
const char *hal_clipboard_get(void);
void hal_clipboard_set(const char *str);

#ifdef __cplusplus
}
#endif
#endif // HAL_CLIPBOARD_HEAD
