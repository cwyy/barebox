/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <asm-generic/barebox.lds.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = TEXT_BASE;

	PRE_IMAGE

	. = ALIGN(4);
	.text      :
	{
		_stext = .;
		_text = .;
		*(.text_entry*)
		*(.text_bare_init*)
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(.rodata*) }

	_etext = .;			/* End of text and rodata section */

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	.got : { *(.got*) }

	. = .;
	__barebox_cmd_start = .;
	.barebox_cmd : { BAREBOX_CMDS }
	__barebox_cmd_end = .;

	__barebox_initcalls_start = .;
	.barebox_initcalls : { INITCALLS }
	__barebox_initcalls_end = .;

	__usymtab_start = .;
	__usymtab : { BAREBOX_SYMS }
	__usymtab_end = .;

	. = ALIGN(4);
	__bss_start = .;
	.bss : { *(.bss*) }
	_end = .;
}
