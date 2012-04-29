/*
 * Copyright © 2003  Sun Microsystems Inc.
 * Copyright © 2007  Christian Persch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef GUCHARMAP_CHARTABLE_ACCESSIBLE_H
#define GUCHARMAP_CHARTABLE_ACCESSIBLE_H

#include <gtk/gtk.h>

#include "mucharmap-chartable.h"

G_BEGIN_DECLS

//class MucharmapChartableAccessible
//{
	#define GUCHARMAP_TYPE_CHARTABLE_ACCESSIBLE             (mucharmap_chartable_accessible_get_type ())
	#define GUCHARMAP_CHARTABLE_ACCESSIBLE(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_CHARTABLE_ACCESSIBLE, MucharmapChartableAccessible))
	#define GUCHARMAP_CHARTABLE_ACCESSIBLE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_CHARTABLE_ACCESSIBLE, MucharmapChartableAccessibleClass))
	#define GUCHARMAP_IS_CHARTABLE_ACCESSIBLE(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_CHARTABLE_ACCESSIBLE))
	#define GUCHARMAP_IS_CHARTABLE_ACCESSIBLE_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_CHARTABLE_ACCESSIBLE))
	#define GUCHARMAP_CHARTABLE_ACCESSIBLE_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_CHARTABLE_ACCESSIBLE, MucharmapChartableAccessibleClass))

	typedef struct _MucharmapChartableAccessible      MucharmapChartableAccessible;
	typedef struct _MucharmapChartableAccessibleClass MucharmapChartableAccessibleClass;

	GType mucharmap_chartable_accessible_get_type (void);

	AtkObject *mucharmap_chartable_accessible_new (MucharmapChartable *chartable);
//}

G_END_DECLS

#endif /* !GUCHARMAP_CHARTABLE_ACCESSIBLE_H */
