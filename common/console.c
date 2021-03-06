/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
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
 */

#include <config.h>
#include <common.h>
#include <stdarg.h>
#include <malloc.h>
#include <param.h>
#include <console.h>
#include <driver.h>
#include <fs.h>
#include <reloc.h>
#include <init.h>
#include <clock.h>
#include <kfifo.h>
#include <module.h>
#include <linux/list.h>

LIST_HEAD(console_list);
EXPORT_SYMBOL(console_list);

#define CONSOLE_UNINITIALIZED	0
#define CONSOLE_INIT_EARLY	1
#define CONSOLE_INIT_FULL	2

extern char version_string[];

static void display_banner (void)
{
	printf (RELOC("\n\n%s\n\n"), RELOC_VAR(version_string));
	printf(RELOC("Board: " CONFIG_BOARDINFO "\n"));
}

static int __initdata initialized = 0;

static int console_std_set(struct device_d *dev, struct param_d *param,
		const char *val)
{
	struct console_device *cdev = dev->type_data;
	unsigned int flag = 0, i = 0;

	if (strchr(val, 'i') && cdev->f_caps & CONSOLE_STDIN) {
		cdev->active[i++] = 'i';
		flag |= CONSOLE_STDIN;
	}

	if (strchr(val, 'o') && cdev->f_caps & CONSOLE_STDOUT) {
		cdev->active[i++] = 'o';
		flag |= CONSOLE_STDOUT;
	}

	if (strchr(val, 'e') && cdev->f_caps & CONSOLE_STDERR) {
		cdev->active[i++] = 'e';
		flag |= CONSOLE_STDERR;
	}

	cdev->active[i] = 0;
	cdev->f_active = flag;

	return 0;
}

static int console_baudrate_set(struct device_d *dev, struct param_d *param,
		const char *val)
{
	struct console_device *cdev = dev->type_data;
	int baudrate;
	unsigned char c;

	baudrate = simple_strtoul(val, NULL, 10);

	if (cdev->f_active) {
		printf("## Switch baudrate to %d bps and press ENTER ...\n",
			baudrate);
		mdelay(50);
		cdev->setbrg(cdev, baudrate);
		mdelay(50);
		do {
			c = getc();
		} while (c != '\r' && c != '\n');
	} else
		cdev->setbrg(cdev, baudrate);

	sprintf(cdev->baudrate_string, "%d", baudrate);

	return 0;
}

static struct kfifo *console_input_buffer;
static struct kfifo *console_output_buffer;

int getc_buffer_flush(void)
{
	console_input_buffer = kfifo_alloc(1024);
	console_output_buffer = kfifo_alloc(1024);
	return 0;
}

postcore_initcall(getc_buffer_flush);

int console_register(struct console_device *newcdev)
{
	struct device_d *dev = &newcdev->class_dev;
	int first = 0;
	char ch;

	strcpy(dev->name, "cs");
	dev->type_data = newcdev;
	register_device(dev);

	if (newcdev->setbrg) {
		newcdev->baudrate_param.set = console_baudrate_set;
		newcdev->baudrate_param.name = "baudrate";
		sprintf(newcdev->baudrate_string, "%d",
			CONFIG_BAUDRATE);
		console_baudrate_set(dev, &newcdev->baudrate_param,
			newcdev->baudrate_string);
		newcdev->baudrate_param.value = newcdev->baudrate_string;
		dev_add_param(dev, &newcdev->baudrate_param);
	}

	newcdev->active_param.set = console_std_set;
	newcdev->active_param.name  = "active";
	newcdev->active_param.value = newcdev->active;
	dev_add_param(dev, &newcdev->active_param);

	initialized = CONSOLE_INIT_FULL;
#ifdef CONFIG_CONSOLE_ACTIVATE_ALL
	console_std_set(dev, &newcdev->active_param, "ioe");
#endif
#ifdef CONFIG_CONSOLE_ACTIVATE_FIRST
	if (list_empty(&console_list)) {
		first = 1;
		console_std_set(dev, &newcdev->active_param, "ioe");
	}
#endif

	list_add_tail(&newcdev->list, &console_list);

	if (console_output_buffer) {
		while (kfifo_getc(console_output_buffer, &ch) == 0)
			console_putc(CONSOLE_STDOUT, ch);
		kfifo_free(console_output_buffer);
		console_output_buffer = NULL;
	}

#ifndef CONFIG_HAS_EARLY_INIT
	if (first)
		display_banner();
#endif

	return 0;
}
EXPORT_SYMBOL(console_register);

static int getc_raw(void)
{
	struct console_device *cdev;
	int active = 0;

	while (1) {
		for_each_console(cdev) {
			if (!(cdev->f_active & CONSOLE_STDIN))
				continue;
			active = 1;
			if (cdev->tstc(cdev))
				return cdev->getc(cdev);
		}
		if (!active)
			/* no active console found. bail out */
			return -1;
	}
}

int getc(void)
{
	unsigned char ch;
	uint64_t start;

	/*
	 * For 100us we read the characters from the serial driver
	 * into a kfifo. This helps us not to lose characters
	 * in small hardware fifos.
	 */
	start = get_time_ns();
	while (1) {
		if (tstc()) {
			kfifo_putc(console_input_buffer, getc_raw());

			start = get_time_ns();
		}
		if (is_timeout(start, 100 * USECOND) &&
				kfifo_len(console_input_buffer))
			break;
	}

	kfifo_getc(console_input_buffer, &ch);
	return ch;
}
EXPORT_SYMBOL(getc);

int fgetc(int fd)
{
	char c;

	if (!fd)
		return getc();
	return read(fd, &c, 1);
}
EXPORT_SYMBOL(fgetc);

int tstc(void)
{
	struct console_device *cdev;

	for_each_console(cdev) {
		if (!(cdev->f_active & CONSOLE_STDIN))
			continue;
		if (cdev->tstc(cdev))
			return 1;
	}

	return 0;
}
EXPORT_SYMBOL(tstc);

void __initdata *early_console_base;

void console_putc(unsigned int ch, char c)
{
	struct console_device *cdev;
	int init = INITDATA(initialized);

	switch (init) {
	case CONSOLE_UNINITIALIZED:
		kfifo_putc(console_output_buffer, c);
		return;

#ifdef CONFIG_HAS_EARLY_INIT
	case CONSOLE_INIT_EARLY:
		early_console_putc(INITDATA(early_console_base), c);
		return;
#endif

	case CONSOLE_INIT_FULL:
		for_each_console(cdev) {
			if (cdev->f_active & ch) {
				if (c == '\n')
					cdev->putc(cdev, '\r');
				cdev->putc(cdev, c);
			}
		}
		return;
	default:
		/* If we have problems inititalizing our data
		 * get them early
		 */
		hang();
	}
}
EXPORT_SYMBOL(console_putc);

int fputc(int fd, char c)
{
	if(list_empty(&console_list)) {
		if(!fd)
			console_putc(0, c);
		return 0;
	}

	if (fd == 1)
		putchar(c);
	else if (fd == 2)
		eputc(c);
	else
		return write(fd, &c, 1);
	return 0;
}
EXPORT_SYMBOL(fputc);

void console_puts(unsigned int ch, const char *str)
{
	const char *s = str;
	while (*s) {
		if (*s == '\n')
			console_putc(ch, '\r');
		console_putc(ch, *s);
		s++;
	}
}
EXPORT_SYMBOL(console_puts);

int fputs(int fd, const char *s)
{
	if (fd == 1)
		puts(s);
	else if (fd == 2)
		eputs(s);
	else
		return write(fd, s, strlen(s));
	return 0;
}
EXPORT_SYMBOL(fputs);

void console_flush(void)
{
	struct console_device *cdev;

	for_each_console(cdev) {
		if (cdev->flush)
			cdev->flush(cdev);
	}
}
EXPORT_SYMBOL(console_flush);

void fprintf (int file, const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CFG_PBSIZE];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	fputs (file, printbuffer);
}
EXPORT_SYMBOL(fprintf);

int printf (const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CFG_PBSIZE];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	puts (printbuffer);

	return i;
}
EXPORT_SYMBOL(printf);

int vprintf (const char *fmt, va_list args)
{
	uint i;
	char printbuffer[CFG_PBSIZE];

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);

	/* Print the string */
	puts (printbuffer);

	return i;
}
EXPORT_SYMBOL(vprintf);

#ifndef ARCH_HAS_CTRLC
/* test if ctrl-c was pressed */
int ctrlc (void)
{
	if (tstc() && getc() == 3)
		return 1;
	return 0;
}
EXPORT_SYMBOL(ctrlc);
#endif /* ARCH_HAS_CTRC */

#ifdef CONFIG_HAS_EARLY_INIT

void early_console_start(const char *name, int baudrate)
{
	void *base = get_early_console_base(name);

	if (base) {
		early_console_init(base, baudrate);
		INITDATA(initialized) = CONSOLE_INIT_EARLY;
		INITDATA(early_console_base) = base;
		display_banner();
	}
}

#endif
