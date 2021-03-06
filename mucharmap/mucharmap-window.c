/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2007, 2008 Christian Persch
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

#include <stdlib.h>
#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "mucharmap-print-operation.h"
#include "mucharmap-search-dialog.h"
#include "mucharmap-settings.h"
#include "mucharmap-window.h"

#if GTK_CHECK_VERSION (3, 0, 0)
#define gdk_cursor_unref g_object_unref
#endif

#define FONT_CHANGE_FACTOR    (1.189207115f) /* 2^(0.25) */

/* #define ENABLE_PRINTING */

static void mucharmap_window_class_init (MucharmapWindowClass* klass);
static void mucharmap_window_init       (MucharmapWindow* window);

G_DEFINE_TYPE (MucharmapWindow, mucharmap_window, GTK_TYPE_WINDOW)

static void
show_error_dialog (GtkWindow *parent,
	               GError *error)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (parent,
	                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                               GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
	                               "%s", error->message);
  g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
  gtk_window_present (GTK_WINDOW (dialog));
}

#ifdef ENABLE_PRINTING

static void
ensure_print_data (MucharmapWindow *guw)
{
  if (!guw->page_setup) {
	guw->page_setup = gtk_page_setup_new ();
  }

  if (!guw->print_settings) {
	guw->print_settings = gtk_print_settings_new ();
  }
}

static void
print_operation_done_cb (GtkPrintOperation *operation,
	                     GtkPrintOperationResult result,
	                     MucharmapWindow *guw)
{
  if (result == GTK_PRINT_OPERATION_RESULT_ERROR) {
	GError *error = NULL;

	gtk_print_operation_get_error (operation, &error);
	show_error_dialog (GTK_WINDOW (guw), error);
	g_error_free (error);
  } else if (result == GTK_PRINT_OPERATION_RESULT_APPLY) {
	if (guw->print_settings)
	  g_object_unref (guw->print_settings);
	guw->print_settings = g_object_ref (gtk_print_operation_get_print_settings (operation));
  }
}

static void
mucharmap_window_print (MucharmapWindow *guw,
	                    GtkPrintOperationAction action)
{
  GtkPrintOperation *op;
  PangoFontDescription *font_desc;
  MucharmapCodepointList *codepoint_list;
  MucharmapChartable *chartable;
  char *chapter, *filename;
  GtkPrintOperationResult rv;
  GError *error = NULL;

  g_object_get (guw->charmap,
	            "active-codepoint-list", &codepoint_list,
	            "font-desc", &font_desc,
	            NULL);

  op = mucharmap_print_operation_new (codepoint_list, font_desc);
  if (codepoint_list)
	g_object_unref (codepoint_list);
  if (font_desc)
	pango_font_description_free (font_desc);

  ensure_print_data (guw);
  if (guw->page_setup)
	gtk_print_operation_set_default_page_setup (op, guw->page_setup);
  if (guw->print_settings)
	gtk_print_operation_set_print_settings (op, guw->print_settings);

  chapter = mucharmap_charmap_get_active_chapter (guw->charmap);
  if (chapter) {
	filename = g_strconcat (chapter, ".pdf", NULL);
	gtk_print_operation_set_export_filename (op, filename);
	g_free (filename);
	g_free (chapter);
  }

  gtk_print_operation_set_allow_async (op, TRUE);
  gtk_print_operation_set_show_progress (op, TRUE);

  g_signal_connect (op, "done",
	                G_CALLBACK (print_operation_done_cb), guw);

  rv = gtk_print_operation_run (op, action, GTK_WINDOW (guw), &error);
  if (rv == GTK_PRINT_OPERATION_RESULT_ERROR) {
	show_error_dialog (GTK_WINDOW (guw), error);
	g_error_free (error);
  }

  g_object_unref (op);
}

#endif /* ENABLE_PRINTING */

static void
status_message (GtkWidget       *widget,
	            const gchar     *message,
	            MucharmapWindow *guw)
{
  gtk_statusbar_pop (GTK_STATUSBAR (guw->status), 0);

  if (message)
	gtk_statusbar_push (GTK_STATUSBAR (guw->status), 0, message);
}

static gboolean
update_progress_bar (MucharmapWindow *guw)
{
	#ifdef ENABLE_PROGRESSBAR_ON_STATUSBAR
  gdouble fraction_completed;

  fraction_completed = mucharmap_search_dialog_get_completed (MUCHARMAP_SEARCH_DIALOG (guw->search_dialog));

  if (fraction_completed < 0 || fraction_completed > 1)
	{
	  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (guw->progress), 0);
	  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (guw->progress), NULL);
	  return FALSE;
	}
  else
	{
	  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (guw->progress), fraction_completed);
	  return TRUE;
	}
	#else
  return FALSE;
	#endif
}

static void
search_start (MucharmapSearchDialog *search_dialog,
	          MucharmapWindow       *guw)
{
  GdkCursor *cursor;
  GtkAction *action;

  cursor = gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (guw)), GDK_WATCH);
  gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (guw)), cursor);
  gdk_cursor_unref (cursor);

  action = gtk_action_group_get_action (guw->action_group, "Find");
  gtk_action_set_sensitive (action, FALSE);
  action = gtk_action_group_get_action (guw->action_group, "FindNext");
  gtk_action_set_sensitive (action, FALSE);
  action = gtk_action_group_get_action (guw->action_group, "FindPrevious");
  gtk_action_set_sensitive (action, FALSE);

	#ifdef ENABLE_PROGRESSBAR_ON_STATUSBAR
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (guw->progress), _("Searching…"));

		g_timeout_add (100, (GSourceFunc) update_progress_bar, guw);
	#endif
}

static void
search_finish (MucharmapSearchDialog *search_dialog,
	           gunichar               found_char,
	           MucharmapWindow       *guw)
{
  GtkAction *action;

	#ifdef ENABLE_PROGRESSBAR_ON_STATUSBAR
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (guw->progress), 0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (guw->progress), NULL);
	#endif

  if (found_char != (gunichar)(-1))
	mucharmap_charmap_set_active_character (guw->charmap, found_char);
  /* not-found dialog handled by MucharmapSearchDialog */

  gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (guw)), NULL);

  action = gtk_action_group_get_action (guw->action_group, "Find");
  gtk_action_set_sensitive (action, TRUE);
  action = gtk_action_group_get_action (guw->action_group, "FindNext");
  gtk_action_set_sensitive (action, TRUE);
  action = gtk_action_group_get_action (guw->action_group, "FindPrevious");
  gtk_action_set_sensitive (action, TRUE);
}

static void
search_find (GtkAction       *action,
	         MucharmapWindow *guw)
{
	g_assert (MUCHARMAP_IS_WINDOW (guw));

	if (guw->search_dialog == NULL)
	{
		guw->search_dialog = mucharmap_search_dialog_new (guw);

		g_signal_connect (guw->search_dialog, "search-start", G_CALLBACK (search_start), guw);
		g_signal_connect (guw->search_dialog, "search-finish", G_CALLBACK (search_finish), guw);
	}

	mucharmap_search_dialog_present (MUCHARMAP_SEARCH_DIALOG (guw->search_dialog));
}

static void
search_find_next (GtkAction       *action,
	              MucharmapWindow *guw)
{
  if (guw->search_dialog)
	mucharmap_search_dialog_start_search (MUCHARMAP_SEARCH_DIALOG (guw->search_dialog), MUCHARMAP_DIRECTION_FORWARD);
  else
	search_find (action, guw);
}

static void
search_find_prev (GtkAction       *action,
	              MucharmapWindow *guw)
{
  if (guw->search_dialog)
	mucharmap_search_dialog_start_search (MUCHARMAP_SEARCH_DIALOG (guw->search_dialog), MUCHARMAP_DIRECTION_BACKWARD);
  else
	search_find (action, guw);
}

#ifdef ENABLE_PRINTING

static void
page_setup_done_cb (GtkPageSetup *page_setup,
	                MucharmapWindow *guw)
{
  if (page_setup) {
	g_object_unref (guw->page_setup);
	guw->page_setup = page_setup;
  }
}

static void
file_page_setup (GtkAction *action,
	             MucharmapWindow *guw)
{
  ensure_print_data (guw);

  gtk_print_run_page_setup_dialog_async (GTK_WINDOW (guw),
	                                     guw->page_setup,
	                                     guw->print_settings,
	                                     (GtkPageSetupDoneFunc) page_setup_done_cb,
	                                     guw);
}

#if 0
static void
file_print_preview (GtkAction *action,
	                MucharmapWindow *guw)
{
  mucharmap_window_print (guw, GTK_PRINT_OPERATION_ACTION_PREVIEW);
}
#endif

static void
file_print (GtkAction *action,
	        MucharmapWindow *guw)
{
  mucharmap_window_print (guw, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG);
}

#endif /* ENABLE_PRINTING */

static void
close_window (GtkAction *action,
	          GtkWidget *widget)
{
  gtk_widget_destroy (widget);
}

static void
font_bigger (GtkAction       *action,
	         MucharmapWindow *guw)
{
  mucharmap_mini_font_selection_change_font_size (MUCHARMAP_MINI_FONT_SELECTION (guw->fontsel),
	                                              FONT_CHANGE_FACTOR);
}

static void
font_smaller (GtkAction       *action,
	          MucharmapWindow *guw)
{
  mucharmap_mini_font_selection_change_font_size (MUCHARMAP_MINI_FONT_SELECTION (guw->fontsel),
	                                              1.0f / FONT_CHANGE_FACTOR);
}

static void
font_default (GtkAction       *action,
	          MucharmapWindow *guw)
{
  mucharmap_mini_font_selection_reset_font_size (MUCHARMAP_MINI_FONT_SELECTION (guw->fontsel));
}

static void
snap_cols_pow2 (GtkAction        *action,
	            MucharmapWindow  *guw)
{
  gboolean is_active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  mucharmap_charmap_set_snap_pow2 (guw->charmap, is_active);
  g_settings_set_boolean (guw->settings, "snap-cols-pow2", is_active);
}

static void
no_font_fallback_toggled_cb (GtkAction       *action,
                             MucharmapWindow *muw)
{
  gboolean is_active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  mucharmap_charmap_set_font_fallback (muw->charmap, !is_active);
/*  mucharmap_settings_set_font_fallback (is_active); */
}

static void
open_url (GtkWindow *parent,
	      const char *uri,
	      guint32 user_time)
{
  GdkScreen *screen;
  GError *error = NULL;

  if (parent)
	screen = gtk_widget_get_screen (GTK_WIDGET (parent));
  else
	screen = gdk_screen_get_default ();

  if (!gtk_show_uri (screen, uri, user_time, &error)) {
	show_error_dialog (parent, error);
	g_error_free (error);
  }
}

static void
help_contents (GtkAction *action,
	           MucharmapWindow *window)
{
  open_url (GTK_WINDOW (window),
            "help:mucharmap", /* DOC_MODULE */
            gtk_get_current_event_time ());
}

#if !GTK_CHECK_VERSION(3, 0, 0)

	static void about_open_url(GtkAboutDialog* about, const char* link, gpointer data)
	{
		open_url(GTK_WINDOW(about), link, gtk_get_current_event_time());
	}

	static void about_email_hook(GtkAboutDialog* about, const char* email_address, gpointer user_data)
	{
		char* escaped;
		char* uri;

		escaped = g_uri_escape_string(email_address, NULL, FALSE);
		uri = g_strdup_printf("mailto:%s", escaped);
		g_free(escaped);

		open_url(GTK_WINDOW(about), uri, gtk_get_current_event_time());
		g_free(uri);
	}

#endif

static void
help_about (GtkAction* action, MucharmapWindow* guw)
{
	const gchar* authors[] = {
		"Noah Levitt <nlevitt@columbia.edu>",
		"Daniel Elstner <daniel.elstner@gmx.net>",
		"Padraig O'Briain <Padraig.Obriain@sun.com>",
		"Christian Persch <" "chpe" "\100" "gnome" "." "org" ">",
		NULL
	};

	const gchar* documenters[] = {
		"Chee Bin HOH <cbhoh@gnome.org>",
		"Sun Microsystems",
		NULL
	};

	const gchar* license[] = {
		N_("Mucharmap is free software; you can redistribute it and/or modify "
			"it under the terms of the GNU General Public License as published by "
			"the Free Software Foundation; either version 3 of the License, or "
			"(at your option) any later version."),
		N_("Permission is hereby granted, free of charge, to any person obtaining "
			"a copy of the Unicode data files to deal in them without restriction, "
			"including without limitation the rights to use, copy, modify, merge, "
			"publish, distribute, and/or sell copies."),
		N_("Mucharmap and the Unicode data files are distributed in the hope that "
			"they will be useful, but WITHOUT ANY WARRANTY; without even the implied "
			"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See "
			"the GNU General Public License and Unicode Copyright for more details."),
		N_("You should have received a copy of the GNU General Public License "
			"along with Mucharmap; if not, write to the Free Software Foundation, Inc., "
			"51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA"),
		N_("Also you should have received a copy of the Unicode Copyright along "
			"with Mucharmap; you can always find it at Unicode's website: "
			"http://www.unicode.org/copyright.html")
	};

	gchar* license_trans = g_strconcat(_(license[0]), "\n\n",
	                                   _(license[1]), "\n\n",
	                                   _(license[2]), "\n\n",
	                                   _(license[3]), "\n\n",
	                                   _(license[4]), "\n\n", NULL);

	#if !GTK_CHECK_VERSION(3, 0, 0)
		gtk_about_dialog_set_url_hook(about_open_url, NULL, NULL);
		gtk_about_dialog_set_email_hook(about_email_hook, NULL, NULL);
	#endif

	gtk_show_about_dialog(GTK_WINDOW(guw),
		"program-name", _("MATE Character Map"),
		"version", VERSION,
		"comments", _("Based on the Unicode Character Database 8.0"),
		"copyright", "Copyright © 2004 Noah Levitt\n"
			"Copyright © 1991–2009 Unicode, Inc.\n"
			"Copyright © 2007–2010 Christian Persch",
		"documenters", documenters,
		"license", license_trans,
		"wrap-license", TRUE,
		"logo-icon-name", MUCHARMAP_ICON_NAME,
		"authors", authors,
		"translator-credits", _("translator-credits"),
		"website", "http://www.mate-desktop.org/",
		NULL);

	g_free (license_trans);
}

static void
next_or_prev_character (GtkAction       *action,
	                    MucharmapWindow *guw)
{
	MucharmapChartable* chartable;
	MucharmapChartableClass* klass;
	const char* name;
	guint keyval = 0;

	name = gtk_action_get_name (action);

	if (strcmp (name, "NextCharacter") == 0)
	{
		keyval = GDK_KEY_Right;
	}
	else if (strcmp (name, "PreviousCharacter") == 0)
	{
		keyval = GDK_KEY_Left;
	}

	chartable = mucharmap_charmap_get_chartable (guw->charmap);
	klass = MUCHARMAP_CHARTABLE_GET_CLASS (chartable);
	gtk_binding_set_activate (gtk_binding_set_by_class (klass),
	                          keyval,
	                          0,
	                          G_OBJECT (chartable));
}

static void
next_chapter (GtkAction* action, MucharmapWindow* guw)
{
	mucharmap_charmap_next_chapter (guw->charmap);
}

static void
prev_chapter (GtkAction* action, MucharmapWindow* guw)
{
	mucharmap_charmap_previous_chapter (guw->charmap);
}

static void
chapters_set_labels (const gchar     *labelnext,
			 const gchar     *labelprev,
			 MucharmapWindow *guw)
{
  GtkAction *action;

  action = gtk_action_group_get_action (guw->action_group, "NextChapter");
  g_object_set ( G_OBJECT (action), "label", labelnext, NULL);
  action = gtk_action_group_get_action (guw->action_group, "PreviousChapter");
  g_object_set ( G_OBJECT (action), "label", labelprev, NULL);
}

enum {
  VIEW_BY_SCRIPT,
  VIEW_BY_BLOCK
};

static void
mucharmap_window_set_chapters_model (MucharmapWindow *guw,
	                                 MucharmapChaptersMode mode)
{
  MucharmapChaptersModel *model = NULL;

  switch (mode)
	{
	  case MUCHARMAP_CHAPTERS_SCRIPT:
		model = mucharmap_script_chapters_model_new ();
	chapters_set_labels (_("Next Script"), _("Previous Script"), guw);
	break;

	  case MUCHARMAP_CHAPTERS_BLOCK:
		model = mucharmap_block_chapters_model_new ();
	chapters_set_labels (_("Next Block"), _("Previous Block"), guw);
	break;

	  default:
	    g_assert_not_reached ();
	}

  mucharmap_charmap_set_chapters_model (guw->charmap, model);
  g_object_unref (model);
}

static void
view_by (GtkAction        *action,
	 GtkRadioAction   *radioaction,
	     MucharmapWindow  *guw)
{
  MucharmapChaptersMode mode;

  switch (gtk_radio_action_get_current_value (radioaction))
	{
	  case VIEW_BY_SCRIPT:
	    mode = MUCHARMAP_CHAPTERS_SCRIPT;
	break;

	  case VIEW_BY_BLOCK:
	    mode = MUCHARMAP_CHAPTERS_BLOCK;
	break;

	  default:
	    g_assert_not_reached ();
	}

  mucharmap_window_set_chapters_model (guw, mode);
  g_settings_set_enum (guw->settings, "group-by", mode);
}

#ifdef DEBUG_chpe
static void
move_to_next_screen_cb (GtkAction *action,
	                    GtkWidget *widget)
{
  GdkScreen *screen;
  GdkDisplay *display;
  int number_of_screens, screen_num;

  screen = gtk_widget_get_screen (widget);
  display = gdk_screen_get_display (screen);
  screen_num = gdk_screen_get_number (screen);
  number_of_screens =  gdk_display_get_n_screens (display);

  if ((screen_num + 1) < number_of_screens) {
	screen = gdk_display_get_screen (display, screen_num + 1);
  } else {
	screen = gdk_display_get_screen (display, 0);
  }

  gtk_window_set_screen (GTK_WINDOW (widget), screen);
}
#endif

/* create the menu entries */

static const char ui_info [] =
  "<menubar name='MenuBar'>"
	"<menu action='File'>"
#ifdef ENABLE_PRINTING
	  "<menuitem action='PageSetup' />"
#if 0
	  "<menuitem action='PrintPreview' />"
#endif
	  "<menuitem action='Print' />"
	  "<separator />"
#endif /* ENABLE_PRINTING */
	  "<menuitem action='Close' />"
#ifdef DEBUG_chpe
	  "<menuitem action='MoveNextScreen' />"
#endif
	"</menu>"
	"<menu action='View'>"
	  "<menuitem action='ByScript' />"
	  "<menuitem action='ByUnicodeBlock' />"
	  "<separator />"
	  "<menuitem action='ShowOnlyGlyphsInFont' />"
	  "<menuitem action='SnapColumns' />"
	  "<separator />"
	  "<menuitem action='ZoomIn' />"
	  "<menuitem action='ZoomOut' />"
	  "<menuitem action='NormalSize' />"
	"</menu>"
	"<menu action='Search'>"
	  "<menuitem action='Find' />"
	  "<menuitem action='FindNext' />"
	  "<menuitem action='FindPrevious' />"
	"</menu>"
	"<menu action='Go'>"
	  "<menuitem action='NextCharacter' />"
	  "<menuitem action='PreviousCharacter' />"
	  "<separator />"
	  "<menuitem action='NextChapter' />"
	  "<menuitem action='PreviousChapter' />"
	"</menu>"
	"<menu action='Help'>"
	  "<menuitem action='HelpContents' />"
	  "<menuitem action='About' />"
	"</menu>"
  "</menubar>";

static void
insert_character_in_text_to_copy (MucharmapChartable *chartable,
	                              MucharmapWindow *guw)
{
  gchar ubuf[7];
  gint pos = -1;
  gunichar wc;

  wc = mucharmap_chartable_get_active_character (chartable);
  /* Can't copy values that are not valid unicode characters */
  if (!mucharmap_unichar_validate (wc))
	return;

  ubuf[g_unichar_to_utf8 (wc, ubuf)] = '\0';
  gtk_editable_delete_selection (GTK_EDITABLE (guw->text_to_copy_entry));
  pos = gtk_editable_get_position (GTK_EDITABLE (guw->text_to_copy_entry));
  gtk_editable_insert_text (GTK_EDITABLE (guw->text_to_copy_entry), ubuf, -1, &pos);
  gtk_editable_set_position (GTK_EDITABLE (guw->text_to_copy_entry), pos);
}

static void
edit_copy (GtkWidget *widget, MucharmapWindow *guw)
{
  /* if nothing is selected, select the whole thing */
  if (! gtk_editable_get_selection_bounds (
	          GTK_EDITABLE (guw->text_to_copy_entry), NULL, NULL))
	gtk_editable_select_region (GTK_EDITABLE (guw->text_to_copy_entry), 0, -1);

  gtk_editable_copy_clipboard (GTK_EDITABLE (guw->text_to_copy_entry));
}

static void
entry_changed_sensitize_button (GtkEditable *editable, GtkWidget *button)
{
  const gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (editable));
  gtk_widget_set_sensitive (button, entry_text[0] != '\0');
}

static void
status_realize (GtkWidget       *status,
	            MucharmapWindow *guw)
{
  GtkAllocation *allocation;
  GtkAllocation widget_allocation;

  gtk_widget_get_allocation (guw->status, &widget_allocation);
  allocation = &widget_allocation;

  /* FIXMEchpe ewww... */
  /* increase the height a bit so it doesn't resize itself */
  gtk_widget_set_size_request (guw->status, -1, allocation->height + 9);
}

static gboolean
save_last_char_idle_cb (MucharmapWindow *guw)
{
  guw->save_last_char_idle_id = 0;

  g_settings_set_uint (guw->settings, "last-char",
                       mucharmap_charmap_get_active_character (guw->charmap));

  return FALSE;
}

static void
fontsel_sync_font_desc (MucharmapMiniFontSelection *fontsel,
	                    GParamSpec *pspec,
	                    MucharmapWindow *guw)
{
  PangoFontDescription *font_desc;
  char *font;

  if (guw->in_notification)
	return;

  font_desc = mucharmap_mini_font_selection_get_font_desc (fontsel);

  guw->in_notification = TRUE;
  mucharmap_charmap_set_font_desc (guw->charmap, font_desc);
  guw->in_notification = FALSE;

  font = pango_font_description_to_string (font_desc);
  g_settings_set (guw->settings, "font", "ms", font);
  g_free (font);
}

static void
charmap_sync_font_desc (MucharmapCharmap *charmap,
	                    GParamSpec *pspec,
	                    MucharmapWindow *guw)
{
  PangoFontDescription *font_desc;

  if (guw->in_notification)
	return;

  font_desc = mucharmap_charmap_get_font_desc (charmap);

  guw->in_notification = TRUE;
  mucharmap_mini_font_selection_set_font_desc (MUCHARMAP_MINI_FONT_SELECTION (guw->fontsel),
	                                           font_desc);
  guw->in_notification = FALSE;
}

static void
charmap_sync_active_character (GtkWidget *widget,
	                           GParamSpec *pspec,
	                           MucharmapWindow *guw)
{
  if (guw->save_last_char_idle_id != 0)
	return;

  guw->save_last_char_idle_id = g_idle_add ((GSourceFunc) save_last_char_idle_cb, guw);
}

static void
mucharmap_window_init (MucharmapWindow *guw)
{
  GtkWidget *big_vbox, *hbox, *bbox, *button, *label;
  MucharmapChartable *chartable;
  /* tooltips are NULL because they are never actually shown in the program */
  const GtkActionEntry menu_entries[] =
  {
	{ "File", NULL, N_("_File"), NULL, NULL, NULL },
	{ "View", NULL, N_("_View"), NULL, NULL, NULL },
	{ "Search", NULL, N_("_Search"), NULL, NULL, NULL },
	{ "Go", NULL, N_("_Go"), NULL, NULL, NULL },
	{ "Help", NULL, N_("_Help"), NULL, NULL, NULL },

#ifdef ENABLE_PRINTING
	{ "PageSetup", NULL, N_("Page _Setup"), NULL,
	  NULL, G_CALLBACK (file_page_setup) },
#if 0
	{ "PrintPreview", GTK_STOCK_PRINT_PREVIEW, NULL, NULL,
	  NULL, G_CALLBACK (file_print_preview) },
#endif
	{ "Print", GTK_STOCK_PRINT, NULL, NULL,
	  NULL, G_CALLBACK (file_print) },
#endif /* ENABLE_PRINTING */
	{ "Close", GTK_STOCK_CLOSE, NULL, NULL,
	  NULL, G_CALLBACK (close_window) },

	{ "ZoomIn", GTK_STOCK_ZOOM_IN, NULL, "<control>plus",
	  NULL, G_CALLBACK (font_bigger) },
	{ "ZoomOut", GTK_STOCK_ZOOM_OUT, NULL, "<control>minus",
	  NULL, G_CALLBACK (font_smaller) },
	{ "NormalSize", GTK_STOCK_ZOOM_100, NULL, "<control>0",
	  NULL, G_CALLBACK (font_default) },

	{ "Find", GTK_STOCK_FIND, NULL, NULL,
	  NULL, G_CALLBACK (search_find) },
	{ "FindNext", NULL, N_("Find _Next"), "<control>G",
	  NULL, G_CALLBACK (search_find_next) },
	{ "FindPrevious", NULL, N_("Find _Previous"), "<shift><control>G",
	  NULL, G_CALLBACK (search_find_prev) },

	{ "NextCharacter", NULL, N_("_Next Character"), "<control>N",
	  NULL, G_CALLBACK (next_or_prev_character) },
	{ "PreviousCharacter", NULL, N_("_Previous Character"), "<control>P",
	  NULL, G_CALLBACK (next_or_prev_character) },
	{ "NextChapter", NULL, N_("Next Script"), "<control>Page_Down",
	  NULL, G_CALLBACK (next_chapter) },
	{ "PreviousChapter", NULL, N_("Previous Script"), "<control>Page_Up",
	  NULL, G_CALLBACK (prev_chapter) },

	{ "HelpContents", GTK_STOCK_HELP, N_("_Contents"), "F1",
	  NULL, G_CALLBACK (help_contents) },
	{ "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
	  NULL, G_CALLBACK (help_about) },

  #ifdef DEBUG_chpe
	{ "MoveNextScreen", NULL, "Move window to next screen", NULL,
	  NULL, G_CALLBACK (move_to_next_screen_cb) },
  #endif
  };
  const GtkRadioActionEntry radio_menu_entries[] =
  {
	{ "ByScript", NULL, N_("By _Script"), NULL,
	  NULL, VIEW_BY_SCRIPT },
	{ "ByUnicodeBlock", NULL, N_("By _Unicode Block"), NULL,
	  NULL, VIEW_BY_BLOCK }
  };
  const GtkToggleActionEntry toggle_menu_entries[] =
  {
	{ "ShowOnlyGlyphsInFont", NULL, N_("Sho_w only glyphs from this font"), NULL,
	  NULL,
	  G_CALLBACK (no_font_fallback_toggled_cb), FALSE },
	{ "SnapColumns", NULL, N_("Snap _Columns to Power of Two"), NULL,
	  NULL, NULL, FALSE },
  };
  GtkWidget *menubar;
  GtkAction *action;
  gunichar active;
  gchar *font;

  guw->settings = g_settings_new ("org.mate.mucharmap");

  gtk_window_set_title (GTK_WINDOW (guw), _("Character Map"));
  gtk_window_set_icon_name (GTK_WINDOW (guw), MUCHARMAP_ICON_NAME);

  /* UI manager setup */
  guw->uimanager = gtk_ui_manager_new();

  gtk_window_add_accel_group ( GTK_WINDOW (guw),
			       gtk_ui_manager_get_accel_group (guw->uimanager) );

  guw->action_group = gtk_action_group_new ("mucharmap_actions");
  gtk_action_group_set_translation_domain (guw->action_group, GETTEXT_PACKAGE);

  gtk_action_group_add_actions (guw->action_group,
				menu_entries,
				G_N_ELEMENTS (menu_entries),
				guw);
  gtk_action_group_add_radio_actions (guw->action_group,
				      radio_menu_entries,
					  G_N_ELEMENTS (radio_menu_entries),
					  g_settings_get_enum (guw->settings, "group-by"),
					  G_CALLBACK (view_by),
					  guw);
  gtk_action_group_add_toggle_actions (guw->action_group,
				       toggle_menu_entries,
					   G_N_ELEMENTS (toggle_menu_entries),
					   guw);

  gtk_ui_manager_insert_action_group (guw->uimanager, guw->action_group, 0);
  g_object_unref (guw->action_group);

  gtk_ui_manager_add_ui_from_string (guw->uimanager, ui_info, strlen (ui_info), NULL);

  /* Now the widgets */
  big_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (guw), big_vbox);

  /* First the menubar */
  menubar = gtk_ui_manager_get_widget (guw->uimanager, "/MenuBar");
  gtk_box_pack_start (GTK_BOX (big_vbox), menubar, FALSE, FALSE, 0);

  /* The font selector */
  guw->fontsel = mucharmap_mini_font_selection_new ();
  gtk_box_pack_start (GTK_BOX (big_vbox), guw->fontsel, FALSE, FALSE, 0);
  gtk_widget_show (GTK_WIDGET (guw->fontsel));

  /* The charmap */
  guw->charmap = MUCHARMAP_CHARMAP (mucharmap_charmap_new ());
  g_signal_connect (guw->charmap, "notify::font-desc",
	                G_CALLBACK (charmap_sync_font_desc), guw);

  gtk_box_pack_start (GTK_BOX (big_vbox), GTK_WIDGET (guw->charmap),
	                  TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (guw->charmap));

  /* Text to copy entry + button */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);

  label = gtk_label_new_with_mnemonic (_("_Text to copy:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  bbox = gtk_hbutton_box_new ();

  button = gtk_button_new_from_stock (GTK_STOCK_COPY);
  gtk_widget_set_tooltip_text (button, _("Copy to the clipboard."));
  g_signal_connect (G_OBJECT (button), "clicked",
	                G_CALLBACK (edit_copy), guw);
  gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  gtk_widget_set_sensitive (button, FALSE);
  guw->text_to_copy_entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), guw->text_to_copy_entry);
  g_signal_connect (G_OBJECT (guw->text_to_copy_entry), "changed",
	                G_CALLBACK (entry_changed_sensitize_button), button);

  gtk_box_pack_start (GTK_BOX (hbox), guw->text_to_copy_entry, TRUE, TRUE, 0);
  gtk_widget_show (guw->text_to_copy_entry);

  gtk_box_pack_end (GTK_BOX (hbox), bbox, FALSE, FALSE, 0);
  gtk_widget_show (bbox);

  gtk_box_pack_start (GTK_BOX (big_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /* FIXMEchpe!! */
  chartable = mucharmap_charmap_get_chartable (guw->charmap);
  g_signal_connect (chartable, "activate", G_CALLBACK (insert_character_in_text_to_copy), guw);

  /* Finally the statusbar */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (big_vbox), hbox, FALSE, FALSE, 0);

#ifdef ENABLE_PROGRESSBAR_ON_STATUSBAR
	guw->status = gtk_statusbar_new ();
#if !GTK_CHECK_VERSION (3, 0, 0)
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (guw->status), FALSE);
#endif
	gtk_box_pack_start (GTK_BOX (hbox), guw->status, TRUE, TRUE, 0);
	gtk_widget_show (guw->status);
	g_signal_connect (guw->status, "realize", G_CALLBACK (status_realize), guw);

	guw->progress = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (hbox), guw->progress, FALSE, FALSE, 0);
#else
	guw->status = gtk_statusbar_new ();
#if !GTK_CHECK_VERSION (3, 0, 0)
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (guw->status), TRUE);
#endif
	gtk_box_pack_start (GTK_BOX (hbox), guw->status, TRUE, TRUE, 0);
	gtk_widget_show(guw->status);
#endif

  gtk_widget_show_all (hbox);

  g_signal_connect (guw->charmap, "status-message", G_CALLBACK (status_message), guw);

  gtk_widget_show (big_vbox);

  gtk_widget_grab_focus (GTK_WIDGET (mucharmap_charmap_get_chartable (guw->charmap)));

  /* connect these only after applying the initial settings in order to
  * avoid unnecessary writes to GSettings.
  */
  g_signal_connect (guw->charmap, "notify::active-character",
  G_CALLBACK (charmap_sync_active_character), guw);
  g_signal_connect (guw->fontsel, "notify::font-desc",
  G_CALLBACK (fontsel_sync_font_desc), guw);

  /* read initial settings */
  /* font */
  g_settings_get (guw->settings, "font", "ms", &font);
  if (font != NULL)
	{
	  mucharmap_window_set_font (guw, font);
	  g_free (font);
	}

  /* snap-to-power-of-two */
  action = gtk_action_group_get_action (guw->action_group, "SnapColumns");
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), g_settings_get_boolean (guw->settings, "snap-cols-pow2"));
  g_signal_connect (action, "activate", G_CALLBACK (snap_cols_pow2), guw);

  /* group by */
  mucharmap_window_set_chapters_model (guw, g_settings_get_enum (guw->settings, "group-by"));

  /* active character */
  active = g_settings_get_uint (guw->settings, "last-char");
  mucharmap_charmap_set_active_character (guw->charmap, active);

  /* window geometry */
  mucharmap_settings_add_window (GTK_WINDOW (guw));

#if GTK_CHECK_VERSION (3, 0, 0)
  gtk_window_set_has_resize_grip (GTK_WINDOW (guw), TRUE);
#endif
}

static void
mucharmap_window_finalize (GObject *object)
{
  MucharmapWindow *guw = MUCHARMAP_WINDOW (object);

  if (guw->save_last_char_idle_id != 0)
	g_source_remove (guw->save_last_char_idle_id);

  if (guw->page_setup)
	g_object_unref (guw->page_setup);

  if (guw->print_settings)
	g_object_unref (guw->print_settings);

  G_OBJECT_CLASS (mucharmap_window_parent_class)->finalize (object);
}

static void
mucharmap_window_class_init (MucharmapWindowClass* klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = mucharmap_window_finalize;
}

/* Public API */

GtkWidget*
mucharmap_window_new (void)
{
	return GTK_WIDGET(g_object_new(mucharmap_window_get_type(), NULL));
}

void
mucharmap_window_set_font (MucharmapWindow* guw, const char* font)
{
	PangoFontDescription* font_desc;

	g_return_if_fail(MUCHARMAP_IS_WINDOW(guw));

		g_assert(!gtk_widget_get_realized(GTK_WIDGET(guw)));

	if (!font)
		return;

	font_desc = pango_font_description_from_string (font);
	mucharmap_charmap_set_font_desc (guw->charmap, font_desc);
	pango_font_description_free (font_desc);
}
