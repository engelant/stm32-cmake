#ifndef INC_RINGBUFFER_H
#define INC_RINGBUFFER_H

#include <string.h>

typedef struct ringbuffer
{
    char *data;
    const size_t size;
    size_t read;
    size_t write;
} ringbuffer;

#endif /* INC_RINGBUFFER_H */