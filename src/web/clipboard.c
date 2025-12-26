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

// Emscripten clipboard using the async Clipboard API via EM_JS
// Note: Browser clipboard access requires user gesture (click/key event)

#ifndef HAL_NO_CLIPBOARD
#include "hal/clipboard.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

// Static buffer for synchronous clipboard access (limited due to async nature)
static char *clipboard_text = NULL;

EM_JS(int, js_clipboard_available, (), {
    return (navigator.clipboard && navigator.clipboard.readText) ? 1 : 0;
});

EM_JS(int, js_clipboard_has_text, (), {
    // Cannot reliably check synchronously due to async API
    // Return true if clipboard API is available
    return (navigator.clipboard) ? 1 : 0;
});

EM_ASYNC_JS(char*, js_clipboard_get_text, (), {
    try {
        if (!navigator.clipboard || !navigator.clipboard.readText) {
            return 0;
        }
        var text = await navigator.clipboard.readText();
        if (!text) return 0;
        var lengthBytes = lengthBytesUTF8(text) + 1;
        var stringOnHeap = _malloc(lengthBytes);
        stringToUTF8(text, stringOnHeap, lengthBytes);
        return stringOnHeap;
    } catch (e) {
        console.warn('Clipboard read failed:', e);
        return 0;
    }
});

EM_ASYNC_JS(int, js_clipboard_set_text, (const char *text), {
    try {
        if (!navigator.clipboard || !navigator.clipboard.writeText) {
            return 0;
        }
        var jsText = UTF8ToString(text);
        await navigator.clipboard.writeText(jsText);
        return 1;
    } catch (e) {
        console.warn('Clipboard write failed:', e);
        return 0;
    }
});

EM_ASYNC_JS(void, js_clipboard_clear, (), {
    try {
        if (navigator.clipboard && navigator.clipboard.writeText) {
            await navigator.clipboard.writeText('');
        }
    } catch (e) {
        console.warn('Clipboard clear failed:', e);
    }
});

bool hal_clipboard_available(void) {
    return js_clipboard_available() != 0;
}

bool hal_clipboard_has_text(void) {
    return js_clipboard_has_text() != 0;
}

char *hal_clipboard_get_text(void) {
    return js_clipboard_get_text();
}

bool hal_clipboard_set_text(const char *text) {
    if (text == NULL)
        return false;
    return js_clipboard_set_text(text) != 0;
}

void hal_clipboard_clear(void) {
    js_clipboard_clear();
}

#endif // HAL_NO_CLIPBOARD
