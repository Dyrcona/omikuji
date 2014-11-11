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
#include <string.h>
#include <stdio.h>
#include <omikujidata.h>

// helper function declarations. all defined below main.
gboolean delimiterOptionCallback(const gchar *option, const gchar *value, gpointer data, GError **err);
gchar *slurpfile(const gchar *fname, GError **err);
GRegex *compileRegularExpression(GError **err);
void writeOmikujiFile(const gchar *fname, GList *comments, GList *fortunes);
void conversionFunc(gpointer data, gpointer user_data);
void stringDelete(gpointer data, gpointer user_data);
gint stringCmp(GString *a, GString *b);

// global variables
// Input file fortune delimiter:
static gint delimiter = '%';
// Encoding of the input file (NULL for default):
static const char *inputEncoding = NULL;
// Filename to use if output is to be combined into 1 file:
static gchar *outputFilename = NULL;
// Skip comments in the input file(s).
static gboolean skipComments = FALSE;
// Sort fortunes "alphabetically" in output
static gboolean sort = FALSE;
// Output only unique fortunes.
static gboolean uniqify = FALSE;

// command line option setup.
static GOptionEntry options[] = {
  {"delimiter", 'd', 0, G_OPTION_ARG_CALLBACK, delimiterOptionCallback, "Set the input file delimiter character", "d"},
  {"encoding", 'e', 0, G_OPTION_ARG_STRING, &inputEncoding, "Character encoding of the input files", "str"},
  {"ouputfile", 'o', 0, G_OPTION_ARG_FILENAME, &outputFilename, "Combine all output in 1 file", "filename"},
  {"skipcomments", 'C', 0, G_OPTION_ARG_NONE, &skipComments, "Skip comments in the input file(s)", NULL},
  {"sort", 's', 0, G_OPTION_ARG_NONE, &sort, "Sort fortunes in output", NULL},
  {"unique", 'u', 0, G_OPTION_ARG_NONE, &uniqify, "Output only unique fortunes. Also has the side effect of sorting output whether that option is selected or not.", NULL},
  {NULL}
};

int main(int argc, char *argv[])
{
  // Set locale required for Glib-2.0:
  setlocale(LC_ALL, "");

  GError *error = NULL;
  GOptionContext *context;
  
  // Process command line arguments.
  context = g_option_context_new(" - convert fortune file to omikuji file");
  g_option_context_add_main_entries(context, options, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_printerr("%s\n", error->message);
    return 1;
  }
  else if (error != NULL) {
    g_printerr("%s\n", error->message);
    g_clear_error(&error);
  }

  if (inputEncoding == NULL)
    g_get_charset(&inputEncoding);

  GRegex *regex = compileRegularExpression(&error);
  if (regex == NULL) {
    g_printerr("%s\n", error->message);
    return 2;
  }

  GList *comments = NULL, *fortunes = NULL;
  gchar *buf;
  // Process input files.
  for (gint i = 1; i < argc; i++) {
    buf = slurpfile(argv[i], &error);
    if (buf == NULL) {
      if (error->domain == G_FILE_ERROR)
        g_printerr("%s\n", error->message);
      else 
        g_printerr("Failed to parse %s: %s\n", argv[i], error->message);
      if (outputFilename)
        return 3;
      else {
        g_clear_error(&error);
        continue;
      }
    }
    GMatchInfo *match_info;
    gint start = 0, end = 0, mstart, mend;
    gboolean isComment = FALSE;
    if (g_regex_match(regex, buf, 0, &match_info)) {
      while (g_match_info_matches(match_info)) {
        if (g_match_info_fetch_pos(match_info, 0, &mstart, &mend)) {
          if (start == 0) {
            start = mend;
            gunichar next = g_utf8_get_char(&buf[mend]);
            if (g_unichar_isspace(next)) {
              start++;
              if (g_unichar_type(next) == G_UNICODE_SPACE_SEPARATOR)
                isComment = TRUE;
              else
                isComment = FALSE;
            }
            else
              isComment = TRUE;
            if (!g_match_info_next(match_info, &error))
              g_clear_error(&error);
          }
          else {
            if ((isComment && !skipComments) || !isComment) {
              end = mstart;
              GString *string = g_string_sized_new(end - start);
              for (gint j = start; j < end; j++)
                g_string_append_c(string, buf[j]);
              g_string_append_c(string, '\0');
              if (isComment)
                comments = g_list_append(comments, string);
              else
                fortunes = g_list_append(fortunes, string);
            }
            start = 0; end = 0;
          }
        }
      }
      g_match_info_free(match_info);
      if (outputFilename == NULL) {
        outputFilename = g_path_get_basename(argv[i]);
        if (outputFilename) {
          gpointer temp = g_try_realloc(outputFilename, strlen(outputFilename) + 5);
          if (temp) {
            gboolean delStr = TRUE;
            outputFilename = (gchar *) temp;
            strcat(outputFilename, ".omi");
            writeOmikujiFile(outputFilename, comments, fortunes);
            if (comments) {
              g_list_foreach(comments, stringDelete, &delStr);
              g_list_free(comments);
              comments = NULL;
            }
            g_list_foreach(fortunes, stringDelete, &delStr);
            g_list_free(fortunes);
            fortunes = NULL;
          }
          g_free(outputFilename);
          outputFilename = NULL;
        }
      }
    }
    g_free(buf);
  }

  g_regex_unref(regex);

  if (outputFilename) {
    gboolean delStr = TRUE;
    writeOmikujiFile(outputFilename, comments, fortunes);
    if (comments) {
      g_list_foreach(comments, stringDelete, &delStr);
      g_list_free(comments);
      comments = NULL;
    }
    g_list_foreach(fortunes, stringDelete, &delStr);
    g_list_free(fortunes);
    fortunes = NULL;
  }

  return 0;
}

gboolean delimiterOptionCallback(const gchar *option, const gchar *value, gpointer data, GError **err)
{
  delimiter = value[0];
  return TRUE;
}

gchar *slurpfile(const gchar *fname, GError **err)
{
  gchar *inbuf = NULL, *outbuf = NULL;
  gsize inlen, outlen;
  if (g_file_get_contents(fname, &inbuf, &inlen, err)) {
    if (g_ascii_strcasecmp(inputEncoding, "UTF-8") != 0) {
      outbuf = g_convert(inbuf, inlen, "UTF-8", inputEncoding, NULL, &outlen, err);
      g_free(inbuf);
    }
    else
      outbuf = inbuf;
  }
  return outbuf;
}

GRegex *compileRegularExpression(GError **err)
{
  const gchar *mustQuote = "*?+[(){}^$|\\./";
  gchar pattern[32];
  GRegex *regex = NULL;

  if (g_utf8_strchr(mustQuote, strlen(mustQuote), delimiter))
    g_snprintf(pattern, 32, "^%c%c{1,2}", '\\', delimiter);
  else
    g_snprintf(pattern, 32, "^%c{1,2}", delimiter);

  regex = g_regex_new(pattern, G_REGEX_MULTILINE, 0, err);

  return regex;
}

void writeOmikujiFile(const gchar *fname, GList *comments, GList *fortunes)
{
  OmikujiHeaderV0 header;
  for (gint i = 0; i < 7; i++)
    header.signature.signature[i] = OMIKUJI_SIGNATURE[i];
  header.signature.version = OMIKUJI_VERSION;
  
  guint32 offset = sizeof(header);

  if (sort || uniqify)
    fortunes = g_list_sort(fortunes, (GCompareFunc)stringCmp);
  if (uniqify) {
    /* Iterate from back to front and compare two entries,
       removing the rearmost entry when stringCmp == 0. */
    gboolean deleteChars = TRUE;
    GList *current, *previous;
    current = g_list_last(fortunes);
    while (current != NULL) {
      previous = g_list_previous(current);
      if (previous != NULL) {
        if (stringCmp(previous->data, current->data) == 0) {
          stringDelete(current->data, &deleteChars);
          fortunes = g_list_delete_link(fortunes, current);
        }
      }
      current = previous;
    }
  }

  if (comments != NULL) {
    header.comment_table_offset = GUINT32_TO_BE(offset);
    header.comment_count = GUINT32_TO_BE(g_list_length(comments));
    offset += g_list_length(comments) * 8;
  }
  else {
    header.comment_table_offset = 0;
    header.comment_count = 0;
  }

  header.fortune_table_offset = GUINT32_TO_BE(offset);
  header.fortune_count = GUINT32_TO_BE(g_list_length(fortunes));
  offset += g_list_length(fortunes) * 8;

  /*
   * Rather than figure out all of the glib io channel stuff, I'm
   * using stdio for now.
   */
  FILE *out = fopen(fname, "w");
  if (out) {
    gint i, len;
    guint32 oval;
    fwrite(&header, sizeof(header), 1, out);
    // Write comments offset table.
    if (comments) {
      len = g_list_length(comments);
      for (i = 0; i < len; i++) {
        GString *comment = g_list_nth_data(comments, i);
        oval = GUINT32_TO_BE(offset);
        fwrite(&oval, sizeof(oval), 1, out);
        oval = GUINT32_TO_BE((comment->len - 1)); // we seem to be getting nul on the end
        fwrite(&oval, sizeof(oval), 1, out);
        offset += (comment->len - 1);
      }
    }

    // Write fortunes offset table.
    len = g_list_length(fortunes);
    for (i = 0; i < len; i++) {
      GString *fortune = g_list_nth_data(fortunes, i);
      oval = GUINT32_TO_BE(offset);
      fwrite(&oval, sizeof(oval), 1, out);
      oval = GUINT32_TO_BE((fortune->len - 1));
      fwrite(&oval, sizeof(oval), 1, out);
      offset += (fortune->len - 1);
    }

    // Write the comments
    if (comments) {
      len = g_list_length(comments);
      for (i = 0; i < len; i++) {
        GString *comment = g_list_nth_data(comments, i);
        fwrite(comment->str, sizeof(gchar), (comment->len - 1), out);
      }
    }

    // Write the fortunes
    len = g_list_length(fortunes);
    for (i = 0; i < len; i++) {
      GString *fortune = g_list_nth_data(fortunes, i);
      fwrite(fortune->str, sizeof(gchar), (fortune->len - 1), out);
    }

    fclose(out);
  }

}

void stringDelete(gpointer data, gpointer user_data)
{
  GString *str = (GString *)data;
  gboolean *b = (gboolean *)user_data;
  g_string_free(str, *b);
}

gint stringCmp(GString *a, GString *b)
{
  gchar *aKey = g_utf8_collate_key(a->str, a->len);
  gchar *bKey = g_utf8_collate_key(b->str, b->len);
  gint cmp = strcmp(aKey, bKey);
  g_free(aKey);
  g_free(bKey);
  return cmp;
}
