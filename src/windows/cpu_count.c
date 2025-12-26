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

/* Windows CPU count using GetLogicalProcessorInformation
   Reference: plyer/plyer/platforms/win/cpu.py */

#ifndef HAL_NO_CPU_COUNT
#include "hal/cpu_count.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

/* LOGICAL_PROCESSOR_RELATIONSHIP enum values */
#define RelationProcessorCore    0
#define RelationProcessorPackage 3

bool hal_cpu_count_available(void) {
    return true;
}

int hal_cpu_count_logical(void) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
}

int hal_cpu_count_physical(void) {
    DWORD buffer_size = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer = NULL;
    int core_count = 0;
    
    /* First call to get required buffer size */
    GetLogicalProcessorInformation(NULL, &buffer_size);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return hal_cpu_count_logical(); /* Fallback */
    
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(buffer_size);
    if (!buffer)
        return hal_cpu_count_logical(); /* Fallback */
    
    /* Second call to get actual data */
    if (!GetLogicalProcessorInformation(buffer, &buffer_size)) {
        free(buffer);
        return hal_cpu_count_logical(); /* Fallback */
    }
    
    /* Count entries with RelationProcessorCore relationship */
    DWORD offset = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *ptr = buffer;
    while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= buffer_size) {
        if (ptr->Relationship == RelationProcessorCore)
            core_count++;
        offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }
    
    free(buffer);
    return core_count > 0 ? core_count : hal_cpu_count_logical();
}
#endif // HAL_NO_CPU_COUNT
