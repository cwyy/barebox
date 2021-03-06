/**
 * @file
 * @brief Provides PRCM divisors and SRAM execution code.
 *
 * FileName: arch/arm/mach-omap/omap3_clock_core.S
 *
 * This provides two things:
 * @li @ref omap3_clock.c cannot have switch or global variables.
 * This file provides the constant data for the file to use.
 *
 * @li @ref prcm_init cannot execute certain critical clock
 * configurations while running in SDRAM/Flash. This provides
 * relocation and execution capability for the same.
 *
 * Orignally from http://linux.omap.com/pub/bootloader/3430sdp/u-boot-v1.tar.gz
 */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 * Nishanth Menon <x0nishan@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <mach/silicon.h>
#include <mach/clocks.h>
#include <mach/gpmc.h>

#ifdef CONFIG_OMAP3_COPY_CLOCK_SRAM
/**
 * @fn void cpy_clk_code(u32 R1)
 *
 * @brief cpy_clk_code: relocates clock code into SRAM where its safer to
 * execute
 *
 * @param[in] R1 = SRAM destination address.
 *
 * @return void
 */
.global cpy_clk_code
 cpy_clk_code:
        /* Copy DPLL code into SRAM */
        adr     r0, go_to_speed         /* get addr of clock setting code */
        mov     r2, #384                /* r2 size to copy (div by 32 bytes) */
        mov     r1, r1                  /* r1 <- dest address (passed in) */
        add     r2, r2, r0              /* r2 <- source end address */
next2:
        ldmia   r0!, {r3-r10}           /* copy from source address [r0]    */
        stmia   r1!, {r3-r10}           /* copy to   target address [r1]    */
        cmp     r0, r2                  /* until source end address [r2]    */
        bne     next2
	mov	pc, lr                  /* back to caller */

/**
 * @fn void go_to_speed(u32 R0, u32 R1, u32 R3)
 *
 * @brief go_to_speed: Function which configures the clocks
 *  Moves to bypass, -Commits clock dividers, -puts dpll at speed
 *               -executed from SRAM.
 * @warning Note: If core unlocks/relocks and SDRAM is running fast already
 *        it gets confused.  A reset of the controller gets it back. Taking
 *        away its L3 when its not in self refresh seems bad for it.
 *        Normally, this code runs from flash before SDR is init so that
 *        should be ok.
 *
 * @param[in] R1 = SRAM destination address.
 * @param[in] R0 = CM_CLKEN_PLL-bypass value
 * @param[in] R1 = CM_CLKSEL1_PLL-m, n, and divider values
 * @param[in] R2 = CM_CLKSEL_CORE-divider values
 * @param[in] R3 = CM_IDLEST_CKGEN - addr dpll lock wait
 *
 * @return void
 */
.global go_to_speed
 go_to_speed:
        stmfd sp!, {r4-r6}

        /* move into fast relock bypass */
        ldr     r4, pll_ctl_add
        str     r0, [r4]
wait1:
        ldr     r5, [r3]       /* get status */
        and     r5, r5, #0x1   /* isolate core status */
        cmp     r5, #0x1       /* still locked? */
        beq     wait1          /* if lock, loop */

	/* set new dpll dividers _after_ in bypass */
	ldr     r5, pll_div_add1
        str     r1, [r5]          /* set m, n, m2 */
        ldr     r5, pll_div_add2
        str     r2, [r5]          /* set l3/l4/.. dividers*/
        ldr     r5, pll_div_add3  /* wkup */
        ldr     r2, pll_div_val3  /* rsm val */
        str     r2, [r5]
        ldr     r5, pll_div_add4  /* gfx */
        ldr     r2, pll_div_val4
        str     r2, [r5]
        ldr     r5, pll_div_add5  /* emu */
        ldr     r2, pll_div_val5
        str     r2, [r5]

#if 0
        /* FIXME: now prepare GPMC (flash) for new dpll speed
	 * For NOR/NAND/OneNAND boot ->make this as Kconfig?
	 */
	/* flash needs to be stable when we jump back to it */
        ldr     r6, flash_cfg_offset
        ldr     r5, flash_cfg_addr /* CFG1 */
        ldr     r2, flash_cfg1_val
        str     r2, [r5]
        add	r5, r5, r6 /* CFG2 */
        ldr     r2, flash_cfg2_val
        str     r2, [r5]
        add	r5, r5, r6 /* CFG3 */
        ldr     r2, flash_cfg3_val
        str     r2, [r5]
        add	r5, r5, r6 /* CFG4 */
        ldr     r2, flash_cfg4_val
        str     r2, [r5]
        add	r5, r5, r6 /* CFG5 */
        ldr     r2, flash_cfg5_val
        str     r2, [r5]
        add	r5, r5, r6 /* CFG6 */
        ldr     r2, flash_cfg6_val
        str     r2, [r5]
#endif /* Debug */

        /* lock DPLL3 and wait a bit */
        orr     r0, r0, #0x7   /* set up for lock mode */
        str     r0, [r4]       /* lock */
        nop                    /* ARM slow at this point working at sys_clk */
        nop
        nop
        nop
wait2:
        ldr     r5, [r3]       /* get status */
        and     r5, r5, #0x1   /* isolate core status */
        cmp     r5, #0x1       /* still locked? */
        bne     wait2          /* if lock, loop */
        nop
        nop
        nop
        nop
        ldmfd sp!, {r4-r6}
        mov     pc, lr           /* back to caller, locked */

_go_to_speed: .word go_to_speed

/* these constants need to be close for PIC code */
/* FIXME: The Nor has to be in the Flash Base CS0 for this condition to happen*/
#if 0
flash_cfg_addr:
    .word GPMC_REG(CONFIG1_0)
flash_cfg_offset:
    .word GPMC_REG(CONFIG2_0) - GPMC_REG(CONFIG1_0)
flash_cfg1_val:
    .word  CONFIG_VALUE_GPMC_CONFIG1
flash_cfg2_val:
    .word  CONFIG_VALUE_GPMC_CONFIG2
flash_cfg3_val:
    .word  CONFIG_VALUE_GPMC_CONFIG3
flash_cfg4_val:
    .word  CONFIG_VALUE_GPMC_CONFIG4
flash_cfg5_val:
    .word  CONFIG_VALUE_GPMC_CONFIG5
flash_cfg6_val:
    .word  CONFIG_VALUE_GPMC_CONFIG6
#endif
pll_ctl_add:
    .word CM_CLKEN_PLL
pll_div_add1:
    .word CM_CLKSEL1_PLL
pll_div_add2:
    .word CM_CLKSEL_CORE
pll_div_add3:
    .word CM_CLKSEL_WKUP
pll_div_val3:
    .word (WKUP_RSM << 1)
pll_div_add4:
    .word CM_CLKSEL_GFX
pll_div_val4:
    .word (GFX_DIV << 0)
pll_div_add5:
    .word CM_CLKSEL1_EMU
pll_div_val5:
    .word CLSEL1_EMU_VAL

#endif /* OMAP3_CLOCK_COPY_SRAM */

	/* the literal pools origin */
	.ltorg

/* DPLL(1-4) PARAM TABLES */
/* Each of the tables has M, N, FREQSEL, M2 values defined for nominal
 * OPP (1.2V). The fields are defined according to dpll_param
 * struct(omap3_clock.c). MAX index is as per omap3_clock.h
 */

mpu_dpll_param:
/* 12MHz */
/* ES2 */
.word 0x0FA,0x05,0x07,0x01

/* 13MHz */
/* ES2 */
.word 0x1F4,0x0C,0x03,0x01

/* 19.2MHz */
/* ES2 */
.word 0x271,0x17,0x03,0x01

/* 26MHz */
/* ES2 */
.word 0x0FA,0x0C,0x07,0x01

/* 38.4MHz */
/* ES2 */
.word 0x271,0x2F,0x03,0x01

.globl get_mpu_dpll_param
get_mpu_dpll_param:
	adr r0, mpu_dpll_param
	mov pc, lr

iva_dpll_param:
/* 12MHz */
/* ES2 */
.word 0x0B4,0x05,0x07,0x01

/* 13MHz */
/* ES2 */
.word 0x168,0x0C,0x03,0x01

/* 19.2MHz */
/* ES2 */
.word 0x0E1,0x0B,0x06,0x01

/* 26MHz */
/* ES2 */
.word 0x0B4,0x0C,0x07,0x01

/* 38.4MHz */
/* ES2 */
.word 0x0E1,0x17,0x06,0x01

.globl get_iva_dpll_param
get_iva_dpll_param:
	adr r0, iva_dpll_param
	mov pc, lr

core_dpll_param:
/* 12MHz */
/* ES2 */
.word 0x0A6,0x05,0x07,0x01

/* 13MHz */
/* ES2 */
.word 0x14C,0x0C,0x03,0x01

/* 19.2MHz */
/* ES2 */
.word 0x19F,0x17,0x03,0x01

/* 26MHz */
/* ES2 */
.word 0x0A6,0x0C,0x07,0x01

/* 38.4MHz */
/* ES2 */
.word 0x19F,0x2F,0x03,0x01

.globl get_core_dpll_param
get_core_dpll_param:
	adr r0, core_dpll_param
	mov pc, lr

/* PER DPLL values are same for both ES1 and ES2 */
per_dpll_param:
/* 12MHz */
.word 0xD8,0x05,0x07,0x09

/* 13MHz */
.word 0x1B0,0x0C,0x03,0x09

/* 19.2MHz */
.word 0xE1,0x09,0x07,0x09

/* 26MHz */
.word 0xD8,0x0C,0x07,0x09

/* 38.4MHz */
.word 0xE1,0x13,0x07,0x09

.globl get_per_dpll_param
get_per_dpll_param:
	adr r0, per_dpll_param
	mov pc, lr
