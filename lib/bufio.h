#ifndef BUFIO_H
#define BUFIO_H


#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef int fd_t;

struct buf_t {
	void * data;
 	size_t size;
	size_t capacity;
};

struct buf_t *buf_new(size_t capacity); //— конструктор пустого буфера, возвращает NULL если не удалось сделать malloc,
void buf_free(struct buf_t * buf); // — деструктор.
size_t buf_capacity(struct buf_t * buf);// — возвращает максимальный возможный размер,
size_t buf_size(struct buf_t * buf); //— возвращает текущую заполненность,
ssize_t buf_fill(fd_t fd, struct buf_t *buf, size_t required); // — заполняет буфер readами до тех пор пока его size не станет как минимум required байт
ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required); // - выписывает данные из буфера до тех пор, пока не будет записано как минимум required байт (больше — так больше) или буфер не опустеет.

#endif // BUFIO_H

