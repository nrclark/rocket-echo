/**
 * @file rkt_buf.h
 * @author Nick Clark
 * @date 03 April 2017
 * @brief Header file for Rocket-echo's ring buffer implementation.
 *
 * This file contains the typedefs and prototypes for the ring-buffer
 * used by Rocket-echo. */

#ifndef _RKT_BUF_H_
#define _RKT_BUF_H_

typedef struct rkt_buf {
    char * top;
    char * read_ptr;
    char * write_ptr;
    char * end;
} rkt_buf;

void rkt_buf_init(rkt_buf * ptr, char * buffer, unsigned int size);
unsigned int rkt_buf_level(rkt_buf *ptr);
int rkt_buf_read(rkt_buf *ptr, char *out, unsigned int size);
int rkt_buf_write(rkt_buf *ptr, char *in, unsigned int size);

#endif

