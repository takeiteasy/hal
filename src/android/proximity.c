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

#ifndef HAL_NO_PROXIMITY
#include "hal/proximity.h"
#include <android/sensor.h>
#include <pthread.h>
#include <stdbool.h>

static ASensorManager *sensor_manager = NULL;
static const ASensor *proximity_sensor = NULL;
static ASensorEventQueue *sensor_event_queue = NULL;
static ALooper *looper = NULL;
static bool is_enabled = false;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;
static float proximity_value = -1.0f;
static bool proximity_valid = false;
static float max_range = 5.0f; // Default max range

static int sensor_callback(int fd, int events, void *data) {
    ASensorEvent event;
    pthread_mutex_lock(&sensor_mutex);
    while (ASensorEventQueue_getEvents(sensor_event_queue, &event, 1) > 0) {
        if (event.type == ASENSOR_TYPE_PROXIMITY) {
            proximity_value = event.distance;
            proximity_valid = true;
        }
    }
    pthread_mutex_unlock(&sensor_mutex);
    return 1;
}

static void init_sensor_manager(void) {
    if (sensor_manager) return;
    sensor_manager = ASensorManager_getInstance();
    if (sensor_manager) {
        proximity_sensor = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_PROXIMITY);
        if (proximity_sensor) {
            max_range = ASensor_getMaxRange(proximity_sensor);
        }
    }
}

bool hal_proximity_available(void) {
    init_sensor_manager();
    return proximity_sensor != NULL;
}

void hal_proximity_enable(void) {
    if (is_enabled) return;
    init_sensor_manager();
    if (!proximity_sensor) return;
    
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    if (!looper) looper = ALooper_forThread();
    if (!looper) return;
    
    sensor_event_queue = ASensorManager_createEventQueue(sensor_manager, looper, ALOOPER_POLL_CALLBACK, sensor_callback, NULL);
    if (!sensor_event_queue) return;
    
    if (ASensorEventQueue_enableSensor(sensor_event_queue, proximity_sensor) < 0) {
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
        return;
    }
    ASensorEventQueue_setEventRate(sensor_event_queue, proximity_sensor, 1000000 / 10);
    is_enabled = true;
}

void hal_proximity_disable(void) {
    if (!is_enabled) return;
    pthread_mutex_lock(&sensor_mutex);
    if (sensor_event_queue) {
        ASensorEventQueue_disableSensor(sensor_event_queue, proximity_sensor);
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
    }
    is_enabled = false;
    proximity_valid = false;
    pthread_mutex_unlock(&sensor_mutex);
}

bool hal_proximity_enabled(void) { return is_enabled; }

float hal_proximity_get(void) {
    pthread_mutex_lock(&sensor_mutex);
    float result = proximity_valid ? proximity_value : -1.0f;
    pthread_mutex_unlock(&sensor_mutex);
    return result;
}

bool hal_proximity_is_near(void) {
    pthread_mutex_lock(&sensor_mutex);
    // Object is near if value is less than max range
    bool near = proximity_valid && (proximity_value < max_range);
    pthread_mutex_unlock(&sensor_mutex);
    return near;
}
#endif
