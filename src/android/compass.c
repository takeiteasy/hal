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

#ifndef HAL_NO_COMPASS
#include "hal/compass.h"
#include <android/sensor.h>
#include <pthread.h>
#include <stdbool.h>

static ASensorManager *sensor_manager = NULL;
static const ASensor *compass_sensor = NULL;
static ASensorEventQueue *sensor_event_queue = NULL;
static ALooper *looper = NULL;
static bool is_enabled = false;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct {
    float x, y, z;
    bool valid;
} sensor_data = {0};

static int sensor_callback(int fd, int events, void *data) {
    ASensorEvent event;
    pthread_mutex_lock(&sensor_mutex);
    while (ASensorEventQueue_getEvents(sensor_event_queue, &event, 1) > 0) {
        if (event.type == ASENSOR_TYPE_MAGNETIC_FIELD) {
            sensor_data.x = event.magnetic.x;
            sensor_data.y = event.magnetic.y;
            sensor_data.z = event.magnetic.z;
            sensor_data.valid = true;
        }
    }
    pthread_mutex_unlock(&sensor_mutex);
    return 1;
}

static void init_sensor_manager(void) {
    if (sensor_manager) return;
    sensor_manager = ASensorManager_getInstance();
    if (sensor_manager)
        compass_sensor = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_MAGNETIC_FIELD);
}

bool hal_compass_available(void) {
    init_sensor_manager();
    return compass_sensor != NULL;
}

void hal_compass_enable(void) {
    if (is_enabled) return;
    init_sensor_manager();
    if (!compass_sensor) return;
    
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    if (!looper) looper = ALooper_forThread();
    if (!looper) return;
    
    sensor_event_queue = ASensorManager_createEventQueue(sensor_manager, looper, ALOOPER_POLL_CALLBACK, sensor_callback, NULL);
    if (!sensor_event_queue) return;
    
    if (ASensorEventQueue_enableSensor(sensor_event_queue, compass_sensor) < 0) {
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
        return;
    }
    ASensorEventQueue_setEventRate(sensor_event_queue, compass_sensor, 1000000 / 60);
    is_enabled = true;
}

void hal_compass_disable(void) {
    if (!is_enabled) return;
    pthread_mutex_lock(&sensor_mutex);
    if (sensor_event_queue) {
        ASensorEventQueue_disableSensor(sensor_event_queue, compass_sensor);
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
    }
    is_enabled = false;
    sensor_data.valid = false;
    pthread_mutex_unlock(&sensor_mutex);
}

bool hal_compass_enabled(void) { return is_enabled; }

bool hal_compass_get(float *x, float *y, float *z) {
    pthread_mutex_lock(&sensor_mutex);
    bool valid = sensor_data.valid;
    if (valid) {
        if (x) *x = sensor_data.x;
        if (y) *y = sensor_data.y;
        if (z) *z = sensor_data.z;
    }
    pthread_mutex_unlock(&sensor_mutex);
    return valid;
}
#endif
