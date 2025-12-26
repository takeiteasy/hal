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

// Emscripten email using mailto: URL

#ifndef HAL_NO_EMAIL
#include "hal/email.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

EM_JS(void, js_open_mailto, (const char *url), {
    var urlStr = UTF8ToString(url);
    window.location.href = urlStr;
});

bool hal_email_available(void) {
    return true;
}

bool hal_email_send(const char *recipient, const char *subject,
                    const char *body, const char *cc, const char *bcc) {
    char url[4096] = "mailto:";
    
    if (recipient) strncat(url, recipient, sizeof(url) - strlen(url) - 1);
    
    char sep = '?';
    if (subject) {
        char tmp[512];
        snprintf(tmp, sizeof(tmp), "%csubject=%s", sep, subject);
        strncat(url, tmp, sizeof(url) - strlen(url) - 1);
        sep = '&';
    }
    if (body) {
        char tmp[2048];
        snprintf(tmp, sizeof(tmp), "%cbody=%s", sep, body);
        strncat(url, tmp, sizeof(url) - strlen(url) - 1);
        sep = '&';
    }
    if (cc) {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "%ccc=%s", sep, cc);
        strncat(url, tmp, sizeof(url) - strlen(url) - 1);
        sep = '&';
    }
    if (bcc) {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "%cbcc=%s", sep, bcc);
        strncat(url, tmp, sizeof(url) - strlen(url) - 1);
    }
    
    js_open_mailto(url);
    return true;
}

#endif // HAL_NO_EMAIL
