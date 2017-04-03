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

/** @brief Function for copying data out of the library. Uses copy_to_user
 * if KERNEL_MODE is defined.
 * @param[out] dest Pointer to output data.
 * @param[in] source Pointer to input data.
 * @param[in] size Number of bytes to copy.
 * @return The number of bytes copied. */
inline static unsigned int copy_in(char *dest, char const *source, unsigned int size)
{
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

/** @brief Function for copying data out of the library. Uses copy_from_user
 * if KERNEL_MODE is defined.
 * @param[out] dest Pointer to output data.
 * @param[in] source Pointer to input data.
 * @param[in] size Number of bytes to copy.
 * @return The number of bytes copied. */
inline static unsigned int copy_out(char *dest, char const *source, unsigned int size)
{
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

/** @detail This function doesn't actually take any action other than
 * configuring the initial contents of an rkt_buf instance. The actual storage
 * buffer must be allocated elsewhere and supplied to this function. */
void rkt_buf_init(rkt_buf * ptr, char * buffer, unsigned int size)
{
    ptr->top = buffer;
    ptr->read_ptr = buffer;
    ptr->write_ptr = buffer;
    ptr->end = buffer + size;
}

/** @detail This function's output should be considered approximate in situations
 * where reads and writes are happening asynchronously to each other and to this
 * function. The read and write pointers are copied at the beginning of the function,
 * so the actual values could theoretically change in a different context while this
 * function is executing. Consider guarding it with atomics. */
unsigned int rkt_buf_level(rkt_buf *ptr)
{
    uintptr_t read_ptr = (uintptr_t) ptr->read_ptr;
    uintptr_t write_ptr = (uintptr_t) ptr->write_ptr;
    unsigned int level;

    if(write_ptr >= read_ptr) {
        level = (write_ptr - read_ptr);
    } else {
        level = ((write_ptr - (uintptr_t)ptr->top) + (uintptr_t)ptr->end) - read_ptr;
    }
    return level;
}

/** @detail This function uses copy_out() to move data, which allows it to be tested
 * in user-mode before taking it into kernel space. The function doesn't currently
 * do any underflow-checking, so that is left as an exercise to the caller. */
int rkt_buf_read(rkt_buf *ptr, char *target, unsigned int count)
{
    unsigned int remaining = count;
    unsigned int chunk;
    unsigned int buf_remaining;
    unsigned int transferred;

    while(remaining != 0) {
        buf_remaining = ptr->end - ptr->read_ptr;
        chunk = (buf_remaining <= remaining) ? buf_remaining : remaining;
        transferred = copy_out(target, ptr->read_ptr, chunk);

        remaining -= transferred;
        target += transferred;
        ptr->read_ptr += transferred;

        if(ptr->read_ptr >= ptr->end) {
            ptr->read_ptr = ptr->top;
        }
    }

    return 0;
}

/** @detail This function uses copy_in() to move data, which allows it to be tested
 * in user-mode before taking it into kernel space. The function doesn't currently
 * do any overflow-checking, so that is left as an exercise to the caller. */
int rkt_buf_write(rkt_buf *ptr, char const * source, unsigned int count)
{
    unsigned int remaining = count;
    unsigned int chunk;
    unsigned int buf_remaining;
    unsigned int transferred;

    while(remaining != 0) {
        buf_remaining = ptr->end - ptr->write_ptr;
        chunk = (buf_remaining < remaining) ? buf_remaining : remaining;
        transferred = copy_in(ptr->write_ptr, source, chunk);

        remaining -= transferred;
        source += transferred;
        ptr->write_ptr += transferred;

        if(ptr->write_ptr >= ptr->end) {
            ptr->write_ptr = ptr->top;
        }
    }

    return 0;
}
