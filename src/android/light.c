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

#ifndef HAL_NO_LIGHT
#include "hal/light.h"
#include <android/sensor.h>
#include <pthread.h>
#include <stdbool.h>

static ASensorManager *sensor_manager = NULL;
static const ASensor *light_sensor = NULL;
static ASensorEventQueue *sensor_event_queue = NULL;
static ALooper *looper = NULL;
static bool is_enabled = false;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;
static float light_value = -1.0f;
static bool light_valid = false;

static int sensor_callback(int fd, int events, void *data) {
    ASensorEvent event;
    pthread_mutex_lock(&sensor_mutex);
    while (ASensorEventQueue_getEvents(sensor_event_queue, &event, 1) > 0) {
        if (event.type == ASENSOR_TYPE_LIGHT) {
            light_value = event.light;
            light_valid = true;
        }
    }
    pthread_mutex_unlock(&sensor_mutex);
    return 1;
}

static void init_sensor_manager(void) {
    if (sensor_manager) return;
    sensor_manager = ASensorManager_getInstance();
    if (sensor_manager)
        light_sensor = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_LIGHT);
}

bool hal_light_available(void) {
    init_sensor_manager();
    return light_sensor != NULL;
}

void hal_light_enable(void) {
    if (is_enabled) return;
    init_sensor_manager();
    if (!light_sensor) return;
    
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    if (!looper) looper = ALooper_forThread();
    if (!looper) return;
    
    sensor_event_queue = ASensorManager_createEventQueue(sensor_manager, looper, ALOOPER_POLL_CALLBACK, sensor_callback, NULL);
    if (!sensor_event_queue) return;
    
    if (ASensorEventQueue_enableSensor(sensor_event_queue, light_sensor) < 0) {
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
        return;
    }
    ASensorEventQueue_setEventRate(sensor_event_queue, light_sensor, 1000000 / 10); // 10Hz is enough for light
    is_enabled = true;
}

void hal_light_disable(void) {
    if (!is_enabled) return;
    pthread_mutex_lock(&sensor_mutex);
    if (sensor_event_queue) {
        ASensorEventQueue_disableSensor(sensor_event_queue, light_sensor);
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
    }
    is_enabled = false;
    light_valid = false;
    pthread_mutex_unlock(&sensor_mutex);
}

bool hal_light_enabled(void) { return is_enabled; }

float hal_light_get(void) {
    pthread_mutex_lock(&sensor_mutex);
    float result = light_valid ? light_value : -1.0f;
    pthread_mutex_unlock(&sensor_mutex);
    return result;
}
#endif
