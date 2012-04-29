/*
 * Copyright © 2005 Jason Allen
 * Copyright © 2007 Christian Persch
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

#include <config.h>

#include <errno.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <mucharmap/mucharmap.h>
#include "mucharmap-settings.h"

#ifdef HAVE_MATECONF
	#include <mateconf/mateconf-client.h>
	static MateConfClient* client;
#else
	// gio?
	static GSettings* client;
#endif

/* The last unicode character we support */
/* keep in sync with mucharmap-private.h! */
#define UNICHAR_MAX    (0x0010FFFFUL)

#define WINDOW_STATE_TIMEOUT    1000 /* ms */

#define MATECONF_PREFIX    "/apps/mucharmap"
#define GUCHARMAP_GSETTINGS_SCHEME    "org.mate.mucharmap"

static MucharmapChaptersMode
get_default_chapters_mode(void)
{
	/* XXX: In the future, do something based on chapters mode and locale
	 * or something. */
	return GUCHARMAP_CHAPTERS_SCRIPT;
}

static gchar*
get_default_font(void)
{
	return NULL;
}

static gboolean
get_default_snap_pow2(void)
{
	return FALSE;
}

void
mucharmap_settings_initialize(void)
{
	#ifdef HAVE_MATECONF
		client = mateconf_client_get_default();
	#else
		/* with g_settings_list_schemas, find if the schema exists */
		const gchar* const* list_schemas = g_settings_list_schemas();
		int i = 0;

		while (list_schemas[i] != NULL)
		{
			if (strcmp (list_schemas[i], GUCHARMAP_GSETTINGS_SCHEME) == 0)
			{
				client = g_settings_new(GUCHARMAP_GSETTINGS_SCHEME);
			}

			i++;
		}

	#endif

	if (client == NULL)
	{
		#ifdef HAVE_MATECONF
			g_message(_("MateConf could not be initialized."));
		#else
			g_message(_("GSettings could not be initialized."));
		#endif

		return;
	}

	#ifdef HAVE_MATECONF
		mateconf_client_add_dir(client, MATECONF_PREFIX,
		                        MATECONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	#else
		g_settings_delay(client);
		//g_settings_sync();
	#endif
}

static gboolean
mucharmap_settings_initialized(void)
{
	return (client != NULL);
}

void
mucharmap_settings_shutdown(void)
{
	if (mucharmap_settings_initialized())
	{
		#ifdef HAVE_MATECONF
			mateconf_client_remove_dir(client, MATECONF_PREFIX, NULL);
		#else
			g_settings_apply(client);
		#endif

		g_object_unref(client);
		client = NULL;
	}
}

MucharmapChaptersMode
mucharmap_settings_get_chapters_mode(void)
{
	MucharmapChaptersMode ret;
	gchar* mode;

	if (!mucharmap_settings_initialized())
	{
		return get_default_chapters_mode();
	}

	#ifdef HAVE_MATECONF
		mode = mateconf_client_get_string(client, MATECONF_PREFIX"/chapters_mode", NULL);
	#else
		mode = g_settings_get_string(client, "chapters-mode");
	#endif

	if (strcmp (mode, "script") == 0)
	{
		ret = GUCHARMAP_CHAPTERS_SCRIPT;
	}
	else if (strcmp (mode, "block") == 0)
	{
		ret = GUCHARMAP_CHAPTERS_BLOCK;
	}
	else
	{
		ret = get_default_chapters_mode();
	}

	g_free(mode);

	return ret;
}

void
mucharmap_settings_set_chapters_mode(MucharmapChaptersMode mode)
{
	if (!mucharmap_settings_initialized())
	{
		return;
	}

	switch (mode)
	{
		case GUCHARMAP_CHAPTERS_SCRIPT:

			#ifdef HAVE_MATECONF
				mateconf_client_set_string(client, MATECONF_PREFIX"/chapters_mode", "script", NULL);
			#else
				g_settings_set_string(client, "chapters-mode", "script");
			#endif

			break;

		case GUCHARMAP_CHAPTERS_BLOCK:

			#ifdef HAVE_MATECONF
				mateconf_client_set_string(client, MATECONF_PREFIX"/chapters_mode", "block", NULL);
			#else
				g_settings_set_string(client, "chapters-mode", "block");
			#endif

			break;
	}
}

gchar*
mucharmap_settings_get_font(void)
{
	char* font;

	if (!mucharmap_settings_initialized())
	{
		return get_default_font();
	}

	#ifdef HAVE_MATECONF
		font = mateconf_client_get_string (client, MATECONF_PREFIX"/font", NULL);
	#else
		font = g_settings_get_string(client, "font");
	#endif

	if (!font || !font[0])
	{

		g_free(font);
		return NULL;
	}

	return font;
}

void
mucharmap_settings_set_font(gchar* fontname)
{
	if (!mucharmap_settings_initialized())
	{
		return;
	}

	#ifdef HAVE_MATECONF
		mateconf_client_set_string(client, MATECONF_PREFIX"/font", fontname, NULL);
	#else
		g_settings_set_string(client, "font", fontname);
	#endif
}

gunichar
mucharmap_settings_get_last_char(void)
{
	/* See bug 469053 ? */
	gchar* str;
	gchar* endptr;
	guint64 value;

	if (!mucharmap_settings_initialized())
	{
		return mucharmap_unicode_get_locale_character();
	}

	#ifdef HAVE_MATECONF
		str = mateconf_client_get_string (client, MATECONF_PREFIX"/last_char", NULL);
	#else
		str = g_settings_get_string(client, "last-char");
	#endif

	if (!str || !g_str_has_prefix(str, "U+"))
	{
		g_free(str);
		return mucharmap_unicode_get_locale_character();
	}

	endptr = NULL;
	errno = 0;
	value = g_ascii_strtoull(str + 2 /* skip the "U+" */, &endptr, 16);

	if (errno || endptr == str || value > UNICHAR_MAX)
	{
		g_free(str);
		return mucharmap_unicode_get_locale_character();
	}

	g_free(str);

	return (gunichar) value;
}

void
mucharmap_settings_set_last_char(gunichar wc)
{
	char str[32];

	if (!mucharmap_settings_initialized())
	{
		return;
	}

	g_snprintf(str, sizeof (str), "U+%04X", wc);
	str[sizeof (str) - 1] = '\0';

	#ifdef HAVE_MATECONF
		mateconf_client_set_string(client, MATECONF_PREFIX"/last_char", str, NULL);
	#else
		g_settings_set_string(client, "last-char", str);
	#endif
}

gboolean
mucharmap_settings_get_snap_pow2(void)
{
	if (!mucharmap_settings_initialized())
	{
		return get_default_snap_pow2();
	}

	#ifdef HAVE_MATECONF
		return mateconf_client_get_bool(client, MATECONF_PREFIX"/snap_cols_pow2", NULL);
	#else
		return g_settings_get_boolean(client, "snap-cols-pow2");
	#endif
}

void
mucharmap_settings_set_snap_pow2(gboolean snap_pow2)
{
	if (!mucharmap_settings_initialized())
	{
		return;
	}

	#ifdef HAVE_MATECONF
		mateconf_client_set_bool(client, MATECONF_PREFIX"/snap_cols_pow2", snap_pow2, NULL);
	#else
		g_settings_set_boolean(client, "snap-cols-pow2", snap_pow2);
	#endif
}

/** Window state functions **/
typedef struct {
	guint timeout_id;
	int width;
	int height;
	guint is_maximised: 1;
	guint is_fullscreen: 1;
} WindowState;

static gboolean
window_state_timeout_cb(WindowState* state)
{
	#ifdef HAVE_MATECONF
		mateconf_client_set_int(client, MATECONF_PREFIX "/width", state->width, NULL);
		mateconf_client_set_int(client, MATECONF_PREFIX "/height", state->height, NULL);
	#else
		g_settings_set_int(client, "width", state->width);
		g_settings_set_int(client, "height", state->height);
	#endif

	state->timeout_id = 0;
	return FALSE;
}

static void
free_window_state (WindowState *state)
{
	if (state->timeout_id != 0)
	{
		g_source_remove(state->timeout_id);

		/* And store now */
		window_state_timeout_cb(state);
	}

	g_slice_free(WindowState, state);
}

static gboolean
window_configure_event_cb(GtkWidget* widget,
                          GdkEventConfigure* event,
                          WindowState* state)
{
	if (!state->is_maximised && !state->is_fullscreen &&
		(state->width != event->width || state->height != event->height))
	{
		state->width = event->width;
		state->height = event->height;

		if (state->timeout_id == 0)
		{
			state->timeout_id = g_timeout_add(WINDOW_STATE_TIMEOUT,
			                                  (GSourceFunc) window_state_timeout_cb,
			                                  state);
		}
	}

	return FALSE;
}

static gboolean
window_state_event_cb(GtkWidget* widget,
                      GdkEventWindowState* event,
                      WindowState* state)
{
	if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
	{
		state->is_maximised = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

		#ifdef HAVE_MATECONF
			mateconf_client_set_bool(client, MATECONF_PREFIX "/maximized", state->is_maximised, NULL);
		#else
			g_settings_set_boolean(client, "maximized", state->is_maximised);
		#endif
	}

	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
	{
		state->is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

		#ifdef HAVE_MATECONF
			mateconf_client_set_bool(client, MATECONF_PREFIX "/fullscreen", state->is_fullscreen, NULL);
		#else
			g_settings_set_boolean(client, "fullscreen", state->is_fullscreen);
		#endif
	}

	return FALSE;
}

/**
 * gucharma_settings_add_window:
 * @window: a #GtkWindow
 *
 * Restore the window configuration, and persist changes to the window configuration:
 * window width and height, and maximised and fullscreen state.
 * @window must not be realised yet.
 */
void
mucharmap_settings_add_window(GtkWindow* window)
{
	WindowState* state;
	int width, height;
	gboolean maximised, fullscreen;

	if (!mucharmap_settings_initialized())
	{
		return;
	}

	g_return_if_fail(GTK_IS_WINDOW(window));

	#if GTK_CHECK_VERSION (2,20,0)
		g_return_if_fail(!gtk_widget_get_realized(GTK_WIDGET(window)));
	#else
		g_return_if_fail(!GTK_WIDGET_REALIZED(window));
	#endif

	state = g_slice_new0(WindowState);
	g_object_set_data_full(G_OBJECT(window), "GamesConf::WindowState",
	                       state, (GDestroyNotify) free_window_state);

	g_signal_connect(window, "configure-event",
	                 G_CALLBACK(window_configure_event_cb), state);
	g_signal_connect(window, "window-state-event",
	                 G_CALLBACK(window_state_event_cb), state);

	#ifdef HAVE_MATECONF

		maximised = mateconf_client_get_bool(client, MATECONF_PREFIX "/maximized", NULL);
		fullscreen = mateconf_client_get_bool(client, MATECONF_PREFIX "/fullscreen", NULL);
		width = mateconf_client_get_int(client, MATECONF_PREFIX "/width", NULL);
		height = mateconf_client_get_int(client, MATECONF_PREFIX "/height", NULL);

	#else /* !HAVE_MATECONF */

		maximised = g_settings_get_boolean(client, "maximized");
		fullscreen = g_settings_get_boolean(client, "fullscreen");
		width = g_settings_get_int(client, "width");
		height = g_settings_get_int(client, "height");

	#endif /* HAVE_MATECONF */

	if (width > 0 && height > 0)
	{
		gtk_window_set_default_size(GTK_WINDOW(window), width, height);
	}

	if (maximised)
	{
		gtk_window_maximize(GTK_WINDOW(window));
	}

	if (fullscreen)
	{
		gtk_window_fullscreen(GTK_WINDOW(window));
	}
}
