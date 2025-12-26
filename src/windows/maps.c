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

// Windows maps using Bing Maps URL

#ifndef HAL_NO_MAPS
#include "hal/maps.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *url_encode(const char *str) {
    if (!str) return strdup("");
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1);
    char *p = encoded;
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else if (c == ' ') {
            *p++ = '+';
        } else {
            sprintf(p, "%%%02X", (unsigned char)c);
            p += 3;
        }
    }
    *p = '\0';
    return encoded;
}

bool hal_maps_available(void) {
    return true;
}

bool hal_maps_open_address(const char *address) {
    if (!address) return false;
    char *enc = url_encode(address);
    char url[1024];
    snprintf(url, sizeof(url), "https://www.bing.com/maps?q=%s", enc);
    free(enc);
    return (INT_PTR)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32;
}

bool hal_maps_open_coordinates(double lat, double lon, const char *label) {
    char url[1024];
    if (label) {
        char *enc = url_encode(label);
        snprintf(url, sizeof(url), "https://www.bing.com/maps?cp=%f~%f&lvl=15&q=%s", lat, lon, enc);
        free(enc);
    } else {
        snprintf(url, sizeof(url), "https://www.bing.com/maps?cp=%f~%f&lvl=15", lat, lon);
    }
    return (INT_PTR)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32;
}

bool hal_maps_search(const char *query, double lat, double lon) {
    if (!query) return false;
    char *enc = url_encode(query);
    char url[1024];
    if (lat != 0 || lon != 0) {
        snprintf(url, sizeof(url), "https://www.bing.com/maps?q=%s&cp=%f~%f", enc, lat, lon);
    } else {
        snprintf(url, sizeof(url), "https://www.bing.com/maps?q=%s", enc);
    }
    free(enc);
    return (INT_PTR)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32;
}

bool hal_maps_route(const char *from, const char *to) {
    if (!from || !to) return false;
    char *enc_from = url_encode(from);
    char *enc_to = url_encode(to);
    char url[2048];
    snprintf(url, sizeof(url), "https://www.bing.com/maps/directions?rtp=adr.%s~adr.%s",
             enc_from, enc_to);
    free(enc_from);
    free(enc_to);
    return (INT_PTR)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32;
}

#endif // HAL_NO_MAPS
