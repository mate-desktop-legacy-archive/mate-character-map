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
 
#ifndef MUCHARMAP_WINDOW_H
#define MUCHARMAP_WINDOW_H

#include <gtk/gtk.h>
#include <mucharmap/mucharmap.h>
#include "mucharmap-mini-fontsel.h"

G_BEGIN_DECLS

//class MucharmapWindow extends GtkWindow
//{
	#define MUCHARMAP_TYPE_WINDOW             (mucharmap_window_get_type ())
	#define MUCHARMAP_WINDOW(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_WINDOW, MucharmapWindow))
	#define MUCHARMAP_WINDOW_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_WINDOW, MucharmapWindowClass))
	#define MUCHARMAP_IS_WINDOW(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_WINDOW))
	#define MUCHARMAP_IS_WINDOW_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_WINDOW))
	#define MUCHARMAP_WINDOW_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_WINDOW, MucharmapWindowClass))

	typedef struct _MucharmapWindow MucharmapWindow;
	typedef struct _MucharmapWindowClass MucharmapWindowClass;

	struct _MucharmapWindow {
		GtkWindow parent;

		MucharmapCharmap *charmap;
		GtkWidget* status;

		GtkWidget* fontsel;
		GtkWidget* text_to_copy_entry;

		GtkUIManager* uimanager;

		GtkActionGroup* action_group;

		GtkWidget* search_dialog; /* takes care of all aspects of searching */

		GtkWidget* progress;

		guint save_last_char_idle_id;

		GtkPageSetup* page_setup;
		GtkPrintSettings* print_settings;

		guint in_notification : 1;
	};

	struct _MucharmapWindowClass {
		GtkWindowClass parent_class;
	};

	#define MUCHARMAP_ICON_NAME "accessories-character-map"

	GType       mucharmap_window_get_type  (void);

	GtkWidget*  mucharmap_window_new       (void);

	void        mucharmap_window_set_font  (MucharmapWindow* guw,
		                                    const char*      font);

	GdkCursor* _mucharmap_window_progress_cursor (void);
//}

G_END_DECLS

#endif /* #ifndef MUCHARMAP_WINDOW_H */
