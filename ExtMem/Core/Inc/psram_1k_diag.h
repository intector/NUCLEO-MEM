/**
  ****************************************************************************************
 * @file           : psram_1k_diag.h
 * @brief          : 1 KB write/read diagnostic for APS6408L PSRAM
 *
 * Writes 1024 bytes of known data, reads back, captures every mismatch
 * with byte-level detail for debugger inspection.
 *
 * Usage: call PSRAM_1K_Diag() after PSRAM_EnableMemMapped()
 *        inspect psram_1k_result in debugger at the __BKPT
 *
 * Session 16: added to analyze bit-level corruption patterns
  ****************************************************************************************
 */

#ifndef __PSRAM_1K_DIAG_H
#define __PSRAM_1K_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "psram.h"

#define DIAG_1K_SIZE        1024U
#define DIAG_1K_MAX_ERRORS  64U  // capture first 64 mismatches

typedef struct
{
    uint32_t offset;    // byte offset within the 1 KB block
    uint8_t  expected;
    uint8_t  actual;
    uint8_t  xor_bits;  // expected ^ actual (shows which bits flipped)
} PSRAM_1K_Error_t;

typedef struct
{
    uint32_t total_errors;                    // total mismatched bytes in 1 KB
    uint32_t error_count;                     // entries captured (capped at MAX_ERRORS)
    PSRAM_1K_Error_t errors[DIAG_1K_MAX_ERRORS];

    // summary: OR of all xor_bits across all errors (shows all affected bits)
    uint8_t  all_flipped_bits;

    // summary: which byte-within-word positions had errors (bit mask, 0..3)
    uint8_t  word_pos_mask;

    // the pattern used for this run
    uint8_t  pattern_id; // 0 = counting, 1 = 0x55, 2 = 0xAA, 3 = walking-1
} PSRAM_1K_Result_t;

extern volatile PSRAM_1K_Result_t psram_1k_result;

void PSRAM_1K_Diag(void);

#ifdef __cplusplus
}
#endif

#endif // __PSRAM_1K_DIAG_H
