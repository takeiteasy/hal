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

#ifndef HAL_CPU_COUNT_HEAD
#define HAL_CPU_COUNT_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_CPU_COUNT
#include "hal.h"

/*!
 @function hal_cpu_count_available
 @return Returns true if CPU count query is available
 @brief Check if CPU count query is available
*/
bool hal_cpu_count_available(void);
/*!
 @function hal_cpu_count_logical
 @return Returns number of logical CPUs, -1 on failure
 @brief Get the number of logical CPUs (threads)
*/
int hal_cpu_count_logical(void);
/*!
 @function hal_cpu_count_physical
 @return Returns number of physical CPU cores, -1 on failure
 @brief Get the number of physical CPU cores
*/
int hal_cpu_count_physical(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_CPU_COUNT_HEAD