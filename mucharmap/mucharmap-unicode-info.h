/*
 * Copyright © 2004 Noah Levitt
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
	#error "Only <mucharmap/mucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_UNICODE_INFO_H
#define GUCHARMAP_UNICODE_INFO_H

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	GUCHARMAP_UNICODE_VERSION_UNASSIGNED,
	GUCHARMAP_UNICODE_VERSION_1_1,
	GUCHARMAP_UNICODE_VERSION_2_0,
	GUCHARMAP_UNICODE_VERSION_2_1,
	GUCHARMAP_UNICODE_VERSION_3_0,
	GUCHARMAP_UNICODE_VERSION_3_1,
	GUCHARMAP_UNICODE_VERSION_3_2,
	GUCHARMAP_UNICODE_VERSION_4_0,
	GUCHARMAP_UNICODE_VERSION_4_1,
	GUCHARMAP_UNICODE_VERSION_5_0,
	GUCHARMAP_UNICODE_VERSION_5_1,
	GUCHARMAP_UNICODE_VERSION_5_2,
	GUCHARMAP_UNICODE_VERSION_LATEST = GUCHARMAP_UNICODE_VERSION_5_2 /* private, will move forward with each revision */
} MucharmapUnicodeVersion;

/* return values are read-only */
const gchar* mucharmap_get_unicode_name(gunichar uc);
const gchar* mucharmap_get_unicode_data_name(gunichar uc);
gint mucharmap_get_unicode_data_name_count(void);
MucharmapUnicodeVersion mucharmap_get_unicode_version(gunichar wc);
const gchar* mucharmap_get_unicode_category_name(gunichar uc);
gint mucharmap_get_unihan_count(void);
const gchar* mucharmap_get_unicode_kDefinition(gunichar uc);
const gchar* mucharmap_get_unicode_kCantonese(gunichar uc);
const gchar* mucharmap_get_unicode_kMandarin(gunichar uc);
const gchar* mucharmap_get_unicode_kTang(gunichar uc);
const gchar* mucharmap_get_unicode_kKorean(gunichar uc);
const gchar* mucharmap_get_unicode_kJapaneseKun(gunichar uc);
const gchar* mucharmap_get_unicode_kJapaneseOn(gunichar uc);

/* nameslist stuff */
const gchar** mucharmap_get_nameslist_stars(gunichar wc);
const gchar** mucharmap_get_nameslist_equals(gunichar wc);
gunichar* mucharmap_get_nameslist_exes(gunichar wc);
const gchar** mucharmap_get_nameslist_pounds(gunichar wc);
const gchar** mucharmap_get_nameslist_colons(gunichar wc);
gboolean mucharmap_unichar_validate(gunichar wc);
gint mucharmap_unichar_to_printable_utf8(gunichar wc, gchar* outbuf);
GUnicodeType mucharmap_unichar_type(gunichar wc);
gboolean mucharmap_unichar_isdefined(gunichar wc);
gboolean mucharmap_unichar_isgraph(gunichar wc);

/* defined in mucharmap-script-codepoint-list.c */
const gchar** mucharmap_unicode_list_scripts(void);
const gchar* mucharmap_unicode_get_script_for_char(gunichar wc);

const gchar* mucharmap_unicode_version_to_string(MucharmapUnicodeVersion version);

/* doesn't really belong here, but no better place was available */
gunichar mucharmap_unicode_get_locale_character(void);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_UNICODE_INFO_H */

