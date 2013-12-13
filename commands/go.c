/*
 * go- execute some code anywhere (misc boot support)
 *
 * (C) Copyright 2000-2003
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
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <fcntl.h>
#include <linux/ctype.h>
#include <errno.h>

static int do_go (cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	void	*addr;
	int     rcode = 1;
	int	fd = -1;

	if (argc < 2)
		return COMMAND_ERROR_USAGE;

	if (!isdigit(*argv[1])) {
		fd = open(argv[1], O_RDONLY);
		if (fd < 0) {
			perror("open");
			goto out;
		}

		addr = memmap(fd, PROT_READ);
		if (addr == (void *)-1) {
			perror("memmap");
			goto out;
		}
	} else
		addr = (void *)simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting application at 0x%08lX ...\n", addr);

	console_flush();

#ifdef ARCH_HAS_EXECUTE
	rcode = arch_execute(addr, argc, &argv[1]);
#else
	rcode = ((ulong (*)(int, char *[]))addr) (--argc, &argv[1]);
#endif

	printf ("## Application terminated, rcode = 0x%lX\n", rcode);
out:
	if (fd > 0)
		close(fd);

	return rcode;
}

static const __maybe_unused char cmd_go_help[] =
"Usage: go addr [arg ...]\n"
"Start application at address 'addr' passing 'arg' as arguments.\n"
"If addr does not start with a digit it is interpreted as a filename\n"
"in which case the file is memmapped and executed\n";

BAREBOX_CMD_START(go)
	.cmd		= do_go,
	.usage		= "start application at address or file",
	BAREBOX_CMD_HELP(cmd_go_help)
BAREBOX_CMD_END
