#ifndef AGENDA_ERROR_H
#define AGENDA_ERROR_H

typedef enum {
	AGENDA_OK = 0, /* success */
	AGENDA_ERR_ARGUMENT = 1, /* invalid argument: NULL pointer, value out of bounds, or inconsistent interval */
	AGENDA_ERR_FORMAT = 2, /* text is not in the expected HH:MM format */
	AGENDA_ERR_FULL = 3, /* container has not capacity left */
	AGENDA_ERR_BUFFER_TOO_SMALL = 4, /* caller-provided buffer cannot hold the result */
	AGENDA_ERR_RANGE = 5, /* index is outside thealid range */
	AGENDA_ERR_NO_MEMORY = 6 /* memory allocation failed */
} agenda_status;

#endif
