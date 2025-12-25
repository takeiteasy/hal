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

#ifndef HAL_NO_ACCELEROMETER
#include "hal/accelerometer.h"
#include <jni.h>
#include <android/sensor.h>
#include <pthread.h>
#include <stdbool.h>

static ASensorManager *sensor_manager = NULL;
static const ASensor *accelerometer_sensor = NULL;
static ASensorEventQueue *sensor_event_queue = NULL;
static ALooper *looper = NULL;
static bool is_enabled = false;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct {
    float x;
    float y;
    float z;
    bool valid;
} sensor_data = {0.0f, 0.0f, 0.0f, false};

static int sensor_callback(int fd, int events, void *data) {
    ASensorEvent event;
    pthread_mutex_lock(&sensor_mutex);

    while (ASensorEventQueue_getEvents(sensor_event_queue, &event, 1) > 0) {
        if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
            sensor_data.x = event.acceleration.x;
            sensor_data.y = event.acceleration.y;
            sensor_data.z = event.acceleration.z;
            sensor_data.valid = true;
        }
    }

    pthread_mutex_unlock(&sensor_mutex);
    return 1;
}

static void init_sensor_manager(void) {
    if (sensor_manager)
        return;

    sensor_manager = ASensorManager_getInstance();
    if (!sensor_manager)
        return;

    accelerometer_sensor = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_ACCELEROMETER);
}

bool hal_accelerometer_available(void) {
    init_sensor_manager();
    return accelerometer_sensor != NULL;
}

void hal_accelerometer_enable(void) {
    if (is_enabled)
        return;

    init_sensor_manager();
    if (!accelerometer_sensor)
        return;

    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    if (!looper)
        looper = ALooper_forThread();

    if (!looper)
        return;

    sensor_event_queue = ASensorManager_createEventQueue(sensor_manager, looper, ALOOPER_POLL_CALLBACK, sensor_callback, NULL);
    if (!sensor_event_queue)
        return;

    if (ASensorEventQueue_enableSensor(sensor_event_queue, accelerometer_sensor) < 0) {
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
        return;
    }

    // Set event rate to 60Hz
    ASensorEventQueue_setEventRate(sensor_event_queue, accelerometer_sensor, 1000000 / 60);
    pthread_mutex_lock(&sensor_mutex);
    is_enabled = true;
    pthread_mutex_unlock(&sensor_mutex);
}

void hal_accelerometer_disable(void) {
    if (!is_enabled)
        return;
    pthread_mutex_lock(&sensor_mutex);
    if (sensor_event_queue) {
        ASensorEventQueue_disableSensor(sensor_event_queue, accelerometer_sensor);
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
    }
    sensor_data.valid = false;
    is_enabled = false;
    pthread_mutex_unlock(&sensor_mutex);
}

bool hal_accelerometer_enabled(void) {
    pthread_mutex_lock(&sensor_mutex);
    bool enabled = is_enabled;
    pthread_mutex_unlock(&sensor_mutex);
    return enabled;
}

bool hal_accelerometer_disabled(void) {
    return !hal_accelerometer_enabled();
}

bool hal_accelerometer_toggle(void) {
    if (hal_accelerometer_enabled()) {
        hal_accelerometer_disable();
        return false;
    } else {
        hal_accelerometer_enable();
        return hal_accelerometer_enabled();
    }
}

bool hal_accelerometer_acceleration(float *x, float *y, float *z) {
    if (!is_enabled || !sensor_data.valid) {
        if (x)
            *x = 0.0f;
        if (y)
            *y = 0.0f;
        if (z)
            *z = 0.0f;
        return false;
    }

    pthread_mutex_lock(&sensor_mutex);

    if (x)
        *x = sensor_data.x;
    if (y)
        *y = sensor_data.y;
    if (z)
        *z = sensor_data.z;

    bool valid = sensor_data.valid;
    pthread_mutex_unlock(&sensor_mutex);
    return valid;
}

#endif // HAL_NO_ACCELEROMETER
