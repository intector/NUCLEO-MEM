/**
  ****************************************************************************************
 * @file           : psram_diag.h
 * @brief          : PSRAM bus diagnostic register dump
 *
 * Drop-in diagnostic for Session 8. Call PSRAM_DiagDump() at the BKPT
 * in PSRAM_BusVerify(), then inspect `psram_diag` in the debugger.
 *
  * Copyright (c) 2020-2026 Intector Inc.
  ****************************************************************************************
 */

#ifndef __PSRAM_DIAG_H
#define __PSRAM_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Register dump struct -- inspect in debugger as `psram_diag`
typedef struct
{
    // SYSCFG
    uint32_t PMCR;           // 0x58000404 -- need PC2SO (bit 21) and PC3SO (bit 23) = 1

    // GPIOA (PA3 = CLK, AF12)
    uint32_t GPIOA_MODER;    // 0x58020000 -- PA3[7:6] = 10 (AF)
    uint32_t GPIOA_OSPEEDR;  // 0x58020008 -- PA3[7:6] = 11 (very high)
    uint32_t GPIOA_AFRL;     // 0x58020020 -- PA3[15:12] = 0xC (AF12)

    // GPIOB (PB2 = DQS, AF10)
    uint32_t GPIOB_MODER;    // 0x58020400 -- PB2[5:4] = 10 (AF)
    uint32_t GPIOB_OSPEEDR;  // 0x58020408 -- PB2[5:4] = 11 (very high)
    uint32_t GPIOB_AFRL;     // 0x58020420 -- PB2[11:8] = 0xA (AF10)

    // GPIOC (PC2 = IO5 AF4, PC3 = IO6 AF4)
    uint32_t GPIOC_MODER;    // 0x58020800 -- PC2[5:4]=10, PC3[7:6]=10
    uint32_t GPIOC_OSPEEDR;  // 0x58020808 -- PC2[5:4]=11, PC3[7:6]=11
    uint32_t GPIOC_AFRL;     // 0x58020820 -- PC2[11:8]=4, PC3[15:12]=4

    // GPIOE (PE2=IO2 AF9, PE7=IO4 AF10, PE10=IO7 AF10, PE11=NCS AF11)
    uint32_t GPIOE_MODER;    // 0x58021000 -- already verified in S7
    uint32_t GPIOE_OSPEEDR;  // 0x58021008
    uint32_t GPIOE_AFRL;     // 0x58021020
    uint32_t GPIOE_AFRH;     // 0x58021024

    // GPIOF (PF6=IO3 AF10, PF8=IO0 AF10, PF9=IO1 AF10)
    uint32_t GPIOF_MODER;    // 0x58021400 -- PF6[13:12]=10, PF8[17:16]=10, PF9[19:18]=10
    uint32_t GPIOF_OSPEEDR;  // 0x58021408
    uint32_t GPIOF_AFRL;     // 0x58021420 -- PF6[27:24] = 0xA (AF10)
    uint32_t GPIOF_AFRH;     // 0x58021424 -- PF8[3:0]=0xA, PF9[7:4]=0xA

    // OCTOSPI1 peripheral
    uint32_t OSPI_CR;        // 0x52005000 -- control register
    uint32_t OSPI_DCR1;      // 0x52005008 -- device config 1
    uint32_t OSPI_DCR2;      // 0x5200500C -- device config 2 (prescaler)
    uint32_t OSPI_SR;        // 0x52005020 -- status register

    // OCTOSPI1 memory-mapped command registers
    uint32_t OSPI_CCR;       // 0x52005100 -- read command config
    uint32_t OSPI_TCR;       // 0x52005108 -- read timing (dummy cycles)
    uint32_t OSPI_IR;        // 0x52005110 -- read instruction (expect 0x20)
    uint32_t OSPI_WCCR;      // 0x52005180 -- write command config
    uint32_t OSPI_WTCR;      // 0x52005188 -- write timing
    uint32_t OSPI_WIR;       // 0x52005190 -- write instruction (expect 0xA0)

    // DLYB
    uint32_t DLYB_CR;        // DEN, SEN
    uint32_t DLYB_CFGR;      // SEL, UNIT, LNG

    // Decoded verdicts (filled by PSRAM_DiagCheck)
    uint8_t pa3_af_ok;       // CLK = AF12?
    uint8_t pb2_af_ok;       // DQS = AF10?
    uint8_t pc2_af_ok;       // IO5 = AF4?
    uint8_t pc3_af_ok;       // IO6 = AF4?
    uint8_t pe2_af_ok;       // IO2 = AF9?
    uint8_t pe7_af_ok;       // IO4 = AF10?
    uint8_t pe10_af_ok;      // IO7 = AF10?
    uint8_t pe11_af_ok;      // NCS = AF11?
    uint8_t pf6_af_ok;       // IO3 = AF10?
    uint8_t pf8_af_ok;       // IO0 = AF10?
    uint8_t pf9_af_ok;       // IO1 = AF10?
    uint8_t pmcr_pc2so_ok;   // PMCR bit 21 = 1?
    uint8_t pmcr_pc3so_ok;   // PMCR bit 23 = 1?
    uint8_t all_speed_ok;    // all pins VERY_HIGH?
    uint8_t all_af_ok;       // all AFs correct?
    uint8_t read_cmd_ok;     // IR contains 0x20?
    uint8_t write_cmd_ok;    // WIR contains 0xA0?
} PSRAM_Diag_t;

extern volatile PSRAM_Diag_t psram_diag;

// Call this right before __BKPT(0) in PSRAM_BusVerify()
static inline void PSRAM_DiagDump(void)
{
    volatile PSRAM_Diag_t *d = &psram_diag;

    // SYSCFG
    d->PMCR = SYSCFG->PMCR;

    // GPIOA
    d->GPIOA_MODER   = GPIOA->MODER;
    d->GPIOA_OSPEEDR = GPIOA->OSPEEDR;
    d->GPIOA_AFRL    = GPIOA->AFR[0];

    // GPIOB
    d->GPIOB_MODER   = GPIOB->MODER;
    d->GPIOB_OSPEEDR = GPIOB->OSPEEDR;
    d->GPIOB_AFRL    = GPIOB->AFR[0];

    // GPIOC
    d->GPIOC_MODER   = GPIOC->MODER;
    d->GPIOC_OSPEEDR = GPIOC->OSPEEDR;
    d->GPIOC_AFRL    = GPIOC->AFR[0];

    // GPIOE
    d->GPIOE_MODER   = GPIOE->MODER;
    d->GPIOE_OSPEEDR = GPIOE->OSPEEDR;
    d->GPIOE_AFRL    = GPIOE->AFR[0];
    d->GPIOE_AFRH    = GPIOE->AFR[1];

    // GPIOF
    d->GPIOF_MODER   = GPIOF->MODER;
    d->GPIOF_OSPEEDR = GPIOF->OSPEEDR;
    d->GPIOF_AFRL    = GPIOF->AFR[0];
    d->GPIOF_AFRH    = GPIOF->AFR[1];

    // OCTOSPI1
    d->OSPI_CR   = OCTOSPI1->CR;
    d->OSPI_DCR1 = OCTOSPI1->DCR1;
    d->OSPI_DCR2 = OCTOSPI1->DCR2;
    d->OSPI_SR   = OCTOSPI1->SR;
    d->OSPI_CCR  = OCTOSPI1->CCR;
    d->OSPI_TCR  = OCTOSPI1->TCR;
    d->OSPI_IR   = OCTOSPI1->IR;
    d->OSPI_WCCR = OCTOSPI1->WCCR;
    d->OSPI_WTCR = OCTOSPI1->WTCR;
    d->OSPI_WIR  = OCTOSPI1->WIR;

    // DLYB
    d->DLYB_CR   = DLYB_OCTOSPI1->CR;
    d->DLYB_CFGR = DLYB_OCTOSPI1->CFGR;

    // --- Decode and check ---

    // Helper: extract AF nibble for a pin
    // AFRL covers pins 0-7, AFRH covers pins 8-15
    // Each pin uses 4 bits: (AFR >> (pin%8 * 4)) & 0xF

    #define PIN_AF(afrl, afrh, pin) \
        (((pin) < 8) ? (((afrl) >> ((pin) * 4)) & 0xFU) : (((afrh) >> (((pin) - 8) * 4)) & 0xFU))

    #define PIN_MODE(moder, pin)  (((moder) >> ((pin) * 2)) & 0x3U) // 2 = AF
    #define PIN_SPEED(ospeedr, pin) (((ospeedr) >> ((pin) * 2)) & 0x3U) // 3 = very high

    // AF checks
    d->pa3_af_ok  = (PIN_MODE(d->GPIOA_MODER, 3) == 2) && (PIN_AF(d->GPIOA_AFRL, 0, 3) == 12);
    d->pb2_af_ok  = (PIN_MODE(d->GPIOB_MODER, 2) == 2) && (PIN_AF(d->GPIOB_AFRL, 0, 2) == 10);
    d->pc2_af_ok  = (PIN_MODE(d->GPIOC_MODER, 2) == 2) && (PIN_AF(d->GPIOC_AFRL, 0, 2) == 4);
    d->pc3_af_ok  = (PIN_MODE(d->GPIOC_MODER, 3) == 2) && (PIN_AF(d->GPIOC_AFRL, 0, 3) == 4);
    d->pe2_af_ok  = (PIN_MODE(d->GPIOE_MODER, 2) == 2) && (PIN_AF(d->GPIOE_AFRL, d->GPIOE_AFRH, 2) == 9);
    d->pe7_af_ok  = (PIN_MODE(d->GPIOE_MODER, 7) == 2) && (PIN_AF(d->GPIOE_AFRL, d->GPIOE_AFRH, 7) == 10);
    d->pe10_af_ok = (PIN_MODE(d->GPIOE_MODER, 10) == 2) && (PIN_AF(d->GPIOE_AFRL, d->GPIOE_AFRH, 10) == 10);
    d->pe11_af_ok = (PIN_MODE(d->GPIOE_MODER, 11) == 2) && (PIN_AF(d->GPIOE_AFRL, d->GPIOE_AFRH, 11) == 11);
    d->pf6_af_ok  = (PIN_MODE(d->GPIOF_MODER, 6) == 2) && (PIN_AF(d->GPIOF_AFRL, d->GPIOF_AFRH, 6) == 10);
    d->pf8_af_ok  = (PIN_MODE(d->GPIOF_MODER, 8) == 2) && (PIN_AF(d->GPIOF_AFRL, d->GPIOF_AFRH, 8) == 10);
    d->pf9_af_ok  = (PIN_MODE(d->GPIOF_MODER, 9) == 2) && (PIN_AF(d->GPIOF_AFRL, d->GPIOF_AFRH, 9) == 10);

    // PMCR: PC2SO = bit 21, PC3SO = bit 23
    d->pmcr_pc2so_ok = (d->PMCR & (1U << 21)) ? 1 : 0;
    d->pmcr_pc3so_ok = (d->PMCR & (1U << 23)) ? 1 : 0;

    // Speed check: all OCTOSPI pins must be VERY_HIGH (0x3)
    d->all_speed_ok =
        (PIN_SPEED(d->GPIOA_OSPEEDR, 3) == 3) &&   // CLK
        (PIN_SPEED(d->GPIOB_OSPEEDR, 2) == 3) &&   // DQS
        (PIN_SPEED(d->GPIOC_OSPEEDR, 2) == 3) &&   // IO5
        (PIN_SPEED(d->GPIOC_OSPEEDR, 3) == 3) &&   // IO6
        (PIN_SPEED(d->GPIOE_OSPEEDR, 2) == 3) &&   // IO2
        (PIN_SPEED(d->GPIOE_OSPEEDR, 7) == 3) &&   // IO4
        (PIN_SPEED(d->GPIOE_OSPEEDR, 10) == 3) &&  // IO7
        (PIN_SPEED(d->GPIOE_OSPEEDR, 11) == 3) &&  // NCS
        (PIN_SPEED(d->GPIOF_OSPEEDR, 6) == 3) &&   // IO3
        (PIN_SPEED(d->GPIOF_OSPEEDR, 8) == 3) &&   // IO0
        (PIN_SPEED(d->GPIOF_OSPEEDR, 9) == 3);     // IO1

    // Overall AF verdict
    d->all_af_ok = d->pa3_af_ok && d->pb2_af_ok &&
                   d->pc2_af_ok && d->pc3_af_ok &&
                   d->pe2_af_ok && d->pe7_af_ok && d->pe10_af_ok && d->pe11_af_ok &&
                   d->pf6_af_ok && d->pf8_af_ok && d->pf9_af_ok;

    // Command register checks
    d->read_cmd_ok  = ((d->OSPI_IR & 0xFF) == 0x20);
    d->write_cmd_ok = ((d->OSPI_WIR & 0xFF) == 0xA0);

    #undef PIN_AF
    #undef PIN_MODE
    #undef PIN_SPEED
}

#ifdef __cplusplus
}
#endif

#endif // __PSRAM_DIAG_H
