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

// Common POSIX CPU count implementation using sysconf()
// Used by macOS, Linux, iOS, Android (NDK)

#ifndef HAL_NO_CPU_COUNT
#include "hal/cpu_count.h"
#include <unistd.h>

bool hal_cpu_count_available(void) {
    return true;
}

int hal_cpu_count_logical(void) {
    long count = sysconf(_SC_NPROCESSORS_ONLN);
    if (count < 1)
        count = sysconf(_SC_NPROCESSORS_CONF);
    return (count > 0) ? (int)count : -1;
}

int hal_cpu_count_physical(void) {
    // POSIX doesn't have a portable way to get physical core count
    // Just return logical count as fallback
    return hal_cpu_count_logical();
}
#endif // HAL_NO_CPU_COUNT
