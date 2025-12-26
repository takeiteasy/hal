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

// Emscripten file chooser using HTML5 input element
// Note: Only open file dialog is supported (browser limitation)
// Save and directory dialogs are not available in browsers

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

static volatile bool g_result_ready = false;
static hal_file_chooser_result_t *g_pending_result = NULL;
static volatile int g_alert_result = -1;

EM_JS(int, js_file_chooser_available, (), {
    return (typeof document !== 'undefined') ? 1 : 0;
});

EM_JS(void, js_show_file_chooser, (int allow_multiple, const char *accept), {
    var input = document.createElement('input');
    input.type = 'file';
    input.style.display = 'none';
    
    if (allow_multiple) {
        input.multiple = true;
    }
    
    if (accept) {
        input.accept = UTF8ToString(accept);
    }
    
    input.onchange = function(event) {
        var files = event.target.files;
        if (files.length > 0) {
            // Store files in virtual filesystem and get paths
            var paths = [];
            for (var i = 0; i < files.length; i++) {
                var file = files[i];
                var path = '/tmp/' + file.name;
                
                var reader = new FileReader();
                reader.onload = (function(filePath, fileIndex, totalFiles) {
                    return function(e) {
                        try {
                            var data = new Uint8Array(e.target.result);
                            FS.writeFile(filePath, data);
                            paths.push(filePath);
                            
                            if (paths.length === totalFiles) {
                                // All files loaded, call C callback
                                var pathsPtr = Module._malloc(paths.length * 4);
                                for (var j = 0; j < paths.length; j++) {
                                    var strPtr = allocateUTF8(paths[j]);
                                    Module.HEAP32[(pathsPtr >> 2) + j] = strPtr;
                                }
                                Module._hal_file_chooser_web_callback(paths.length, pathsPtr);
                            }
                        } catch (err) {
                            console.error('Error writing file:', err);
                        }
                    };
                })(path, i, files.length);
                reader.readAsArrayBuffer(file);
            }
        } else {
            Module._hal_file_chooser_web_callback(0, 0);
        }
        document.body.removeChild(input);
    };
    
    input.oncancel = function() {
        Module._hal_file_chooser_web_callback(0, 0);
        document.body.removeChild(input);
    };
    
    document.body.appendChild(input);
    input.click();
});

EM_JS(int, js_alert_show, (int type, const char *title, const char *message), {
    var titleStr = title ? UTF8ToString(title) : '';
    var msgStr = message ? UTF8ToString(message) : '';
    
    if (type === 3) { // HAL_ALERT_QUESTION
        return confirm(titleStr + '\n' + msgStr) ? 0 : 1;
    } else {
        alert(titleStr + '\n' + msgStr);
        return 0;
    }
});

// Callback from JavaScript
EMSCRIPTEN_KEEPALIVE
void hal_file_chooser_web_callback(int count, char **paths) {
    if (count > 0 && paths != NULL) {
        g_pending_result = malloc(sizeof(hal_file_chooser_result_t));
        g_pending_result->count = count;
        g_pending_result->paths = malloc(sizeof(char *) * (count + 1));
        
        for (int i = 0; i < count; i++) {
            g_pending_result->paths[i] = strdup(paths[i]);
            free(paths[i]);
        }
        g_pending_result->paths[count] = NULL;
        free(paths);
    } else {
        g_pending_result = NULL;
    }
    g_result_ready = true;
}

bool hal_file_chooser_available(void) {
    return js_file_chooser_available() != 0;
}

static const char *get_accept_from_filters(const char **filters) {
    static char accept[256];
    accept[0] = '\0';
    
    if (filters == NULL || filters[0] == NULL)
        return NULL;
    
    for (int i = 0; filters[i] != NULL; i++) {
        if (i > 0) strncat(accept, ",", sizeof(accept) - strlen(accept) - 1);
        
        const char *filter = filters[i];
        // Convert *.ext to .ext for HTML accept attribute
        if (filter[0] == '*') filter++;
        strncat(accept, filter, sizeof(accept) - strlen(accept) - 1);
    }
    
    return accept[0] ? accept : NULL;
}

hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options) {
    if (options == NULL)
        return NULL;
    
    // Only open mode is supported in browsers
    if (options->mode == HAL_FILE_CHOOSER_SAVE || 
        options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
        return NULL;
    }
    
    g_result_ready = false;
    g_pending_result = NULL;
    
    const char *accept = get_accept_from_filters(options->filters);
    js_show_file_chooser(options->allow_multiple ? 1 : 0, accept);
    
    // Wait for callback (async)
    while (!g_result_ready) {
        emscripten_sleep(10);
    }
    
    return g_pending_result;
}

void hal_file_chooser_result_free(hal_file_chooser_result_t *result) {
    if (result == NULL)
        return;
    
    if (result->paths != NULL) {
        for (int i = 0; i < result->count; i++) {
            free(result->paths[i]);
        }
        free(result->paths);
    }
    free(result);
}

int hal_alert_show(hal_alert_type_t type, const char *title, const char *message,
                   const char **buttons, int button_count) {
    (void)buttons;
    (void)button_count;
    return js_alert_show((int)type, title, message);
}

#endif // HAL_NO_FILE_CHOOSER
