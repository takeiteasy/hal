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

// Windows CPU count using GetSystemInfo

#ifndef HAL_NO_CPU_COUNT
#include "hal/cpu_count.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool hal_cpu_count_available(void) {
    return true;
}

int hal_cpu_count_logical(void) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
}

int hal_cpu_count_physical(void) {
    // Windows doesn't have a simple API for physical cores
    // GetLogicalProcessorInformation can be used but is complex
    // Return logical count as fallback
    return hal_cpu_count_logical();
}
#endif // HAL_NO_CPU_COUNT
