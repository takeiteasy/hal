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

#ifndef HAL_NO_CLIPBOARD
#include "../clipboard.h"
#include <windows.h>

bool hal_clipboard_available(void) {
    return IsClipboardFormatAvailable(CF_TEXT);
}

const char *hal_clipboard_get(void) {
    OpenClipboard(NULL);
    char *result = NULL;
    if (!IsClipboardFormatAvailable(CF_TEXT))
        goto BAIL;
    HGLOBAL hMem = GetClipboardData(format);
    if (!hMem)
        goto BAIL;
    char *text = (char*)GlobalLock(hMem);
    result = text ? strdup(text) : NULL;
BAIL:
    GlobalUnlock(hMem);
    CloseClipboard();
    return result;
}

void hal_clipboard_set(const char *str) {
    size_t length = strlen(str);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, length + 1);
    if (!hMem)
        return 0;
    char *ptr = (char*)GlobalLock(hMem);
    memcpy(ptr, str, length * sizeof(char));
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    GlobalUnlock(hMem);
    CloseClipboard();
}
#endif // HAL_NO_CLIPBOARD
