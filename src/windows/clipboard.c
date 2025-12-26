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

// Windows clipboard using Win32 API with CF_UNICODETEXT for Unicode support

#ifndef HAL_NO_CLIPBOARD
#include "hal/clipboard.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>

bool hal_clipboard_available(void) {
    return true;
}

bool hal_clipboard_has_text(void) {
    if (!OpenClipboard(NULL))
        return false;
    
    BOOL hasText = IsClipboardFormatAvailable(CF_UNICODETEXT);
    CloseClipboard();
    
    return hasText != 0;
}

char *hal_clipboard_get_text(void) {
    if (!OpenClipboard(NULL))
        return NULL;
    
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL) {
        CloseClipboard();
        return NULL;
    }
    
    WCHAR *pszText = (WCHAR*)GlobalLock(hData);
    if (pszText == NULL) {
        CloseClipboard();
        return NULL;
    }
    
    // Convert UTF-16 to UTF-8
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, pszText, -1, NULL, 0, NULL, NULL);
    if (utf8_len <= 0) {
        GlobalUnlock(hData);
        CloseClipboard();
        return NULL;
    }
    
    char *result = (char*)malloc(utf8_len);
    if (result == NULL) {
        GlobalUnlock(hData);
        CloseClipboard();
        return NULL;
    }
    
    WideCharToMultiByte(CP_UTF8, 0, pszText, -1, result, utf8_len, NULL, NULL);
    
    GlobalUnlock(hData);
    CloseClipboard();
    
    return result;
}

bool hal_clipboard_set_text(const char *text) {
    if (text == NULL)
        return false;
    
    // Convert UTF-8 to UTF-16
    int wchar_len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (wchar_len <= 0)
        return false;
    
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wchar_len * sizeof(WCHAR));
    if (hMem == NULL)
        return false;
    
    WCHAR *pszDest = (WCHAR*)GlobalLock(hMem);
    if (pszDest == NULL) {
        GlobalFree(hMem);
        return false;
    }
    
    MultiByteToWideChar(CP_UTF8, 0, text, -1, pszDest, wchar_len);
    GlobalUnlock(hMem);
    
    if (!OpenClipboard(NULL)) {
        GlobalFree(hMem);
        return false;
    }
    
    EmptyClipboard();
    
    if (SetClipboardData(CF_UNICODETEXT, hMem) == NULL) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    
    CloseClipboard();
    return true;
}

void hal_clipboard_clear(void) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        CloseClipboard();
    }
}

#endif // HAL_NO_CLIPBOARD
