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

/* macOS CPU count using sysctlbyname */

#ifndef HAL_NO_CPU_COUNT
#include "hal/cpu_count.h"
#include <sys/types.h>
#include <sys/sysctl.h>

bool hal_cpu_count_available(void) {
    return true;
}

static int macos_cpu_count(const char *type) {
    int num_cores;
    size_t len = sizeof(num_cores);
    return sysctlbyname(type, &num_cores, &len, NULL, 0) == 0 ? num_cores : -1;
}

int hal_cpu_count_logical(void) {
    return macos_cpu_count("hw.logicalcpu");
}

int hal_cpu_count_physical(void) {
    return macos_cpu_count("hw.physicalcpu");
}
#endif // HAL_NO_CPU_COUNT
