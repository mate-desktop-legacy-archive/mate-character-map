/*
*  Copyright © 2007, 2008 Christian Persch
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3, or (at your option)
*  any later version.
*
*  This program is distributed in the hope print_operation it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MUCHARMAP_PRINT_OPERATION_H
#define MUCHARMAP_PRINT_OPERATION_H

#include <gtk/gtk.h>

#include <mucharmap/mucharmap.h>

G_BEGIN_DECLS

#define MUCHARMAP_TYPE_PRINT_OPERATION          (mucharmap_print_operation_get_type ())
#define MUCHARMAP_PRINT_OPERATION(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), MUCHARMAP_TYPE_PRINT_OPERATION, MucharmapPrintOperation))
#define MUCHARMAP_PRINT_OPERATION_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST((k), MUCHARMAP_TYPE_PRINT_OPERATION, MucharmapPrintOperationClass))
#define MUCHARMAP_IS_PRINT_OPERATION(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), MUCHARMAP_TYPE_PRINT_OPERATION))
#define MUCHARMAP_IS_PRINT_OPERATION_CLASS(k)   (G_TYPE_CHECK_CLASS_TYPE ((k), MUCHARMAP_TYPE_PRINT_OPERATION))
#define MUCHARMAP_PRINT_OPERATION_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), MUCHARMAP_TYPE_PRINT_OPERATION, MucharmapPrintOperationClass))

typedef struct _MucharmapPrintOperation         MucharmapPrintOperation;
typedef struct _MucharmapPrintOperationClass    MucharmapPrintOperationClass;
typedef struct _MucharmapPrintOperationPrivate  MucharmapPrintOperationPrivate;

struct _MucharmapPrintOperation
{
  GtkPrintOperation parent_instance;

  /*< private >*/
  MucharmapPrintOperationPrivate *priv;
};

struct _MucharmapPrintOperationClass
{
  GtkPrintOperationClass parent_class;
};

GType               mucharmap_print_operation_get_type (void);

GtkPrintOperation * mucharmap_print_operation_new      (MucharmapCodepointList *codepoint_list,
	                                                    PangoFontDescription *font_desc);

G_END_DECLS

#endif /* !MUCHARMAP_PRINT_OPERATION_H */
