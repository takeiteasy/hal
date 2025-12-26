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

// Windows email using mailto: URL

#ifndef HAL_NO_EMAIL
#include "hal/email.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *url_encode(const char *str) {
    if (str == NULL) return strdup("");
    
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1);
    char *p = encoded;
    
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else if (c == ' ') {
            *p++ = '%'; *p++ = '2'; *p++ = '0';
        } else {
            sprintf(p, "%%%02X", (unsigned char)c);
            p += 3;
        }
    }
    *p = '\0';
    return encoded;
}

bool hal_email_available(void) {
    return true;
}

bool hal_email_send(const char *recipient, const char *subject,
                    const char *body, const char *cc, const char *bcc) {
    char url[4096] = "mailto:";
    
    if (recipient) {
        char *enc = url_encode(recipient);
        strncat(url, enc, sizeof(url) - strlen(url) - 1);
        free(enc);
    }
    
    char params[3072] = "";
    char sep = '?';
    
    if (subject) {
        char *enc = url_encode(subject);
        char param[1024];
        snprintf(param, sizeof(param), "%csubject=%s", sep, enc);
        strncat(params, param, sizeof(params) - strlen(params) - 1);
        free(enc);
        sep = '&';
    }
    if (body) {
        char *enc = url_encode(body);
        char param[1024];
        snprintf(param, sizeof(param), "%cbody=%s", sep, enc);
        strncat(params, param, sizeof(params) - strlen(params) - 1);
        free(enc);
        sep = '&';
    }
    if (cc) {
        char *enc = url_encode(cc);
        char param[512];
        snprintf(param, sizeof(param), "%ccc=%s", sep, enc);
        strncat(params, param, sizeof(params) - strlen(params) - 1);
        free(enc);
        sep = '&';
    }
    if (bcc) {
        char *enc = url_encode(bcc);
        char param[512];
        snprintf(param, sizeof(param), "%cbcc=%s", sep, enc);
        strncat(params, param, sizeof(params) - strlen(params) - 1);
        free(enc);
    }
    
    strncat(url, params, sizeof(url) - strlen(url) - 1);
    
    HINSTANCE result = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    return (INT_PTR)result > 32;
}

#endif // HAL_NO_EMAIL
