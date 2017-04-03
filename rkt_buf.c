/**
 * @file rkt_buf.c
 * @author Nick Clark
 * @date 03 April 2017
 * @brief Ring-buffer implementation for Rocket-echo kernel module.
 *
 * This file contains the ring-buffer implementation used in Rocket-echo.
 * It can be compiled/tested as a standalone module (using memcpy for
 * moving data) or as part of a kernel module (using copy_to_user/copy_from_user
 * for moving data). */

#ifdef KERNEL_MODE
#include <linux/string.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#else
#include <stdint.h>
#include <string.h>
#endif

#include "_rkt_buf.h"

inline static int copy_in(char *dest, char *source, unsigned int size) {
#ifdef KERNEL
    int errcount;
    errcount = copy_to_user(dest, source, size);
    if(errcount != 0) {
        printk(KERN_ALERT "Rocket echo: copy_to_user error!\n");
    }
    return (size - errcount);
#else
    memcpy(dest, source, size);
    return size;
#endif
}

inline static int copy_out(char *dest, char *source, unsigned int size) {
#ifdef KERNEL
    int errcount;
    errcount = copy_from_user(dest, source, size);
    if(errcount != 0) {
        printk(KERN_ALERT "Rocket echo: copy_from_user error!\n");
    }
    return (size - errcount);
#else
    memcpy(dest, source, size);
    return size;
#endif
}

void rkt_buf_init(rkt_buf * ptr, char * buffer, unsigned int size) {
    ptr->top = buffer;
    ptr->read_ptr = buffer;
    ptr->write_ptr = buffer;
    ptr->end = buffer + size;
}

unsigned int rkt_buf_level(rkt_buf *ptr) {
    uintptr_t read_ptr = (uintptr_t) ptr->read_ptr;
    uintptr_t write_ptr = (uintptr_t) ptr->write_ptr;
    unsigned int level;

    if(write_ptr >= read_ptr) {
        level = (write_ptr - read_ptr);
    }
    else {
        level = ((write_ptr - (uintptr_t)ptr->top) + (uintptr_t)ptr->end) - read_ptr;
    }
    return level;
}

int rkt_buf_read(rkt_buf *ptr, char *out, unsigned int size) {
    unsigned int remaining = size;
    unsigned int chunk;
    unsigned int buf_remaining;
    unsigned int transferred;

    while(remaining != 0) {
        buf_remaining = ptr->end - ptr->read_ptr;
        chunk = (buf_remaining <= remaining) ? buf_remaining : remaining;
        transferred = copy_out(out, ptr->read_ptr, chunk);

        remaining -= transferred;
        out += transferred;
        ptr->read_ptr += transferred;

        if(ptr->read_ptr >= ptr->end) {
            ptr->read_ptr = ptr->top;
        }
    }
    
    return 0;
}

int rkt_buf_write(rkt_buf *ptr, char *in, unsigned int size) {
    unsigned int remaining = size;
    unsigned int chunk;
    unsigned int buf_remaining;
    unsigned int transferred;

    while(remaining != 0) {
        buf_remaining = ptr->end - ptr->write_ptr;
        chunk = (buf_remaining < remaining) ? buf_remaining : remaining;
        transferred = copy_in(ptr->write_ptr, in, chunk);

        remaining -= transferred;
        in += transferred;
        ptr->write_ptr += transferred;

        if(ptr->write_ptr >= ptr->end) {
            ptr->write_ptr = ptr->top;
        }
    }
    
    return 0;
}
