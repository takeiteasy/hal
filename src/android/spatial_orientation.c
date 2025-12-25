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

// Android spatial orientation using rotation vector sensor

#ifndef HAL_NO_SPATIAL_ORIENTATION
#include "hal/spatial_orientation.h"
#include <android/sensor.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>

static ASensorManager *sensor_manager = NULL;
static const ASensor *rotation_sensor = NULL;
static ASensorEventQueue *sensor_event_queue = NULL;
static ALooper *looper = NULL;
static bool is_enabled = false;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct {
    float yaw, pitch, roll;
    bool valid;
} orientation_data = {0};

// Convert rotation vector to Euler angles
static void rotation_to_euler(float *rotation, float *yaw, float *pitch, float *roll) {
    // Rotation vector contains x, y, z components of axis*sin(angle/2)
    float x = rotation[0];
    float y = rotation[1];
    float z = rotation[2];
    float w = sqrtf(1.0f - x*x - y*y - z*z);
    
    // Convert quaternion to Euler angles
    *yaw = atan2f(2.0f * (w*z + x*y), 1.0f - 2.0f * (y*y + z*z));
    *pitch = asinf(2.0f * (w*y - z*x));
    *roll = atan2f(2.0f * (w*x + y*z), 1.0f - 2.0f * (x*x + y*y));
}

static int sensor_callback(int fd, int events, void *data) {
    ASensorEvent event;
    pthread_mutex_lock(&sensor_mutex);
    while (ASensorEventQueue_getEvents(sensor_event_queue, &event, 1) > 0) {
        if (event.type == ASENSOR_TYPE_ROTATION_VECTOR) {
            float rotation[3] = {event.data[0], event.data[1], event.data[2]};
            rotation_to_euler(rotation, &orientation_data.yaw, &orientation_data.pitch, &orientation_data.roll);
            orientation_data.valid = true;
        }
    }
    pthread_mutex_unlock(&sensor_mutex);
    return 1;
}

static void init_sensor_manager(void) {
    if (sensor_manager) return;
    sensor_manager = ASensorManager_getInstance();
    if (sensor_manager)
        rotation_sensor = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_ROTATION_VECTOR);
}

bool hal_spatial_orientation_available(void) {
    init_sensor_manager();
    return rotation_sensor != NULL;
}

void hal_spatial_orientation_enable(void) {
    if (is_enabled) return;
    init_sensor_manager();
    if (!rotation_sensor) return;
    
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    if (!looper) looper = ALooper_forThread();
    if (!looper) return;
    
    sensor_event_queue = ASensorManager_createEventQueue(sensor_manager, looper, ALOOPER_POLL_CALLBACK, sensor_callback, NULL);
    if (!sensor_event_queue) return;
    
    if (ASensorEventQueue_enableSensor(sensor_event_queue, rotation_sensor) < 0) {
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
        return;
    }
    ASensorEventQueue_setEventRate(sensor_event_queue, rotation_sensor, 1000000 / 60);
    is_enabled = true;
}

void hal_spatial_orientation_disable(void) {
    if (!is_enabled) return;
    pthread_mutex_lock(&sensor_mutex);
    if (sensor_event_queue) {
        ASensorEventQueue_disableSensor(sensor_event_queue, rotation_sensor);
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = NULL;
    }
    is_enabled = false;
    orientation_data.valid = false;
    pthread_mutex_unlock(&sensor_mutex);
}

bool hal_spatial_orientation_enabled(void) { return is_enabled; }

bool hal_spatial_orientation_get(float *yaw, float *pitch, float *roll) {
    pthread_mutex_lock(&sensor_mutex);
    bool valid = orientation_data.valid;
    if (valid) {
        if (yaw) *yaw = orientation_data.yaw;
        if (pitch) *pitch = orientation_data.pitch;
        if (roll) *roll = orientation_data.roll;
    }
    pthread_mutex_unlock(&sensor_mutex);
    return valid;
}
#endif
