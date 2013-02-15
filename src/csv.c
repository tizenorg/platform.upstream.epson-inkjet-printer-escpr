/*
 * Epson Inkjet Printer Driver (ESC/P-R) for Linux
 * Copyright (C) 2002-2005 AVASYS CORPORATION.
 * Copyright (C) Seiko Epson Corporation 2002-2012.
 *
 *  This program is free software; you can redistribute it and/or modify
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "str.h"
#include "err.h"
#include "mem.h"
#include "csv.h"

BEGIN_C
static int csvlist_makelist (csvlist_t *, long);
END_C

csvlist_t *
csvlist_open (const char *path)
{
	csvlist_t *csv_p;
	FILE *fp;
	char *buffer;
	long size;

	if (!path)
		return NULL;

	csv_p = mem_new0 (csvlist_t, 1);
	csv_p->path = str_clone ((char *) path, strlen (path));

	fp = fopen (path, "rb");
	if (!fp)
		err_system (path);

	fseek (fp, 0L, SEEK_END);
	size = ftell (fp);
	rewind (fp);

	buffer = mem_new (char, size);

	if (fread (buffer, size, 1, fp) < size)
	{
		if (ferror (fp))
			err_system ("csv_open");
	}
	fclose (fp);

	csv_p->top_p = buffer;
	if (csvlist_makelist (csv_p, size))
	{
		csvlist_close (csv_p);
		return NULL;
	}

	return csv_p;
}


void
csvlist_close (csvlist_t *csv_p)
{
	if (csv_p)
	{
		if (csv_p->path)
			mem_free (csv_p->path);

		if (csv_p->top_p)
			mem_free (csv_p->top_p);

		if (csv_p->list)
			mem_free (csv_p->list);

		mem_free (csv_p);
	}

	return;
}


int
csvlist_search_keyword (csvlist_t *csv_p, int pos, const char *keyword)
{
	char **list = csv_p->list;
	int i;

	for (i = pos; i < csv_p->lnum; i ++)
	{
		if (!strcmp (list[i], keyword))
			return i;
	}

	return -1;
}


char *
csvlist_get_word (csvlist_t *csv_p, int pos)
{
	if (pos < 0 || pos > csv_p->lnum)
		return NULL;

	return csv_p->list[pos];
}


int
csvlist_get_max (csvlist_t *csv_p)
{
	return csv_p->lnum;
}


static int
csvlist_makelist (csvlist_t *csv_p, long size)
{
	char *from_buf = csv_p->top_p;
	char *to_buf = from_buf;
	char **list;
	int list_max, list_num;
	long from_point, to_point;
	bool_t quote_in;

	list_max = 16;		/* Initial value = 16 */
	list = mem_new (char *, list_max);
	
	list_num = 1;
	list[0] = to_buf;
	to_point = 0;
	quote_in = 0;

	for (from_point = 0; from_point < size; from_point ++)
	{
		if (!quote_in
		    && from_buf[from_point] == '#')	/* In comment */
		{
			from_point += strcspn (from_buf + from_point, "\n\0");
			continue;
		}

		else if (!quote_in
			 && (from_buf[from_point] == '\n' || from_buf[from_point] == ','))
		{
			to_buf[to_point++] = '\0';

			list[list_num++] = to_buf + to_point;

			if (list_num >= list_max)
			{
				list_max *= 2;
				list = mem_renew (char *, list, list_max);
			}
		}

		else if (from_buf[from_point] == '"')
		{
			quote_in ^= 1;
			continue;
		}

		else
		{
			to_buf[to_point++] = from_buf[from_point];
		}
	}

	to_buf = mem_renew (char, to_buf, to_point);
	list = mem_renew (char *, list, list_num + 1);

	csv_p->top_p = to_buf;
	csv_p->size = to_point;
	csv_p->list = list;
	csv_p->lnum = list_num;

	return 0;
}
