#include "bufio.h"
#include <sys/types.h>

#ifdef  DEBUG
#define DEBUG_ASSERT(cond) if(!(cond)) abort();
#else
#define DEBUG_ASSERT(cound)
#endif

struct buf_t * buf_new(size_t capacity) {
	struct buf_t * buf = (struct buf_t *) malloc(sizeof(struct buf_t));
	if (buf == NULL) {
		return NULL;
	} else {
		buf->data = malloc(capacity);
		if (buf->data == NULL) {
			free(buf);
			return NULL; 
		} else {
            buf->capacity = capacity;
            buf->size = 0;
            return buf;
		}
	}
}


void buf_free(struct buf_t *buf) {
	DEBUG_ASSERT(buf != NULL);
	free(buf->data);
    free(buf);
}

size_t buf_capacity(struct buf_t *buf) {
    DEBUG_ASSERT(buf != NULL);
    return buf->capacity;
}

size_t buf_size(struct buf_t *buf) {
    DEBUG_ASSERT(buf != NULL);
    return buf->size;
}


ssize_t buf_fill(fd_t fd, struct buf_t * buf, size_t required) {
	DEBUG_ASSERT(BUFF != NULL && required > buf->capacity);

	buf->size = 0;
    size_t read_counter;

    do {
        read_counter = read(fd, buf->data + buf->size, buf->capacity - buf->size);

        if (read_counter == -1) {
            return -1;
        }

        buf->size += read_counter;
    } while (buf->size < required && read_counter > 0);

    return buf->size;
}

ssize_t buf_flush(fd_t fd, struct buf_t * buf, size_t required) {
	DEBUG_ASSERT(buf != NULL);

	size_t offset = 0;
    size_t write_counter;
    int error = 0;

    if (required > buf->size) {
        required = buf->size;
    }

    do {
        write_counter = write(fd, buf->data + offset, buf->size);

        if (write_counter == -1) {
            error = 1;
            break;
        }

        offset += write_counter;
        buf->size -= write_counter;
    } while (offset < required && write_counter > 0);

    memcpy(buf->data, buf->data + offset, buf->size);

    if (error) {
        return -1;
    } else {
        return buf->size;
    }
}

