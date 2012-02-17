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
#include <locale.h>
#include <omikujidata.h>
#include <stdio.h>

void displayRandomFortune(FILE *inF, OmikujiHeaderV0 head, const char *enc);
void displayComments(FILE *inF, OmikujiHeaderV0 head, const char *enc);

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	FILE *in;
	OmikujiHeaderV0 header;
	gboolean showComments = FALSE;
	const char *encoding = NULL;

	GOptionEntry options[] = {
		{"outputencoding", 'e', 0, G_OPTION_ARG_STRING, &encoding, "Set the output character encoding if not the default", "encoding"},
		{"listcomments", 'C', 0, G_OPTION_ARG_NONE, &showComments, "Show the comments (if any) from the file", NULL},
		{NULL}
	};

	GError *error = NULL;
	GOptionContext *context= g_option_context_new(" - lookup entries in an omikuji file");
	g_option_context_add_main_entries(context, options, NULL);
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_printerr("%s\n", error->message);
		return 1;
	}
	else if (error != NULL) {
		g_printerr("%s\n", error->message);
		g_clear_error(&error);
	}
	
	if (encoding == NULL)
		g_get_charset(&encoding);

	for (gint i = 1; i < argc; i++) {
		in = fopen(argv[i], "r");
		if (in) {
			if (fread(&header, sizeof(header), 1, in)) {
				OmikujiResult result = verifyOmikujiSignature(header.signature);
				if (result == OMIKUJI_OK) {
					if (showComments)
						displayComments(in, header, encoding);
					else
						displayRandomFortune(in, header, encoding);
				}
				else if (result == OMIKUJI_BADSIGNATURE)
					g_printerr("%s is not an omikuji file\n", argv[i]);
				else
					g_printerr("unsupported omikuji version in %s.\nPerhaps you need to upgrade omikuji?\n", argv[i]);
			}
			
			fclose(in);
		}
	}

	return 0;
}

void displayRandomFortune(FILE *inF, OmikujiHeaderV0 head, const char *enc)
{
	GRand *rand = g_rand_new();
	guint32 table_start = GUINT32_FROM_BE(head.fortune_table_offset);
	guint32 fortune_count = GUINT32_FROM_BE(head.fortune_count);
	guint32 which = g_rand_int_range(rand, 0, fortune_count);
	fseek(inF, table_start + (which * 8), SEEK_SET);
	guint32 fortune_start, fortune_length;
	fread(&fortune_start, sizeof(guint32), 1, inF);
	fread(&fortune_length, sizeof(guint32), 1, inF);
	fortune_start = GUINT32_FROM_BE(fortune_start);
	fortune_length = GUINT32_FROM_BE(fortune_length);
	gchar *buf = g_malloc0(fortune_length + 1);
	if (buf) {
		fseek(inF, fortune_start, SEEK_SET);
		fread(buf, sizeof(gchar), fortune_length, inF);
		if (g_ascii_strcasecmp(enc, "UTF-8") != 0) {		
			gchar *outbuf = NULL;
			gsize outlen;
			outbuf = g_convert(buf, fortune_length, enc, "UTF-8", NULL, &outlen, NULL);
			g_print("%s", outbuf);
			g_free(outbuf);
		}
		else
			g_print("%s", buf);
		g_free(buf);
	}
	if (rand)
		g_rand_free(rand);
}

void displayComments(FILE *inF, OmikujiHeaderV0 head, const char *enc)
{
	guint32 table_start = GUINT32_FROM_BE(head.comment_table_offset);
	guint32 comment_count = GUINT32_FROM_BE(head.comment_count);
	if (table_start > 0 && comment_count > 0) {
		gchar *buf = NULL;
		for (gint i = 0; i < comment_count; i++) {
			fseek(inF, table_start + (i * 8), SEEK_SET);
			guint32 comment_start, comment_len;
			fread(&comment_start, sizeof(guint32), 1, inF);
			fread(&comment_len, sizeof(guint32), 1, inF);
			comment_start = GUINT32_FROM_BE(comment_start);
			comment_len = GUINT32_FROM_BE(comment_len);
			buf = g_malloc0(comment_len + 1);
			if (buf) {
				fseek(inF, comment_start, SEEK_SET);
				fread(buf, sizeof(gchar), comment_len, inF);
				if (g_ascii_strcasecmp(enc, "UTF-8") != 0) {
					gchar *outbuf = NULL;
					gsize outlen = 0;
					outbuf = g_convert(buf, comment_len, enc, "UTF-8", NULL, &outlen, NULL);
					g_print("%s", outbuf);
					g_free(outbuf);
				}
				else
					g_print("%s", buf);
				g_free(buf);
			}
		}
	}
}
