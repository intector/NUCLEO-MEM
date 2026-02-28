/**
  ****************************************************************************************
 * @file           : psram_1k_diag.c
 * @brief          : 1 KB write/read diagnostic for APS6408L PSRAM
 *
 * Session 16: analyze bit-level corruption patterns across a 1 KB block.
 * Uses non-cached DMA region so every access hits the PSRAM bus.
  ****************************************************************************************
 */

#include "psram_1k_diag.h"
#include <string.h>

volatile PSRAM_1K_Result_t psram_1k_result;

void PSRAM_1K_Diag(void)
{
    volatile uint8_t *base = (volatile uint8_t *)(PSRAM_DMA_BASE + 0x1000);

    memset((void *)&psram_1k_result, 0, sizeof(psram_1k_result));

    // Pattern 0: counting -- each byte is unique (offset & 0xFF)
    // This lets us distinguish position-dependent vs pattern-dependent corruption
    psram_1k_result.pattern_id = 0;

    // Write phase
    for (uint32_t i = 0; i < DIAG_1K_SIZE; i++)
        base[i] = (uint8_t)(i & 0xFFU);

    __DSB();

    // Read-back phase
    for (uint32_t i = 0; i < DIAG_1K_SIZE; i++)
    {
        uint8_t expected = (uint8_t)(i & 0xFFU);
        uint8_t actual   = base[i];

        if (actual != expected)
        {
            uint8_t xor_bits = expected ^ actual;

            psram_1k_result.total_errors++;
            psram_1k_result.all_flipped_bits |= xor_bits;
            psram_1k_result.word_pos_mask    |= (1U << (i & 3U));

            if (psram_1k_result.error_count < DIAG_1K_MAX_ERRORS)
            {
                PSRAM_1K_Error_t *e = (PSRAM_1K_Error_t *)&psram_1k_result.errors[psram_1k_result.error_count];
                e->offset   = i;
                e->expected = expected;
                e->actual   = actual;
                e->xor_bits = xor_bits;
                psram_1k_result.error_count++;
            }
        }
    }

    // __BKPT(0); // inspect psram_1k_result in debugger
}
