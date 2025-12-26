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

#ifndef HAL_EMAIL_HEAD
#define HAL_EMAIL_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_EMAIL
#include "hal.h"

/*!
 @function hal_email_available
 @return Returns true if email functionality is available
 @brief Check if email client can be opened
*/
bool hal_email_available(void);
/*!
 @function hal_email_send
 @param recipient Email recipient address (may be NULL)
 @param subject Email subject (may be NULL)
 @param body Email body text (may be NULL)
 @param cc Carbon copy recipients, comma-separated (may be NULL)
 @param bcc Blind carbon copy recipients, comma-separated (may be NULL)
 @return Returns true if email client was opened successfully
 @brief Open email client with pre-filled fields
*/
bool hal_email_send(const char *recipient, const char *subject, 
                    const char *body, const char *cc, const char *bcc);

#ifdef __cplusplus
}
#endif
#endif // HAL_EMAIL_HEAD
