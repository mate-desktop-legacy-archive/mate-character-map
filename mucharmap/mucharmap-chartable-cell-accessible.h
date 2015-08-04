/*
 * Copyright © 2003  Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE_H
#define MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE_H

#include <atk/atk.h>

#include "mucharmap-chartable.h"

G_BEGIN_DECLS

#define MUCHARMAP_TYPE_CHARTABLE_CELL_ACCESSIBLE             (mucharmap_chartable_cell_accessible_get_type ())
#define MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_CHARTABLE_CELL_ACCESSIBLE, MucharmapChartableCellAccessible))
#define MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_CHARTABLE_CELL_ACCESSIBLE, MucharmapChartableCellAccessibleClass))
#define MUCHARMAP_IS_CHARTABLE_CELL_ACCESSIBLE(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_CHARTABLE_CELL_ACCESSIBLE))
#define MUCHARMAP_IS_CHARTABLE_CELL_ACCESSIBLE_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_CHARTABLE_CELL_ACCESSIBLE))
#define MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_CHARTABLE_CELL_ACCESSIBLE, MucharmapChartableCellAccessibleClass))

typedef struct _MucharmapChartableCellAccessible      MucharmapChartableCellAccessible;
typedef struct _MucharmapChartableCellAccessibleClass MucharmapChartableCellAccessibleClass;

struct _MucharmapChartableCellAccessible
{
  AtkObject parent;

  GtkWidget    *widget;
  int           index;
  AtkStateSet  *state_set;
  gchar        *activate_description;
  guint         action_idle_handler;
};

struct _MucharmapChartableCellAccessibleClass
{
  AtkObjectClass parent_class;
};

GType mucharmap_chartable_cell_accessible_get_type (void);

AtkObject* mucharmap_chartable_cell_accessible_new (void);

void mucharmap_chartable_cell_accessible_initialise (MucharmapChartableCellAccessible *cell,
	                                                 GtkWidget          *widget,
	                                                 AtkObject          *parent,
	                                                 gint                index);

gboolean mucharmap_chartable_cell_accessible_add_state (MucharmapChartableCellAccessible *cell,
	                                                    AtkStateType        state_type,
	                                                    gboolean            emit_signal);

gboolean mucharmap_chartable_cell_accessible_remove_state (MucharmapChartableCellAccessible *cell,
	                                                       AtkStateType        state_type,
	                                                       gboolean            emit_signal);

G_END_DECLS

#endif /* MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE_H */
