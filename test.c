#include <stdio.h>
#include "bit_queue.h"

int main()
{
    bit_queue_t * bq1, * bq2;
    uint16_t buffer = 0xaaaa;
    uint8_t a = 0xa;
    uint16_t res;
    bq1 = bit_queue_init((uint8_t*)&buffer, 2, false);
    bq2 = bit_queue_base_init(2);
    bit_queue_write_bits(bq2, (uint8_t*)&buffer, 2, 16);
    bit_queue_read_bits(bq1, (uint8_t*)&res, 2, 8);
    printf("m1 = %d\n", res);
    res = 0;
    buffer = 0;
    bit_queue_write_bits(bq1, (uint8_t*)&a, 1, 8);
    bit_queue_read_bits(bq2, (uint8_t*)&res, 2, 5);
    printf("m2 = %d\n", res);
    res = 0;
    bit_queue_read_bits(bq2, (uint8_t*)&res, 2, 1);
    printf("m3 = %d\n", res);
    bit_queue_destroy(bq1);
    bit_queue_destroy(bq2);
    return 0;
}