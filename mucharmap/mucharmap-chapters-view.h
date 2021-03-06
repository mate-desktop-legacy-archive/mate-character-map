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
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

/* block means unicode block */

#if !defined (__MUCHARMAP_MUCHARMAP_H_INSIDE__) && !defined (MUCHARMAP_COMPILATION)
#error "Only <mucharmap/mucharmap.h> can be included directly."
#endif

#ifndef MUCHARMAP_CHAPTERS_VIEW_H
#define MUCHARMAP_CHAPTERS_VIEW_H

#include <gtk/gtk.h>

#include <mucharmap/mucharmap-chapters-model.h>

G_BEGIN_DECLS

#define MUCHARMAP_TYPE_CHAPTERS_VIEW             (mucharmap_chapters_view_get_type ())
#define MUCHARMAP_CHAPTERS_VIEW(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_CHAPTERS_VIEW, MucharmapChaptersView))
#define MUCHARMAP_CHAPTERS_VIEW_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_CHAPTERS_VIEW, MucharmapChaptersViewClass))
#define MUCHARMAP_IS_CHAPTERS_VIEW(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_CHAPTERS_VIEW))
#define MUCHARMAP_IS_CHAPTERS_VIEW_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_CHAPTERS_VIEW))
#define MUCHARMAP_CHAPTERS_VIEW_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_CHAPTERS_VIEW, MucharmapChaptersViewClass))

typedef struct _MucharmapChaptersView         MucharmapChaptersView;
typedef struct _MucharmapChaptersViewPrivate  MucharmapChaptersViewPrivate;
typedef struct _MucharmapChaptersViewClass    MucharmapChaptersViewClass;

struct _MucharmapChaptersView
{
  GtkTreeView parent_instance;

  /*< private >*/
  MucharmapChaptersViewPrivate *priv;
};

struct _MucharmapChaptersViewClass
{
  GtkTreeViewClass parent_class;
};

GType       mucharmap_chapters_view_get_type (void);

GtkWidget * mucharmap_chapters_view_new      (void);

void                    mucharmap_chapters_view_set_model (MucharmapChaptersView *view,
	                                                       MucharmapChaptersModel *model);
MucharmapChaptersModel *mucharmap_chapters_view_get_model (MucharmapChaptersView *view);

gboolean           mucharmap_chapters_view_select_character (MucharmapChaptersView *view,
	                                                         gunichar           wc);
MucharmapCodepointList * mucharmap_chapters_view_get_codepoint_list      (MucharmapChaptersView *view);
MucharmapCodepointList * mucharmap_chapters_view_get_book_codepoint_list (MucharmapChaptersView *view);

void               mucharmap_chapters_view_next         (MucharmapChaptersView *view);
void               mucharmap_chapters_view_previous     (MucharmapChaptersView *view);

gchar *            mucharmap_chapters_view_get_selected  (MucharmapChaptersView *view);
gboolean           mucharmap_chapters_view_set_selected  (MucharmapChaptersView *view,
	                                                      const gchar       *name);
G_END_DECLS

#endif /* #ifndef MUCHARMAP_CHAPTERS_VIEW_H */
