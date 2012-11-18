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

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <mucharmap/mucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_SCRIPT_CODEPOINT_LIST_H
#define GUCHARMAP_SCRIPT_CODEPOINT_LIST_H

#include <glib-object.h>

#include <mucharmap/mucharmap-codepoint-list.h>

G_BEGIN_DECLS

//class MucharmapScriptCodepointList
//{
	#define GUCHARMAP_TYPE_SCRIPT_CODEPOINT_LIST             (mucharmap_script_codepoint_list_get_type ())
	#define GUCHARMAP_SCRIPT_CODEPOINT_LIST(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_SCRIPT_CODEPOINT_LIST, MucharmapScriptCodepointList))
	#define GUCHARMAP_SCRIPT_CODEPOINT_LIST_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_SCRIPT_CODEPOINT_LIST, MucharmapScriptCodepointListClass))
	#define GUCHARMAP_IS_SCRIPT_CODEPOINT_LIST(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_SCRIPT_CODEPOINT_LIST))
	#define GUCHARMAP_IS_SCRIPT_CODEPOINT_LIST_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_SCRIPT_CODEPOINT_LIST))
	#define GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_SCRIPT_CODEPOINT_LIST, MucharmapScriptCodepointListClass))

	typedef struct _MucharmapScriptCodepointList        MucharmapScriptCodepointList;
	typedef struct _MucharmapScriptCodepointListPrivate MucharmapScriptCodepointListPrivate;
	typedef struct _MucharmapScriptCodepointListClass   MucharmapScriptCodepointListClass;

	struct _MucharmapScriptCodepointList
	{
	  MucharmapCodepointList parent;

	  /*< private >*/
	  MucharmapScriptCodepointListPrivate *priv;
	};

	struct _MucharmapScriptCodepointListClass
	{
	  MucharmapCodepointListClass parent_class;
	};

	GType                    mucharmap_script_codepoint_list_get_type       (void);
	MucharmapCodepointList * mucharmap_script_codepoint_list_new            (void);
	gboolean                 mucharmap_script_codepoint_list_set_script     (MucharmapScriptCodepointList  *list,
			                                                             const gchar                   *script);
	gboolean                 mucharmap_script_codepoint_list_set_scripts    (MucharmapScriptCodepointList  *list,
			                                                             const gchar                  **scripts);
	gboolean                 mucharmap_script_codepoint_list_append_script  (MucharmapScriptCodepointList  *list,
		                                                                     const gchar                   *script);
	/* XXX: mucharmap_script_codepoint_list_get_script? seems unnecessary */
//}

G_END_DECLS

#endif /* #ifndef GUCHARMAP_SCRIPT_CODEPOINT_LIST_H */
