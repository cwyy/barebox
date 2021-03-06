/*
 * cp.c - copy files
 *
 * Copyright (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
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

/**
 * @file
 * @brief cp: copy file command
 */
#include <common.h>
#include <command.h>
#include <xfuncs.h>
#include <linux/stat.h>
#include <libbb.h>
#include <fs.h>
#include <malloc.h>

/**
 * @param[in] cmdtp FIXME
 * @param[in] argc Argument count from command line
 * @param[in] argv List of input arguments
 */
static int do_cp ( cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	int ret = 1;
	struct stat statbuf;
	int last_is_dir = 0;
	int i;

	if (argc < 3)
		return COMMAND_ERROR_USAGE;

	if (!stat(argv[argc - 1], &statbuf)) {
		if (S_ISDIR(statbuf.st_mode))
			last_is_dir = 1;
	}
	
	if (argc > 3 && !last_is_dir) {
		printf("cp: target `%s' is not a directory\n", argv[argc - 1]);
		return 1;
	}

	for (i = 1; i < argc - 1; i++) {
		if (last_is_dir) {
			char *dst;
			dst = concat_path_file(argv[argc - 1], argv[i]);
			ret = copy_file(argv[i], dst);
			if (ret)
				goto out;
			free(dst);
		} else {
			ret = copy_file(argv[i], argv[argc - 1]);
			if (ret)
				goto out;
		}
	}

	ret = 0;
out:
	return ret;
}

static const __maybe_unused char cmd_cp_help[] =
"Usage: cp <source> <destination>\n"
"cp copies file <source> to <destination>.\n"
"Currently only this form is supported and you have to specify the exact target\n"
"filename (not a target directory).\n"
"This command is file based only. See memcpy for memory copy\n";

BAREBOX_CMD_START(cp)
	.cmd		= do_cp,
	.usage		= "copy files",
	BAREBOX_CMD_HELP(cmd_cp_help)
BAREBOX_CMD_END

/**
 * @page cp_command cp: Copy file
 *
 * Usage: cp \<source> [\<source>] \<destination>
 *
 * \c cp copies file \<source> to \<destination>
 *
 * Currently only this form is supported and you have to specify the exact
 * target filename (not a target directory).\n
 * This command is file based only. See memcpy for generic memory copy
 */
