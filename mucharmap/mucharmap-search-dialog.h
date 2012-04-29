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

/* MucharmapSearchDialog handles all aspects of searching */

#ifndef GUCHARMAP_SEARCH_DIALOG_H
#define GUCHARMAP_SEARCH_DIALOG_H

#include <gtk/gtk.h>
#include <mucharmap/mucharmap.h>
#include "mucharmap-window.h"

G_BEGIN_DECLS

//class MucharmapSearchDialog
//{
	#define GUCHARMAP_TYPE_SEARCH_DIALOG             (mucharmap_search_dialog_get_type ())
	#define GUCHARMAP_SEARCH_DIALOG(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_SEARCH_DIALOG, MucharmapSearchDialog))
	#define GUCHARMAP_SEARCH_DIALOG_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_SEARCH_DIALOG, MucharmapSearchDialogClass))
	#define GUCHARMAP_IS_SEARCH_DIALOG(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_SEARCH_DIALOG))
	#define GUCHARMAP_IS_SEARCH_DIALOG_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_SEARCH_DIALOG))
	#define GUCHARMAP_SEARCH_DIALOG_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_SEARCH_DIALOG, MucharmapSearchDialogClass))

	typedef struct _MucharmapSearchDialog MucharmapSearchDialog;
	typedef struct _MucharmapSearchDialogClass MucharmapSearchDialogClass;

	struct _MucharmapSearchDialog
	{
	  GtkDialog parent;
	};

	struct _MucharmapSearchDialogClass
	{
	  GtkDialogClass parent_class;

	  /* signals */
	  void (* search_start)  (void);
	  void (* search_finish) (gunichar found_char);
	};

	typedef enum
	{
	  GUCHARMAP_DIRECTION_BACKWARD = -1,
	  GUCHARMAP_DIRECTION_FORWARD = 1
	}
	MucharmapDirection;

	GType       mucharmap_search_dialog_get_type      (void);
	GtkWidget * mucharmap_search_dialog_new           (MucharmapWindow *parent);
	void        mucharmap_search_dialog_present       (MucharmapSearchDialog *search_dialog);
	void        mucharmap_search_dialog_start_search  (MucharmapSearchDialog *search_dialog,
		                                               MucharmapDirection     direction);
	gdouble     mucharmap_search_dialog_get_completed (MucharmapSearchDialog *search_dialog); 
//}

G_END_DECLS

#endif /* #ifndef GUCHARMAP_SEARCH_DIALOG_H */
