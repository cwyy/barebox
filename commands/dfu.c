/*
 * dfu.c - device firmware update command
 *
 * Copyright (c) 2009 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
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
#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <getopt.h>
#include <fs.h>
#include <xfuncs.h>
#include <usb/dfu.h>

#define PARSE_DEVICE	0
#define PARSE_NAME	1
#define PARSE_FLAGS	2

static int dfu_do_parse_one(char *partstr, char **endstr, struct usb_dfu_dev *dfu)
{
	int i = 0, state = PARSE_DEVICE;
	char device[PATH_MAX];
	char name[PATH_MAX];

	memset(device, 0, sizeof(device));
	memset(name, 0, sizeof(name));
	dfu->flags = 0;

	while (*partstr && *partstr != ',') {
		switch (state) {
		case PARSE_DEVICE:
			if (*partstr == '(') {
				state = PARSE_NAME;
				i = 0;
			} else {
				device[i++] = *partstr;
			}
			break;
		case PARSE_NAME:
			if (*partstr == ')') {
				state = PARSE_FLAGS;
				i = 0;
			} else {
				name[i++] = *partstr;
			}
			break;
		case PARSE_FLAGS:
			switch (*partstr) {
			case 's':
				dfu->flags |= DFU_FLAG_SAVE;
				break;
			case 'r':
				dfu->flags |= DFU_FLAG_READBACK;
				break;
			default:
				return -EINVAL;
			}
			break;
		default:
			return -EINVAL;
		}
		partstr++;
	}

	if (state != PARSE_FLAGS)
		return -EINVAL;

	dfu->name = xstrdup(name);
	dfu->dev = xstrdup(device);
	if (*partstr == ',')
		partstr++;
	*endstr = partstr;

	return 0;
}

/* dfu /dev/self0(bootloader)sr,/dev/nand0.root.bb(root)
 *
 * s = save mode (download whole image before flashing)
 * r = read back (firmware image can be downloaded back from host)
 */
static int do_dfu(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	int opt, n = 0;
	struct usb_dfu_pdata pdata;
	char *endptr, *argstr;
	struct usb_dfu_dev *dfu_alts = NULL;
	char *manufacturer = "barebox";
	char *productname = CONFIG_BOARDINFO;
	u16 idVendor = 0, idProduct = 0;


	while((opt = getopt(argc, argv, "m:p:V:P:")) > 0) {
		switch(opt) {
		case 'm':
			manufacturer = optarg;
			break;
		case 'p':
			productname = optarg;
			break;
		case 'V':
			idVendor = simple_strtoul(optarg, NULL, 0);
			break;
		case 'P':
			idProduct = simple_strtoul(optarg, NULL, 0);
			break;
		}
	}

	if (argc != optind + 1)
		return COMMAND_ERROR_USAGE;

	argstr = argv[optind];

	if (!idProduct || !idVendor) {
		printf("productid or vendorid not given\n");
		return 1;
	}

	for (n = 0; *argstr; n++) {
		dfu_alts = xrealloc(dfu_alts, sizeof(*dfu_alts) * (n + 1));
		if (dfu_do_parse_one(argstr, &endptr, &dfu_alts[n])) {
			printf("parse error\n");
			goto out;
		}
		argstr = endptr;
	}

	pdata.alts = dfu_alts;
	pdata.num_alts = n;

	pdata.manufacturer = manufacturer;
	pdata.productname = productname;
	pdata.idVendor = idVendor;
	pdata.idProduct = idProduct;

	usb_dfu_register(&pdata);

out:
	while (n) {
		n--;
		free(dfu_alts[n].name);
		free(dfu_alts[n].dev);
	};
	free(dfu_alts);
	return 1;
}

static const __maybe_unused char cmd_dfu_help[] =
"Usage: dfu [OPTION]... description\n"
"start dfu firmware update\n"
" -m <str> Manufacturer string (barebox)\n"
" -p <str> product string (" CONFIG_BOARDINFO ")\n"
" -V <id>  vendor id\n"
" -P <id>  product id\n"
"description has the form\n"
"device1(name1)[sr],device2(name2)[sr]\n"
"where s is for save mode and r for read back of firmware\n";

BAREBOX_CMD_START(dfu)
	.cmd		= do_dfu,
	.usage		= "Device firmware update",
	BAREBOX_CMD_HELP(cmd_dfu_help)
BAREBOX_CMD_END
