/* https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

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

#ifndef PAUL_CLIPBOARD_HEAD
#define PAUL_CLIPBOARD_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define PAUL_ONLY_CLIPBOARD
#include "../paul.h"

bool paul_clipboard_available(void);
const char* paul_clipboard_get(void);
void paul_clipboard_set(const char *str);

#ifdef __cplusplus
}
#endif
#endif // PAUL_CLIPBOARD_HEAD
