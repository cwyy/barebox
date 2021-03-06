/*
 * For clock initialization, see chapter 3 of the "MCIMX27 Multimedia
 * Applications Processor Reference Manual, Rev. 0.2".
 *
 */

#include <config.h>
#include <mach/imx-regs.h>

#define writel(val, reg) \
	ldr		r0,	=reg;	\
	ldr		r1,	=val;	\
	str		r1,   [r0];


#define ESDCTL0_VAL (ESDCTL0_SDE | ESDCTL0_ROW13 | ESDCTL0_COL10)

.macro sdram_init
	/*
	 * DDR on CSD0
	 */
	writel(0x00000008, ESDMISC) /* Enable DDR SDRAM operation */

	writel(0x55555555, DSCR(3)) /* Set the driving strength   */
	writel(0x55555555, DSCR(5))
	writel(0x55555555, DSCR(6))
	writel(0x00005005, DSCR(7))
	writel(0x15555555, DSCR(8))

	writel(0x00000004, ESDMISC) /* Initial reset */
	writel(0x006ac73a, ESDCFG0)

	writel(ESDCTL0_VAL | ESDCTL0_SMODE_PRECHARGE, ESDCTL0) /* precharge CSD0 all banks */
	writel(0x00000000, 0xA0000F00)	/* CSD0 precharge address (A10 = 1) */
	writel(ESDCTL0_VAL | ESDCTL0_SMODE_AUTO_REFRESH, ESDCTL0)

	ldr	r0, =0xa0000f00
	mov	r1, #0
	mov	r2, #8
1:
	str	r1, [r0]
	subs	r2, #1
	bne	1b

	writel(ESDCTL0_VAL | ESDCTL0_SMODE_LOAD_MODE, ESDCTL0)
	ldr		r0, =0xA0000033
	mov		r1, #0xda
	strb		r1, [r0]
	ldr		r0, =0xA1000000
	mov		r1, #0xff
	strb		r1, [r0]
	writel(ESDCTL0_VAL | ESDCTL0_DSIZ_31_0 | ESDCTL0_REF4 | ESDCTL0_BL | ESDCTL0_SMODE_NORMAL, ESDCTL0)
.endm

	.section ".text_bare_init","ax"

.globl board_init_lowlevel
board_init_lowlevel:

	mov	r10, lr

	/* ahb lite ip interface */
	writel(0x20040304, AIPI1_PSR0)
	writel(0xDFFBFCFB, AIPI1_PSR1)
	writel(0x00000000, AIPI2_PSR0)
	writel(0xFFFFFFFF, AIPI2_PSR1)

	/* skip sdram initialization if we run from ram */
	cmp	pc, #0xa0000000
	bls	1f
	cmp	pc, #0xc0000000
	bhi	1f

	mov	pc,r10
1:
	sdram_init

#ifdef CONFIG_NAND_IMX_BOOT
	ldr	sp, =0xa0f00000		/* Setup a temporary stack in SDRAM */

	ldr	r0, =IMX_NFC_BASE		/* start of NFC SRAM                */
	ldr	r2, =IMX_NFC_BASE + 0x1000	/* end of NFC SRAM                  */

	/* skip NAND boot if not running from NFC space */
	cmp	pc, r0
	bls	ret
	cmp	pc, r2
	bhi	ret

	/* Move ourselves out of NFC SRAM */
	ldr	r1, =TEXT_BASE

copy_loop:
	ldmia	r0!, {r3-r9}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r9}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop

	ldr	pc, =1f			/* Jump to SDRAM                    */
1:
	bl	nand_boot		/* Load barebox from NAND Flash      */

	ldr	r1, =IMX_NFC_BASE - TEXT_BASE
	sub	r10, r10, r1		/* adjust return address from NFC SRAM */
					/* to SDRAM                            */

#endif /* CONFIG_NAND_IMX_BOOT */

ret:
	mov	pc,r10

