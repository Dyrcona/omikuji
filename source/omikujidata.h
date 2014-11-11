/*
 * Copyright © 2012 Jason J.A. Stephenson <jason@sigio.com>
 *
 * This file is part of Omikuji.
 *
 * Omikuji is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Omikuji is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Omikuji.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <glib.h>
#ifndef OMIKUJIDATA_H
#define OMIKUJIDATA_H

extern const gchar *OMIKUJI_SIGNATURE;
extern const guint8 OMIKUJI_VERSION;

typedef enum {
  OMIKUJI_OK = 0,
  OMIKUJI_BADSIGNATURE = 1,
  OMIKUJI_BADVERSION = 2
} OmikujiResult;

typedef struct {
  gchar signature[7];
  guint8 version;
} OmikujiSignature;

typedef struct {
  OmikujiSignature signature;
  guint32 comment_table_offset;
  guint32 comment_count;
  guint32 fortune_table_offset;
  guint32 fortune_count;
} OmikujiHeaderV0;

extern OmikujiResult verifyOmikujiSignature(OmikujiSignature sig);

#endif
