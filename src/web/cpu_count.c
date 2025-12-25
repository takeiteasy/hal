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

// Web/Emscripten CPU count using navigator.hardwareConcurrency

#ifndef HAL_NO_CPU_COUNT
#include "hal/cpu_count.h"
#include <emscripten.h>

EM_JS(int, js_hardware_concurrency, (), {
    return navigator.hardwareConcurrency || 1;
});

bool hal_cpu_count_available(void) {
    return true;
}

int hal_cpu_count_logical(void) {
    return js_hardware_concurrency();
}

int hal_cpu_count_physical(void) {
    // Web only provides logical cores
    return js_hardware_concurrency();
}
#endif // HAL_NO_CPU_COUNT
