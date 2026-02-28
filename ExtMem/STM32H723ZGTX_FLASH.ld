/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : Intector
**
**  Abstract    : Linker script for STM32H7 series
**                1024Kbytes FLASH and 560Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STM32H723XGx
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
*****************************************************************************
**
** Copyright (c) 2024 Intector.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);    /* end of DTCMRAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  /* -------------------------------------------------------------------------- */
  ITCMRAM               (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K
  FLASH                 (rx)    : ORIGIN = 0x08000000, LENGTH = 896K
  FW_DATA               (rx)    : ORIGIN = 0x080E0000, LENGTH = 128K
  DTCMRAM               (xrw)   : ORIGIN = 0x20000000, LENGTH = 128K
  /* -------------------------------------------------------------------------- */
  Mem_ETH_Rx_DMA_Descr  (xrw)   : ORIGIN = 0x24000000, LENGTH = 512
  Mem_ETH_Tx_DMA_Descr  (xrw)   : ORIGIN = 0x24000200, LENGTH = 512
  Mem_ETH_Rx_Buff       (xrw)   : ORIGIN = 0x24000400, LENGTH = 1536
  RAM                   (xrw)   : ORIGIN = 0x24000A00, LENGTH = 325120
  /* -------------------------------------------------------------------------- */
  RAM_SRAM1             (xrw)   : ORIGIN = 0x30000000, LENGTH = 16K
  RAM_SRAM2             (xrw)   : ORIGIN = 0x30004000, LENGTH = 16K
  RAM_SRAM4             (xrw)   : ORIGIN = 0x38000000, LENGTH = 16K
  BACKUP_RAM            (xrw)   : ORIGIN = 0x38800000, LENGTH = 4K
  /* -------------------------------------------------------------------------- */
  /* 4 MB PSRAM - ThreadX heap ------------------------------------------------ */
  Mem_TxPool            (xrw)   : ORIGIN = 0x90000000, LENGTH = 500K
  Mem_FxPool            (xrw)   : ORIGIN = 0x9007D000, LENGTH = 500K
  Mem_NxPool            (xrw)   : ORIGIN = 0x900FA000, LENGTH = 500K
  Mem_FTP_Server        (xrw)   : ORIGIN = 0x90177000, LENGTH = 100K
  Mem_Buff1             (xrw)   : ORIGIN = 0x90190000, LENGTH = 2496K
  /* -------------------------------------------------------------------------- */
  /* 4 MB PSRAM - DMA Buffers ------------------------------------------------- */
  Mem_LabelDescr        (xrw)   : ORIGIN = 0x90400000, LENGTH = 100K
  Mem_Label1Data        (xrw)   : ORIGIN = 0x90419000, LENGTH = 48K
  Mem_Label2Data        (xrw)   : ORIGIN = 0x90425000, LENGTH = 48K
  Mem_Buff2             (xrw)   : ORIGIN = 0x90431000, LENGTH = 3900K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH
  .ARM (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >DTCMRAM

  /* ########################################################### */
  /*      standard memory definitions                            */
  /* ########################################################### */

 /*  Unitialized ITCMRAM section into "ITCMRAM" ITCMRAM type memory */
 .ITCMRAM_Section :
  {
    . = ALIGN(4);
    KEEP (*(.ITCMRAM_Section))
    . = ALIGN(4);
  } >ITCMRAM

 /*  Unitialized DTCMRAM section into "DTCMRAM" DTCMRAM type memory */
 .DTCMRAM_Section :
  {
    . = ALIGN(4);
    KEEP (*(.DTCMRAM_Section))
    . = ALIGN(4);
  } >DTCMRAM

 /*  Unitialized RAM_SRAM1 section into "RAM_SRAM1" RAM_SRAM1 type memory */
 .RAM_SRAM1_Section :
  {
    . = ALIGN(4);
    KEEP (*(.RAM_SRAM1_Section))
    . = ALIGN(4);
  } >RAM_SRAM1

 /*  Unitialized RAM_SRAM4 section into "RAM_SRAM4" RAM_SRAM4 type memory */
 .RAM_SRAM4_Section :
  {
    . = ALIGN(4);
    KEEP (*(.RAM_SRAM4_Section))
    . = ALIGN(4);
  } >RAM_SRAM4

 /*  Unitialized BACKUP_RAM section into "BACKUP_RAM" BACKUP_RAM type memory */
 .BACKUP_RAM_Section :
  {
    . = ALIGN(4);
    KEEP (*(.BACKUP_RAM_Section))
    . = ALIGN(4);
  } >BACKUP_RAM


  /* ########################################################### */
  /*      definitions for the firmware settings data             */
  /*  FLASH - Sector 7                                           */
  /*  location: defined by FW_DATA memory region                 */
  /*  total size: 128 KByte                                      */
  /* ########################################################### */

  .fws_memory : {
    
    /* FW download ID - 32 Bytes */
    . = ALIGN(4);
    *(.FWS_DownloadID)

    /* Network settings - 32 Bytes */
    . = ALIGN(4);
    *(.FWS_Network)

    /* Future settings can be added here */

  } > FW_DATA

 /* ########################################################### */
 /*      definitions for tcp Rx and Tx -DescripSection          */
 /* ########################################################### */

 /*  Unitialized Mem_ETH_Rx_DMA_Descr section into "Mem_ETH_Rx_DMA_Descr" Mem_ETH_Rx_DMA_Descr type memory */
 .Mem_ETH_Rx_DMA_Descr_Section :
  {
    . = ALIGN(4);
    KEEP (*(.Mem_ETH_Rx_DMA_Descr_Section))
    . = ALIGN(4);
  } >Mem_ETH_Rx_DMA_Descr

 /*  Unitialized Mem_ETH_Tx_DMA_Descr section into "Mem_ETH_Tx_DMA_Descr" Mem_ETH_Tx_DMA_Descr type memory */
 .Mem_ETH_Tx_DMA_Descr_Section :
  {
    . = ALIGN(4);
    KEEP (*(.Mem_ETH_Tx_DMA_Descr_Section))
    . = ALIGN(4);
  } >Mem_ETH_Tx_DMA_Descr

 /*  Unitialized Mem_ETH_Rx_Buff section into "Mem_ETH_Rx_Buff" Mem_ETH_Rx_Buff type memory */
 .Mem_ETH_Rx_Buff_Section :
  {
    . = ALIGN(4);
    KEEP (*(.Mem_ETH_Rx_Buff_Section))
    . = ALIGN(4);
  } >Mem_ETH_Rx_Buff

  /* ########################################################### */
  /*   DMA buffer section - AXI SRAM (IDMA accessible)           */
  /* ########################################################### */
  .dma_buffer(NOLOAD) :
  {
    . = ALIGN(32);
    *(.dma_buffer)
    . = ALIGN(4);
  } > RAM

/* ########################################################### */
/*   definitions for AZURE RTOS - ThreadX, FileX, NetXDuo      */
/* ########################################################### */
.TxPool_sec (NOLOAD) :
  {
    /* .TxPoolSection section - 500 KBytes */
    . = ALIGN(4);
    *(.TxPoolSection)
  } > Mem_TxPool

  .FxPool_sec (NOLOAD) :
  {
    /* .FxPoolSection section - 500 KBytes */
    . = ALIGN(4);
    *(.FxPoolSection)
  } > Mem_FxPool

  .NxPool_sec (NOLOAD) :
  {
    /* .NxPoolSection section - 500 KBytes */
    . = ALIGN(4);
    *(.NxPoolSection)
  } > Mem_NxPool

  /* ########################################################### */
  /*   definitions for AZURE RTOS - HTTP, FTP, WS                */
  /* ########################################################### */
  .FTP_Pool_sec (NOLOAD) :
  {
    /* FTP-Server pool section - 100 KBytes */
    . = ALIGN(4);
    *(.Nx_FTP_ServerPoolSection)
  } > Mem_FTP_Server

  /* ########################################################### */
  /*   definitions for display buffer                            */
  /* ########################################################### */
  .Label_Descriptor_sec (NOLOAD) :
  {
    /* Label Descriptor section - 100 KBytes */
    . = ALIGN(4);
    *(.LabelDescriptorSection)
  } > Mem_LabelDescr

  .Label_1_Data_sec (NOLOAD) :
  {
    /* Label 1 Data section - 48 KBytes */
    . = ALIGN(4);
    *(.Label1DataSection)
  } > Mem_Label1Data

  .Label_2_Data_sec (NOLOAD) :
  {
    /* Label 2 Data section - 48 KBytes */
    . = ALIGN(4);
    *(.Label2DataSection)
  } > Mem_Label2Data


  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a(* )
    libm.a(* )
    libgcc.a(* )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }


}
