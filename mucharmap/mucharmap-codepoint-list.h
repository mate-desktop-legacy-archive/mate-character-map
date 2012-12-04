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

#if !defined (__MUCHARMAP_MUCHARMAP_H_INSIDE__) && !defined (MUCHARMAP_COMPILATION)
#error "Only <mucharmap/mucharmap.h> can be included directly."
#endif

#ifndef MUCHARMAP_CODEPOINT_LIST_H
#define MUCHARMAP_CODEPOINT_LIST_H

#include <glib-object.h>

G_BEGIN_DECLS

//class MucharmapCodepointList extends GObject
//{
	#define MUCHARMAP_TYPE_CODEPOINT_LIST             (mucharmap_codepoint_list_get_type ())
	#define MUCHARMAP_CODEPOINT_LIST(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_CODEPOINT_LIST, MucharmapCodepointList))
	#define MUCHARMAP_CODEPOINT_LIST_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_CODEPOINT_LIST, MucharmapCodepointListClass))
	#define MUCHARMAP_IS_CODEPOINT_LIST(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_CODEPOINT_LIST))
	#define MUCHARMAP_IS_CODEPOINT_LIST_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_CODEPOINT_LIST))
	#define MUCHARMAP_CODEPOINT_LIST_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_CODEPOINT_LIST, MucharmapCodepointListClass))

	typedef struct _MucharmapCodepointList        MucharmapCodepointList;
	typedef struct _MucharmapCodepointListPrivate MucharmapCodepointListPrivate;
	typedef struct _MucharmapCodepointListClass   MucharmapCodepointListClass;

	struct _MucharmapCodepointList
	{
	  GObject parent_instance;

	  /*< private >*/
	  MucharmapCodepointListPrivate *priv;
	};

	struct _MucharmapCodepointListClass
	{
	  GObjectClass parent_class;

	  /* zero is the first index */
	  gint     (*get_last_index) (MucharmapCodepointList *list);
	  gunichar (*get_char)       (MucharmapCodepointList *list, 
								  gint                    index);
	  gint     (*get_index)      (MucharmapCodepointList *list, 
								  gunichar                wc);
	};

	GType                    mucharmap_codepoint_list_get_type       (void);

	gunichar                 mucharmap_codepoint_list_get_char       (MucharmapCodepointList *list,
																	  gint                    index);

	gint                     mucharmap_codepoint_list_get_index      (MucharmapCodepointList *list, 
																	  gunichar                wc);

	gint                     mucharmap_codepoint_list_get_last_index (MucharmapCodepointList *list);
//}

G_END_DECLS

#endif /* #ifndef MUCHARMAP_CODEPOINT_LIST_H */
