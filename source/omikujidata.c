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
#include <omikujidata.h>

const gchar *OMIKUJI_SIGNATURE = "omikuji";
const guint8 OMIKUJI_VERSION = 0;

OmikujiResult verifyOmikujiSignature(OmikujiSignature signature)
{
	OmikujiResult isValid = OMIKUJI_OK;
	gint cmp = 0;
	for (gint i = 0; i < 7 && cmp == 0; i++)
		cmp = signature.signature[i] - OMIKUJI_SIGNATURE[i];
	if (cmp != 0)
		isValid = OMIKUJI_BADSIGNATURE;
	else if (signature.version > OMIKUJI_VERSION)
		isValid = OMIKUJI_BADVERSION;
	return isValid;
}
