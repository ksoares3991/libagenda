#ifndef AGENDA_ERROR_H
#define AGENDA_ERROR_H

/* Nos headers, os nomes abreviados de erros se referem aos códigos deste enum. */
typedef enum {
    AGENDA_OK = 0,
    AGENDA_ERR_ARGUMENT = 1,
    AGENDA_ERR_FORMAT = 2,
    AGENDA_ERR_FULL = 3,
    AGENDA_ERR_BUFFER_TOO_SMALL = 4,
    AGENDA_ERR_RANGE = 5,
    AGENDA_ERR_NO_MEMORY = 6
} agenda_status;

#endif
