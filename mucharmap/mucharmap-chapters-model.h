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
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#if !defined (__MUCHARMAP_MUCHARMAP_H_INSIDE__) && !defined (MUCHARMAP_COMPILATION)
#error "Only <mucharmap/mucharmap.h> can be included directly."
#endif

#ifndef MUCHARMAP_CHAPTERS_MODEL_H
#define MUCHARMAP_CHAPTERS_MODEL_H

#include <gtk/gtk.h>

#include <mucharmap/mucharmap-codepoint-list.h>

G_BEGIN_DECLS

//class MucharmapChaptersModel extends GtkListStore
//{
	#define MUCHARMAP_TYPE_CHAPTERS_MODEL             (mucharmap_chapters_model_get_type ())
	#define MUCHARMAP_CHAPTERS_MODEL(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_CHAPTERS_MODEL, MucharmapChaptersModel))
	#define MUCHARMAP_CHAPTERS_MODEL_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_CHAPTERS_MODEL, MucharmapChaptersModelClass))
	#define MUCHARMAP_IS_CHAPTERS_MODEL(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_CHAPTERS_MODEL))
	#define MUCHARMAP_IS_CHAPTERS_MODEL_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_CHAPTERS_MODEL))
	#define MUCHARMAP_CHAPTERS_MODEL_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_CHAPTERS_MODEL, MucharmapChaptersModelClass))

	typedef struct _MucharmapChaptersModel        MucharmapChaptersModel;
	typedef struct _MucharmapChaptersModelPrivate MucharmapChaptersModelPrivate;
	typedef struct _MucharmapChaptersModelClass   MucharmapChaptersModelClass;

	struct _MucharmapChaptersModel
	{
	  GtkListStore parent_instance;

	  /*< private >*/
	  MucharmapChaptersModelPrivate *priv;
	};

	struct _MucharmapChaptersModelClass
	{
	  GtkListStoreClass parent_class;

	  const char *title;
	  gboolean (* character_to_iter) (MucharmapChaptersModel *chapters,
		                              gunichar wc,
		                              GtkTreeIter *iter);
	  MucharmapCodepointList * (* get_codepoint_list) (MucharmapChaptersModel *chapters,
		                                               GtkTreeIter *iter);
	  MucharmapCodepointList * (* get_book_codepoint_list) (MucharmapChaptersModel *chapters);
	};


	enum {
	  MUCHARMAP_CHAPTERS_MODEL_COLUMN_ID    = 0,
	  MUCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL = 1
	};

	GType                    mucharmap_chapters_model_get_type                (void);
	MucharmapCodepointList * mucharmap_chapters_model_get_codepoint_list      (MucharmapChaptersModel *chapters,
		                                                                                      GtkTreeIter            *iter);
	const char *             mucharmap_chapters_model_get_title               (MucharmapChaptersModel *chapters);
	MucharmapCodepointList * mucharmap_chapters_model_get_book_codepoint_list (MucharmapChaptersModel *chapters);
	gboolean                 mucharmap_chapters_model_character_to_iter       (MucharmapChaptersModel *chapters,
		                                                                       gunichar                wc,
		                                                                       GtkTreeIter            *iter);
	gboolean                 mucharmap_chapters_model_id_to_iter              (MucharmapChaptersModel *model,
		                                                                       const char             *id,
		                                                                       GtkTreeIter            *iter);
//}

G_END_DECLS

#endif /* #ifndef MUCHARMAP_CHAPTERS_MODEL_H */
