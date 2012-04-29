/*
 * Copyright © 2004 Noah Levitt
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

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <mucharmap/mucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_CHARTABLE_H
#define GUCHARMAP_CHARTABLE_H

#include <gtk/gtk.h>

#include <mucharmap/mucharmap-codepoint-list.h>

G_BEGIN_DECLS

//class MucharmapChartable extends GtkDrawingArea
//{
	#define GUCHARMAP_TYPE_CHARTABLE             (mucharmap_chartable_get_type ())
	#define GUCHARMAP_CHARTABLE(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_CHARTABLE, MucharmapChartable))
	#define GUCHARMAP_CHARTABLE_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_CHARTABLE, MucharmapChartableClass))
	#define GUCHARMAP_IS_CHARTABLE(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_CHARTABLE))
	#define GUCHARMAP_IS_CHARTABLE_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_CHARTABLE))
	#define GUCHARMAP_CHARTABLE_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_CHARTABLE, MucharmapChartableClass))

	typedef struct _MucharmapChartable        MucharmapChartable;
	typedef struct _MucharmapChartablePrivate MucharmapChartablePrivate;
	typedef struct _MucharmapChartableClass   MucharmapChartableClass;

	struct _MucharmapChartable
	{
	  GtkDrawingArea parent_instance;

	  /*< private >*/
	  MucharmapChartablePrivate *priv;
	};

	struct _MucharmapChartableClass
	{
	  GtkDrawingAreaClass parent_class;

	  void    (* set_scroll_adjustments) (MucharmapChartable *chartable,
		                                  GtkAdjustment      *hadjustment,
		                                  GtkAdjustment      *vadjustment);
	  gboolean (* move_cursor)           (MucharmapChartable *chartable,
		                                  GtkMovementStep     step,
		                                  gint                count);
	  void (* activate) (MucharmapChartable *chartable);
	  void (* copy_clipboard) (MucharmapChartable *chartable);
	  void (* paste_clipboard) (MucharmapChartable *chartable);

	  void (* set_active_char) (MucharmapChartable *chartable, guint ch);
	  void (* status_message) (MucharmapChartable *chartable, const gchar *message);
	};

	GType mucharmap_chartable_get_type (void);
	GtkWidget * mucharmap_chartable_new (void);
	void mucharmap_chartable_set_font_desc (MucharmapChartable *chartable,
		                                    PangoFontDescription *font_desc);
	PangoFontDescription * mucharmap_chartable_get_font_desc (MucharmapChartable *chartable);
	gunichar mucharmap_chartable_get_active_character (MucharmapChartable *chartable);
	void mucharmap_chartable_set_active_character (MucharmapChartable *chartable, 
		                                           gunichar uc);
	void mucharmap_chartable_set_zoom_enabled (MucharmapChartable *chartable,
		                                       gboolean enabled);
	gboolean mucharmap_chartable_get_zoom_enabled (MucharmapChartable *chartable);
	void mucharmap_chartable_set_snap_pow2 (MucharmapChartable *chartable,
		                                    gboolean snap);
	gboolean mucharmap_chartable_get_snap_pow2 (MucharmapChartable *chartable);
	void mucharmap_chartable_set_codepoint_list (MucharmapChartable         *chartable,
		                                         MucharmapCodepointList *list);
	MucharmapCodepointList * mucharmap_chartable_get_codepoint_list (MucharmapChartable *chartable);
//}

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARTABLE_H */
