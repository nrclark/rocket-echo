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

/** @brief Rocket-echo buffer storage type. */
typedef struct rkt_buf {
    char * top; /**< top of the storage buffer. Generally constant. */
    char * end; /**< End of the storage buffer. Generally constant. */
    char * read_ptr; /**< Current read pointer. Changes on reads. */
    char * write_ptr; /**< Current write pointer. Changes on writes. */
} rkt_buf;

/** @brief Initializer for a rkt_buf data structure.
 * @param[in,out] ptr Pointer to rkt_buf instance to initialize.
 * @param[in] buffer Pointer to memory that the rkt_buf will use for storage.
 * @param[in] size Size (in bytes) of the supplied buffer. */

void rkt_buf_init(rkt_buf * ptr, char * buffer, unsigned int size);

/** @brief Returns the current level of the rkt_buf instance.
 * @param[in] ptr Pointer to target rkt_buf instance.
 * @param[in] size Size (in bytes) of the supplied buffer.
 * @returns Number of bytes waiting to be read back. */

unsigned int rkt_buf_level(rkt_buf *ptr);

/** @brief Reads data from a rkt_buf instance into an output buffer.
 * @param[in] ptr Pointer to a rkt_buf instance with data in it.
 * @param[out] target Pointer to output memory.
 * @param[in] count Number of bytes to read out from the rkt_buf.
 * @returns Status of command.
 * @retval 0 Exit success (currently the only implemented return code) */

int rkt_buf_read(rkt_buf *ptr, char *target, unsigned int count);

/** @brief Reads data from a rkt_buf instance into an output buffer.
 * @param[in] ptr Pointer to a rkt_buf instance with data in it.
 * @param[out] target Pointer to output memory.
 * @param[in] count Number of bytes to read out from the rkt_buf.
 * @returns Status of command.
 * @retval 0 Exit success (currently the only implemented return code) */

int rkt_buf_write(rkt_buf *ptr, char const * source, unsigned int size);

#endif
