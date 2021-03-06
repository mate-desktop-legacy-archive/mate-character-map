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

#ifndef MUCHARMAP_SCRIPT_CHAPTERS_MODEL_H
#define MUCHARMAP_SCRIPT_CHAPTERS_MODEL_H

#include <mucharmap/mucharmap-chapters-model.h>

G_BEGIN_DECLS

//class MucharmapScriptChaptersModel
//{
	#define MUCHARMAP_TYPE_SCRIPT_CHAPTERS_MODEL             (mucharmap_script_chapters_model_get_type ())
	#define MUCHARMAP_SCRIPT_CHAPTERS_MODEL(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_SCRIPT_CHAPTERS_MODEL, MucharmapScriptChaptersModel))
	#define MUCHARMAP_SCRIPT_CHAPTERS_MODEL_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_SCRIPT_CHAPTERS_MODEL, MucharmapScriptChaptersModelClass))
	#define MUCHARMAP_IS_SCRIPT_CHAPTERS_MODEL(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_SCRIPT_CHAPTERS_MODEL))
	#define MUCHARMAP_IS_SCRIPT_CHAPTERS_MODEL_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_SCRIPT_CHAPTERS_MODEL))
	#define MUCHARMAP_SCRIPT_CHAPTERS_MODEL_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_SCRIPT_CHAPTERS_MODEL, MucharmapScriptChaptersModelClass))

	typedef struct _MucharmapScriptChaptersModel        MucharmapScriptChaptersModel;
	typedef struct _MucharmapScriptChaptersModelPrivate MucharmapScriptChaptersModelPrivate;
	typedef struct _MucharmapScriptChaptersModelClass   MucharmapScriptChaptersModelClass;

	struct _MucharmapScriptChaptersModel
	{
	  MucharmapChaptersModel parent;

	  /*< private >*/
	  MucharmapScriptChaptersModelPrivate *priv;
	};

	struct _MucharmapScriptChaptersModelClass
	{
	  MucharmapChaptersModelClass parent_class;
	};

	GType                   mucharmap_script_chapters_model_get_type (void);
	MucharmapChaptersModel* mucharmap_script_chapters_model_new      (void);
//}

G_END_DECLS

#endif /* #ifndef MUCHARMAP_SCRIPT_CHAPTERS_MODEL_H */
