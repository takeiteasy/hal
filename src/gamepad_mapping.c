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

/* Gamepad mapping implementation - translates raw input to standard layout */

#ifndef HAL_NO_GAMEPAD
#include "hal/gamepad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Define platform macro for mapping selection */
#if defined(_WIN32) || defined(_WIN64)
  #define HAL_PLATFORM_WINDOWS
#elif defined(__APPLE__)
  #include <TargetConditionals.h>
  #if TARGET_OS_IOS
    #define HAL_PLATFORM_IOS
  #else
    #define HAL_PLATFORM_MACOS
  #endif
#elif defined(__ANDROID__)
  #define HAL_PLATFORM_ANDROID
#elif defined(__linux__)
  #define HAL_PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__)
  #define HAL_PLATFORM_WEB
#endif

/* Mapping entry for a single device */
typedef struct {
    char guid[33];
    char *name;
    char *mapping_string;
    int button_map[HAL_GAMEPAD_BUTTON_MAX];
    int axis_map[HAL_GAMEPAD_AXIS_MAX];
    /* Hat switch mappings for D-pad */
    int hat_map[4]; /* up, down, left, right -> hat index */
    /* Axis-as-button mappings (for triggers treated as buttons) */
    int axis_button_map[HAL_GAMEPAD_BUTTON_MAX];
    bool axis_button_inverted[HAL_GAMEPAD_BUTTON_MAX];
} hal_gamepad_mapping_t;

/* Dynamic array of custom mappings */
static hal_gamepad_mapping_t *custom_mappings = NULL;
static unsigned int custom_mapping_count = 0;
static unsigned int custom_mapping_capacity = 0;

/* Parse a single binding like "b0", "a1", "h0.1", "+a2", "-a3" */
static void parse_binding(const char *binding, int *button_out, int *axis_out, int *hat_out, int *hat_mask, bool *inverted) {
    *button_out = -1;
    *axis_out = -1;
    *hat_out = -1;
    *hat_mask = 0;
    *inverted = false;
    
    if (!binding || !*binding)
        return;
    
    /* Handle inversion prefix */
    if (*binding == '+' || *binding == '-') {
        *inverted = (*binding == '-');
        binding++;
    }
    
    switch (binding[0]) {
        case 'b':
            *button_out = atoi(binding + 1);
            break;
        case 'a':
            *axis_out = atoi(binding + 1);
            break;
        case 'h':
            /* Hat switch: h0.1, h0.2, h0.4, h0.8 */
            *hat_out = atoi(binding + 1);
            {
                const char *dot = strchr(binding, '.');
                if (dot)
                    *hat_mask = atoi(dot + 1);
            }
            break;
    }
}

/* Parse an SDL mapping string and populate the mapping struct */
static bool parse_mapping_string(const char *mapping_str, hal_gamepad_mapping_t *mapping) {
    /* Initialize all mappings to -1 (unmapped) */
    for (int i = 0; i < HAL_GAMEPAD_BUTTON_MAX; i++) {
        mapping->button_map[i] = -1;
        mapping->axis_button_map[i] = -1;
        mapping->axis_button_inverted[i] = false;
    }
    for (int i = 0; i < HAL_GAMEPAD_AXIS_MAX; i++)
        mapping->axis_map[i] = -1;
    for (int i = 0; i < 4; i++)
        mapping->hat_map[i] = -1;
    
    /* Parse GUID (first 32 chars) */
    if (strlen(mapping_str) < 33)
        return false;
    
    strncpy(mapping->guid, mapping_str, 32);
    mapping->guid[32] = '\0';
    
    /* Find name (between first and second comma) */
    const char *name_start = strchr(mapping_str, ',');
    if (!name_start)
        return false;
    name_start++;
    
    const char *name_end = strchr(name_start, ',');
    if (!name_end)
        return false;
    
    size_t name_len = name_end - name_start;
    mapping->name = malloc(name_len + 1);
    strncpy(mapping->name, name_start, name_len);
    mapping->name[name_len] = '\0';
    
    /* Store full mapping string */
    mapping->mapping_string = strdup(mapping_str);
    
    /* Parse each key:value pair */
    const char *pair = name_end + 1;
    while (pair && *pair) {
        /* Find the colon */
        const char *colon = strchr(pair, ':');
        if (!colon)
            break;
        
        /* Extract key */
        size_t key_len = colon - pair;
        char key[32];
        if (key_len >= sizeof(key))
            key_len = sizeof(key) - 1;
        strncpy(key, pair, key_len);
        key[key_len] = '\0';
        
        /* Extract value (up to comma or end) */
        const char *value = colon + 1;
        const char *comma = strchr(value, ',');
        size_t value_len = comma ? (size_t)(comma - value) : strlen(value);
        char val[32];
        if (value_len >= sizeof(val))
            value_len = sizeof(val) - 1;
        strncpy(val, value, value_len);
        val[value_len] = '\0';
        
        /* Parse the binding */
        int button, axis, hat, hat_mask;
        bool inverted;
        parse_binding(val, &button, &axis, &hat, &hat_mask, &inverted);
        
        /* Map to standard layout */
        hal_gamepad_button_t btn = HAL_GAMEPAD_BUTTON_MAX;
        hal_gamepad_axis_t ax = HAL_GAMEPAD_AXIS_MAX;
        
        if (strcmp(key, "a") == 0) btn = HAL_GAMEPAD_BUTTON_A;
        else if (strcmp(key, "b") == 0) btn = HAL_GAMEPAD_BUTTON_B;
        else if (strcmp(key, "x") == 0) btn = HAL_GAMEPAD_BUTTON_X;
        else if (strcmp(key, "y") == 0) btn = HAL_GAMEPAD_BUTTON_Y;
        else if (strcmp(key, "back") == 0) btn = HAL_GAMEPAD_BUTTON_BACK;
        else if (strcmp(key, "guide") == 0) btn = HAL_GAMEPAD_BUTTON_GUIDE;
        else if (strcmp(key, "start") == 0) btn = HAL_GAMEPAD_BUTTON_START;
        else if (strcmp(key, "leftstick") == 0) btn = HAL_GAMEPAD_BUTTON_LEFTSTICK;
        else if (strcmp(key, "rightstick") == 0) btn = HAL_GAMEPAD_BUTTON_RIGHTSTICK;
        else if (strcmp(key, "leftshoulder") == 0) btn = HAL_GAMEPAD_BUTTON_LEFTSHOULDER;
        else if (strcmp(key, "rightshoulder") == 0) btn = HAL_GAMEPAD_BUTTON_RIGHTSHOULDER;
        else if (strcmp(key, "dpup") == 0) btn = HAL_GAMEPAD_BUTTON_DPAD_UP;
        else if (strcmp(key, "dpdown") == 0) btn = HAL_GAMEPAD_BUTTON_DPAD_DOWN;
        else if (strcmp(key, "dpleft") == 0) btn = HAL_GAMEPAD_BUTTON_DPAD_LEFT;
        else if (strcmp(key, "dpright") == 0) btn = HAL_GAMEPAD_BUTTON_DPAD_RIGHT;
        else if (strcmp(key, "leftx") == 0) ax = HAL_GAMEPAD_AXIS_LEFTX;
        else if (strcmp(key, "lefty") == 0) ax = HAL_GAMEPAD_AXIS_LEFTY;
        else if (strcmp(key, "rightx") == 0) ax = HAL_GAMEPAD_AXIS_RIGHTX;
        else if (strcmp(key, "righty") == 0) ax = HAL_GAMEPAD_AXIS_RIGHTY;
        else if (strcmp(key, "lefttrigger") == 0) ax = HAL_GAMEPAD_AXIS_LEFTTRIGGER;
        else if (strcmp(key, "righttrigger") == 0) ax = HAL_GAMEPAD_AXIS_RIGHTTRIGGER;
        
        /* Store the mapping */
        if (btn != HAL_GAMEPAD_BUTTON_MAX) {
            if (button >= 0) {
                mapping->button_map[btn] = button;
            } else if (axis >= 0) {
                mapping->axis_button_map[btn] = axis;
                mapping->axis_button_inverted[btn] = inverted;
            } else if (hat >= 0) {
                /* D-pad from hat switch */
                if (btn >= HAL_GAMEPAD_BUTTON_DPAD_UP && btn <= HAL_GAMEPAD_BUTTON_DPAD_RIGHT) {
                    mapping->hat_map[btn - HAL_GAMEPAD_BUTTON_DPAD_UP] = hat_mask;
                }
            }
        }
        
        if (ax != HAL_GAMEPAD_AXIS_MAX && axis >= 0) {
            mapping->axis_map[ax] = axis;
        }
        
        /* Move to next pair */
        pair = comma ? comma + 1 : NULL;
    }
    
    return true;
}

/* Generate GUID from device vendor/product IDs */
static void generate_guid(hal_gamepad_device_t *device, char guid[33]) {
    /* Create a GUID similar to SDL's format */
    snprintf(guid, 33, "%04x%04x%04x%04x%04x%04x%04x%04x",
        0, 0,  /* Bus type */
        (unsigned)device->vendor_id & 0xFFFF, 0,
        (unsigned)device->product_id & 0xFFFF, 0,
        0, 0   /* Version */
    );
}

/* Find mapping for a device */
static hal_gamepad_mapping_t *find_mapping(hal_gamepad_device_t *device) {
    char guid[33];
    generate_guid(device, guid);
    
    /* Search custom mappings first */
    for (unsigned int i = 0; i < custom_mapping_count; i++) {
        if (strcmp(custom_mappings[i].guid, guid) == 0)
            return &custom_mappings[i];
    }
    
    /* Search builtin mappings */
#if defined(HAL_PLATFORM_WINDOWS) || defined(HAL_PLATFORM_MACOS) || \
    defined(HAL_PLATFORM_LINUX) || defined(HAL_PLATFORM_ANDROID) || defined(HAL_PLATFORM_IOS)
    for (unsigned int i = 0; i < hal_gamepad_builtin_mapping_count; i++) {
        if (strncmp(hal_gamepad_builtin_mappings[i], guid, 32) == 0) {
            /* Found a match - parse and cache it */
            static hal_gamepad_mapping_t cached;
            static char cached_guid[33] = {0};
            
            if (strcmp(cached_guid, guid) != 0) {
                if (cached.name) free(cached.name);
                if (cached.mapping_string) free(cached.mapping_string);
                memset(&cached, 0, sizeof(cached));
                parse_mapping_string(hal_gamepad_builtin_mappings[i], &cached);
                strncpy(cached_guid, guid, 32);
            }
            return &cached;
        }
    }
#endif
    
    return NULL;
}

/* Public API implementations */

bool hal_gamepad_add_mapping(const char *mapping_string) {
    if (!mapping_string || strlen(mapping_string) < 34)
        return false;
    
    /* Grow array if needed */
    if (custom_mapping_count >= custom_mapping_capacity) {
        custom_mapping_capacity = custom_mapping_capacity == 0 ? 16 : custom_mapping_capacity * 2;
        custom_mappings = realloc(custom_mappings, sizeof(hal_gamepad_mapping_t) * custom_mapping_capacity);
    }
    
    /* Check if we're updating an existing mapping */
    char guid[33];
    strncpy(guid, mapping_string, 32);
    guid[32] = '\0';
    
    for (unsigned int i = 0; i < custom_mapping_count; i++) {
        if (strcmp(custom_mappings[i].guid, guid) == 0) {
            /* Update existing */
            free(custom_mappings[i].name);
            free(custom_mappings[i].mapping_string);
            return parse_mapping_string(mapping_string, &custom_mappings[i]);
        }
    }
    
    /* Add new mapping */
    hal_gamepad_mapping_t *mapping = &custom_mappings[custom_mapping_count];
    memset(mapping, 0, sizeof(*mapping));
    
    if (parse_mapping_string(mapping_string, mapping)) {
        custom_mapping_count++;
        return true;
    }
    
    return false;
}

int hal_gamepad_load_mappings(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f)
        return -1;
    
    int count = 0;
    char line[2048];
    
    while (fgets(line, sizeof(line), f)) {
        /* Skip comments and empty lines */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        
        if (len == 0 || line[0] == '#')
            continue;
        
        if (hal_gamepad_add_mapping(line))
            count++;
    }
    
    fclose(f);
    return count;
}

bool hal_gamepad_get_button(hal_gamepad_device_t *device, hal_gamepad_button_t button) {
    if (!device || button >= HAL_GAMEPAD_BUTTON_MAX)
        return false;
    
    hal_gamepad_mapping_t *mapping = find_mapping(device);
    if (!mapping) {
        /* No mapping - try direct index */
        if ((unsigned int)button < device->num_buttons)
            return device->button_states[button];
        return false;
    }
    
    /* Check button mapping */
    if (mapping->button_map[button] >= 0) {
        int idx = mapping->button_map[button];
        if ((unsigned int)idx < device->num_buttons)
            return device->button_states[idx];
    }
    
    /* Check axis-as-button mapping */
    if (mapping->axis_button_map[button] >= 0) {
        int idx = mapping->axis_button_map[button];
        if ((unsigned int)idx < device->num_axes) {
            float val = device->axis_states[idx];
            if (mapping->axis_button_inverted[button])
                return val < -0.5f;
            else
                return val > 0.5f;
        }
    }
    
    /* Check hat mapping for D-pad */
    if (button >= HAL_GAMEPAD_BUTTON_DPAD_UP && button <= HAL_GAMEPAD_BUTTON_DPAD_RIGHT) {
        int hat_mask = mapping->hat_map[button - HAL_GAMEPAD_BUTTON_DPAD_UP];
        if (hat_mask > 0) {
            /* Hat values are typically stored in axes with special encoding */
            /* This would need platform-specific handling */
        }
    }
    
    return false;
}

float hal_gamepad_get_axis(hal_gamepad_device_t *device, hal_gamepad_axis_t axis) {
    if (!device || axis >= HAL_GAMEPAD_AXIS_MAX)
        return 0.0f;
    
    hal_gamepad_mapping_t *mapping = find_mapping(device);
    if (!mapping) {
        /* No mapping - try direct index */
        if ((unsigned int)axis < device->num_axes)
            return device->axis_states[axis];
        return 0.0f;
    }
    
    if (mapping->axis_map[axis] >= 0) {
        int idx = mapping->axis_map[axis];
        if ((unsigned int)idx < device->num_axes)
            return device->axis_states[idx];
    }
    
    return 0.0f;
}

bool hal_gamepad_is_mapped(hal_gamepad_device_t *device) {
    return device && find_mapping(device) != NULL;
}

const char *hal_gamepad_get_mapping(hal_gamepad_device_t *device) {
    if (!device)
        return NULL;
    
    hal_gamepad_mapping_t *mapping = find_mapping(device);
    return mapping ? mapping->mapping_string : NULL;
}

#endif /* HAL_NO_GAMEPAD */
