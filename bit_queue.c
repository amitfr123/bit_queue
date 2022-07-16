/**
 * @file bit_queue.c
 * @author amitfr1
 * @brief This is an adt for bit queuing
 * @version 0.1
 * @date 2021-12-11
 * 
 * @ingroup bit_queue
 * 
 */
#include <stdlib.h>
#include <errno.h>
#include "bit_queue.h"

/**
 * @brief The number of bits in a byte
 * @ingroup bit_queue
 */
#define BITS_IN_BYTE 8

/**
 * @brief This is the mask of a byte
 * @ingroup bit_queue
 */
#define BYTE_MASK 0x000000ff

/**
 * @brief This define calculates the mask its shifted the the end of the byte
 * @ingroup bit_queue
 */ 
#define CREATE_BYTE_MASK(bit_offset) ((BYTE_MASK << bit_offset) & BYTE_MASK)

/**
 * @brief This define calculates the mask and it starts from the LSB
 * @ingroup bit_queue
 */
#define CREATE_BYTE_MASK_LSB(bit_offset) (CREATE_BYTE_MASK(bit_offset) >> bit_offset)

/**
 * @brief This stuct holds all the fields used in the bit queue
 * 
 * @ingroup bit_queue
 */
struct _bit_queue_t
{
    uint8_t * buffer; /// The buffer that holds all of the data
    uint8_t r_bit_offset; /// An index used to follow the bit progression in a byte while reading
    size_t r_byte_offset; /// An index used to follow byte progression while reading
    uint8_t w_bit_offset; /// An index used to follow the bit progression in a byte while writing
    size_t w_byte_offset; /// An index used to follow byte progression while writing
    size_t written_bits; /// The number of bits that hold data in the buffer
    size_t buffer_size; /// The buffer size in bits
    bool free_buff; /// An indication that the buffer should be freed with the bit queue
};

/**
 * @brief This function copies bits from source buffer to the destination buffer
 * The function will copy as mush as it can but it doesn't promise to copy all of the bits
 * When using it you should check how many bits where actually written
 * 
 * errno options:
 * 1) Sets errno EINVAL if buffer = NULL or bq = NULL or bq->buffer = NULL or if the given offsets and sizes are invalid or contridicting
 * 2) Sets errno to EMSGSIZE if the dst_buff is to small to store the requested bits
 * 
 * @ingroup bit_queue
 * 
 * @param dst_buff The destination buffer
 * @param src_buff The source buffer
 * @param dst_byte_offset The dst_buff byte offset
 * @param dst_bit_offset The dst_buff bit offset
 * @param dst_buff_size The dst_buff size
 * @param src_byte_offset The dst_buff byte offset
 * @param src_bit_offset The dst_buff bit offset
 * @param src_buff_size The src_buff size
 * @param bit_count The number of bits that we want to copy
 * @return int The number of bits copied or or -1 in failure
 */
static int bit_queue_bit_buffer_copy(uint8_t * dst_buff, uint8_t * src_buff, size_t dst_byte_offset, uint8_t dst_bit_offset, size_t dst_buff_size, size_t src_byte_offset, size_t src_bit_offset, size_t src_buff_size, size_t bit_count);

/**
 * @brief This function checks if there is enough space to write all of the bits
 * 
 * Sets errno to EINVAL if bq = NULL
 * 
 * @ingroup bit_queue
 * 
 * @param bq The bit queue
 * @param bit_count The number of bits we want to write
 * @return true if there is sufficient space in the queue false otherwise 
 */
static bool bit_queue_has_space(bit_queue_t *bq, size_t bit_count);

/**
 * @brief This function checks if there is enough data to read
 * 
 * Sets errno to EINVAL if bq = NULL
 * 
 * @ingroup bit_queue
 * 
 * @param bq The bit queue
 * @param bit_count The number of bits we want to read
 * @return true if there is sufficient data in the queue false otherwise 
 */
static bool bit_queue_has_data(bit_queue_t *bq, size_t bit_count);

bit_queue_t * bit_queue_base_init(size_t byte_count)
{
    bit_queue_t * bq = NULL;
    if (!byte_count)
    {
        errno = EINVAL;
    }
    else if (!(bq = calloc(1, sizeof(struct _bit_queue_t))))
    {
        // errno is set by calloc and bq = NULL
    }
    else if (!(bq->buffer = calloc(byte_count, sizeof(uint8_t))))
    {
        // errno is set by calloc and bq->buffer = NULL
        free(bq);
        bq = NULL;
    }
    else
    {
        bq->buffer_size = byte_count;
        bq->written_bits = 0;
        bq->free_buff = true;
    }
    return bq;
}

bit_queue_t * bit_queue_init(uint8_t * buffer, size_t byte_count, bool free_buff)
{
    bit_queue_t * bq = NULL;
    if (!byte_count || buffer == NULL)
    {
        errno = EINVAL;
    }
    else if (!(bq = calloc(1, sizeof(struct _bit_queue_t))))
    {
        // errno is set by calloc and bq = NULL
    }
    else
    {
        bq->buffer = buffer;
        bq->buffer_size = byte_count;
        bq->written_bits = byte_count * BITS_IN_BYTE;
        bq->free_buff = free_buff;
    }
    return bq;
}

int bit_queue_read_bits(bit_queue_t *bq, uint8_t *buffer, size_t buffer_size, size_t bit_count)
{
    int ret_val = -1;
    size_t b_byte_offset;
    uint8_t b_bit_offset;
    size_t r_bits;
    if (bq == NULL || buffer == NULL || bit_count == 0 || buffer_size * BITS_IN_BYTE < bit_count)
    {
        // ret_val already set
        errno = EINVAL;
    }
    else if (bq->buffer == NULL)
    {
        // this means that bq is invalid
        // ret_val already set
        errno = EINVAL;
    }
    else if (bit_count > bq->buffer_size * BITS_IN_BYTE)
    {
        // ret_val already set
        errno = EMSGSIZE;
    }
    else if (!bit_queue_has_data(bq, bit_count))
    {
        // ret_val already set
        errno = EAGAIN;
    }
    else
    {
        r_bits = bit_count;
        b_bit_offset = 0;
        b_byte_offset = 0;
        do
        {
            ret_val = bit_queue_bit_buffer_copy(buffer, bq->buffer, b_byte_offset, b_bit_offset, buffer_size, bq->r_byte_offset, bq->r_bit_offset, bq->buffer_size, r_bits);
            if (ret_val == -1)
            {
                break;
            }
            // update the buffer counters
            b_bit_offset += ret_val;
            b_byte_offset += b_bit_offset / BITS_IN_BYTE;
            b_bit_offset = b_bit_offset % BITS_IN_BYTE;

            // update the bit queue buffer counters
            bq->r_bit_offset += ret_val;
            bq->r_byte_offset += bq->r_bit_offset / BITS_IN_BYTE;
            bq->r_bit_offset = bq->r_bit_offset % BITS_IN_BYTE;
            if (bq->r_byte_offset == bq->buffer_size)
            {
                bq->r_byte_offset = 0;
                // bq->bit_offset should already be 0
                bq->r_bit_offset = 0;
            }
            bq->written_bits -= ret_val;
            r_bits -= ret_val;
        } while (r_bits > 0);
        if (ret_val != -1)
        {
            ret_val = bit_count;
        }
    }
    return ret_val;
}

int bit_queue_write_bits(bit_queue_t *bq, uint8_t *buffer, size_t buffer_size, size_t bit_count)
{
    int ret_val = -1;
    size_t b_byte_offset;
    uint8_t b_bit_offset;
    size_t r_bits;
    if (bq == NULL || buffer == NULL || bit_count == 0 || buffer_size * BITS_IN_BYTE < bit_count)
    {
        // ret_val already set
        errno = EINVAL;
    }
    else if (bq->buffer == NULL)
    {
        // this means that bq is invalid
        // ret_val already set
        errno = EINVAL;
    }
    else if (bit_count > bq->buffer_size * BITS_IN_BYTE)
    {
        // ret_val already set
        errno = EMSGSIZE;
    }
    else if (!bit_queue_has_space(bq, bit_count))
    {
        // ret_val already set
        errno = EAGAIN;
    }
    else
    {
        r_bits = bit_count;
        b_bit_offset = 0;
        b_byte_offset = 0;
        do
        {
            ret_val = bit_queue_bit_buffer_copy(bq->buffer, buffer, bq->w_byte_offset, bq->w_bit_offset, bq->buffer_size, b_byte_offset, b_bit_offset, buffer_size, r_bits);
            if (ret_val == -1)
            {
                break;
            }
            // update the buffer counters
            b_bit_offset += ret_val;
            b_byte_offset += b_bit_offset / BITS_IN_BYTE;
            b_bit_offset = b_bit_offset % BITS_IN_BYTE;

            // update the bit queue buffer counters
            bq->w_bit_offset += ret_val;
            bq->w_byte_offset += bq->w_bit_offset / BITS_IN_BYTE;
            bq->w_bit_offset = bq->w_bit_offset % BITS_IN_BYTE;
            if (bq->w_byte_offset == bq->buffer_size)
            {
                bq->w_byte_offset = 0;
                // bq->bit_offset should already be 0
                bq->w_byte_offset = 0;
            }
            bq->written_bits += ret_val;
            r_bits -= ret_val;
        } while (r_bits > 0);
        if (ret_val != -1)
        {
            ret_val = bit_count;
        }
    }
    return ret_val;
}

int bit_queue_destroy(bit_queue_t *bq)
{
    int ret_val = -1;
    if (bq == NULL)
    {
        errno = EINVAL;
    }
    else if (bq->buffer == NULL)
    {
        errno = EINVAL;
    }
    else
    {
        if (bq->free_buff)
        {
            free(bq->buffer);
        }
        bq->buffer = NULL;
        free(bq);
        ret_val = 0;
    }
    return ret_val;
}

// static functions

static int bit_queue_bit_buffer_copy(uint8_t * dst_buff, uint8_t * src_buff, size_t dst_byte_offset, uint8_t dst_bit_offset, size_t dst_buff_size, size_t src_byte_offset, size_t src_bit_offset, size_t src_buff_size, size_t bit_count)
{
    int ret_val = -1;
    size_t offset_bit_count;
    size_t r_bits;
    if (dst_buff == NULL || src_buff == NULL)
    {
        // ret_val already set
        errno = EINVAL;
    }
    // checking that the size values are valid
    else if (bit_count == 0 || dst_buff_size < dst_byte_offset || src_buff_size < src_byte_offset || dst_bit_offset >= BITS_IN_BYTE || src_bit_offset >= BITS_IN_BYTE || src_buff_size * BITS_IN_BYTE < bit_count)
    {
        // ret_val already set
        errno = EINVAL;
    }
    else if ((dst_buff_size - dst_byte_offset) * BITS_IN_BYTE - dst_bit_offset < bit_count)
    {
        // ret_val already set
        errno = EMSGSIZE;
    }
    else
    {
        if (bit_count > (src_buff_size - src_byte_offset) * BITS_IN_BYTE - src_bit_offset)
        {
            // we cant copy all the bits requested
            bit_count = (src_buff_size - src_byte_offset) * BITS_IN_BYTE - src_bit_offset;
        }
        // set our target bit count
        r_bits = bit_count;
        do
        {
            if (r_bits + dst_bit_offset <= BITS_IN_BYTE && r_bits <= BITS_IN_BYTE - src_bit_offset)
            {
                // we can read all the bits without needing to worry about crossing bytes
                offset_bit_count = BITS_IN_BYTE - r_bits;
            }
            // will we cross the buffer 
            else if (dst_bit_offset >= src_bit_offset)
            {
                // we will cross the buffer byte in the next copy
                offset_bit_count = dst_bit_offset;
            }
            else // dst_bit_offset < src_bit_offset
            {
                // we will cross the bit queue buffer byte in the next copy
                offset_bit_count = src_bit_offset;
            }
            // copy all the bits we can into the buffer
            // because the buffer and the bq buffer can be on diffrent bit offsets we need to shift it back and forth
            dst_buff[dst_byte_offset] |= ((src_buff[src_byte_offset] & (CREATE_BYTE_MASK_LSB(offset_bit_count) << src_bit_offset)) >> src_bit_offset) << dst_bit_offset;

            // update the bit counters
            src_bit_offset += (BITS_IN_BYTE - offset_bit_count);
            dst_bit_offset += (BITS_IN_BYTE - offset_bit_count);
            r_bits -= (BITS_IN_BYTE - offset_bit_count);

            // check if we are crossing bytes
            if (src_bit_offset == BITS_IN_BYTE)
            {
                src_bit_offset = 0;
                src_byte_offset++;
            }
            if (dst_bit_offset == BITS_IN_BYTE)
            {
                dst_bit_offset = 0;
                dst_byte_offset++;
            }
        } while (r_bits > 0);
        // update the bit queue and the retval
        ret_val = bit_count;
    }
    return ret_val;
}


static bool bit_queue_has_space(bit_queue_t *bq, size_t bit_count)
{
    bool ret_val = false;
    if (bq == NULL)
    {
        errno = EINVAL;
    }
    else if ((bq->buffer_size * BITS_IN_BYTE) - bq->written_bits >= bit_count)
    {
        ret_val = true;
    }
    return ret_val;
}

static bool bit_queue_has_data(bit_queue_t *bq, size_t bit_count)
{
    bool ret_val = false;
    if (bq == NULL)
    {
        errno = EINVAL;
    }
    else if (bq->written_bits >= bit_count)
    {
        ret_val = true;
    }
    return ret_val;
}