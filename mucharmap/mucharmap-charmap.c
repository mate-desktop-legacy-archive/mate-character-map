/*
 * Copyright (c) 2004 Noah Levitt
 * Copyright (c) 2007, 2008 Christian Persch
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
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <config.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>

#include "mucharmap-charmap.h"
#include "mucharmap-unicode-info.h"
#include "mucharmap-marshal.h"
#include "mucharmap-private.h"

struct _MucharmapCharmapPrivate {

  GtkWidget *notebook;
  MucharmapChaptersView *chapters_view;
  MucharmapChartable *chartable;
  GtkTextView *details_view;
  GtkTextTag *text_tag_gimongous;
  GtkTextTag *text_tag_big;

  PangoFontDescription *font_desc;

  GdkCursor *hand_cursor;
  GdkCursor *regular_cursor;

  guint active_page;

  guint hovering_over_link   : 1;
  guint last_character_set   : 1;
};

enum
{
  STATUS_MESSAGE,
  LINK_CLICKED,
  NUM_SIGNALS
};

enum {
  PROP_0,
  PROP_CHAPTERS_MODEL,
  PROP_ACTIVE_CHAPTER,
  PROP_ACTIVE_CHARACTER,
  PROP_ACTIVE_CODEPOINT_LIST,
  PROP_ACTIVE_PAGE,
  PROP_SNAP_POW2,
  PROP_FONT_DESC,
  PROP_FONT_FALLBACK
};

static guint mucharmap_charmap_signals[NUM_SIGNALS];

static void mucharmap_charmap_class_init (MucharmapCharmapClass *klass);
static void mucharmap_charmap_init       (MucharmapCharmap *charmap);

G_DEFINE_TYPE (MucharmapCharmap, mucharmap_charmap, GTK_TYPE_PANED)

static void
mucharmap_charmap_finalize (GObject *object)
{
  MucharmapCharmap *charmap = MUCHARMAP_CHARMAP (object);
  MucharmapCharmapPrivate *priv = charmap->priv;

  gdk_cursor_unref (priv->hand_cursor);
  gdk_cursor_unref (priv->regular_cursor);

  if (priv->font_desc)
	pango_font_description_free (priv->font_desc);

  G_OBJECT_CLASS (mucharmap_charmap_parent_class)->finalize (object);
}

static void
mucharmap_charmap_get_property (GObject *object,
	                            guint prop_id,
	                            GValue *value,
	                            GParamSpec *pspec)
{
  MucharmapCharmap *charmap = MUCHARMAP_CHARMAP (object);
  MucharmapCharmapPrivate *priv = charmap->priv;

  switch (prop_id) {
	case PROP_CHAPTERS_MODEL:
	  g_value_set_object (value, mucharmap_charmap_get_chapters_model (charmap));
	  break;
	case PROP_ACTIVE_CHAPTER:
	  g_value_take_string (value, mucharmap_chapters_view_get_selected (priv->chapters_view));
	  break;
	case PROP_ACTIVE_CHARACTER:
	  g_value_set_uint (value, mucharmap_charmap_get_active_character (charmap));
	  break;
	case PROP_ACTIVE_CODEPOINT_LIST:
	  g_value_set_object (value, mucharmap_charmap_get_active_codepoint_list (charmap));
	  break;
	case PROP_ACTIVE_PAGE:
	  g_value_set_uint (value, mucharmap_charmap_get_active_page (charmap));
	  break;
	case PROP_FONT_DESC:
	  g_value_set_boxed (value, mucharmap_charmap_get_font_desc (charmap));
	  break;
	case PROP_FONT_FALLBACK:
	  g_value_set_boolean (value, mucharmap_charmap_get_font_fallback (charmap));
	  break;
	case PROP_SNAP_POW2:
	  g_value_set_boolean (value, mucharmap_charmap_get_snap_pow2 (charmap));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
mucharmap_charmap_set_property (GObject *object,
	                            guint prop_id,
	                            const GValue *value,
	                            GParamSpec *pspec)
{
  MucharmapCharmap *charmap = MUCHARMAP_CHARMAP (object);
  MucharmapCharmapPrivate *priv = charmap->priv;

  switch (prop_id) {
	case PROP_CHAPTERS_MODEL:
	  mucharmap_charmap_set_chapters_model (charmap, g_value_get_object (value));
	  break;
	case PROP_ACTIVE_CHAPTER:
	  mucharmap_chapters_view_set_selected (priv->chapters_view,
	                                        g_value_get_string (value));
	  break;
	case PROP_ACTIVE_CHARACTER:
	  mucharmap_charmap_set_active_character (charmap, g_value_get_uint (value));
	  break;
	case PROP_ACTIVE_PAGE:
	  mucharmap_charmap_set_active_page (charmap, g_value_get_uint (value));
	  break;
	case PROP_FONT_DESC:
	  mucharmap_charmap_set_font_desc (charmap, g_value_get_boxed (value));
	  break;
	case PROP_FONT_FALLBACK:
	  mucharmap_charmap_set_font_fallback (charmap, g_value_get_boolean (value));
	  break;
	case PROP_SNAP_POW2:
	  mucharmap_charmap_set_snap_pow2 (charmap, g_value_get_boolean (value));
	  break;
	case PROP_ACTIVE_CODEPOINT_LIST: /* read-only */
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
mucharmap_charmap_class_init (MucharmapCharmapClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  _mucharmap_intl_ensure_initialized ();

  object_class->get_property = mucharmap_charmap_get_property;
  object_class->set_property = mucharmap_charmap_set_property;
  object_class->finalize = mucharmap_charmap_finalize;

  mucharmap_charmap_signals[STATUS_MESSAGE] =
	  g_signal_new (I_("status-message"), mucharmap_charmap_get_type (),
	                G_SIGNAL_RUN_FIRST,
	                G_STRUCT_OFFSET (MucharmapCharmapClass, status_message),
	                NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE,
	                1, G_TYPE_STRING);

  mucharmap_charmap_signals[LINK_CLICKED] =
	  g_signal_new (I_("link-clicked"), mucharmap_charmap_get_type (),
	                G_SIGNAL_RUN_FIRST,
	                G_STRUCT_OFFSET (MucharmapCharmapClass, link_clicked),
	                NULL, NULL, _mucharmap_marshal_VOID__UINT_UINT, G_TYPE_NONE,
	                2, G_TYPE_UINT, G_TYPE_UINT);

  g_object_class_install_property
	(object_class,
	 PROP_CHAPTERS_MODEL,
	 g_param_spec_object ("chapters-model", NULL, NULL,
	                     MUCHARMAP_TYPE_CHAPTERS_MODEL,
	                     G_PARAM_WRITABLE |
	                     G_PARAM_CONSTRUCT |
	                     G_PARAM_STATIC_NAME |
	                     G_PARAM_STATIC_NICK |
	                     G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_ACTIVE_CHAPTER,
	 g_param_spec_string ("active-chapter", NULL, NULL,
	                      NULL,
	                      G_PARAM_READWRITE |
	                      G_PARAM_STATIC_NAME |
	                      G_PARAM_STATIC_NICK |
	                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_ACTIVE_CHARACTER,
	 g_param_spec_uint ("active-character", NULL, NULL,
	                    0,
	                    UNICHAR_MAX,
	                    0,
	                    G_PARAM_READWRITE |
	                    G_PARAM_STATIC_NAME |
	                    G_PARAM_STATIC_NICK |
	                    G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_ACTIVE_CODEPOINT_LIST,
	 g_param_spec_object ("active-codepoint-list", NULL, NULL,
	                      MUCHARMAP_TYPE_CODEPOINT_LIST,
	                      G_PARAM_READABLE |
	                      G_PARAM_STATIC_NAME |
	                      G_PARAM_STATIC_NICK |
	                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_ACTIVE_PAGE,
	 g_param_spec_uint ("active-page", NULL, NULL,
	                    0,
	                    G_MAXUINT,
	                    0,
	                    G_PARAM_READWRITE |
	                    G_PARAM_STATIC_NAME |
	                    G_PARAM_STATIC_NICK |
	                    G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_FONT_DESC,
	 g_param_spec_boxed ("font-desc", NULL, NULL,
	                     PANGO_TYPE_FONT_DESCRIPTION,
	                     G_PARAM_READWRITE |
	                     G_PARAM_STATIC_NAME |
	                     G_PARAM_STATIC_NICK |
	                     G_PARAM_STATIC_BLURB));

  /**
   * MucharmapCharmap:font-fallback:
   *
   * Whether font fallback is enabled.
   *
   */
  g_object_class_install_property
    (object_class,
     PROP_FONT_FALLBACK,
     g_param_spec_boolean ("font-fallback", NULL, NULL,
                           TRUE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property
	(object_class,
	 PROP_SNAP_POW2,
	 g_param_spec_boolean ("snap-power-2", NULL, NULL,
	                       FALSE,
	                       G_PARAM_READWRITE |
	                       G_PARAM_STATIC_NAME |
	                       G_PARAM_STATIC_NICK |
	                       G_PARAM_STATIC_BLURB));

  g_type_class_add_private (object_class, sizeof (MucharmapCharmapPrivate));
}

static void
mucharmap_charmap_update_text_tags (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GtkStyle *style;
  int default_font_size;

  style = gtk_widget_get_style (GTK_WIDGET (priv->details_view));
  default_font_size = pango_font_description_get_size (style->font_desc);

  if (priv->font_desc)
	g_object_set (priv->text_tag_gimongous, "font-desc", priv->font_desc, NULL);

  /* FIXME: do we need to consider whether the font size is absolute or not? */
  g_object_set (priv->text_tag_gimongous,
	            "size", 8 * default_font_size,
	            "left-margin", PANGO_PIXELS (5 * default_font_size),
	            NULL);
  g_object_set (priv->text_tag_big,
	            "size", default_font_size * 5 / 4,
	            NULL);
}

static void
mucharmap_charmap_set_font_desc_internal (MucharmapCharmap *charmap,
	                                      PangoFontDescription *font_desc /* adopting */,
	                                      gboolean in_notification)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GObject *object = G_OBJECT (charmap);
  gboolean equal;

  g_object_freeze_notify (object);

  equal = priv->font_desc != NULL &&
	      pango_font_description_equal (priv->font_desc, font_desc);

  if (priv->font_desc)
	pango_font_description_free (priv->font_desc);

  priv->font_desc = font_desc; /* adopted */

  if (!in_notification)
	mucharmap_chartable_set_font_desc (priv->chartable, font_desc);

  if (gtk_widget_get_style (GTK_WIDGET (priv->details_view)))
	mucharmap_charmap_update_text_tags (charmap);

  if (!equal)
	g_object_notify (G_OBJECT (charmap), "font-desc");

  g_object_thaw_notify (object);
}

static void
insert_vanilla_detail (MucharmapCharmap *charmap,
	                   GtkTextBuffer *buffer,
	                   GtkTextIter *iter,
	                   const gchar *name,
	                   const gchar *value)
{
  gtk_text_buffer_insert (buffer, iter, name, -1);
  gtk_text_buffer_insert (buffer, iter, " ", -1);
  gtk_text_buffer_insert_with_tags_by_name (buffer, iter, value, -1,
	                                        "detail-value", NULL);
  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}

/* makes a nice string and makes it a link to the character */
static void
insert_codepoint (MucharmapCharmap *charmap,
	              GtkTextBuffer *buffer,
	              GtkTextIter *iter,
	              gunichar uc)
{
  gchar *str;
  GtkTextTag *tag;

  str = g_strdup_printf ("U+%4.4X %s", uc,
	                     mucharmap_get_unicode_name (uc));

  tag = gtk_text_buffer_create_tag (buffer, NULL,
	                                "foreground", "blue",
	                                "underline", PANGO_UNDERLINE_SINGLE,
	                                NULL);
  /* add one so that zero is the "nothing" value, since U+0000 is a character */
  g_object_set_data (G_OBJECT (tag), "link-character", GUINT_TO_POINTER (uc + 1));

  gtk_text_buffer_insert_with_tags (buffer, iter, str, -1, tag, NULL);

  g_free (str);
}

static void
insert_chocolate_detail_codepoints (MucharmapCharmap *charmap,
	                                GtkTextBuffer *buffer,
	                                GtkTextIter *iter,
	                                const gchar *name,
	                                const gunichar *ucs)
{
  gint i;

  gtk_text_buffer_insert (buffer, iter, name, -1);
  gtk_text_buffer_insert (buffer, iter, "\n", -1);

  for (i = 0;  ucs[i] != (gunichar)(-1);  i++)
	{
	  gtk_text_buffer_insert (buffer, iter, " • ", -1);
	  insert_codepoint (charmap, buffer, iter, ucs[i]);
	  gtk_text_buffer_insert (buffer, iter, "\n", -1);
	}

  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}

#define is_hex_digit(c) (((c) >= '0' && (c) <= '9') \
	                     || ((c) >= 'A' && (c) <= 'F'))

/* - "XXXX-YYYY" used in annotation of U+003D
 * - Annotation of U+03C0 uses ".XXXX" as a fractional number,
 *   so don't add "." to the list.
 * Add here if you know more. */
#define is_blank(c) (  ((c) == ' ')	\
			|| ((c) == '-') )

#define is_blank_or_hex_or(a,b) (  !((a) < len)	\
				|| is_blank(str[a])	\
				|| (is_hex_digit(str[a]) && (b)) )

/* returns a pointer to the start of (?=[^ -])[0-9A-F]{4,5,6}[^0-9A-F],
 * or null if not found */
static const gchar *
find_codepoint (const gchar *str)
{
  guint i, len;

  /* what we are searching for is ascii; in this case, we don't have to
   * worry about multibyte characters at all */
  len = strlen (str);
  for (i = 0;  i + 3 < len;  i++)
	{
	  if ( ( !(i > 0) || is_blank(str[i-1]) )
	  && is_hex_digit (str[i+0]) && is_hex_digit (str[i+1])
	  && is_hex_digit (str[i+2]) && is_hex_digit (str[i+3])
	  && is_blank_or_hex_or(i+4,    is_blank_or_hex_or(i+5,
			  (i+6 < len) || !is_hex_digit (str[i+6]))) )
	return str + i;
	}

  return NULL;
}

static void
insert_string_link_codepoints (MucharmapCharmap *charmap,
	                           GtkTextBuffer *buffer,
	                           GtkTextIter *iter,
	                           const gchar *str)
{
  const gchar *p1, *p2;

  p1 = str;
  for (;;)
	{
	  p2 = find_codepoint (p1);
	  if (p2 != NULL)
	    {
	      gunichar uc;
	      gtk_text_buffer_insert (buffer, iter, p1, p2 - p1);
	      uc = strtoul (p2, (gchar **) &p1, 16);
	      insert_codepoint (charmap, buffer, iter, uc);
	    }
	  else
	    {
	      gtk_text_buffer_insert (buffer, iter, p1, -1);
	      break;
	    }
	}
}

/* values is a null-terminated array of strings */
static void
insert_chocolate_detail (MucharmapCharmap *charmap,
	                     GtkTextBuffer *buffer,
	                     GtkTextIter *iter,
	                     const gchar *name,
	                     const gchar **values,
	                     gboolean expand_codepoints)
{
  gint i;

  gtk_text_buffer_insert (buffer, iter, name, -1);
  gtk_text_buffer_insert (buffer, iter, "\n", -1);

  for (i = 0;  values[i];  i++)
	{
	  gtk_text_buffer_insert (buffer, iter, " • ", -1);
	  if (expand_codepoints)
	    insert_string_link_codepoints (charmap, buffer, iter, values[i]);
	  else
	    gtk_text_buffer_insert (buffer, iter, values[i], -1);
	  gtk_text_buffer_insert (buffer, iter, "\n", -1);
	}

  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}


static void
insert_heading (MucharmapCharmap *charmap,
	            GtkTextBuffer *buffer,
	            GtkTextIter *iter,
	            const gchar *heading)
{
  gtk_text_buffer_insert (buffer, iter, "\n", -1);
  gtk_text_buffer_insert_with_tags_by_name (buffer, iter, heading, -1,
	                                        "bold", NULL);
  gtk_text_buffer_insert (buffer, iter, "\n\n", -1);
}

static void
conditionally_insert_canonical_decomposition (MucharmapCharmap *charmap,
	                                          GtkTextBuffer *buffer,
	                                          GtkTextIter *iter,
	                                          gunichar uc)
{
  gunichar *decomposition;
  gsize result_len;
  guint i;

  decomposition = g_unicode_canonical_decomposition (uc, &result_len);

  if (result_len == 1)
	{
	  g_free (decomposition);
	  return;
	}

  gtk_text_buffer_insert (buffer, iter, _("Canonical decomposition:"), -1);
  gtk_text_buffer_insert (buffer, iter, " ", -1);

  insert_codepoint (charmap, buffer, iter, decomposition[0]);
  for (i = 1;  i < result_len;  i++)
	{
	  gtk_text_buffer_insert (buffer, iter, " + ", -1);
	  insert_codepoint (charmap, buffer, iter, decomposition[i]);
	}

  gtk_text_buffer_insert (buffer, iter, "\n", -1);

  g_free (decomposition);
}

static void
set_details (MucharmapCharmap *charmap,
	         gunichar uc)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GString *gstemp;
  gchar *temp;
  const gchar *csp;
  gchar buf[12];
  guchar utf8[7];
  gint n, i;
  const gchar **csarr;
  gunichar *ucs;
  gunichar2 *utf16;
  MucharmapUnicodeVersion version;

  buffer = gtk_text_view_get_buffer (priv->details_view);
  gtk_text_buffer_set_text (buffer, "", 0);

  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_buffer_place_cursor (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, "\n", -1);

  n = mucharmap_unichar_to_printable_utf8 (uc, buf);
  if (n == 0)
	gtk_text_buffer_insert (
	        buffer, &iter, _("[not a printable character]"), -1);
  else
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, buf, n,
	                                          "gimongous", NULL);

  gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

  /* character name */
  temp = g_strdup_printf ("U+%4.4X %s\n",
	                      uc, mucharmap_get_unicode_name (uc));
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, temp, -1,
	                                        "big", "bold", NULL);
  g_free (temp);

  insert_heading (charmap, buffer, &iter, _("General Character Properties"));

  /* Unicode version */
  version = mucharmap_get_unicode_version (uc);
  if (version != MUCHARMAP_UNICODE_VERSION_UNASSIGNED)
	insert_vanilla_detail (charmap, buffer, &iter,
	                       _("In Unicode since:"),
	                       mucharmap_unicode_version_to_string (version));

  /* character category */
  insert_vanilla_detail (charmap, buffer, &iter, _("Unicode category:"),
	                     mucharmap_get_unicode_category_name (uc));

  /* canonical decomposition */
  conditionally_insert_canonical_decomposition (charmap, buffer, &iter, uc);

  /* representations */
  if (g_unichar_break_type(uc) != G_UNICODE_BREAK_SURROGATE)
	{
	  insert_heading (charmap, buffer, &iter, _("Various Useful Representations"));

	  n = g_unichar_to_utf8 (uc, (gchar *)utf8);
	  utf16 = g_ucs4_to_utf16 (&uc, 1, NULL, NULL, NULL);

	  /* UTF-8 */
	  gstemp = g_string_new (NULL);
	  for (i = 0;  i < n;  i++)
	g_string_append_printf (gstemp, "0x%2.2X ", utf8[i]);
	  g_string_erase (gstemp, gstemp->len - 1, -1);
	  insert_vanilla_detail (charmap, buffer, &iter, _("UTF-8:"), gstemp->str);
	  g_string_free (gstemp, TRUE);

	  /* UTF-16 */
	  gstemp = g_string_new (NULL);
	  g_string_append_printf (gstemp, "0x%4.4X", utf16[0]);
	  if (utf16[0] != '\0' && utf16[1] != '\0')
	g_string_append_printf (gstemp, " 0x%4.4X", utf16[1]);
	  insert_vanilla_detail (charmap, buffer, &iter, _("UTF-16:"), gstemp->str);
	  g_string_free (gstemp, TRUE);

	  /* an empty line */
	  gtk_text_buffer_insert (buffer, &iter, "\n", -1);

	  /* C octal \012\234 */
	  gstemp = g_string_new (NULL);
	  for (i = 0;  i < n;  i++)
	g_string_append_printf (gstemp, "\\%3.3o", utf8[i]);
	  insert_vanilla_detail (charmap, buffer, &iter,
				 _("C octal escaped UTF-8:"), gstemp->str);
	  g_string_free (gstemp, TRUE);

	  /* XML decimal entity */
	  if ((0x0001 <= uc && uc <= 0xD7FF) ||
	  (0xE000 <= uc && uc <= 0xFFFD) ||
	  (0x10000 <= uc && uc <= 0x10FFFF))
	{
	  temp = g_strdup_printf ("&#%d;", uc);
	  insert_vanilla_detail (charmap, buffer, &iter,
				 _("XML decimal entity:"), temp);
	  g_free (temp);
	}

	  g_free(utf16);
	}

  /* annotations */
  if (_mucharmap_unicode_has_nameslist_entry (uc))
	{
	  insert_heading (charmap, buffer, &iter,
	                  _("Annotations and Cross References"));

	  /* nameslist equals (alias names) */
	  csarr = mucharmap_get_nameslist_equals (uc);
	  if (csarr != NULL)
	    {
	      insert_chocolate_detail (charmap, buffer, &iter,
	                               _("Alias names:"), csarr, FALSE);
	      g_free (csarr);
	    }

	  /* nameslist stars (notes) */
	  csarr = mucharmap_get_nameslist_stars (uc);
	  if (csarr != NULL)
	    {
	      insert_chocolate_detail (charmap, buffer, &iter,
	                               _("Notes:"), csarr, TRUE);
	      g_free (csarr);
	    }

	  /* nameslist exes (see also) */
	  ucs = mucharmap_get_nameslist_exes (uc);
	  if (ucs != NULL)
	    {
	      insert_chocolate_detail_codepoints (charmap, buffer, &iter,
	                                          _("See also:"), ucs);
	      g_free (ucs);
	    }

	  /* nameslist pounds (approximate equivalents) */
	  csarr = mucharmap_get_nameslist_pounds (uc);
	  if (csarr != NULL)
	    {
	      insert_chocolate_detail (charmap, buffer, &iter,
	                               _("Approximate equivalents:"), csarr, TRUE);
	      g_free (csarr);
	    }

	  /* nameslist colons (equivalents) */
	  csarr = mucharmap_get_nameslist_colons (uc);
	  if (csarr != NULL)
	    {
	      insert_chocolate_detail (charmap, buffer, &iter,
	                               _("Equivalents:"), csarr, TRUE);
	      g_free (csarr);
	    }
	}

#if ENABLE_UNIHAN

  /* this isn't so bad efficiency-wise */
  if (mucharmap_get_unicode_kDefinition (uc)
	  || mucharmap_get_unicode_kCantonese (uc)
	  || mucharmap_get_unicode_kMandarin (uc)
	  || mucharmap_get_unicode_kJapaneseOn (uc)
	  || mucharmap_get_unicode_kJapaneseKun (uc)
	  || mucharmap_get_unicode_kTang (uc)
	  || mucharmap_get_unicode_kKorean (uc))
	{
	  insert_heading (charmap, buffer, &iter, _("CJK Ideograph Information"));

	  csp = mucharmap_get_unicode_kDefinition (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Definition in English:"), csp);

	  csp = mucharmap_get_unicode_kMandarin (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Mandarin Pronunciation:"), csp);

	  csp = mucharmap_get_unicode_kCantonese (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Cantonese Pronunciation:"), csp);

	  csp = mucharmap_get_unicode_kJapaneseOn (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Japanese On Pronunciation:"), csp);

	  csp = mucharmap_get_unicode_kJapaneseKun (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Japanese Kun Pronunciation:"), csp);

	  csp = mucharmap_get_unicode_kTang (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Tang Pronunciation:"), csp);

	  csp = mucharmap_get_unicode_kKorean (uc);
	  if (csp)
	    insert_vanilla_detail (charmap, buffer, &iter,
	                           _("Korean Pronunciation:"), csp);
	}
#endif /* #if ENABLE_UNIHAN */
}

static void
chartable_status_message (MucharmapCharmap *charmap,
	                      const gchar *message)
{
  g_signal_emit (charmap, mucharmap_charmap_signals[STATUS_MESSAGE],
	             0, message);
}

static void
chartable_sync_active_char (GtkWidget *widget,
	                        GParamSpec *pspec,
	                        MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GString *gs;
  const gchar *temp;
  const gchar **temps;
  gint i;
  gunichar wc;

  wc = mucharmap_chartable_get_active_character (priv->chartable);

  /* Forward the notification */
  g_object_notify (G_OBJECT (charmap), "active-character");

  if (priv->active_page == MUCHARMAP_CHARMAP_PAGE_DETAILS)
	set_details (charmap, wc);

  gs = g_string_sized_new (256);
  g_string_append_printf (gs, "U+%4.4X %s", wc,
	                      mucharmap_get_unicode_name (wc));

#if ENABLE_UNIHAN
  temp = mucharmap_get_unicode_kDefinition (wc);
  if (temp)
	g_string_append_printf (gs, "   %s", temp);
#endif

  temps = mucharmap_get_nameslist_equals (wc);
  if (temps)
	{
	  g_string_append_printf (gs, "   = %s", temps[0]);
	  for (i = 1;  temps[i];  i++)
	    g_string_append_printf (gs, "; %s", temps[i]);
	  g_free (temps);
	}

  temps = mucharmap_get_nameslist_stars (wc);
  if (temps)
	{
	  g_string_append_printf (gs, "   • %s", temps[0]);
	  for (i = 1;  temps[i];  i++)
	    g_string_append_printf (gs, "; %s", temps[i]);
	  g_free (temps);
	}

  chartable_status_message (charmap, gs->str);
  g_string_free (gs, TRUE);
}

static void
chartable_sync_font_desc (MucharmapChartable *chartable,
	                      GParamSpec *pspec,
	                      MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  PangoFontDescription *font_desc;

  font_desc = mucharmap_chartable_get_font_desc (chartable);
  mucharmap_charmap_set_font_desc_internal (charmap,
	                                        pango_font_description_copy (font_desc),
	                               //FIXME         FALSE,
	                                        priv->font_desc != NULL /* Do notify if we didn't have a font desc yet */);
}

static void
chartable_notify_cb (GtkWidget *widget,
	                 GParamSpec *pspec,
	                 GObject *charmap)
{
  const char *prop_name;

  if (pspec->name == I_("codepoint-list"))
	prop_name = "active-codepoint-list";
  else if (pspec->name == I_("snap-pow2"))
	prop_name = pspec->name;
  else
	return;

  /* Forward the notification */
  g_object_notify (charmap, prop_name);
}

static void
follow_if_link (MucharmapCharmap *charmap,
	            GtkTextIter *iter)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GSList *tags = NULL, *tagp = NULL;

  tags = gtk_text_iter_get_tags (iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
	{
	  GtkTextTag *tag = tagp->data;
	  gunichar uc;

	  /* subtract 1 because U+0000 is a character; see above where we set
	   * "link-character" */
	  uc = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (tag), "link-character")) - 1;

	  if (uc != (gunichar)(-1))
	    {
	      g_signal_emit (charmap, mucharmap_charmap_signals[LINK_CLICKED], 0,
	                     mucharmap_chartable_get_active_character (priv->chartable),
	                     uc);
	      mucharmap_charmap_set_active_character (charmap, uc);
	      break;
	    }
	}

  if (tags)
	g_slist_free (tags);
}

static void
details_style_set (GtkWidget *widget,
	               GtkStyle *previous_style,
	               MucharmapCharmap *charmap)
{
  mucharmap_charmap_update_text_tags (charmap);
}

static gboolean
details_key_press_event (GtkWidget *text_view,
	                     GdkEventKey *event,
	                     MucharmapCharmap *charmap)
{
  GtkTextIter iter;
  GtkTextBuffer *buffer;

  switch (event->keyval)
	{
	  case GDK_KEY_Return:
	  case GDK_KEY_ISO_Enter:
	  case GDK_KEY_KP_Enter:
	    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
	                                      gtk_text_buffer_get_insert (buffer));
	    follow_if_link (charmap, &iter);
	    break;

	  default:
	    break;
	}

  return FALSE;
}

static gboolean
details_event_after (GtkWidget *text_view,
	                 GdkEvent *ev,
	                 MucharmapCharmap *charmap)
{
  GtkTextIter start, end, iter;
  GtkTextBuffer *buffer;
  GdkEventButton *event;
  gint x, y;

  if (ev->type != GDK_BUTTON_RELEASE)
	return FALSE;

  event = (GdkEventButton *) ev;

  if (event->button != 1)
	return FALSE;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  /* we shouldn't follow a link if the user has selected something */
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
	return FALSE;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
	                                     GTK_TEXT_WINDOW_WIDGET,
	                                     event->x, event->y, &x, &y);

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

  follow_if_link (charmap, &iter);

  return FALSE;
}

static void
set_cursor_if_appropriate (MucharmapCharmap *charmap,
	                       gint x,
	                       gint y)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GSList *tags = NULL, *tagp = NULL;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  gboolean hovering_over_link = FALSE;

  buffer = gtk_text_view_get_buffer (priv->details_view);

  gtk_text_view_get_iter_at_location (priv->details_view,
	                                  &iter, x, y);

  tags = gtk_text_iter_get_tags (&iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
	{
	  GtkTextTag *tag = tagp->data;
	  gunichar uc;

	  /* subtract 1 because U+0000 is a character; see above where we set
	   * "link-character" */
	  uc = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (tag), "link-character")) - 1;

	  if (uc != (gunichar)(-1))
	    {
	      hovering_over_link = TRUE;
	      break;
	    }
	}

  if (hovering_over_link != priv->hovering_over_link)
	{
	  priv->hovering_over_link = hovering_over_link;

	  if (hovering_over_link)
	    gdk_window_set_cursor (gtk_text_view_get_window (priv->details_view, GTK_TEXT_WINDOW_TEXT), priv->hand_cursor);
	  else
	    gdk_window_set_cursor (gtk_text_view_get_window (priv->details_view, GTK_TEXT_WINDOW_TEXT), priv->regular_cursor);
	}

  if (tags)
	g_slist_free (tags);
}

static gboolean
details_motion_notify_event (GtkWidget *text_view,
	                         GdkEventMotion *event,
	                         MucharmapCharmap *charmap)
{
  gint x, y;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
	                                     GTK_TEXT_WINDOW_WIDGET,
	                                     event->x, event->y, &x, &y);

  set_cursor_if_appropriate (charmap, x, y);

  gdk_window_get_pointer (gtk_widget_get_window (text_view), NULL, NULL, NULL);
  return FALSE;
}

static gboolean
details_visibility_notify_event (GtkWidget *text_view,
	                             GdkEventVisibility *event,
	                             MucharmapCharmap *charmap)
{
  gint wx, wy, bx, by;

  gdk_window_get_pointer (gtk_widget_get_window (text_view), &wx, &wy, NULL);

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
	                                     GTK_TEXT_WINDOW_WIDGET,
	                                     wx, wy, &bx, &by);

  set_cursor_if_appropriate (charmap, bx, by);

  return FALSE;
}

static void
notebook_switch_page (GtkNotebook *notebook,
	                  GtkWidget *page,
	                  guint page_num,
	                  MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  priv->active_page = page_num;

  if (page_num == MUCHARMAP_CHARMAP_PAGE_DETAILS)
	set_details (charmap, mucharmap_chartable_get_active_character (priv->chartable));
  else if (page_num == MUCHARMAP_CHARMAP_PAGE_CHARTABLE)
	{
	  GtkTextBuffer *buffer;

	  buffer = gtk_text_view_get_buffer (priv->details_view);
	  gtk_text_buffer_set_text (buffer, "", 0);
	}

  g_object_notify (G_OBJECT (charmap), "active-page");
}

static void
chapters_view_selection_changed_cb (GtkTreeSelection *selection,
	                                MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  MucharmapCodepointList *codepoint_list;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
	return;

  codepoint_list = mucharmap_chapters_view_get_codepoint_list (priv->chapters_view);
  mucharmap_chartable_set_codepoint_list (priv->chartable, codepoint_list);
  g_object_unref (codepoint_list);

  g_object_notify (G_OBJECT (charmap), "active-chapter");
}

static void
mucharmap_charmap_init (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv;
  GtkWidget *scrolled_window, *view, *chartable, *textview;
  GtkTreeSelection *selection;
  GtkTextBuffer *buffer;
  int page;

  priv = charmap->priv = G_TYPE_INSTANCE_GET_PRIVATE (charmap, MUCHARMAP_TYPE_CHARMAP, MucharmapCharmapPrivate);

  /* FIXME: move this to realize */
  priv->hand_cursor = gdk_cursor_new (GDK_HAND2);
  priv->regular_cursor = gdk_cursor_new (GDK_XTERM);
  priv->hovering_over_link = FALSE;

  /* Left pane */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
	                              GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
	                                   GTK_SHADOW_ETCHED_IN);

#if GTK_CHECK_VERSION (3, 15, 9)
  gtk_scrolled_window_set_overlay_scrolling (GTK_SCROLLED_WINDOW (scrolled_window), FALSE);
#endif

  view = mucharmap_chapters_view_new ();
  priv->chapters_view = MUCHARMAP_CHAPTERS_VIEW (view);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  g_signal_connect (selection, "changed",
	                G_CALLBACK (chapters_view_selection_changed_cb), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), view);
  gtk_widget_show (view);
  gtk_paned_pack1 (GTK_PANED (charmap), scrolled_window, FALSE, TRUE);
  gtk_widget_show (scrolled_window);

  /* Right pane */
  priv->notebook = gtk_notebook_new ();

  /* Chartable page */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
	                              GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
	                                   GTK_SHADOW_NONE);

  chartable = mucharmap_chartable_new ();
  priv->chartable = MUCHARMAP_CHARTABLE (chartable);

  g_signal_connect_swapped (chartable, "status-message",
	                        G_CALLBACK (chartable_status_message), charmap);
  g_signal_connect (chartable, "notify::active-character",
	                G_CALLBACK (chartable_sync_active_char), charmap);
  g_signal_connect (chartable, "notify::font-desc",
	                G_CALLBACK (chartable_sync_font_desc), charmap);
  g_signal_connect (chartable, "notify::codepoint-list",
	                G_CALLBACK (chartable_notify_cb), charmap);
  g_signal_connect (chartable, "notify::snap-pow2",
	                G_CALLBACK (chartable_notify_cb), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), chartable);
  gtk_widget_show (chartable);

  page = gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
	                               scrolled_window,
	                               gtk_label_new_with_mnemonic (_("Characte_r Table")));
  g_assert (page == MUCHARMAP_CHARMAP_PAGE_CHARTABLE);
  gtk_widget_show (scrolled_window);

  /* Details page */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
	                              GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
	                                   GTK_SHADOW_ETCHED_IN);

  textview = gtk_text_view_new ();
  priv->details_view = GTK_TEXT_VIEW (textview);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview),
	                           GTK_WRAP_WORD);

  g_signal_connect (textview, "style-set",
	                G_CALLBACK (details_style_set), charmap);
  g_signal_connect (textview, "key-press-event",
	                G_CALLBACK (details_key_press_event), charmap);
  g_signal_connect (textview, "event-after",
	                G_CALLBACK (details_event_after), charmap);
  g_signal_connect (textview, "motion-notify-event",
	                G_CALLBACK (details_motion_notify_event), charmap);
  g_signal_connect (textview, "visibility-notify-event",
	                G_CALLBACK (details_visibility_notify_event), charmap);

  buffer = gtk_text_view_get_buffer (priv->details_view);
  priv->text_tag_gimongous =
	gtk_text_buffer_create_tag (buffer, "gimongous",
	                            NULL);
  priv->text_tag_big =
	gtk_text_buffer_create_tag (buffer, "big",
	                            NULL);
  gtk_text_buffer_create_tag (buffer, "bold",
	                          "weight", PANGO_WEIGHT_BOLD,
	                          NULL);
  gtk_text_buffer_create_tag (buffer, "detail-value",
	                          NULL);

  gtk_container_add (GTK_CONTAINER (scrolled_window), textview);
  gtk_widget_show (textview);

  page = gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
	                               scrolled_window,
	                               gtk_label_new_with_mnemonic (_("Character _Details")));
  g_assert (page == MUCHARMAP_CHARMAP_PAGE_DETAILS);
  gtk_widget_show (scrolled_window);

  priv->active_page = 0;
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);
  g_signal_connect (priv->notebook, "switch-page",
	                G_CALLBACK (notebook_switch_page), charmap);

  gtk_paned_pack2 (GTK_PANED (charmap), priv->notebook, TRUE, TRUE);
  gtk_widget_show (priv->notebook);
}

GtkWidget *
mucharmap_charmap_new (void)
{
  return g_object_new (MUCHARMAP_TYPE_CHARMAP,
	                   "orientation", GTK_ORIENTATION_HORIZONTAL,
	                   NULL);
}

#ifndef MUCHARMAP_DISABLE_DEPRECATED_SOURCE

/**
 * mucharmap_charmap_set_orientation:
 * @charmap:
 * @orientation:
 *
 * Deprecated: 2.25.0
 */
void
mucharmap_charmap_set_orientation (MucharmapCharmap *charmap,
	                               GtkOrientation orientation)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (charmap), orientation);
}

/**
 * mucharmap_charmap_get_orientation:
 * @charmap:
 *
 * Deprecated: 2.25.0
 */
GtkOrientation
mucharmap_charmap_get_orientation (MucharmapCharmap *charmap)
{
  g_return_val_if_fail (MUCHARMAP_IS_CHARMAP (charmap), GTK_ORIENTATION_HORIZONTAL);

  return gtk_orientable_get_orientation (GTK_ORIENTABLE (charmap));
}

#endif /* !MUCHARMAP_DISABLE_DEPRECATED_SOURCE */

/**
 * mucharmap_charmap_set_font_desc:
 * @charmap:
 * @font_desc: a #PangoFontDescription
 *
 * Sets @font_desc as the font to use to display the character table.
 */
void
mucharmap_charmap_set_font_desc (MucharmapCharmap *charmap,
	                             PangoFontDescription *font_desc)
{
  MucharmapCharmapPrivate *priv;

  g_return_if_fail (MUCHARMAP_IS_CHARMAP (charmap));
  g_return_if_fail (font_desc != NULL);

  priv = charmap->priv;
  if (priv->font_desc &&
	  pango_font_description_equal (font_desc, priv->font_desc))
	return;

  mucharmap_charmap_set_font_desc_internal (charmap,
	                                        pango_font_description_copy (font_desc),
	                                        FALSE);
}

/**
 * mucharmap_charmap_get_font_desc:
 * @charmap: a #MucharmapCharmap
 *
 * Returns: the #PangoFontDescription used to display the character table.
 *   The returned object is owned by @charmap and must not be modified or freed.
 */
PangoFontDescription *
mucharmap_charmap_get_font_desc (MucharmapCharmap *charmap)
{
  g_return_val_if_fail (MUCHARMAP_IS_CHARMAP (charmap), NULL);

  return charmap->priv->font_desc;
}

/**
 * mucharmap_charmap_set_font_fallback:
 * @charmap: a #GucharmapCharmap
 * @enable_font_fallback: whether to enable font fallback
 *
 */
void
mucharmap_charmap_set_font_fallback (MucharmapCharmap *charmap,
                                     gboolean enable_font_fallback)
{
  g_return_if_fail (MUCHARMAP_IS_CHARMAP (charmap));

  mucharmap_chartable_set_font_fallback (charmap->priv->chartable,
                                         enable_font_fallback);

  g_object_notify (G_OBJECT (charmap), "font-fallback");
}

/**
 * mucharmap_charmap_get_font_fallback:
 * @charmap: a #MucharmapCharmap
 *
 * Returns: whether font fallback is enabled
 *
 */
gboolean
mucharmap_charmap_get_font_fallback (MucharmapCharmap *charmap)
{
  g_return_val_if_fail (MUCHARMAP_IS_CHARMAP (charmap), FALSE);

  return mucharmap_chartable_get_font_fallback (charmap->priv->chartable);
}

void
mucharmap_charmap_set_active_character (MucharmapCharmap *charmap,
	                                    gunichar          wc)
{
  MucharmapCharmapPrivate *priv;

   if (wc > UNICHAR_MAX)
	return;

  priv = charmap->priv;
  if (!mucharmap_chapters_view_select_character (priv->chapters_view, wc)) {
	g_warning ("mucharmap_chapters_view_select_character failed (U+%04X)\n", wc);
	return;
  }

  mucharmap_chartable_set_active_character (priv->chartable, wc);
}

gunichar
mucharmap_charmap_get_active_character (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return mucharmap_chartable_get_active_character (priv->chartable);
}

void
mucharmap_charmap_set_active_chapter (MucharmapCharmap *charmap,
	                                  const gchar *chapter)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  mucharmap_chapters_view_set_selected (priv->chapters_view, chapter);
}

char *
mucharmap_charmap_get_active_chapter (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return mucharmap_chapters_view_get_selected (priv->chapters_view);
}

void
mucharmap_charmap_next_chapter (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  mucharmap_chapters_view_next (priv->chapters_view);
}

void
mucharmap_charmap_previous_chapter (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  mucharmap_chapters_view_previous (priv->chapters_view);
}

MucharmapChartable *
mucharmap_charmap_get_chartable (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return priv->chartable;
}

void
mucharmap_charmap_set_chapters_model (MucharmapCharmap  *charmap,
	                                  MucharmapChaptersModel *model)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GObject *object = G_OBJECT (charmap);
  gunichar wc;

  g_object_freeze_notify (object);

  g_object_notify (G_OBJECT (charmap), "chapters-model");

  mucharmap_chapters_view_set_model (priv->chapters_view, model);
  if (!model) {
	g_object_thaw_notify (object);
	return;
  }

  if (priv->last_character_set) {
	wc = mucharmap_chartable_get_active_character (priv->chartable);
	mucharmap_charmap_set_active_character (charmap, wc);
  }

  priv->last_character_set = TRUE;

  g_object_thaw_notify (object);
}

MucharmapChaptersModel *
mucharmap_charmap_get_chapters_model (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return mucharmap_chapters_view_get_model (priv->chapters_view);
}

MucharmapChaptersView *
mucharmap_charmap_get_chapters_view  (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return priv->chapters_view;
}

MucharmapCodepointList *
mucharmap_charmap_get_book_codepoint_list (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return mucharmap_chapters_view_get_book_codepoint_list (priv->chapters_view);
}

void
mucharmap_charmap_set_chapters_visible (MucharmapCharmap *charmap,
	                                    gboolean visible)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  g_object_set (priv->chapters_view, "visible", visible, NULL);
}

gboolean
mucharmap_charmap_get_chapters_visible (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return gtk_widget_get_visible (GTK_WIDGET (priv->chapters_view));
}

void
mucharmap_charmap_set_page_visible (MucharmapCharmap *charmap,
	                                int page,
	                                gboolean visible)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GtkWidget *page_widget;

  page_widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
  if (!page_widget)
	return;

  g_object_set (page_widget, "visible", visible, NULL);
}

gboolean
mucharmap_charmap_get_page_visible (MucharmapCharmap *charmap,
	                                int page)
{
  MucharmapCharmapPrivate *priv = charmap->priv;
  GtkWidget *page_widget;

  page_widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
  if (!page_widget)
	return FALSE;

  return gtk_widget_get_visible (page_widget);
}

void
mucharmap_charmap_set_active_page (MucharmapCharmap *charmap,
	                               int page)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), page);
}

int
mucharmap_charmap_get_active_page (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return priv->active_page;
}

MucharmapCodepointList *
mucharmap_charmap_get_active_codepoint_list (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return mucharmap_chartable_get_codepoint_list (priv->chartable);
}

/**
 * mucharmap_charmap_set_snap_pow2:
 * @charmap: a #MucharmapCharmap
 * @snap: whether to enable or disable snapping
 *
 * Sets whether the number columns the character table shows should
 * always be a power of 2.
 */
void
mucharmap_charmap_set_snap_pow2 (MucharmapCharmap *charmap,
	                             gboolean snap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  /* This will cause us to emit our prop notification too */
  mucharmap_chartable_set_snap_pow2 (priv->chartable, snap);
}

/**
 * mucharmap_charmap_get_snap_pow2:
 * @charmap: a #MucharmapCharmap
 *
 * Returns whether the number of columns the character table shows is
 * always a power of 2.
 */
gboolean
mucharmap_charmap_get_snap_pow2 (MucharmapCharmap *charmap)
{
  MucharmapCharmapPrivate *priv = charmap->priv;

  return mucharmap_chartable_get_snap_pow2 (priv->chartable);
}
