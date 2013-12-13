/*
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
 * @brief A tiny editor implementation
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <fs.h>
#include <linux/ctype.h>
#include <fcntl.h>
#include <readkey.h>
#include <errno.h>
#include <xfuncs.h>
#include <linux/stat.h>

#define TABSPACE 8

struct line {
	int length;
	struct line *next;
	struct line *prev;
	char *data;
};

static struct line *buffer;

static struct line *lastscrline;

static int screenwidth  = 80;
static int screenheight = 25;

static int cursx  = 0;		/* position on screen */
static int cursy  = 0;

static int textx  = 0;		/* position in text */

static struct line *curline;	/* line where the cursor is */

static struct line *scrline;	/* the first line on screen */
int scrcol = 0;			/* the first column on screen */

static void pos(int x, int y)
{
	printf("%c[%d;%dH", 27, y + 1, x + 1);
}

static char *screenline(char *line, int *pos)
{
	int i, outpos = 0;
	static char lbuf[1024];

	memset(lbuf, 0, 1024);

	if (!line) {
		lbuf[0] = '~';
		return lbuf;
	}

	for (i = 0; outpos < 1024; i++) {
		if (i == textx && pos)
			*pos = outpos;
		if (!line[i])
			break;
		if (line[i] == '\t') {
			lbuf[outpos++] = ' ';
			while (outpos % TABSPACE)
				lbuf[outpos++] = ' ';
			continue;
		}
		lbuf[outpos++] = line[i];
	}

	return lbuf;
}

static int setpos(char *line, int position)
{
	int i = 0;
	int linepos = 0;

	while(line[linepos]) {
		if (line[linepos] == '\t')
			while ((i + 1) % TABSPACE)
				i++;
		if (i >= position)
			return linepos;
		linepos++;
		i++;
	}

	return linepos;
}

static void refresh_line(struct line *line, int ypos)
{
	char *str = screenline(line->data, NULL) + scrcol;
	pos(0, ypos);
	str[screenwidth] = 0;
	printf("%s%c[K", str, 27);
	pos(cursx, cursy);
}

/*
 * Most sane terminal programs can do ansi screen scrolling.
 * Unfortunately one of the most popular programs cannot:
 * minicom.
 * Grmpf!
 */
static int smartscroll = 0;

static void refresh(int full)
{
	int i;
	struct line *l = scrline;

	if (!full) {
		if (smartscroll) {
			if (scrline->next == lastscrline) {
				printf("%c[1T", 27);
				refresh_line(scrline, 0);
				pos(0, screenheight);
				printf("%*s", screenwidth, "");
				return;
			}

			if (scrline->prev == lastscrline) {
				printf("%c[1S", 27);
				for (i = 0; i < screenheight - 1; i++) {
					l = l->next;
					if (!l)
						return;
				}
				refresh_line(l, screenheight - 1);
				return;
			}
		} else {
			refresh(1);
			return;
		}
	}

	for (i = 0; i < screenheight; i++) {
		refresh_line(l, i);
		l = l->next;
		if (!l)
			break;
	}

	i++;
	while (i < screenheight) {
		pos(0, i++);
		printf("~");
	}
}

static void line_free(struct line *line)
{
	free(line->data);
	free(line);
}

static struct line *line_realloc(int len, struct line *line)
{
	int size = 32;

	if (!line) {
		line = xzalloc(sizeof(struct line));
		line->data = malloc(32);
	}

	while (size < len)
		size <<= 1;

	line->data = xrealloc(line->data, size);
	return line;
}

static int edit_read_file(const char *path)
{
	struct line *line;
	struct line *lastline = NULL;
	char *filebuffer;
	char *linestr, *lineend;
	struct stat s;

	if (!stat(path, &s)) {
		filebuffer = read_file(path, NULL);
		if (!filebuffer) {
			printf("could not read %s: %s\n", path, errno_str());
			return -1;
		}

		linestr = filebuffer;
		while (1) {
			if (!*linestr)
				break;

			lineend = strchr(linestr, '\n');

			if (!lineend && !*linestr)
				break;

			if (lineend)
				*lineend = 0;

			line = line_realloc(strlen(linestr) + 1, NULL);
			if (!buffer)
				buffer = line;
			memcpy(line->data, linestr, strlen(linestr) + 1);
			line->prev = lastline;
			if (lastline)
				lastline->next = line;
			line->next = 0;
			lastline = line;

			if (!lineend)
				break;

			linestr = lineend + 1;
		}
		free(filebuffer);
	}

	if (!buffer) {
		buffer = line_realloc(0, NULL);
		buffer->data[0] = 0;
	}

	return 0;
}

static void free_buffer(void)
{
	struct line *line, *tmp;

	line = buffer;

	while(line) {
		tmp = line->next;
		line_free(line);
		line = tmp;
	}
}

static int save_file(const char *path)
{
	struct line *line, *tmp;
	int fd;

	fd = open(path, O_WRONLY | O_TRUNC | O_CREAT);
	if (fd < 0) {
		printf("could not open file for writing: %s\n", errno_str());
		return -1;
	}

	line = buffer;

	while(line) {
		tmp = line->next;
		write(fd, line->data, strlen(line->data));
		write(fd, "\n", 1);
		line = tmp;
	}
	close(fd);
	return 0;
}

static void insert_char(char c)
{
	int pos = textx;
	char *line;
	int end = strlen(curline->data);

	line_realloc(strlen(curline->data) + 2, curline);
	line = curline->data;

	while (end >= pos) {
		line[end + 1] = line[end];
		end--;
	}
	line[pos] = c;
	textx++;
	refresh_line(curline, cursy);
}

static void delete_char(int pos)
{
	char *line = curline->data;
	int end = strlen(line);

	while (pos < end) {
		line[pos] = line[pos + 1];
		pos++;
	}
	refresh_line(curline, cursy);
}

static void split_line(void)
{
	int length = strlen(curline->data + textx);
	struct line *newline = line_realloc(length + 1, NULL);
	struct line *tmp;

	memcpy(newline->data, curline->data + textx, length + 1);

	curline->data[textx] = 0;

	tmp = curline->next;
	curline->next = newline;
	newline->prev = curline;
	newline->next = tmp;
	if (tmp)
		tmp->prev = newline;

	textx = 0;
	cursy++;
	curline = curline->next;
	refresh(1);
}

static void merge_line(struct line *line)
{
	struct line *tmp;

	line_realloc(strlen(line->data) + strlen(line->next->data) + 1, line);

	tmp = line->next;

	line->next = line->next->next;
	if (line->next)
		line->next->prev = line;
	strcat(line->data, tmp->data);

	line_free(tmp);

	refresh(1);
}

/* not a good idea on slow serial lines */
/* #define GETWINSIZE */

#ifdef GETWINSIZE
static void getwinsize(void) {
	int y, yy = 25, xx = 80, i, n, r;
	char buf[100];
	char *endp;

	for (y = 25; y < 320; y++) {
		pos(y, y);
		printf("%c[6n", 27);
		i = 0;
		while ((r = getc()) != 'R') {
			buf[i] = r;
			i++;
		}
		n = simple_strtoul(buf + 2, &endp, 10);
		if (n == y + 1)
			yy = y + 1;
		n = simple_strtoul(endp + 1, NULL, 10);
		if (n == y + 1)
			xx = y + 1;
	}
	pos(0,0);
	screenheight = yy;
	screenwidth = xx;
	printf("%d %d\n", xx, yy);
	mdelay(1000);
}
#endif

static int do_edit(cmd_tbl_t * cmdtp, int argc, char *argv[])
{
	int lastscrcol;
	int i;
	int linepos;
	int c;

	if (argc != 2)
		return COMMAND_ERROR_USAGE;

	/* check if we are called as "sedit" insted of "edit" */
	if (*argv[0] == 's')
		smartscroll = 1;

	buffer = NULL;
	if(edit_read_file(argv[1]))
		return 1;

#ifdef GETWINSIZE
	getwinsize();
#endif

	cursx  = 0;
	cursy  = 0;
	textx  = 0;
	scrcol = 0;
	curline = buffer;
	scrline = curline;
	lastscrline = scrline;
	lastscrcol = 0;

	printf("%c[2J", 27);
	refresh(1);

	while (1) {
		int curlen = strlen(curline->data);

		if (textx > curlen)
			textx = curlen;
		if (textx < 0)
			textx = 0;

		screenline(curline->data, &linepos);

		if (linepos > scrcol + screenwidth)
			scrcol = linepos - screenwidth;

		if (scrcol > linepos)
			scrcol = linepos;

		cursx = linepos - scrcol;

		while (cursy >= screenheight) {
			cursy--;
			scrline = scrline->next;
		}

		while (cursy < 0) {
			cursy++;
			scrline = scrline->prev;
		}

		if (scrline != lastscrline || scrcol != lastscrcol)
			refresh(0);

		lastscrcol  = scrcol;
		lastscrline = scrline;
		pos(cursx, cursy);

		c = read_key();
		switch (c) {
		case KEY_UP:
			if (!curline->prev)
				continue;

			curline = curline->prev;
			cursy--;
			textx = setpos(curline->data, linepos);
			break;
		case KEY_DOWN:
			if (!curline->next)
				continue;

			curline = curline->next;
			cursy++;
			textx = setpos(curline->data, linepos);
			break;
		case KEY_RIGHT:
			textx++;
			break;
		case KEY_LEFT:
			textx--;
			break;
		case KEY_HOME:
			textx = 0;
			break;
		case KEY_END:
			textx = curlen;
			break;
		case KEY_PAGEUP:
			for (i = 0; i < screenheight - 1; i++) {
				if (!curline->prev)
					break;
				cursy--;
				curline = curline->prev;
			}
			textx = setpos(curline->data, linepos);
			break;
		case KEY_PAGEDOWN:
			for (i = 0; i < screenheight - 1; i++) {
				if (!curline->next)
					break;
				cursy++;
				curline = curline->next;
			}
			textx = setpos(curline->data, linepos);
			break;
		case KEY_DEL:
			if (textx == curlen) {
				if (curline->next)
					merge_line(curline);
			} else
				delete_char(textx);
			break;
		case 13:
		case 10:
			split_line();
			break;
		case 127:
		case 8:
			if (textx > 0) {
				textx--;
				delete_char(textx);
			} else {
				if (!curline->prev)
					break;
				curline = curline->prev;
				cursy--;
				textx = strlen(curline->data);
				merge_line(curline);
			}
			break;
		case 4:
			save_file(argv[1]);
			goto out;
		case 3:
			goto out;
		default:
			if ((signed char)c != -1)
				insert_char(c);
		}
	}
out:
	free_buffer();
	printf("%c[2J", 27);
	printf("\n");
	return 0;
}

static const char *edit_aliases[] = { "sedit", NULL};

static const __maybe_unused char cmd_edit_help[] =
"Usage: (s)edit <file>\n"
"This is a very small editor. Its only features are moving the cursor with\n"
"the usual keys and typing characters.\n"
"<ctrl-c> quits the editor without saving,\n"
"<ctrl-d> quits the editor with saving the current file.\n"
"\n"
"If called as sedit the editor uses ansi codes to scroll the screen.\n";

static const __maybe_unused char cmd_edit_usage[] = "edit a file";

BAREBOX_CMD_START(edit)
	.cmd		= do_edit,
	.aliases	= edit_aliases,
	.usage		= cmd_edit_usage,
	BAREBOX_CMD_HELP(cmd_edit_help)
BAREBOX_CMD_END


/**
 * @page edit_command edit (editor)
 *
 * Usage is: [s]edit \<file\>
 *
 * This is a very small editor. It's only features are moving the cursor with
 * the usual keys and typing characters.
 *
 * \b \<ctrl-c\> quits the editor without saving,\n
 * \b \<ctrl-d\> quits the editor with saving the current file.
 *
 * If called as \c sedit the editor uses ansi codes to scroll the screen.
 */
