/* linux/drivers/mtd/nand/s3c2410.c
 *
 * Copyright (C) 2009 Juergen Beisert, Pengutronix
 *
 * Copyright © 2004-2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Samsung S3C2410 NAND driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <config.h>
#include <common.h>
#include <driver.h>
#include <malloc.h>
#include <init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <mach/s3c24xx-generic.h>
#include <mach/s3c24x0-iomap.h>
#include <mach/s3c24x0-nand.h>
#include <asm/io.h>
#include <asm-generic/errno.h>

#ifdef CONFIG_S3C24XX_NAND_BOOT
# define __nand_boot_init __bare_init
# ifndef BOARD_DEFAULT_NAND_TIMING
#  define BOARD_DEFAULT_NAND_TIMING 0x0737
# endif
#else
# define __nand_boot_init
#endif

/**
 * Define this symbol for testing purpose. It will add a command to read an
 * image from the NAND like it the boot strap code will do.
 */
#define CONFIG_NAND_S3C24XX_BOOT_DEBUG

/* NAND controller's register */

#define NFCONF 0x00

#ifdef CONFIG_CPU_S3C2410

#define NFCMD 0x04
#define NFADDR 0x08
#define NFDATA 0x0c
#define NFSTAT 0x10
#define NFECC 0x14

/* S3C2410 specific bits */
#define NFSTAT_BUSY (1)
#define NFCONF_nFCE (1 << 11)
#define NFCONF_INITECC (1 << 12)
#define NFCONF_EN (1 << 15)

#endif	/* CONFIG_CPU_S3C2410 */

#ifdef CONFIG_CPU_S3C2440

#define NFCONT 0x04
#define NFCMD 0x08
#define NFADDR 0x0C
#define NFDATA 0x10

#define NFECC 0x1C
#define NFSTAT 0x20

/* S3C2440 specific bits */
#define NFSTAT_BUSY (1)
#define NFCONT_nFCE (1 << 1)
#define NFCONF_INITECC (1 << 12)
#define NFCONT_EN (1)

#endif	/* CONFIG_CPU_S3C2440 */


struct s3c24x0_nand_host {
	struct mtd_info		mtd;
	struct nand_chip	nand;
	struct mtd_partition	*parts;
	struct device_d		*dev;

	unsigned long		base;
};

/**
 * oob placement block for use with hardware ecc generation
 */
static struct nand_ecclayout nand_hw_eccoob = {
	.eccbytes = 3,
	.eccpos = { 0, 1, 2},
	.oobfree = {
		{
			.offset = 8,
			.length = 8
		}
	}
};

/* - Functions shared between the boot strap code and the regular driver - */

/**
 * Issue the specified command to the NAND device
 * @param[in] host Base address of the NAND controller
 * @param[in] cmd Command for NAND flash
 */
static void __nand_boot_init send_cmd(unsigned long host, uint8_t cmd)
{
	writeb(cmd, host + NFCMD);
}

/**
 * Issue the specified address to the NAND device
 * @param[in] host Base address of the NAND controller
 * @param[in] addr Address for the NAND flash
 */
static void __nand_boot_init send_addr(unsigned long host, uint8_t addr)
{
	writeb(addr, host + NFADDR);
}

/**
 * Enable the NAND flash access
 * @param[in] host Base address of the NAND controller
 */
static void __nand_boot_init enable_cs(unsigned long host)
{
#ifdef CONFIG_CPU_S3C2410
	writew(readw(host + NFCONF) & ~NFCONF_nFCE, host + NFCONF);
#endif
#ifdef CONFIG_CPU_S3C2440
	writew(readw(host + NFCONT) & ~NFCONT_nFCE, host + NFCONT);
#endif
}

/**
 * Disable the NAND flash access
 * @param[in] host Base address of the NAND controller
 */
static void __nand_boot_init disable_cs(unsigned long host)
{
#ifdef CONFIG_CPU_S3C2410
	writew(readw(host + NFCONF) | NFCONF_nFCE, host + NFCONF);
#endif
#ifdef CONFIG_CPU_S3C2440
	writew(readw(host + NFCONT) | NFCONT_nFCE, host + NFCONT);
#endif
}

/**
 * Enable the NAND flash controller
 * @param[in] host Base address of the NAND controller
 * @param[in] timing Timing to access the NAND memory
 */
static void __nand_boot_init enable_nand_controller(unsigned long host, uint32_t timing)
{
#ifdef CONFIG_CPU_S3C2410
	writew(timing + NFCONF_EN + NFCONF_nFCE, host + NFCONF);
#endif
#ifdef CONFIG_CPU_S3C2440
	writew(NFCONT_EN + NFCONT_nFCE, host + NFCONT);
	writew(timing, host + NFCONF);
#endif
}

/**
 * Diable the NAND flash controller
 * @param[in] host Base address of the NAND controller
 */
static void __nand_boot_init disable_nand_controller(unsigned long host)
{
#ifdef CONFIG_CPU_S3C2410
	writew(NFCONF_nFCE, host + NFCONF);
#endif
#ifdef CONFIG_CPU_S3C2440
	writew(NFCONT_nFCE, host + NFCONT);
#endif
}

/* ----------------------------------------------------------------------- */

/**
 * Check the ECC and try to repair the data if possible
 * @param[in] mtd_info FIXME
 * @param[inout] dat Pointer to the data buffer that might contain a bit error
 * @param[in] read_ecc ECC data from the OOB space
 * @param[in] calc_ecc ECC data calculated from the data
 * @return 0 no error, 1 repaired error, -1 no way...
 *
 * @note: Alsways 512 byte of data
 */
static int s3c2410_nand_correct_data(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	unsigned int diff0, diff1, diff2;
	unsigned int bit, byte;

	diff0 = read_ecc[0] ^ calc_ecc[0];
	diff1 = read_ecc[1] ^ calc_ecc[1];
	diff2 = read_ecc[2] ^ calc_ecc[2];

	if (diff0 == 0 && diff1 == 0 && diff2 == 0)
		return 0;		/* ECC is ok */

	/* sometimes people do not think about using the ECC, so check
	 * to see if we have an 0xff,0xff,0xff read ECC and then ignore
	 * the error, on the assumption that this is an un-eccd page.
	 */
	if (read_ecc[0] == 0xff && read_ecc[1] == 0xff && read_ecc[2] == 0xff
		/* && info->platform->ignore_unset_ecc */)
		return 0;

	/* Can we correct this ECC (ie, one row and column change).
	 * Note, this is similar to the 256 error code on smartmedia */

	if (((diff0 ^ (diff0 >> 1)) & 0x55) == 0x55 &&
	    ((diff1 ^ (diff1 >> 1)) & 0x55) == 0x55 &&
	    ((diff2 ^ (diff2 >> 1)) & 0x55) == 0x55) {
		/* calculate the bit position of the error */

		bit  = ((diff2 >> 3) & 1) |
		       ((diff2 >> 4) & 2) |
		       ((diff2 >> 5) & 4);

		/* calculate the byte position of the error */

		byte = ((diff2 << 7) & 0x100) |
		       ((diff1 << 0) & 0x80)  |
		       ((diff1 << 1) & 0x40)  |
		       ((diff1 << 2) & 0x20)  |
		       ((diff1 << 3) & 0x10)  |
		       ((diff0 >> 4) & 0x08)  |
		       ((diff0 >> 3) & 0x04)  |
		       ((diff0 >> 2) & 0x02)  |
		       ((diff0 >> 1) & 0x01);

		dat[byte] ^= (1 << bit);
		return 1;
	}

	/* if there is only one bit difference in the ECC, then
	 * one of only a row or column parity has changed, which
	 * means the error is most probably in the ECC itself */

	diff0 |= (diff1 << 8);
	diff0 |= (diff2 << 16);

	if ((diff0 & ~(1<<fls(diff0))) == 0)
		return 1;

	return -1;
}

static void s3c2410_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s3c24x0_nand_host *host = nand_chip->priv;

	writel(readl(host->base + NFCONF) | NFCONF_INITECC , host->base + NFCONF);
}

static int s3c2410_nand_calculate_ecc(struct mtd_info *mtd, const uint8_t *dat, uint8_t *ecc_code)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s3c24x0_nand_host *host = nand_chip->priv;

	ecc_code[0] = readb(host->base + NFECC);
	ecc_code[1] = readb(host->base + NFECC + 1);
	ecc_code[2] = readb(host->base + NFECC + 2);

	return 0;
}

static void s3c24x0_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s3c24x0_nand_host *host = nand_chip->priv;

	if (chip == -1)
		disable_cs(host->base);
	else
		enable_cs(host->base);
}

static int s3c24x0_nand_devready(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s3c24x0_nand_host *host = nand_chip->priv;

	return readw(host->base + NFSTAT) & NFSTAT_BUSY;
}

static void s3c24x0_nand_hwcontrol(struct mtd_info *mtd, int cmd,
					unsigned int ctrl)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s3c24x0_nand_host *host = nand_chip->priv;

	if (cmd == NAND_CMD_NONE)
		return;
	/*
	* If the CLE should be active, this call is a NAND command
	*/
	if (ctrl & NAND_CLE)
		send_cmd(host->base, cmd);
	/*
	* If the ALE should be active, this call is a NAND address
	*/
	if (ctrl & NAND_ALE)
		send_addr(host->base, cmd);
}

static int s3c24x0_nand_inithw(struct s3c24x0_nand_host *host)
{
	struct s3c24x0_nand_platform_data *pdata = host->dev->platform_data;
	uint32_t tmp;

	/* reset the NAND controller */
	disable_nand_controller(host->base);

	if (pdata != NULL)
		tmp = pdata->nand_timing;
	else
		/* else slowest possible timing */
		tmp = CALC_NFCONF_TIMING(4, 8, 8);

	/* reenable the NAND controller */
	enable_nand_controller(host->base, tmp);

	return 0;
}

static int s3c24x0_nand_probe(struct device_d *dev)
{
	struct nand_chip *chip;
	struct mtd_info *mtd;
	struct s3c24x0_nand_host *host;
	int ret;

	/* Allocate memory for MTD device structure and private data */
	host = kzalloc(sizeof(struct s3c24x0_nand_host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;

	host->dev = dev;
	host->base = dev->map_base;

	/* structures must be linked */
	chip = &host->nand;
	mtd = &host->mtd;
	mtd->priv = chip;

	/* init the default settings */
#if 0
	/* TODO: Will follow later */
	init_nand_chip_bw8(chip);
#endif
	/* 50 us command delay time */
	chip->chip_delay = 50;
	chip->priv = host;

	chip->IO_ADDR_R = chip->IO_ADDR_W = (void*)(dev->map_base + NFDATA);

	chip->cmd_ctrl = s3c24x0_nand_hwcontrol;
	chip->dev_ready = s3c24x0_nand_devready;
	chip->select_chip = s3c24x0_nand_select_chip;

	/* we are using the hardware ECC feature of this device */
	chip->ecc.calculate = s3c2410_nand_calculate_ecc;
	chip->ecc.correct = s3c2410_nand_correct_data;
	chip->ecc.hwctl = s3c2410_nand_enable_hwecc;
	chip->ecc.calculate = s3c2410_nand_calculate_ecc;

	/* our hardware capabilities */
	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = 512;
	chip->ecc.bytes = 3;
	chip->ecc.layout = &nand_hw_eccoob;

	ret = s3c24x0_nand_inithw(host);
	if (ret != 0)
		goto on_error;

	/* Scan to find existence of the device */
	ret = nand_scan(mtd, 1);
	if (ret != 0) {
		ret = -ENXIO;
		goto on_error;
	}

	return add_mtd_device(mtd);
	
on_error:
	free(host);
	return ret;
}

static struct driver_d s3c24x0_nand_driver = {
	.name  = "s3c24x0_nand",
	.probe = s3c24x0_nand_probe,
};

#ifdef CONFIG_S3C24XX_NAND_BOOT

static void __nand_boot_init wait_for_completion(unsigned long host)
{
	while (!(readw(host + NFSTAT) & NFSTAT_BUSY))
		;
}

static void __nand_boot_init nfc_addr(unsigned long host, uint32_t offs)
{
	send_addr(host, offs & 0xff);
	send_addr(host, (offs >> 9) & 0xff);
	send_addr(host, (offs >> 17) & 0xff);
	send_addr(host, (offs >> 25) & 0xff);
}

/**
 * Load a sequential count of blocks from the NAND into memory
 * @param[out] dest Pointer to target area (in SDRAM)
 * @param[in] size Bytes to read from NAND device
 * @param[in] page Start page to read from
 * @param[in] pagesize Size of each page in the NAND
 *
 * This function must be located in the first 4kiB of the barebox image
 * (guess why). When this routine is running the SDRAM is up and running
 * and it runs from the correct address (physical=linked address).
 * TODO Could we access the platform data from the boardfile?
 * Due to it makes no sense this function does not return in case of failure.
 */
void __nand_boot_init s3c24x0_nand_load_image(void *dest, int size, int page, int pagesize)
{
	unsigned long host = S3C24X0_NAND_BASE;
	int i;

	/*
	 * Reenable the NFC and use the default (but slow) access
	 * timing or the board specific setting if provided.
	 */
	enable_nand_controller(host, BOARD_DEFAULT_NAND_TIMING);
	enable_cs(host);

	/* Reset the NAND device */
	send_cmd(host, NAND_CMD_RESET);
	wait_for_completion(host);
	disable_cs(host);

	do {
		enable_cs(host);
		send_cmd(host, NAND_CMD_READ0);
		nfc_addr(host, page * pagesize);
		wait_for_completion(host);
		/* copy one page (do *not* use readsb() here!)*/
		for (i = 0; i < pagesize; i++)
			writeb(readb(host + NFDATA), (unsigned long)(dest + i));
		disable_cs(host);

		page++;
		dest += pagesize;
		size -= pagesize;
	} while (size >= 0);

	/* disable the controller again */
	disable_nand_controller(host);
}

#ifdef CONFIG_NAND_S3C24XX_BOOT_DEBUG
#include <command.h>

static int do_nand_boot_test(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	void *dest;
	int size, pagesize;

	if (argc < 3)
		return COMMAND_ERROR_USAGE;

	dest = (void *)strtoul_suffix(argv[1], NULL, 0);
	size = strtoul_suffix(argv[2], NULL, 0);
	pagesize = strtoul_suffix(argv[3], NULL, 0);

	s3c24x0_nand_load_image(dest, size, 0, pagesize);

	return 0;
}

static const __maybe_unused char cmd_nand_boot_test_help[] =
"Usage: nand_boot_test <dest> <size> <pagesize>\n";

BAREBOX_CMD_START(nand_boot_test)
	.cmd		= do_nand_boot_test,
	.usage		= "load an image from NAND",
	BAREBOX_CMD_HELP(cmd_nand_boot_test_help)
BAREBOX_CMD_END
#endif

#endif /* CONFIG_S3C24XX_NAND_BOOT */

/*
 * Main initialization routine
 * @return 0 if successful; non-zero otherwise
 */
static int __init s3c24x0_nand_init(void)
{
	return register_driver(&s3c24x0_nand_driver);
}

device_initcall(s3c24x0_nand_init);
