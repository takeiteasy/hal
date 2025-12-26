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

#ifndef HAL_NO_GAMEPAD
#include "gamepad_common.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <linux/input.h>
#define __USE_UNIX98
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    pthread_t thread;
    int fd;
    char *path;
    int button_map[KEY_CNT - BTN_MISC];
    int axis_map[ABS_CNT];
    struct input_absinfo axis_info[ABS_CNT];
} hal_gamepad_private_t;

static hal_gamepad_device_t **devices = NULL;
static unsigned int num_devices = 0;
static unsigned int next_device_id = 0;
static time_t last_input_stat_time = 0;
static pthread_mutex_t devices_mutex;

static hal_gamepad_queued_event_t *event_queue = NULL;
static size_t event_queue_size = 0;
static size_t event_count = 0;
static pthread_mutex_t event_queue_mutex;

static bool inited = false;

#define test_bit(bit_index, array) \
    ((array[(bit_index) / (sizeof(int) * 8)] >> ((bit_index) % (sizeof(int) * 8))) & 0x1)

static void queue_event(unsigned int device_id, hal_gamepad_event_type_t event_type, void *event_data) {
    hal_gamepad_queued_event_t event;
    event.device_id = device_id;
    event.event_type = event_type;
    event.event_data = event_data;

    pthread_mutex_lock(&event_queue_mutex);
    if (event_count >= event_queue_size) {
        event_queue_size = event_queue_size == 0 ? 1 : event_queue_size * 2;
        event_queue = realloc(event_queue, sizeof(hal_gamepad_queued_event_t) * event_queue_size);
    }
    event_queue[event_count++] = event;
    pthread_mutex_unlock(&event_queue_mutex);
}

static void queue_axis_event(hal_gamepad_device_t *device, double timestamp, unsigned int axis_id, float value, float last_value) {
    hal_gamepad_axis_event_t *event = malloc(sizeof(hal_gamepad_axis_event_t));
    event->device = device;
    event->timestamp = timestamp;
    event->axis_id = axis_id;
    event->value = value;
    event->last_value = last_value;
    queue_event(device->device_id, HAL_GAMEPAD_EVENT_AXIS_MOVED, event);
}

static void queue_button_event(hal_gamepad_device_t *device, double timestamp, unsigned int button_id, bool down) {
    hal_gamepad_button_event_t *event = malloc(sizeof(hal_gamepad_button_event_t));
    event->device = device;
    event->timestamp = timestamp;
    event->button_id = button_id;
    event->down = down;
    queue_event(device->device_id, down ? HAL_GAMEPAD_EVENT_BUTTON_DOWN : HAL_GAMEPAD_EVENT_BUTTON_UP, event);
}

static void dispose_device(hal_gamepad_device_t *device) {
    hal_gamepad_private_t *priv = device->private_data;
    close(priv->fd);
    free(priv->path);
    free(priv);
    free((void *)device->description);
    free(device->axis_states);
    free(device->button_states);
    free(device);
}

static void *device_thread(void *context) {
    hal_gamepad_device_t *device = context;
    hal_gamepad_private_t *priv = device->private_data;
    struct input_event event;

    while (read(priv->fd, &event, sizeof(struct input_event)) > 0) {
        if (event.type == EV_ABS) {
            if (event.code > ABS_MAX || priv->axis_map[event.code] == -1)
                continue;

            float value = (event.value - priv->axis_info[event.code].minimum) /
                (float)(priv->axis_info[event.code].maximum - priv->axis_info[event.code].minimum) * 2.0f - 1.0f;

            queue_axis_event(device,
                event.time.tv_sec + event.time.tv_usec * 0.000001,
                priv->axis_map[event.code],
                value,
                device->axis_states[priv->axis_map[event.code]]);

            device->axis_states[priv->axis_map[event.code]] = value;

        } else if (event.type == EV_KEY) {
            if (event.code < BTN_MISC || event.code > KEY_MAX || priv->button_map[event.code - BTN_MISC] == -1)
                continue;

            queue_button_event(device,
                event.time.tv_sec + event.time.tv_usec * 0.000001,
                priv->button_map[event.code - BTN_MISC],
                !!event.value);

            device->button_states[priv->button_map[event.code - BTN_MISC]] = !!event.value;
        }
    }

    queue_event(device->device_id, HAL_GAMEPAD_EVENT_REMOVED, device);

    pthread_mutex_lock(&devices_mutex);
    for (unsigned int i = 0; i < num_devices; i++) {
        if (devices[i] == device) {
            num_devices--;
            for (unsigned int j = i; j < num_devices; j++)
                devices[j] = devices[j + 1];
            break;
        }
    }
    pthread_mutex_unlock(&devices_mutex);

    return NULL;
}

bool hal_gamepad_available(void) {
    return true;
}

void hal_gamepad_init(void) {
    if (!inited) {
        pthread_mutexattr_t recursive_lock;
        pthread_mutexattr_init(&recursive_lock);
        pthread_mutexattr_settype(&recursive_lock, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&devices_mutex, &recursive_lock);
        pthread_mutex_init(&event_queue_mutex, &recursive_lock);
        inited = true;
        hal_gamepad_detect_devices();
    }
}

void hal_gamepad_shutdown(void) {
    if (inited) {
        unsigned int devices_left;

        do {
            pthread_mutex_lock(&devices_mutex);
            devices_left = num_devices;
            if (devices_left > 0) {
                hal_gamepad_private_t *priv = devices[0]->private_data;
                pthread_cancel(priv->thread);
                pthread_join(priv->thread, NULL);
                dispose_device(devices[0]);
                num_devices--;
                for (unsigned int i = 0; i < num_devices; i++)
                    devices[i] = devices[i + 1];
            }
            pthread_mutex_unlock(&devices_mutex);
        } while (devices_left > 0);

        pthread_mutex_destroy(&devices_mutex);
        pthread_mutex_destroy(&event_queue_mutex);
        free(devices);
        devices = NULL;
        last_input_stat_time = 0;

        for (size_t i = 0; i < event_count; i++) {
            if (event_queue[i].event_type == HAL_GAMEPAD_EVENT_REMOVED)
                dispose_device(event_queue[i].event_data);
            else if (event_queue[i].event_type != HAL_GAMEPAD_EVENT_ATTACHED)
                free(event_queue[i].event_data);
        }

        event_queue_size = 0;
        event_count = 0;
        free(event_queue);
        event_queue = NULL;
        inited = false;
    }
}

unsigned int hal_gamepad_num_devices(void) {
    unsigned int result;
    pthread_mutex_lock(&devices_mutex);
    result = num_devices;
    pthread_mutex_unlock(&devices_mutex);
    return result;
}

hal_gamepad_device_t *hal_gamepad_device_at(unsigned int index) {
    hal_gamepad_device_t *result;
    pthread_mutex_lock(&devices_mutex);
    result = (index >= num_devices) ? NULL : devices[index];
    pthread_mutex_unlock(&devices_mutex);
    return result;
}

void hal_gamepad_detect_devices(void) {
    struct input_id id;
    DIR *dev_input;
    struct dirent *entity;
    unsigned int chars_consumed;
    int num;
    int fd;
    int ev_cap_bits[(EV_CNT - 1) / sizeof(int) / 8 + 1];
    int ev_key_bits[(KEY_CNT - 1) / sizeof(int) / 8 + 1];
    int ev_abs_bits[(ABS_CNT - 1) / sizeof(int) / 8 + 1];
    char file_name[PATH_MAX];
    bool duplicate;
    struct stat stat_buf;
    char name[128];
    char *description;
    time_t current_time;

    if (!inited)
        return;

    pthread_mutex_lock(&devices_mutex);

    dev_input = opendir("/dev/input");
    current_time = time(NULL);
    if (dev_input != NULL) {
        while ((entity = readdir(dev_input)) != NULL) {
            chars_consumed = 0;
            if (sscanf(entity->d_name, "event%d%n", &num, &chars_consumed) && chars_consumed == strlen(entity->d_name)) {
                snprintf(file_name, PATH_MAX, "/dev/input/%s", entity->d_name);
                if (stat(file_name, &stat_buf) || stat_buf.st_mtime < last_input_stat_time)
                    continue;

                duplicate = false;
                for (unsigned int i = 0; i < num_devices; i++) {
                    hal_gamepad_private_t *priv = devices[i]->private_data;
                    if (!strcmp(priv->path, file_name)) {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate)
                    continue;

                fd = open(file_name, O_RDONLY, 0);
                if (fd < 0)
                    continue;

                memset(ev_cap_bits, 0, sizeof(ev_cap_bits));
                memset(ev_key_bits, 0, sizeof(ev_key_bits));
                memset(ev_abs_bits, 0, sizeof(ev_abs_bits));
                if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_cap_bits)), ev_cap_bits) < 0 ||
                    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(ev_key_bits)), ev_key_bits) < 0 ||
                    ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(ev_abs_bits)), ev_abs_bits) < 0) {
                    close(fd);
                    continue;
                }

                if (!test_bit(EV_KEY, ev_cap_bits) || !test_bit(EV_ABS, ev_cap_bits) ||
                    !test_bit(ABS_X, ev_abs_bits) || !test_bit(ABS_Y, ev_abs_bits) ||
                    (!test_bit(BTN_TRIGGER, ev_key_bits) && !test_bit(BTN_A, ev_key_bits) && !test_bit(BTN_1, ev_key_bits))) {
                    close(fd);
                    continue;
                }

                hal_gamepad_device_t *device = malloc(sizeof(hal_gamepad_device_t));
                device->device_id = next_device_id++;
                devices = realloc(devices, sizeof(hal_gamepad_device_t *) * (num_devices + 1));
                devices[num_devices++] = device;

                hal_gamepad_private_t *priv = malloc(sizeof(hal_gamepad_private_t));
                priv->fd = fd;
                priv->path = malloc(strlen(file_name) + 1);
                strcpy(priv->path, file_name);
                memset(priv->button_map, 0xFF, sizeof(priv->button_map));
                memset(priv->axis_map, 0xFF, sizeof(priv->axis_map));
                device->private_data = priv;

                if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) > 0) {
                    description = malloc(strlen(name) + 1);
                    strcpy(description, name);
                } else {
                    description = malloc(strlen(file_name) + 1);
                    strcpy(description, file_name);
                }
                device->description = description;

                if (!ioctl(fd, EVIOCGID, &id)) {
                    device->vendor_id = id.vendor;
                    device->product_id = id.product;
                } else {
                    device->vendor_id = device->product_id = 0;
                }

                memset(ev_key_bits, 0, sizeof(ev_key_bits));
                memset(ev_abs_bits, 0, sizeof(ev_abs_bits));
                ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(ev_key_bits)), ev_key_bits);
                ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(ev_abs_bits)), ev_abs_bits);

                device->num_axes = 0;
                for (int bit = 0; bit < ABS_CNT; bit++) {
                    if (test_bit(bit, ev_abs_bits)) {
                        if (ioctl(fd, EVIOCGABS(bit), &priv->axis_info[bit]) < 0 ||
                            priv->axis_info[bit].minimum == priv->axis_info[bit].maximum)
                            continue;
                        priv->axis_map[bit] = device->num_axes;
                        device->num_axes++;
                    }
                }

                device->num_buttons = 0;
                for (int bit = BTN_MISC; bit < KEY_CNT; bit++) {
                    if (test_bit(bit, ev_key_bits)) {
                        priv->button_map[bit - BTN_MISC] = device->num_buttons;
                        device->num_buttons++;
                    }
                }

                device->axis_states = calloc(sizeof(float), device->num_axes);
                device->button_states = calloc(sizeof(bool), device->num_buttons);

                if (hal_gamepad_attach_cb != NULL)
                    hal_gamepad_attach_cb(device, hal_gamepad_attach_ctx);

                pthread_create(&priv->thread, NULL, device_thread, device);
            }
        }
        closedir(dev_input);
    }

    last_input_stat_time = current_time;
    pthread_mutex_unlock(&devices_mutex);
}

static void process_queued_event(hal_gamepad_queued_event_t event) {
    switch (event.event_type) {
        case HAL_GAMEPAD_EVENT_ATTACHED:
            if (hal_gamepad_attach_cb != NULL)
                hal_gamepad_attach_cb(event.event_data, hal_gamepad_attach_ctx);
            break;

        case HAL_GAMEPAD_EVENT_REMOVED:
            if (hal_gamepad_remove_cb != NULL)
                hal_gamepad_remove_cb(event.event_data, hal_gamepad_remove_ctx);
            break;

        case HAL_GAMEPAD_EVENT_BUTTON_DOWN:
            if (hal_gamepad_button_down_cb != NULL) {
                hal_gamepad_button_event_t *e = event.event_data;
                hal_gamepad_button_down_cb(e->device, e->button_id, e->timestamp, hal_gamepad_button_down_ctx);
            }
            break;

        case HAL_GAMEPAD_EVENT_BUTTON_UP:
            if (hal_gamepad_button_up_cb != NULL) {
                hal_gamepad_button_event_t *e = event.event_data;
                hal_gamepad_button_up_cb(e->device, e->button_id, e->timestamp, hal_gamepad_button_up_ctx);
            }
            break;

        case HAL_GAMEPAD_EVENT_AXIS_MOVED:
            if (hal_gamepad_axis_cb != NULL) {
                hal_gamepad_axis_event_t *e = event.event_data;
                hal_gamepad_axis_cb(e->device, e->axis_id, e->value, e->last_value, e->timestamp, hal_gamepad_axis_ctx);
            }
            break;
    }
}

void hal_gamepad_process_events(void) {
    static bool in_process_events = false;

    if (!inited || in_process_events)
        return;

    in_process_events = true;
    pthread_mutex_lock(&event_queue_mutex);
    for (size_t i = 0; i < event_count; i++) {
        process_queued_event(event_queue[i]);
        if (event_queue[i].event_type == HAL_GAMEPAD_EVENT_REMOVED)
            dispose_device(event_queue[i].event_data);
        else if (event_queue[i].event_type != HAL_GAMEPAD_EVENT_ATTACHED)
            free(event_queue[i].event_data);
    }
    event_count = 0;
    pthread_mutex_unlock(&event_queue_mutex);
    in_process_events = false;
}

#endif /* HAL_NO_GAMEPAD */
