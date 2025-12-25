/* https://github.com/takeiteasy/hal
   Dummy shell implementation for unsupported platforms */

#ifndef HAL_NO_SHELL
#include "hal/shell.h"

int hal_shell(const char *cmd, hal_shell_io_t *io) {
    (void)cmd;
    (void)io;
    return HAL_SHELL_ERR_GENERIC;
}

int hal_shell_fmt(hal_shell_io_t *io, const char *fmt, ...) {
    (void)io;
    (void)fmt;
    return HAL_SHELL_ERR_GENERIC;
}

#endif /* HAL_NO_SHELL */
