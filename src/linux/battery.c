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

#ifndef HAL_NO_BATTERY
#include "hal/battery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define POWER_SUPPLY_PATH "/sys/class/power_supply"

static char battery_path[512] = {0};

static bool find_battery(void) {
    if (battery_path[0] != '\0')
        return true;
    
    DIR *dir = opendir(POWER_SUPPLY_PATH);
    if (!dir)
        return false;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        
        char type_path[512];
        snprintf(type_path, sizeof(type_path), "%s/%s/type", POWER_SUPPLY_PATH, entry->d_name);
        
        FILE *f = fopen(type_path, "r");
        if (!f)
            continue;
        
        char type[32] = {0};
        if (fgets(type, sizeof(type), f)) {
            // Remove newline
            char *nl = strchr(type, '\n');
            if (nl) *nl = '\0';
            
            if (strcmp(type, "Battery") == 0) {
                snprintf(battery_path, sizeof(battery_path), "%s/%s", POWER_SUPPLY_PATH, entry->d_name);
                fclose(f);
                closedir(dir);
                return true;
            }
        }
        fclose(f);
    }
    
    closedir(dir);
    return false;
}

static int read_int_file(const char *filename) {
    if (!find_battery())
        return -1;
    
    char path[768];
    snprintf(path, sizeof(path), "%s/%s", battery_path, filename);
    
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;
    
    int value = -1;
    if (fscanf(f, "%d", &value) != 1)
        value = -1;
    
    fclose(f);
    return value;
}

static bool read_string_file(const char *filename, char *buffer, size_t size) {
    if (!find_battery())
        return false;
    
    char path[768];
    snprintf(path, sizeof(path), "%s/%s", battery_path, filename);
    
    FILE *f = fopen(path, "r");
    if (!f)
        return false;
    
    if (!fgets(buffer, size, f)) {
        fclose(f);
        return false;
    }
    
    // Remove newline
    char *nl = strchr(buffer, '\n');
    if (nl) *nl = '\0';
    
    fclose(f);
    return true;
}

bool hal_battery_available(void) {
    return find_battery();
}

int hal_battery_level(void) {
    int capacity = read_int_file("capacity");
    if (capacity >= 0)
        return capacity;
    
    // Fallback: calculate from energy/charge values
    int now = read_int_file("energy_now");
    int full = read_int_file("energy_full");
    
    if (now < 0 || full <= 0) {
        now = read_int_file("charge_now");
        full = read_int_file("charge_full");
    }
    
    if (now >= 0 && full > 0)
        return (now * 100) / full;
    
    return -1;
}

hal_battery_status_t hal_battery_status(void) {
    char status[32];
    if (!read_string_file("status", status, sizeof(status)))
        return HAL_BATTERY_STATUS_UNKNOWN;
    
    if (strcmp(status, "Charging") == 0)
        return HAL_BATTERY_STATUS_CHARGING;
    if (strcmp(status, "Discharging") == 0)
        return HAL_BATTERY_STATUS_DISCHARGING;
    if (strcmp(status, "Full") == 0)
        return HAL_BATTERY_STATUS_FULL;
    if (strcmp(status, "Not charging") == 0)
        return HAL_BATTERY_STATUS_NOT_CHARGING;
    
    return HAL_BATTERY_STATUS_UNKNOWN;
}

bool hal_battery_is_charging(void) {
    return hal_battery_status() == HAL_BATTERY_STATUS_CHARGING;
}

bool hal_battery_is_plugged(void) {
    // Check AC adapter
    DIR *dir = opendir(POWER_SUPPLY_PATH);
    if (!dir)
        return false;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        
        char type_path[512];
        snprintf(type_path, sizeof(type_path), "%s/%s/type", POWER_SUPPLY_PATH, entry->d_name);
        
        FILE *f = fopen(type_path, "r");
        if (!f)
            continue;
        
        char type[32] = {0};
        if (fgets(type, sizeof(type), f)) {
            char *nl = strchr(type, '\n');
            if (nl) *nl = '\0';
            
            if (strcmp(type, "Mains") == 0 || strcmp(type, "USB") == 0) {
                fclose(f);
                
                // Check if online
                char online_path[512];
                snprintf(online_path, sizeof(online_path), "%s/%s/online", POWER_SUPPLY_PATH, entry->d_name);
                
                FILE *of = fopen(online_path, "r");
                if (of) {
                    int online = 0;
                    if (fscanf(of, "%d", &online) == 1 && online) {
                        fclose(of);
                        closedir(dir);
                        return true;
                    }
                    fclose(of);
                }
            }
        }
        fclose(f);
    }
    
    closedir(dir);
    return false;
}

#endif // HAL_NO_BATTERY
