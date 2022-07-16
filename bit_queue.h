/**
 * @file bit_queue.h
 * @author amitfr1
 * @brief This is an adt for bit queuing
 * @version 0.1
 * @date 2021-12-11
 * @defgroup bit_queue
 * This module was created for supporting read and write operations on a buffer with diffrent bit sizes.
 * This module supports a normal circular queue for bits and working with a given data full queue.
 */
#include <stdint.h>
#include <stdbool.h>

#ifndef BIT_QUEUE_H_
#define BIT_QUEUE_H_

typedef struct _bit_queue_t bit_queue_t;

/**
 * @brief This function allocates the bit_queue and buffer and initializes it 
 * errno options:
 * 1) Sets errno EINVAL if byte_count = 0
 * 2) The errno is set by the allocation method
 * 
 * @ingroup bit_queue
 * 
 * @param byte_count the size of the bit queue buffer in bytes
 * 
 * @return bit_queue_t* Address of the created bit queue or NULL in failure
 */
bit_queue_t * bit_queue_base_init(size_t byte_count);

/**
 * @brief This function allocates the bit_queue sets the buffer and initializes it. The function assumes that the buffer is full of data.
 * 
 * errno options:
 * 1) Sets errno EINVAL if byte_count = 0 or buffer = NULL
 * 2) The errno is set by the allocation method
 * 
 * @ingroup bit_queue
 * 
 * @param buffer The buffer to use for the queue
 * @param byte_count The size of the buffer in bytes
 * @param free_buff This flag is used to tell the bit queue if durring the destroy function it should free the buffer that was given.
 * 
 * @return bit_queue_t* 
 */
bit_queue_t * bit_queue_init(uint8_t *buffer, size_t byte_count, bool free_buff);

/**
 * @brief This function copys bits from the bit queue buffer into the buffer
 * 
 * errno options:
 * 1) Sets errno EINVAL if byte_count = 0 or buffer = NULL or bq = NULL or bq->buffer = NULL
 * 2) Sets errno to EMSGSIZE if the bit count is larger the the entire bit queue buffer 
 * 3) Sets errno to EAGAIN if there isn't enough data in the queue
 * 
 * @ingroup bit_queue
 * 
 * @param bq The source bit queue
 * @param buffer The destintion buffer
 * @param buffer_size The size of the received buffer
 * @param bit_count The amount of bits to read
 * 
 * @return int The number of bits read or -1 in failure
 */
int bit_queue_read_bits(bit_queue_t *bq, uint8_t *buffer, size_t buffer_size, size_t bit_count);

/**
 * @brief This function copys bits from the buffer into the bit queue buffer
 * 
 * errno options:
 * 1) Sets errno EINVAL if byte_count = 0 or buffer = NULL or bq = NULL or bq->buffer = NULL
 * 2) Sets errno to EMSGSIZE if the bit count is larger the the entire bit queue buffer 
 * 3) Sets errno to EAGAIN if there isn't enough data in the queue
 * 
 * @ingroup bit_queue
 * 
 * @param bq The destination bit queue
 * @param buffer The source buffer
 * @param buffer_size The size of the received buffer
 * @param bit_count The amount of bits to write
 * 
 * @return int The number of bits written or -1 in failure
 */
int bit_queue_write_bits(bit_queue_t *bq, uint8_t *buffer, size_t buffer_size, size_t bit_count);

/**
 * @brief Destroyes the bit queue and frees allocated data
 * 
 * Sets errno to EINVAL if bq = NULL or bq->buffer = NULL
 * 
 * @param bq The bit queue to destroy
 * 
 * @return int 0 in success or -1 in failure
 */
int bit_queue_destroy(bit_queue_t *bq);

#endif /// BIT_QUEUE_H_