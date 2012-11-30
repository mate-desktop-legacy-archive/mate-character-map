/*
 * Copyright © 2003  Sun Microsystems Inc.
 * Copyright © 2007  Christian Persch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <stdlib.h>

#include <gtk/gtk.h>

#include "mucharmap.h"
#include "mucharmap-chartable.h"
#include "mucharmap-chartable-accessible.h"
#include "mucharmap-chartable-cell-accessible.h"
#include "mucharmap-private.h"

//class MucharmapChartableCellAccessible extends AtkObject
//{
	static void mucharmap_chartable_cell_accessible_class_init (MucharmapChartableCellAccessibleClass *klass);
	static void mucharmap_chartable_cell_accessible_init (MucharmapChartableCellAccessible *cell);
	static void mucharmap_chartable_cell_accessible_component_interface_init (AtkComponentIface *iface);
	static void mucharmap_chartable_cell_accessible_action_interface_init (AtkActionIface *iface);

	G_DEFINE_TYPE_WITH_CODE (MucharmapChartableCellAccessible,
		                     mucharmap_chartable_cell_accessible,
		                     ATK_TYPE_OBJECT,
		                     G_IMPLEMENT_INTERFACE (ATK_TYPE_COMPONENT,
		                                            mucharmap_chartable_cell_accessible_component_interface_init)
		                     G_IMPLEMENT_INTERFACE (ATK_TYPE_ACTION,
		                                            mucharmap_chartable_cell_accessible_action_interface_init))

	static void
	mucharmap_chartable_cell_accessible_destroyed (GtkWidget       *widget,
		                MucharmapChartableCellAccessible        *cell)
	{
	  /*
	   * This is the signal handler for the "destroy" signal for the
	   * GtkWidget. We set the  pointer location to NULL;
	   */
	  cell->widget = NULL;
	}

	static gint
	mucharmap_chartable_cell_accessible_get_index_in_parent (AtkObject *obj)
	{
	  MucharmapChartableCellAccessible *cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (obj);

	  return cell->index;
	}


	static AtkStateSet *
	mucharmap_chartable_cell_accessible_ref_state_set (AtkObject *obj)
	{
	  MucharmapChartableCellAccessible *cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (obj);
	  g_return_val_if_fail (cell->state_set, NULL);

	  g_object_ref(cell->state_set);
	  return cell->state_set;
	}

	static void
	mucharmap_chartable_cell_accessible_get_extents (AtkComponent *component,
		                             gint         *x,
		                             gint         *y,
		                             gint         *width,
		                             gint         *height,
		                             AtkCoordType coord_type)
	{
	  MucharmapChartableCellAccessible *cell;
	  AtkObject *cell_parent;
	  MucharmapChartable *chartable;
	  MucharmapChartablePrivate *chartable_priv;
	  gint real_x, real_y, real_width, real_height;
	  gint row, column;

	  cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (component);

	  cell_parent = atk_object_get_parent (ATK_OBJECT (cell));

	  /*
	   * Is the cell visible on the screen
	   */
	  chartable = MUCHARMAP_CHARTABLE (cell->widget);
	  chartable_priv = chartable->priv;

	  if (cell->index >= chartable_priv->page_first_cell && cell->index < chartable_priv->page_first_cell + chartable_priv->rows * chartable_priv->cols)
		{
		  atk_component_get_extents (ATK_COMPONENT (cell_parent),
		                             &real_x, &real_y, &real_width, &real_height,
		                             coord_type);
		  row = (cell->index - chartable_priv->page_first_cell)/ chartable_priv->cols;
		  column = _mucharmap_chartable_cell_column (chartable, cell->index);
		  *x = real_x + _mucharmap_chartable_x_offset (chartable, column);
		  *y = real_y + _mucharmap_chartable_y_offset (chartable, row);
		  *width = _mucharmap_chartable_column_width (chartable, column);
		  *height = _mucharmap_chartable_row_height (chartable, row);
		}
	  else
		{
		  *x = G_MININT;
		  *y = G_MININT;
		}
	}


	static gboolean
	mucharmap_chartable_cell_accessible_grab_focus (AtkComponent *component)
	{
	  MucharmapChartableCellAccessible *cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (component);
	  MucharmapChartable *chartable;

	  chartable = MUCHARMAP_CHARTABLE (cell->widget);
	  /* FIXME: this looks wrong, index is the index in the codepoint list, not the character itself */
	  mucharmap_chartable_set_active_character (chartable, cell->index);

	  return TRUE;
	}


	static gint
	mucharmap_chartable_cell_accessible_action_get_n_actions (AtkAction *action)
	{
	  return 1;
	}


	static gboolean
	idle_do_action (gpointer data)
	{
	  MucharmapChartableCellAccessible *cell;
	  MucharmapChartable *chartable;

	  cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (data);

	  chartable = MUCHARMAP_CHARTABLE (cell->widget);
	  mucharmap_chartable_set_active_character (chartable, cell->index);
	  g_signal_emit_by_name (chartable, "activate");
	  return FALSE;
	}


	static gboolean
	mucharmap_chartable_cell_accessible_action_do_action (AtkAction *action,
		                       gint      index)
	{
	  MucharmapChartableCellAccessible *cell;

	  cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (action);
	  if (index == 0)
		{
		  if (cell->action_idle_handler)
		    return FALSE;
		  cell->action_idle_handler = g_idle_add (idle_do_action, cell);
		  return TRUE;
		}
	  else
		return FALSE;
	}


	static const gchar* mucharmap_chartable_cell_accessible_action_get_name(AtkAction* action, gint index)
	{
		if (index == 0)
		{
			return "activate";
		}
		else
		{
			return NULL;
		}
	}


	static const gchar* mucharmap_chartable_cell_accessible_action_get_description(AtkAction* action, gint index)
	{
		MucharmapChartableCellAccessible* cell;

		cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE(action);

		if (index == 0)
		{
			return cell->activate_description;
		}
		else
		{
			return NULL;
		}
	}


	static gboolean
	mucharmap_chartable_cell_accessible_action_set_description (AtkAction   *action,
					     gint        index,
		                             const gchar *desc)
	{
	  MucharmapChartableCellAccessible *cell;

	  cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (action);
	  if (index == 0)
		{
		  g_free (cell->activate_description);
		  cell->activate_description = g_strdup (desc);
		  return TRUE;
		}
	  else
		return FALSE;

	}

	static void
	mucharmap_chartable_cell_accessible_init (MucharmapChartableCellAccessible *cell)
	{
	  cell->state_set = atk_state_set_new ();
	  cell->widget = NULL;
	  cell->index = 0;
	  cell->action_idle_handler = 0;
	  atk_state_set_add_state (cell->state_set, ATK_STATE_TRANSIENT);
	  atk_state_set_add_state (cell->state_set, ATK_STATE_ENABLED);
	}

	static void
	mucharmap_chartable_cell_accessible_object_finalize (GObject *obj)
	{
	  MucharmapChartableCellAccessible *cell = MUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (obj);

	  g_free (cell->activate_description);

	  if (cell->action_idle_handler)
		{
		  g_source_remove (cell->action_idle_handler);
		  cell->action_idle_handler = 0;
		}

	  if (cell->state_set)
		g_object_unref (cell->state_set);

	  G_OBJECT_CLASS (mucharmap_chartable_cell_accessible_parent_class)->finalize (obj);
	}

	static void
	mucharmap_chartable_cell_accessible_class_init (MucharmapChartableCellAccessibleClass *klass)
	{
	  AtkObjectClass *atk_object_class = ATK_OBJECT_CLASS (klass);
	  GObjectClass *object_class = G_OBJECT_CLASS (klass);

	  object_class->finalize = mucharmap_chartable_cell_accessible_object_finalize;

	  atk_object_class->get_index_in_parent = mucharmap_chartable_cell_accessible_get_index_in_parent;
	  atk_object_class->ref_state_set = mucharmap_chartable_cell_accessible_ref_state_set;
	}

	static void
	mucharmap_chartable_cell_accessible_component_interface_init (AtkComponentIface *iface)
	{
	  iface->get_extents = mucharmap_chartable_cell_accessible_get_extents;
	  iface->grab_focus = mucharmap_chartable_cell_accessible_grab_focus;
	}

	static void
	mucharmap_chartable_cell_accessible_action_interface_init (AtkActionIface *iface)
	{
	  iface->get_n_actions = mucharmap_chartable_cell_accessible_action_get_n_actions;
	  iface->do_action = mucharmap_chartable_cell_accessible_action_do_action;
	  iface->get_name = mucharmap_chartable_cell_accessible_action_get_name;
	  iface->get_description = mucharmap_chartable_cell_accessible_action_get_description;
	  iface->set_description = mucharmap_chartable_cell_accessible_action_set_description;
	}

	/* API */

	AtkObject *
	mucharmap_chartable_cell_accessible_new (void)
	{
	  GObject *object;
	  AtkObject *atk_object;

	  object = g_object_new (mucharmap_chartable_cell_accessible_get_type (), NULL);

	  atk_object = ATK_OBJECT (object);
	  atk_object->role = ATK_ROLE_TABLE_CELL;

	  return atk_object;
	}

	void
	mucharmap_chartable_cell_accessible_initialise (MucharmapChartableCellAccessible   *cell,
		                                            GtkWidget *widget,
		                                            AtkObject *parent,
		                                            gint      index)
	{
	  cell->widget = widget;
	  atk_object_set_parent (ATK_OBJECT (cell), parent);
	  cell->index = index;
	  cell->activate_description = g_strdup ("Activate the cell"); /* FIXMEchpe i18n !! */

	  g_signal_connect_object (G_OBJECT (widget),
		                       "destroy",
		                       G_CALLBACK (mucharmap_chartable_cell_accessible_destroyed),
		                       cell, 0);
	}

	gboolean
	mucharmap_chartable_cell_accessible_add_state (MucharmapChartableCellAccessible      *cell,
		                                           AtkStateType state_type,
		                                           gboolean     emit_signal)
	{
	  if (!atk_state_set_contains_state (cell->state_set, state_type))
		{
		  gboolean rc;

		  rc = atk_state_set_add_state (cell->state_set, state_type);
		  /*
		   * The signal should only be generated if the value changed,
		   * not when the cell is set up.  So states that are set
		   * initially should pass FALSE as the emit_signal argument.
		   */

		  if (emit_signal)
		    {
		      atk_object_notify_state_change (ATK_OBJECT (cell), state_type, TRUE);
		      /* If state_type is ATK_STATE_VISIBLE, additional notification */
		      if (state_type == ATK_STATE_VISIBLE)
		        g_signal_emit_by_name (cell, "visible_data_changed");
		    }

		  return rc;
		}
	  else
		return FALSE;
	}

	gboolean
	mucharmap_chartable_cell_accessible_remove_state (MucharmapChartableCellAccessible *cell,
		                                              AtkStateType state_type,
		                                              gboolean emit_signal)
	{
	  if (atk_state_set_contains_state (cell->state_set, state_type))
		{
		  gboolean rc;

		  rc = atk_state_set_remove_state (cell->state_set, state_type);
		  /*
		   * The signal should only be generated if the value changed,
		   * not when the cell is set up.  So states that are set
		   * initially should pass FALSE as the emit_signal argument.
		   */

		  if (emit_signal)
		    {
		      atk_object_notify_state_change (ATK_OBJECT (cell), state_type, FALSE);
		      /* If state_type is ATK_STATE_VISIBLE, additional notification */
		      if (state_type == ATK_STATE_VISIBLE)
		        g_signal_emit_by_name (cell, "visible_data_changed");
		    }

		  return rc;
		}
	  else
		return FALSE;
	}
//}
