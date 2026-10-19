#ifndef AGENDA_ERROR_H
#define AGENDA_ERROR_H

/*
 * agenda_error.h - single error type shared by every libagenda module.
 *
 * Every function that can fail returns an agenda_status. AGENDA_OK is 0, so
 * callers can write `if (rc != AGENDA_OK)`. Values are fixed and new codes must
 * only be appended at the end, because the shared library exposes them to
 * clients as part of its binary interface.
 *
 * Which function returns which code is documented in the header of each
 * module, next to the function itself.
 */
typedef enum {
    AGENDA_OK = 0,                    /* success */
    AGENDA_ERR_ARGUMENT = 1,          /* invalid argument: NULL pointer, value out of bounds, or inconsistent interval */
    AGENDA_ERR_FORMAT = 2,            /* text is not in the expected format */
    AGENDA_ERR_FULL = 3,              /* container has no capacity left */
    AGENDA_ERR_BUFFER_TOO_SMALL = 4,  /* caller-provided buffer cannot hold the result */
    AGENDA_ERR_RANGE = 5,             /* index is outside the valid range */
    AGENDA_ERR_NO_MEMORY = 6          /* memory allocation failed */
} agenda_status;

#endif /* AGENDA_ERROR_H */
