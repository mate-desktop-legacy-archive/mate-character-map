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

#ifndef MUCHARMAP_BLOCK_CODEPOINT_LIST_H
#define MUCHARMAP_BLOCK_CODEPOINT_LIST_H

#include <glib-object.h>

#include <mucharmap/mucharmap-codepoint-list.h>

G_BEGIN_DECLS

//class MucharmapBlockCodepointList extends MucharmapCodepointList
//{
	#define MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST             (mucharmap_block_codepoint_list_get_type ())
	#define MUCHARMAP_BLOCK_CODEPOINT_LIST(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST, MucharmapBlockCodepointList))
	#define MUCHARMAP_BLOCK_CODEPOINT_LIST_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST, MucharmapBlockCodepointListClass))
	#define MUCHARMAP_IS_BLOCK_CODEPOINT_LIST(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST))
	#define MUCHARMAP_IS_BLOCK_CODEPOINT_LIST_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST))
	#define MUCHARMAP_BLOCK_CODEPOINT_LIST_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST, MucharmapBlockCodepointListClass))

	typedef struct _MucharmapBlockCodepointList         MucharmapBlockCodepointList;
	typedef struct _MucharmapBlockCodepointListPrivate  MucharmapBlockCodepointListPrivate;
	typedef struct _MucharmapBlockCodepointListClass    MucharmapBlockCodepointListClass;

	struct _MucharmapBlockCodepointList {
		MucharmapCodepointList parent_instance;

		/*< private >*/
		MucharmapBlockCodepointListPrivate* priv;
	};

	struct _MucharmapBlockCodepointListClass {
		MucharmapCodepointListClass parent_class;
	};

	GType                    mucharmap_block_codepoint_list_get_type (void);

	MucharmapCodepointList*  mucharmap_block_codepoint_list_new      (gunichar start,
		                                                              gunichar end);
//}
G_END_DECLS

#endif /* #ifndef MUCHARMAP_BLOCK_CODEPOINT_LIST_H */

