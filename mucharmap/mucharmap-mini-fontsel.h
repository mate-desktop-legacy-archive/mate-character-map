/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2008 Christian Persch
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

#ifndef GUCHARMAP_MINI_FONTSEL_H
#define GUCHARMAP_MINI_FONTSEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

//class MucharmapMiniFontSelection
//{
	#define GUCHARMAP_TYPE_MINI_FONT_SELECTION             (mucharmap_mini_font_selection_get_type ())
	#define GUCHARMAP_MINI_FONT_SELECTION(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_MINI_FONT_SELECTION, MucharmapMiniFontSelection))
	#define GUCHARMAP_MINI_FONT_SELECTION_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_MINI_FONT_SELECTION, MucharmapMiniFontSelectionClass))
	#define GUCHARMAP_IS_MINI_FONT_SELECTION(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_MINI_FONT_SELECTION))
	#define GUCHARMAP_IS_MINI_FONT_SELECTION_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_MINI_FONT_SELECTION))
	#define GUCHARMAP_MINI_FONT_SELECTION_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_MINI_FONT_SELECTION, MucharmapMiniFontSelectionClass))

	typedef struct _MucharmapMiniFontSelection MucharmapMiniFontSelection;
	typedef struct _MucharmapMiniFontSelectionClass MucharmapMiniFontSelectionClass;

	struct _MucharmapMiniFontSelection
	{
	  GtkHBox parent;

	  GtkListStore         *family_store;
	  GtkWidget            *family; /* combo box */
	  GtkWidget            *bold;   /* toggle button*/
	  GtkWidget            *italic; /* toggle button*/

	  GtkObject            *size_adj; 
	  GtkWidget            *size;   /* spin button */
	  
	  PangoFontDescription *font_desc;

	  gint                  default_size;
	};

	struct _MucharmapMiniFontSelectionClass
	{
	  GtkHBoxClass parent_class;
	};


	GType                  mucharmap_mini_font_selection_get_type         (void);

	GtkWidget *            mucharmap_mini_font_selection_new              (void);

	void                   mucharmap_mini_font_selection_set_font_desc    (MucharmapMiniFontSelection *fontsel,
		                                                                   PangoFontDescription       *font_desc);

	PangoFontDescription * mucharmap_mini_font_selection_get_font_desc    (MucharmapMiniFontSelection *fontsel);

	void                   mucharmap_mini_font_selection_change_font_size (MucharmapMiniFontSelection *fontsel,
		                                                                   float factor);

	void                   mucharmap_mini_font_selection_reset_font_size  (MucharmapMiniFontSelection *fontsel);
//}
G_END_DECLS

#endif /* #ifndef GUCHARMAP_MINI_FONTSEL_H */
