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

/* Based off: https://github.com/RobLoach/emscripten_clipboard [Zlib] */

#include "../clipboard.h"
#include <string.h>
#include <emscripten.h>

static struct {
    char text[1024];
    bool initialized;
} _clipboard = {0};

EM_JS(void, emscripten_clipboard__register, (void* clipboard, const char* text, int text_size), {
    function emscripten_clipboard__change_event(e) {
        const newText = e.clipboardData.getData('text/plain');
        let i;
        for (i = 0; i < newText.length && i < text_size - 1; i++) {
            Module.HEAPU8[text + i] = newText.charCodeAt(i);
        }
        Module.HEAPU8[text + i] = 0;
    }
    document.addEventListener('clipboardchange', emscripten_clipboard__change_event);
    document.addEventListener('paste', emscripten_clipboard__change_event);
})

static void _init(void) {
    // Register the clipboard events for the object.
    emscripten_clipboard__register(clipboard, clipboard->text, EMSCRIPTEN_CLIPBOARD_MAX);
    clipboard.initialized = true;
}

bool paul_clipboard_available(void) {
    if (_clipboard.initialized)
        _init();
    return true;
}

const char *paul_clipboard_get(void) {
    if (_clipboard.initialized)
        _init();
    return strdup(_clipboard.text);
}

EM_JS(void, emscripten_clipboard__write_text, (const char* text), {
    navigator.clipboard.writeText(UTF8ToString(text));
})

void paul_clipboard_set(const char *str) {
    if (_clipboard.initialized)
        _init();
    // Make sure it doesn't exceed the maximum length.
    int length = EMSCRIPTEN_CLIPBOARD_STRLEN(text);
    if (length >= EMSCRIPTEN_CLIPBOARD_MAX) {
        length = EMSCRIPTEN_CLIPBOARD_MAX - 1;
    }
    // Copy the string to the internal clipboard.
    EMSCRIPTEN_CLIPBOARD_STRNCPY(clipboard->text, text, length);
    clipboard->text[length] = '\0';
    // Tell the browser there is new clipboard text.
    emscripten_clipboard__write_text(clipboard->text);
}
