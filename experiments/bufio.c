#include "bufio.h"
#include <sys/types.h>
#include <stdio.h>
#include <helpers.h>

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
// Reads a line from buffer. It uses in simplesh for commands parsing
ssize_t buf_readline(char* dst, fd_t fd, struct buf_t * buf, size_t limit) {
    //try to fill the buffer if it's empty
     if (buf-> size == 0) {
         int c = read_until(fd, buf->data, buf->capacity, '\n');
         if (c == -1) {
            return -1;
         }
         buf->size = c;
         if (c == 0) {
             return 0;
         }
     }
    
    //write to dst until '\n' or 0
     int pos = 0;
     int str_found = 0;
     while (!str_found && pos < buf->size && limit > 0) {
        dst[0] = ((char*)buf->data)[pos];
        if (dst[0] == '\n' || dst[0] == 0 ) {
            str_found = 1;
        }
        dst++;
        pos++;
        limit--;
     }

    //move the last part of the buffer to the start
     memmove(buf->data, buf->data + pos, buf->size - pos);
     buf->size -= pos;
     //if buffer is empty and '\n' wasn't found - try to fill buffer again
     if (!str_found && limit > 0) {
        int c =  buf_readline(dst, fd, buf, limit);
        if (c == -1) {
            return -1;
        } else {
            return pos + c;
        }
     } else {
         return pos;
     }
     
}

ssize_t buf_fill(fd_t fd, struct buf_t * buf, size_t required) {
	DEBUG_ASSERT(BUFF != NULL && required > buf->capacity);
    
    while (buf->size < required) {
        ssize_t rres = read(fd, buf->data + buf->size, buf->capacity - buf->size);
        if (rres < 0) {
            return -1;
        } 
        if (rres == 0) {
            return buf->size;
        }
        buf->size += rres;
    }

    return buf->size;
}

ssize_t buf_flush(fd_t fd, struct buf_t * buf, size_t required) {
	DEBUG_ASSERT(buf != NULL);

	size_t offset = 0;

    while (buf->size > 0 && offset < required) {
        ssize_t wres = write(fd, buf->data + offset, buf->size - offset);
        if (wres < 0) {
            memmove(buf->data, buf->data + offset, buf->size - offset);
            buf->size -= offset;
            return -1;
        }
        offset += wres;
    }
    memmove(buf->data, buf->data + offset, buf->size - offset);
    buf->size -= offset;
    return offset;
}
