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

#ifndef DEF_H
#define DEF_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>

typedef int bool_t;

/* defines */
#ifdef __cplusplus
#define BEGIN_C extern "C" {
#define END_C }
#else
#define BEGIN_C
#define END_C
#endif /* __cplusplus */


enum exbit_set {
	EXBIT_COLOR_SEPARATE = 0x00000001
};

#endif /* DEF_H */
