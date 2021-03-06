/*
 * (C) Copyright 2002-2004
 * Brad Kemp, Seranoa Networks, Brad.Kemp@seranoa.com
 *
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Copyright (C) 2004
 * Ed Okerson
 *
 * Copyright (C) 2006
 * Tolunay Orkun <listmember@orkun.us>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifdef CONFIG_ENABLE_FLASH_NOISE
# define DEBUG
#endif

#include <common.h>
#include <asm/byteorder.h>
#include <environment.h>
#include <clock.h>
#include <init.h>
#include <malloc.h>
#include <cfi_flash.h>
#include <errno.h>

#define FLASH_CMD_CFI			0x98
#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xD0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xD0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_WRITE_TO_BUFFER	0xE8
#define FLASH_CMD_WRITE_BUFFER_CONFIRM	0xD0

#define FLASH_STATUS_DONE		0x80
#define FLASH_STATUS_ESS		0x40
#define FLASH_STATUS_ECLBS		0x20
#define FLASH_STATUS_PSLBS		0x10
#define FLASH_STATUS_VPENS		0x08
#define FLASH_STATUS_PSS		0x04
#define FLASH_STATUS_DPS		0x02
#define FLASH_STATUS_R			0x01
#define FLASH_STATUS_PROTECT		0x01

#define AMD_CMD_RESET			0xF0
#define AMD_CMD_WRITE			0xA0
#define AMD_CMD_ERASE_START		0x80
#define AMD_CMD_ERASE_SECTOR		0x30
#define AMD_CMD_UNLOCK_START		0xAA
#define AMD_CMD_UNLOCK_ACK		0x55
#define AMD_CMD_WRITE_TO_BUFFER		0x25
#define AMD_CMD_WRITE_BUFFER_CONFIRM	0x29

#define AMD_STATUS_TOGGLE		0x40
#define AMD_STATUS_ERROR		0x20

#define AMD_ADDR_ERASE_START	((info->portwidth == FLASH_CFI_8BIT) ? 0xAAA : 0x555)
#define AMD_ADDR_START		((info->portwidth == FLASH_CFI_8BIT) ? 0xAAA : 0x555)
#define AMD_ADDR_ACK		((info->portwidth == FLASH_CFI_8BIT) ? 0x555 : 0x2AA)

#define FLASH_OFFSET_MANUFACTURER_ID	0x00
#define FLASH_OFFSET_DEVICE_ID		0x01
#define FLASH_OFFSET_DEVICE_ID2		0x0E
#define FLASH_OFFSET_DEVICE_ID3		0x0F
#define FLASH_OFFSET_CFI		0x55
#define FLASH_OFFSET_CFI_ALT		0x555
#define FLASH_OFFSET_CFI_RESP		0x10
#define FLASH_OFFSET_PRIMARY_VENDOR	0x13
#define FLASH_OFFSET_EXT_QUERY_T_P_ADDR	0x15	/* extended query table primary addr */
#define FLASH_OFFSET_WTOUT		0x1F
#define FLASH_OFFSET_WBTOUT		0x20
#define FLASH_OFFSET_ETOUT		0x21
#define FLASH_OFFSET_CETOUT		0x22
#define FLASH_OFFSET_WMAX_TOUT		0x23
#define FLASH_OFFSET_WBMAX_TOUT		0x24
#define FLASH_OFFSET_EMAX_TOUT		0x25
#define FLASH_OFFSET_CEMAX_TOUT		0x26
#define FLASH_OFFSET_SIZE		0x27
#define FLASH_OFFSET_INTERFACE		0x28
#define FLASH_OFFSET_BUFFER_SIZE	0x2A
#define FLASH_OFFSET_NUM_ERASE_REGIONS	0x2C
#define FLASH_OFFSET_ERASE_REGIONS	0x2D
#define FLASH_OFFSET_PROTECT		0x02
#define FLASH_OFFSET_USER_PROTECTION	0x85
#define FLASH_OFFSET_INTEL_PROTECTION	0x81

#define CFI_CMDSET_NONE			0
#define CFI_CMDSET_INTEL_EXTENDED	1
#define CFI_CMDSET_AMD_STANDARD		2
#define CFI_CMDSET_INTEL_STANDARD	3
#define CFI_CMDSET_AMD_EXTENDED		4
#define CFI_CMDSET_MITSU_STANDARD	256
#define CFI_CMDSET_MITSU_EXTENDED	257
#define CFI_CMDSET_SST			258

#ifdef CFG_FLASH_CFI_AMD_RESET /* needed for STM_ID_29W320DB on UC100 */
# undef  FLASH_CMD_RESET
# define FLASH_CMD_RESET	AMD_CMD_RESET /* use AMD-Reset instead */
#endif

typedef union {
	unsigned char c;
	unsigned short w;
	unsigned long l;
	unsigned long long ll;
} cfiword_t;

typedef union {
	volatile unsigned char *cp;
	volatile unsigned short *wp;
	volatile unsigned long *lp;
	volatile unsigned long long *llp;
} cfiptr_t;

#define NUM_ERASE_REGIONS	4 /* max. number of erase regions */

static uint flash_offset_cfi[2]={FLASH_OFFSET_CFI,FLASH_OFFSET_CFI_ALT};

/*
 * Check if chip width is defined. If not, start detecting with 8bit.
 */
#ifndef CFG_FLASH_CFI_WIDTH
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#endif


/*-----------------------------------------------------------------------
 * Functions
 */

typedef unsigned long flash_sect_t;

static void flash_add_byte (flash_info_t * info, cfiword_t * cword, uchar c);
static void flash_make_cmd (flash_info_t * info, uchar cmd, void *cmdbuf);
static void flash_write_cmd (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static void flash_unlock_seq (flash_info_t * info, flash_sect_t sect);
static int flash_isequal (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static int flash_isset (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static int flash_toggle (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static void flash_read_jedec_ids (flash_info_t * info);
static int flash_detect_cfi (flash_info_t * info);
static int flash_write_cfiword (flash_info_t * info, ulong dest, cfiword_t cword);
static int flash_full_status_check (flash_info_t * info, flash_sect_t sector,
				    uint64_t tout, char *prompt);
static ulong flash_get_size (flash_info_t *info, ulong base);
#ifdef CONFIG_CFI_BUFFER_WRITE
static int flash_write_cfibuffer (flash_info_t * info, ulong dest, const uchar * cp, int len);
#endif
static int write_buff (flash_info_t *info, const uchar *src, ulong addr, ulong cnt);

static int flash_real_protect(flash_info_t *info, long sector, int prot);

/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
static inline uchar *flash_make_addr (flash_info_t * info, flash_sect_t sect, uint offset)
{
	return ((uchar *) (info->start[sect] + (offset * info->portwidth)));
}

#ifdef DEBUG
/*-----------------------------------------------------------------------
 * Debug support
 */
static void print_longlong (char *str, unsigned long long data)
{
	int i;
	char *cp;

	cp = (unsigned char *) &data;
	for (i = 0; i < 8; i++)
		sprintf (&str[i * 2], "%2.2x", *cp++);
}
static void flash_printqry (flash_info_t * info, flash_sect_t sect)
{
	cfiptr_t cptr;
	int x, y;

	for (x = 0; x < 0x40; x += 16U / info->portwidth) {
		cptr.cp =
			flash_make_addr (info, sect,
					 x + FLASH_OFFSET_CFI_RESP);
		debug ("%p : ", cptr.cp);
		for (y = 0; y < 16; y++) {
			debug ("%2.2x ", cptr.cp[y]);
		}
		debug (" ");
		for (y = 0; y < 16; y++) {
			if (cptr.cp[y] >= 0x20 && cptr.cp[y] <= 0x7e) {
				debug ("%c", cptr.cp[y]);
			} else {
				debug (".");
			}
		}
		debug ("\n");
	}
}
#endif


/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
static inline uchar flash_read_uchar (flash_info_t * info, uint offset)
{
	uchar *cp;

	cp = flash_make_addr (info, 0, offset);
#if defined(__LITTLE_ENDIAN)
	return (cp[0]);
#else
	return (cp[info->portwidth - 1]);
#endif
}

/*-----------------------------------------------------------------------
 * read a short word by swapping for ppc format.
 */
static ushort flash_read_ushort (flash_info_t * info, flash_sect_t sect, uint offset)
{
	uchar *addr;
	ushort retval;

#ifdef DEBUG
	int x;
#endif
	addr = flash_make_addr (info, sect, offset);

#ifdef DEBUG
	debug ("ushort addr is at %p info->portwidth = %d\n", addr,
	       info->portwidth);
	for (x = 0; x < 2 * info->portwidth; x++) {
		debug ("addr[%x] = 0x%x\n", x, addr[x]);
	}
#endif
#if defined(__LITTLE_ENDIAN)
	retval = ((addr[(info->portwidth)] << 8) | addr[0]);
#else
	retval = ((addr[(2 * info->portwidth) - 1] << 8) |
		  addr[info->portwidth - 1]);
#endif

	debug ("retval = 0x%x\n", retval);
	return retval;
}

/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maximum
 * port size word. Swap for ppc format.
 */
static ulong flash_read_long (flash_info_t * info, flash_sect_t sect, uint offset)
{
	uchar *addr;
	ulong retval;

#ifdef DEBUG
	int x;
#endif
	addr = flash_make_addr (info, sect, offset);

#ifdef DEBUG
	debug ("long addr is at %p info->portwidth = %d\n", addr,
	       info->portwidth);
	for (x = 0; x < 4 * info->portwidth; x++) {
		debug ("addr[%x] = 0x%x\n", x, addr[x]);
	}
#endif
#if defined(__LITTLE_ENDIAN)
	retval = (addr[0] << 16) | (addr[(info->portwidth)] << 24) |
		(addr[(2 * info->portwidth)]) | (addr[(3 * info->portwidth)] << 8);
#else
	retval = (addr[(2 * info->portwidth) - 1] << 24) |
		(addr[(info->portwidth) - 1] << 16) |
		(addr[(4 * info->portwidth) - 1] << 8) |
		addr[(3 * info->portwidth) - 1];
#endif
	return retval;
}

static int flash_find_sector(flash_info_t * info, unsigned long adr)
{
	int i;
	unsigned long end;

	for (i = 0; i < info->sector_count; i++) {
		if (i + 1 == info->sector_count)
			end = info->start[0] + info->size - 1;
		else
			end = info->start[i + 1] - 1;
		if (adr >= info->start[i] && adr <= end)
			return i;
	}
	return -1;
}

/*-----------------------------------------------------------------------
 */
static int flash_erase_one (flash_info_t * info, long sect)
{
	int rcode = 0;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		flash_write_cmd (info, sect, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, sect, 0, FLASH_CMD_BLOCK_ERASE);
		flash_write_cmd (info, sect, 0, FLASH_CMD_ERASE_CONFIRM);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		flash_unlock_seq (info, sect);
		flash_write_cmd (info, sect, AMD_ADDR_ERASE_START,
					AMD_CMD_ERASE_START);
		flash_unlock_seq (info, sect);
		flash_write_cmd (info, sect, 0, AMD_CMD_ERASE_SECTOR);
		break;
	default:
		debug ("Unkown flash vendor %d\n",
		       info->vendor);
		break;
	}
	if (flash_full_status_check
	    (info, sect, info->erase_blk_tout, "erase")) {
		rcode = 1;
	} else
		putchar('.');

	return rcode;
}

static int cfi_erase(struct cdev *cdev, size_t count, unsigned long offset)
{
	flash_info_t *finfo = (flash_info_t *)cdev->priv;
	unsigned long start, end;
	int i, ret = 0;

	debug("%s: erase 0x%08x (size %d)\n", __FUNCTION__, offset, count);

	start = flash_find_sector(finfo, cdev->dev->map_base + offset);
	end   = flash_find_sector(finfo, cdev->dev->map_base + offset + count - 1);

	for (i = start; i <= end; i++) {
		ret = flash_erase_one (finfo, i);
		if (ret)
			goto out;
	}
out:
	putchar('\n');
	return ret;
}

static int cfi_protect(struct cdev *cdev, size_t count, unsigned long offset, int prot)
{
	flash_info_t *finfo = (flash_info_t *)cdev->priv;
	unsigned long start, end;
	int i, ret = 0;

	debug("%s: protect 0x%08x (size %d)\n", __FUNCTION__, offset, count);

	start = flash_find_sector(finfo, cdev->dev->map_base + offset);
	end   = flash_find_sector(finfo, cdev->dev->map_base + offset + count - 1);

	for (i = start; i <= end; i++) {
		ret = flash_real_protect (finfo, i, prot);
		if (ret)
			goto out;
	}
out:
	putchar('\n');
	return ret;
}

static ssize_t cfi_write(struct cdev *cdev, const void* buf, size_t count, unsigned long offset, ulong flags)
{
	flash_info_t *finfo = (flash_info_t *)cdev->priv;
	int ret;

	debug("cfi_write: buf=0x%08x addr=0x%08x count=0x%08x\n",buf, cdev->dev->map_base + offset, count);

	ret = write_buff (finfo, buf, cdev->dev->map_base + offset, count);
	return ret == 0 ? count : -1;
}

static void cfi_info (struct device_d* dev)
{
	flash_info_t *info = (flash_info_t *)dev->priv;
	int i;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("missing or unknown FLASH type\n");
		return;
	}

	printf ("CFI conformant FLASH (%d x %d)"
		"  Size: %ld MB in %d Sectors\n"
		"  ",
		info->portwidth << 3,info->chipwidth << 3,
		info->size >> 20, info->sector_count);

	switch (info->vendor) {
		case CFI_CMDSET_INTEL_STANDARD:
			printf ("Intel Standard");
			break;
		case CFI_CMDSET_INTEL_EXTENDED:
			printf ("Intel Extended");
			break;
		case CFI_CMDSET_AMD_STANDARD:
			printf ("AMD Standard");
			break;
		case CFI_CMDSET_AMD_EXTENDED:
			printf ("AMD Extended");
			break;
		default:
			printf ("Unknown (%d)", info->vendor);
			break;
	}
	printf (" command set, Manufacturer ID: 0x%02X, Device ID: 0x%02X",
		info->manufacturer_id, info->device_id);
	if (info->device_id == 0x7E) {
		printf("%04X", info->device_id2);
	}
	printf ("\n  Erase timeout: %ld ms, write timeout: %ld ms\n",
		info->erase_blk_tout,
		info->write_tout);
	if (info->buffer_size > 1) {
		printf ("  Buffer write timeout: %ld ms, buffer size: %d bytes\n",
		info->buffer_write_tout,
		info->buffer_size);
	}

	puts ("\n  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n");
#ifdef CFG_FLASH_EMPTY_INFO
	{
		int k;
		int size;
		int erased;
		volatile unsigned long *flash;

		/*
		 * Check if whole sector is erased
		 */
		if (i != (info->sector_count - 1))
			size = info->start[i + 1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned long *) info->start[i];
		size = size >> 2;	/* divide by 4 for longword access */
		for (k = 0; k < size; k++) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		/* print empty and read-only info */
		printf ("  %08lX %c %s ",
			info->start[i],
			erased ? 'E' : ' ',
			info->protect[i] ? "RO" : "  ");
	}
#else	/* ! CFG_FLASH_EMPTY_INFO */
		printf ("  %08lX   %s ",
			info->start[i],
			info->protect[i] ? "RO" : "  ");
#endif
	}
	putchar('\n');
	return;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_buff (flash_info_t * info, const uchar * src, ulong addr, ulong cnt)
{
	ulong wp;
	ulong cp;
	int aln;
	cfiword_t cword;
	int i, rc;

#ifdef CONFIG_CFI_BUFFER_WRITE
	int buffered_size;
#endif
	/* get lower aligned address */
	/* get lower aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/* handle unaligned start */
	if ((aln = addr - wp) != 0) {
		cword.l = 0;
		cp = wp;
		for (i = 0; i < aln; ++i, ++cp)
			flash_add_byte (info, &cword, (*(uchar *) cp));

		for (; (i < info->portwidth) && (cnt > 0); i++) {
			flash_add_byte (info, &cword, *src++);
			cnt--;
			cp++;
		}
		for (; (cnt == 0) && (i < info->portwidth); ++i, ++cp)
			flash_add_byte (info, &cword, (*(uchar *) cp));
		if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
			return rc;
		wp = cp;
	}

	/* handle the aligned part */
#ifdef CONFIG_CFI_BUFFER_WRITE
	buffered_size = (info->portwidth / info->chipwidth);
	buffered_size *= info->buffer_size;
	while (cnt >= info->portwidth) {
		/* prohibit buffer write when buffer_size is 1 */
		if (info->buffer_size == 1) {
			cword.l = 0;
			for (i = 0; i < info->portwidth; i++)
				flash_add_byte (info, &cword, *src++);
			if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
				return rc;
			wp += info->portwidth;
			cnt -= info->portwidth;
			continue;
		}

		/* write buffer until next buffered_size aligned boundary */
		i = buffered_size - (wp % buffered_size);
		if (i > cnt)
			i = cnt;
		if ((rc = flash_write_cfibuffer (info, wp, src, i)) != ERR_OK)
			return rc;
		i -= i & (info->portwidth - 1);
		wp += i;
		src += i;
		cnt -= i;
	}
#else
	while (cnt >= info->portwidth) {
		cword.l = 0;
		for (i = 0; i < info->portwidth; i++) {
			flash_add_byte (info, &cword, *src++);
		}
		if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
			return rc;
		wp += info->portwidth;
		cnt -= info->portwidth;
	}
#endif /* CONFIG_CFI_BUFFER_WRITE */
	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	cword.l = 0;
	for (i = 0, cp = wp; (i < info->portwidth) && (cnt > 0); ++i, ++cp) {
		flash_add_byte (info, &cword, *src++);
		--cnt;
	}
	for (; i < info->portwidth; ++i, ++cp) {
		flash_add_byte (info, &cword, (*(uchar *) cp));
	}

	return flash_write_cfiword (info, wp, cword);
}

static int flash_real_protect (flash_info_t * info, long sector, int prot)
{
	int retcode = 0;

	flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT);
	if (prot)
		flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT_SET);
	else
		flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT_CLEAR);

	if ((retcode =
	     flash_full_status_check (info, sector, info->erase_blk_tout,
				      prot ? "protect" : "unprotect")) == 0) {

		info->protect[sector] = prot;

		/*
		 * On some of Intel's flash chips (marked via legacy_unlock)
		 * unprotect unprotects all locking.
		 */
		if ((prot == 0) && (info->legacy_unlock)) {
			flash_sect_t i;

			for (i = 0; i < info->sector_count; i++) {
				if (info->protect[i])
					flash_real_protect (info, i, 1);
			}
		}
	}
	return retcode;
}

#if 0
/*-----------------------------------------------------------------------
 * flash_read_user_serial - read the OneTimeProgramming cells
 */
static void flash_read_user_serial (flash_info_t * info, void *buffer, int offset,
			     int len)
{
	uchar *src;
	uchar *dst;

	dst = buffer;
	src = flash_make_addr (info, 0, FLASH_OFFSET_USER_PROTECTION);
	flash_write_cmd (info, 0, 0, FLASH_CMD_READ_ID);
	memcpy (dst, src + offset, len);
	flash_write_cmd (info, 0, 0, info->cmd_reset);
}

/*
 * flash_read_factory_serial - read the device Id from the protection area
 */
static void flash_read_factory_serial (flash_info_t * info, void *buffer, int offset,
				int len)
{
	uchar *src;

	src = flash_make_addr (info, 0, FLASH_OFFSET_INTEL_PROTECTION);
	flash_write_cmd (info, 0, 0, FLASH_CMD_READ_ID);
	memcpy (buffer, src + offset, len);
	flash_write_cmd (info, 0, 0, info->cmd_reset);
}
#endif


/*
 * flash_is_busy - check to see if the flash is busy
 * This routine checks the status of the chip and returns true if the chip is busy
 */
static int flash_is_busy (flash_info_t * info, flash_sect_t sect)
{
	int retval;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		retval = !flash_isset (info, sect, 0, FLASH_STATUS_DONE);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		retval = flash_toggle (info, sect, 0, AMD_STATUS_TOGGLE);
		break;
	default:
		retval = 0;
	}
	debug ("flash_is_busy: %d\n", retval);
	return retval;
}

/*-----------------------------------------------------------------------
 *  wait for XSR.7 to be set. Time out with an error if it does not.
 *  This routine does not set the flash to read-array mode.
 */
static int flash_status_check (flash_info_t * info, flash_sect_t sector,
			       uint64_t tout, char *prompt)
{
	uint64_t start;

	tout *= 1000000;

	/* Wait for command completion */
	start = get_time_ns();
	while (flash_is_busy (info, sector)) {
		if (is_timeout(start, tout)) {
			printf ("Flash %s timeout at address %lx data %lx\n",
				prompt, info->start[sector],
				flash_read_long (info, sector, 0));
			flash_write_cmd (info, sector, 0, info->cmd_reset);
			return ERR_TIMOUT;
		}
		udelay (1);		/* also triggers watchdog */
	}
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Wait for XSR.7 to be set, if it times out print an error, otherwise do a full status check.
 * This routine sets the flash to read-array mode.
 */
static int flash_full_status_check (flash_info_t * info, flash_sect_t sector,
				    uint64_t tout, char *prompt)
{
	int retcode;

	retcode = flash_status_check (info, sector, tout, prompt);
	switch (info->vendor) {
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		if ((retcode == ERR_OK)
		    && !flash_isequal (info, sector, 0, FLASH_STATUS_DONE)) {
			retcode = ERR_INVAL;
			printf ("Flash %s error at address %lx\n", prompt,
				info->start[sector]);
			if (flash_isset (info, sector, 0, FLASH_STATUS_ECLBS | FLASH_STATUS_PSLBS)) {
				puts ("Command Sequence Error.\n");
			} else if (flash_isset (info, sector, 0, FLASH_STATUS_ECLBS)) {
				puts ("Block Erase Error.\n");
				retcode = ERR_NOT_ERASED;
			} else if (flash_isset (info, sector, 0, FLASH_STATUS_PSLBS)) {
				puts ("Locking Error\n");
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_DPS)) {
				puts ("Block locked.\n");
				retcode = ERR_PROTECTED;
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_VPENS))
				puts ("Vpp Low Error.\n");
		}
		flash_write_cmd (info, sector, 0, info->cmd_reset);
		break;
	default:
		break;
	}
	return retcode;
}

/*-----------------------------------------------------------------------
 */
static void flash_add_byte (flash_info_t * info, cfiword_t * cword, uchar c)
{
#if defined(__LITTLE_ENDIAN)
	unsigned short	w;
	unsigned int	l;
	unsigned long long ll;
#endif

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cword->c = c;
		break;
	case FLASH_CFI_16BIT:
#if defined(__LITTLE_ENDIAN)
		w = c;
		w <<= 8;
		cword->w = (cword->w >> 8) | w;
#else
		cword->w = (cword->w << 8) | c;
#endif
		break;
	case FLASH_CFI_32BIT:
#if defined(__LITTLE_ENDIAN)
		l = c;
		l <<= 24;
		cword->l = (cword->l >> 8) | l;
#else
		cword->l = (cword->l << 8) | c;
#endif
		break;
	case FLASH_CFI_64BIT:
#if defined(__LITTLE_ENDIAN)
		ll = c;
		ll <<= 56;
		cword->ll = (cword->ll >> 8) | ll;
#else
		cword->ll = (cword->ll << 8) | c;
#endif
		break;
	}
}


/*-----------------------------------------------------------------------
 * make a proper sized command based on the port and chip widths
 */
static void flash_make_cmd (flash_info_t * info, uchar cmd, void *cmdbuf)
{
	int i;
	uchar *cp = (uchar *) cmdbuf;

#if defined(__LITTLE_ENDIAN)
	for (i = info->portwidth; i > 0; i--)
#else
	for (i = 1; i <= info->portwidth; i++)
#endif
		*cp++ = (i & (info->chipwidth - 1)) ? '\0' : cmd;
}

/*
 * Write a proper sized command to the correct address
 */
static void flash_write_cmd (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{

	volatile cfiptr_t addr;
	cfiword_t cword;

	addr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		debug("fwc addr %p cmd %x %x 8bit x %d bit\n", addr.cp, cmd,
		       cword.c, info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		*addr.cp = cword.c;
#ifdef CONFIG_BLACKFIN
		asm("ssync;");
#endif
		break;
	case FLASH_CFI_16BIT:
		debug("fwc addr %p cmd %x %4.4x 16bit x %d bit\n", addr.wp,
		       cmd, cword.w,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		*addr.wp = cword.w;
#ifdef CONFIG_BLACKFIN
		asm("ssync;");
#endif
		break;
	case FLASH_CFI_32BIT:
		debug("fwc addr %p cmd %x %8.8lx 32bit x %d bit\n", addr.lp,
		       cmd, cword.l,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		*addr.lp = cword.l;
#ifdef CONFIG_BLACKFIN
		asm("ssync;");
#endif
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG
		{
			char str[20];

			print_longlong (str, cword.ll);

			debug ("fwrite addr %p cmd %x %s 64 bit x %d bit\n",
			       addr.llp, cmd, str,
			       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		}
#endif
		*addr.llp = cword.ll;
#ifdef CONFIG_BLACKFIN
		asm("ssync;");
#endif
		break;
	}
}

static void flash_unlock_seq (flash_info_t * info, flash_sect_t sect)
{
	flash_write_cmd (info, sect, AMD_ADDR_START, AMD_CMD_UNLOCK_START);
	flash_write_cmd (info, sect, AMD_ADDR_ACK, AMD_CMD_UNLOCK_ACK);
}

/*-----------------------------------------------------------------------
 */
static int flash_isequal (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;

	cptr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);

	debug ("is= cmd %x(%c) addr %p ", cmd, cmd, cptr.cp);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		debug ("is= %x %x\n", cptr.cp[0], cword.c);
		retval = (cptr.cp[0] == cword.c);
		break;
	case FLASH_CFI_16BIT:
		debug ("is= %4.4x %4.4x\n", cptr.wp[0], cword.w);
		retval = (cptr.wp[0] == cword.w);
		break;
	case FLASH_CFI_32BIT:
		debug ("is= %8.8lx %8.8lx\n", cptr.lp[0], cword.l);
		retval = (cptr.lp[0] == cword.l);
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG
		{
			char str1[20];
			char str2[20];

			print_longlong (str1, cptr.llp[0]);
			print_longlong (str2, cword.ll);
			debug ("is= %s %s\n", str1, str2);
		}
#endif
		retval = (cptr.llp[0] == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_isset (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;

	cptr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((cptr.cp[0] & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		retval = ((cptr.wp[0] & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		retval = ((cptr.lp[0] & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		retval = ((cptr.llp[0] & cword.ll) == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_toggle (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;

	cptr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((cptr.cp[0] & cword.c) != (cptr.cp[0] & cword.c));
		break;
	case FLASH_CFI_16BIT:
		retval = ((cptr.wp[0] & cword.w) != (cptr.wp[0] & cword.w));
		break;
	case FLASH_CFI_32BIT:
		retval = ((cptr.lp[0] & cword.l) != (cptr.lp[0] & cword.l));
		break;
	case FLASH_CFI_64BIT:
		retval = ((cptr.llp[0] & cword.ll) !=
			  (cptr.llp[0] & cword.ll));
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 * read jedec ids from device and set corresponding fields in info struct
 *
 * Note: assume cfi->vendor, cfi->portwidth and cfi->chipwidth are correct
 *
*/
static void flash_read_jedec_ids (flash_info_t * info)
{
	info->manufacturer_id = 0;
	info->device_id       = 0;
	info->device_id2      = 0;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
		flash_write_cmd(info, 0, 0, FLASH_CMD_READ_ID);
		udelay(1000); /* some flash are slow to respond */
		info->manufacturer_id = flash_read_uchar (info,
						FLASH_OFFSET_MANUFACTURER_ID);
		info->device_id = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID);
		flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
		flash_unlock_seq(info, 0);
		flash_write_cmd(info, 0, AMD_ADDR_START, FLASH_CMD_READ_ID);
		udelay(1000); /* some flash are slow to respond */
		info->manufacturer_id = flash_read_uchar (info,
						FLASH_OFFSET_MANUFACTURER_ID);
		info->device_id = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID);
		if (info->device_id == 0x7E) {
			/* AMD 3-byte (expanded) device ids */
			info->device_id2 = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID2);
			info->device_id2 <<= 8;
			info->device_id2 |= flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID3);
		}
		flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
		break;
	default:
		break;
	}
}

/*-----------------------------------------------------------------------
 * detect if flash is compatible with the Common Flash Interface (CFI)
 * http://www.jedec.org/download/search/jesd68.pdf
 *
*/
static int flash_detect_cfi (flash_info_t * info)
{
	int cfi_offset;
	debug ("flash detect cfi\n");

	for (info->portwidth = CFG_FLASH_CFI_WIDTH;
	     info->portwidth <= FLASH_CFI_64BIT; info->portwidth <<= 1) {
		for (info->chipwidth = FLASH_CFI_BY8;
		     info->chipwidth <= info->portwidth;
		     info->chipwidth <<= 1) {
			flash_write_cmd (info, 0, 0, info->cmd_reset);
			for (cfi_offset=0; cfi_offset < sizeof(flash_offset_cfi)/sizeof(uint); cfi_offset++) {
				flash_write_cmd (info, 0, flash_offset_cfi[cfi_offset], FLASH_CMD_CFI);
				if (flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP, 'Q')
				 && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP + 1, 'R')
				 && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP + 2, 'Y')) {
					info->interface = flash_read_ushort (info, 0, FLASH_OFFSET_INTERFACE);
					info->cfi_offset=flash_offset_cfi[cfi_offset];
					debug ("device interface is %d\n",
						info->interface);
					debug ("found port %d chip %d ",
						info->portwidth, info->chipwidth);
					debug ("port %d bits chip %d bits\n",
						info->portwidth << CFI_FLASH_SHIFT_WIDTH,
						info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
					return 1;
				}
			}
		}
	}
	debug ("not found\n");
	return 0;
}

/*
 * The following code cannot be run from FLASH!
 *
 */
static ulong flash_get_size (flash_info_t *info, ulong base)
{
	int i, j;
	flash_sect_t sect_cnt;
	unsigned long sector;
	unsigned long tmp;
	int size_ratio;
	uchar num_erase_regions;
	int erase_region_size;
	int erase_region_count;
	int geometry_reversed = 0;

	info->ext_addr = 0;
	info->cfi_version = 0;
	info->legacy_unlock = 0;

	/* first only malloc space for the first sector */
	info->start = malloc(sizeof(ulong));

	info->start[0] = base;
	info->protect = 0;

	if (flash_detect_cfi (info)) {
		info->vendor = flash_read_ushort (info, 0,
					FLASH_OFFSET_PRIMARY_VENDOR);
		flash_read_jedec_ids (info);
		flash_write_cmd (info, 0, info->cfi_offset, FLASH_CMD_CFI);
		num_erase_regions = flash_read_uchar (info,
					FLASH_OFFSET_NUM_ERASE_REGIONS);
		info->ext_addr = flash_read_ushort (info, 0,
					FLASH_OFFSET_EXT_QUERY_T_P_ADDR);
		if (info->ext_addr) {
			info->cfi_version = (ushort) flash_read_uchar (info,
						info->ext_addr + 3) << 8;
			info->cfi_version |= (ushort) flash_read_uchar (info,
						info->ext_addr + 4);
		}
#ifdef DEBUG
		flash_printqry (info, 0);
#endif
		switch (info->vendor) {
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
		default:
			info->cmd_reset = FLASH_CMD_RESET;
			/* read legacy lock/unlock bit from intel flash */
			if (info->ext_addr) {
				info->legacy_unlock = flash_read_uchar (info,
						info->ext_addr + 5) & 0x08;
			}
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
			info->cmd_reset = AMD_CMD_RESET;
			/* check if flash geometry needs reversal */
			if (num_erase_regions <= 1)
				break;
			/* reverse geometry if top boot part */
			if (info->cfi_version < 0x3131) {
				/* CFI < 1.1, try to guess from device id */
				if ((info->device_id & 0x80) != 0) {
					geometry_reversed = 1;
				}
				break;
			}
			/* CFI >= 1.1, deduct from top/bottom flag */
			/* note: ext_addr is valid since cfi_version > 0 */
			if (flash_read_uchar(info, info->ext_addr + 0xf) == 3) {
				geometry_reversed = 1;
			}
			break;
		}

		debug ("manufacturer is %d\n", info->vendor);
		debug ("manufacturer id is 0x%x\n", info->manufacturer_id);
		debug ("device id is 0x%x\n", info->device_id);
		debug ("device id2 is 0x%x\n", info->device_id2);
		debug ("cfi version is 0x%04x\n", info->cfi_version);

		size_ratio = info->portwidth / info->chipwidth;
		/* if the chip is x8/x16 reduce the ratio by half */
		if ((info->interface == FLASH_CFI_X8X16)
		    && (info->chipwidth == FLASH_CFI_BY8)) {
			size_ratio >>= 1;
		}
		debug ("size_ratio %d port %d bits chip %d bits\n",
		       size_ratio, info->portwidth << CFI_FLASH_SHIFT_WIDTH,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		debug ("found %d erase regions\n", num_erase_regions);
		sect_cnt = 0;
		sector = base;

		for (i = 0; i < num_erase_regions; i++) {
			if (i > NUM_ERASE_REGIONS) {
				printf ("%d erase regions found, only %d used\n",
					num_erase_regions, NUM_ERASE_REGIONS);
				break;
			}
			if (geometry_reversed)
				tmp = flash_read_long (info, 0,
					       FLASH_OFFSET_ERASE_REGIONS +
					       (num_erase_regions - 1 - i) * 4);
			else
				tmp = flash_read_long (info, 0,
					       FLASH_OFFSET_ERASE_REGIONS +
					       i * 4);
			erase_region_size =
				(tmp & 0xffff) ? ((tmp & 0xffff) * 256) : 128;
			tmp >>= 16;
			erase_region_count = (tmp & 0xffff) + 1;
			debug ("erase_region_count = %d erase_region_size = %d\n",
				erase_region_count, erase_region_size);

			/* increase the space malloced for the sector start addresses */
			info->start = realloc(info->start, sizeof(ulong) * (erase_region_count + sect_cnt));
			info->protect = realloc(info->protect, sizeof(ulong) * (erase_region_count + sect_cnt));

			for (j = 0; j < erase_region_count; j++) {
				info->start[sect_cnt] = sector;
				sector += (erase_region_size * size_ratio);

				/*
				 * Only read protection status from supported devices (intel...)
				 */
				switch (info->vendor) {
				case CFI_CMDSET_INTEL_EXTENDED:
				case CFI_CMDSET_INTEL_STANDARD:
					info->protect[sect_cnt] =
						flash_isset (info, sect_cnt,
							     FLASH_OFFSET_PROTECT,
							     FLASH_STATUS_PROTECT);
					break;
				default:
					info->protect[sect_cnt] = 0; /* default: not protected */
				}

				sect_cnt++;
			}
		}

		info->sector_count = sect_cnt;
		/* multiply the size by the number of chips */
		info->size = (1 << flash_read_uchar (info, FLASH_OFFSET_SIZE)) * size_ratio;
		info->buffer_size = (1 << flash_read_ushort (info, 0, FLASH_OFFSET_BUFFER_SIZE));
		tmp = 1 << flash_read_uchar (info, FLASH_OFFSET_ETOUT);
		info->erase_blk_tout = (tmp * (1 << flash_read_uchar (info, FLASH_OFFSET_EMAX_TOUT)));
		tmp = (1 << flash_read_uchar (info, FLASH_OFFSET_WBTOUT)) *
			(1 << flash_read_uchar (info, FLASH_OFFSET_WBMAX_TOUT));
		info->buffer_write_tout = tmp * 1000;
		tmp = (1 << flash_read_uchar (info, FLASH_OFFSET_WTOUT)) *
		      (1 << flash_read_uchar (info, FLASH_OFFSET_WMAX_TOUT));
		info->write_tout = tmp * 1000;
		info->flash_id = FLASH_MAN_CFI;
		if ((info->interface == FLASH_CFI_X8X16) && (info->chipwidth == FLASH_CFI_BY8)) {
			info->portwidth >>= 1;	/* XXX - Need to test on x8/x16 in parallel. */
		}
	}

	flash_write_cmd (info, 0, 0, info->cmd_reset);
	return (info->size);
}

/* loop through the sectors from the highest address
 * when the passed address is greater or equal to the sector address
 * we have a match
 */
static flash_sect_t find_sector (flash_info_t * info, ulong addr)
{
	flash_sect_t sector;

	for (sector = info->sector_count - 1; sector >= 0; sector--) {
		if (addr >= info->start[sector])
			break;
	}
	return sector;
}

/*-----------------------------------------------------------------------
 */
static int flash_write_cfiword (flash_info_t * info, ulong dest,
				cfiword_t cword)
{
	cfiptr_t ctladdr;
	cfiptr_t cptr;
	int flag;

	ctladdr.cp = flash_make_addr (info, 0, 0);
	cptr.cp = (uchar *) dest;


	/* Check if Flash is (sufficiently) erased */
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flag = ((cptr.cp[0] & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		flag = ((cptr.wp[0] & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		flag = ((cptr.lp[0] & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		flag = ((cptr.llp[0] & cword.ll) == cword.ll);
		break;
	default:
		return 2;
	}
	if (!flag)
		return 2;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		flash_write_cmd (info, 0, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, 0, 0, FLASH_CMD_WRITE);
		break;
	case CFI_CMDSET_AMD_EXTENDED:
	case CFI_CMDSET_AMD_STANDARD:
		flash_unlock_seq (info, 0);
		flash_write_cmd (info, 0, AMD_ADDR_START, AMD_CMD_WRITE);
		break;
	}

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cptr.cp[0] = cword.c;
		break;
	case FLASH_CFI_16BIT:
		cptr.wp[0] = cword.w;
		break;
	case FLASH_CFI_32BIT:
		cptr.lp[0] = cword.l;
		break;
	case FLASH_CFI_64BIT:
		cptr.llp[0] = cword.ll;
		break;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	return flash_full_status_check (info, find_sector (info, dest),
					info->write_tout, "write");
}

#ifdef CONFIG_CFI_BUFFER_WRITE

static int flash_write_cfibuffer (flash_info_t * info, ulong dest, const uchar * cp,
				  int len)
{
	flash_sect_t sector;
	int cnt;
	int retcode;
	volatile cfiptr_t src;
	volatile cfiptr_t dst;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		src.cp = (uchar *)cp;
		dst.cp = (uchar *) dest;
		sector = find_sector (info, dest);
		flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, sector, 0, FLASH_CMD_WRITE_TO_BUFFER);
		if ((retcode = flash_status_check (info, sector, info->buffer_write_tout,
						   "write to buffer")) == ERR_OK) {
			/* reduce the number of loops by the width of the port	*/
			switch (info->portwidth) {
			case FLASH_CFI_8BIT:
				cnt = len;
				break;
			case FLASH_CFI_16BIT:
				cnt = len >> 1;
				break;
			case FLASH_CFI_32BIT:
				cnt = len >> 2;
				break;
			case FLASH_CFI_64BIT:
				cnt = len >> 3;
				break;
			default:
				return ERR_INVAL;
				break;
			}
			flash_write_cmd (info, sector, 0, (uchar) cnt - 1);
			while (cnt-- > 0) {
				switch (info->portwidth) {
				case FLASH_CFI_8BIT:
					*dst.cp++ = *src.cp++;
					break;
				case FLASH_CFI_16BIT:
					*dst.wp++ = *src.wp++;
					break;
				case FLASH_CFI_32BIT:
					*dst.lp++ = *src.lp++;
					break;
				case FLASH_CFI_64BIT:
					*dst.llp++ = *src.llp++;
					break;
				default:
					return ERR_INVAL;
					break;
				}
			}
			flash_write_cmd (info, sector, 0,
					 FLASH_CMD_WRITE_BUFFER_CONFIRM);
			retcode = flash_full_status_check (info, sector,
							   info->buffer_write_tout,
							   "buffer write");
		}
		return retcode;

	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		src.cp = (uchar *)cp;
		dst.cp = (uchar *) dest;
		sector = find_sector (info, dest);

		flash_unlock_seq(info,0);
		flash_write_cmd (info, sector, 0, AMD_CMD_WRITE_TO_BUFFER);

		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			cnt = len;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) *dst.cp++ = *src.cp++;
			break;
		case FLASH_CFI_16BIT:
			cnt = len >> 1;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) *dst.wp++ = *src.wp++;
			break;
		case FLASH_CFI_32BIT:
			cnt = len >> 2;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) *dst.lp++ = *src.lp++;
			break;
		case FLASH_CFI_64BIT:
			cnt = len >> 3;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) *dst.llp++ = *src.llp++;
			break;
		default:
			return ERR_INVAL;
		}

		flash_write_cmd (info, sector, 0, AMD_CMD_WRITE_BUFFER_CONFIRM);
		retcode = flash_full_status_check (info, sector, info->buffer_write_tout,
						   "buffer write");
		return retcode;

	default:
		debug ("Unknown Command Set\n");
		return ERR_INVAL;
	}
}
#endif /* CONFIG_CFI_BUFFER_WRITE */

struct file_operations cfi_ops = {
	.read    = mem_read,
	.write   = cfi_write,
	.lseek  = dev_lseek_default,
	.erase   = cfi_erase,
	.protect = cfi_protect,
	.memmap  = generic_memmap_ro,
};

static int cfi_probe (struct device_d *dev)
{
	unsigned long size = 0;
	flash_info_t *info = xzalloc(sizeof(flash_info_t));

	dev->priv = (void *)info;

	printf("cfi_probe: %s base: 0x%08x size: 0x%08x\n", dev->name, dev->map_base, dev->size);

	/* Init: no FLASHes known */
	info->flash_id = FLASH_UNKNOWN;
	size += info->size = flash_get_size(info, dev->map_base);
	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank at 0x%08x - Size = 0x%08lx = %ld MB\n",
			dev->map_base, info->size, info->size << 20);
		return -ENODEV;
	}

	info->cdev.name = asprintf("nor%d", dev->id);
	info->cdev.size = info->size;
	info->cdev.dev = dev;
	info->cdev.ops = &cfi_ops;
	info->cdev.priv = info;
	devfs_create(&info->cdev);

	return 0;
}

static struct driver_d cfi_driver = {
	.name   = "cfi_flash",
	.probe  = cfi_probe,
	.info   = cfi_info,
};

static int cfi_init(void)
{
	return register_driver(&cfi_driver);
}

device_initcall(cfi_init);

/**
 * @file
 * @brief This file implements a Common Flash Interface (CFI) driver for barebox.
 *
 * This file implements a Common Flash Interface (CFI) driver for barebox.
 * The width of the port and the width of the chips are determined at initialization.
 * These widths are used to calculate the address for access CFI data structures.
 *
 * References
 *
 * - JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * - JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * - Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * - Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 * - AMD CFI Specification, Release 2.0 December 1, 2001
 * - AMD/Spansion Application Note: Migration from Single-byte to Three-byte
 *    Device IDs, Publication Number 25538 Revision A, November 8, 2001
 *
 */
