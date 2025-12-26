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

// Web GPS using Geolocation API

#ifndef HAL_NO_GPS
#include "hal/gps.h"
#include <emscripten.h>
#include <stdlib.h>

static hal_gps_location_cb g_location_cb = NULL;
static hal_gps_status_cb g_status_cb = NULL;
static void *g_ctx = NULL;
static int g_watch_id = -1;

EMSCRIPTEN_KEEPALIVE
void hal_gps_web_location(double lat, double lon, double alt, double speed, double heading) {
    if (g_location_cb) {
        g_location_cb(lat, lon, alt, speed, heading, g_ctx);
    }
}

EMSCRIPTEN_KEEPALIVE
void hal_gps_web_error(const char *msg) {
    if (g_status_cb) {
        g_status_cb(msg, g_ctx);
    }
}

EM_JS(int, js_gps_available, (), {
    return (typeof navigator !== 'undefined' && 'geolocation' in navigator) ? 1 : 0;
});

EM_JS(int, js_gps_start, (), {
    if (!navigator.geolocation) return -1;
    
    var options = {
        enableHighAccuracy: true,
        maximumAge: 1000
    };
    
    var watchId = navigator.geolocation.watchPosition(
        function(pos) {
            Module._hal_gps_web_location(
                pos.coords.latitude,
                pos.coords.longitude,
                pos.coords.altitude || 0,
                pos.coords.speed || 0,
                pos.coords.heading || 0
            );
        },
        function(err) {
            var msg = allocateUTF8(err.message);
            Module._hal_gps_web_error(msg);
            _free(msg);
        },
        options
    );
    
    return watchId;
});

EM_JS(void, js_gps_stop, (int watchId), {
    if (navigator.geolocation && watchId >= 0) {
        navigator.geolocation.clearWatch(watchId);
    }
});

bool hal_gps_available(void) {
    return js_gps_available() != 0;
}

void hal_gps_configure(hal_gps_location_cb on_location, hal_gps_status_cb on_status, void *ctx) {
    g_location_cb = on_location;
    g_status_cb = on_status;
    g_ctx = ctx;
}

bool hal_gps_start(int min_time_ms, float min_distance_m) {
    (void)min_time_ms; (void)min_distance_m;
    
    if (g_watch_id >= 0) {
        js_gps_stop(g_watch_id);
    }
    
    g_watch_id = js_gps_start();
    return g_watch_id >= 0;
}

void hal_gps_stop(void) {
    if (g_watch_id >= 0) {
        js_gps_stop(g_watch_id);
        g_watch_id = -1;
    }
}

#endif // HAL_NO_GPS
