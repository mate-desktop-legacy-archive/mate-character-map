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

#include <config.h>
#include <glib.h>

#include "mucharmap.h"
#include "mucharmap-private.h"

struct _MucharmapBlockCodepointListPrivate {
	gunichar start;
	gunichar end;
};

enum {
	PROP_0,
	PROP_FIRST_CODEPOINT,
	PROP_LAST_CODEPOINT
};

G_DEFINE_TYPE (MucharmapBlockCodepointList, mucharmap_block_codepoint_list, MUCHARMAP_TYPE_CODEPOINT_LIST)

static gunichar
get_char (MucharmapCodepointList *list,
	      gint                    index)
{
  MucharmapBlockCodepointList *block_list = MUCHARMAP_BLOCK_CODEPOINT_LIST (list);
  MucharmapBlockCodepointListPrivate *priv = block_list->priv;

  if (index > (gint)priv->end - priv->start)
	return (gunichar)(-1);
  else
	return (gunichar) priv->start + index;
}

static gint
get_index (MucharmapCodepointList *list,
	       gunichar                wc)
{
  MucharmapBlockCodepointList *block_list = MUCHARMAP_BLOCK_CODEPOINT_LIST (list);
  MucharmapBlockCodepointListPrivate *priv = block_list->priv;

  if (wc < priv->start || wc > priv->end)
	return -1;
  else
	return wc - priv->start;
}

static gint
get_last_index (MucharmapCodepointList *list)
{
  MucharmapBlockCodepointList *block_list = MUCHARMAP_BLOCK_CODEPOINT_LIST (list);
  MucharmapBlockCodepointListPrivate *priv = block_list->priv;

  return priv->end - priv->start;
}

static void
mucharmap_block_codepoint_list_init (MucharmapBlockCodepointList *list)
{
  list->priv = G_TYPE_INSTANCE_GET_PRIVATE (list, MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST, MucharmapBlockCodepointListPrivate);
}

static GObject *
mucharmap_block_codepoint_list_constructor (GType type,
	                                        guint n_construct_properties,
	                                        GObjectConstructParam *construct_params)
{
  GObject *object;
  MucharmapBlockCodepointList *block_codepoint_list;
  MucharmapBlockCodepointListPrivate *priv;

  object = G_OBJECT_CLASS (mucharmap_block_codepoint_list_parent_class)->constructor
	         (type, n_construct_properties, construct_params);

  block_codepoint_list = MUCHARMAP_BLOCK_CODEPOINT_LIST (object);
  priv = block_codepoint_list->priv;

  g_assert (priv->start <= priv->end);

  return object;
}

static void
mucharmap_block_codepoint_list_set_property (GObject *object,
	                                         guint prop_id,
	                                         const GValue *value,
	                                         GParamSpec *pspec)
{
  MucharmapBlockCodepointList *block_codepoint_list = MUCHARMAP_BLOCK_CODEPOINT_LIST (object);
  MucharmapBlockCodepointListPrivate *priv = block_codepoint_list->priv;

  switch (prop_id) {
	case PROP_FIRST_CODEPOINT:
	  priv->start = g_value_get_uint (value);
	  break;
	case PROP_LAST_CODEPOINT:
	  priv->end = g_value_get_uint (value);
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
mucharmap_block_codepoint_list_get_property (GObject *object,
	                                         guint prop_id,
	                                         GValue *value,
	                                         GParamSpec *pspec)
{
  MucharmapBlockCodepointList *block_codepoint_list = MUCHARMAP_BLOCK_CODEPOINT_LIST (object);
  MucharmapBlockCodepointListPrivate *priv = block_codepoint_list->priv;

  switch (prop_id) {
	case PROP_FIRST_CODEPOINT:
	  g_value_set_uint (value, priv->start);
	  break;
	case PROP_LAST_CODEPOINT:
	  g_value_set_uint (value, priv->end);
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
mucharmap_block_codepoint_list_class_init (MucharmapBlockCodepointListClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  MucharmapCodepointListClass *codepoint_list_class = MUCHARMAP_CODEPOINT_LIST_CLASS (klass);

  object_class->get_property = mucharmap_block_codepoint_list_get_property;
  object_class->set_property = mucharmap_block_codepoint_list_set_property;
  object_class->constructor = mucharmap_block_codepoint_list_constructor;

  g_type_class_add_private (klass, sizeof (MucharmapBlockCodepointListPrivate));

  codepoint_list_class->get_char = get_char;
  codepoint_list_class->get_index = get_index;
  codepoint_list_class->get_last_index = get_last_index;

  /* Not using g_param_spec_unichar on purpose, since it disallows certain values
   * we want (it's performing a g_unichar_validate).
   */
  g_object_class_install_property
	(object_class,
	 PROP_FIRST_CODEPOINT,
	 g_param_spec_uint ("first-codepoint", NULL, NULL,
	                    0,
	                    UNICHAR_MAX,
	                    0,
	                    G_PARAM_READWRITE |
	                    G_PARAM_CONSTRUCT_ONLY |
	                    G_PARAM_STATIC_NAME |
	                    G_PARAM_STATIC_NICK |
	                    G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_LAST_CODEPOINT,
	 g_param_spec_uint ("last-codepoint", NULL, NULL,
	                    0,
	                    UNICHAR_MAX,
	                    0,
	                    G_PARAM_READWRITE |
	                    G_PARAM_CONSTRUCT_ONLY |
	                    G_PARAM_STATIC_NAME |
	                    G_PARAM_STATIC_NICK |
	                    G_PARAM_STATIC_BLURB));
}

/**
 * mucharmap_block_codepoint_list_new:
 * @start: the first codepoint
 * @end: the last codepoint
 *
 * Creates a new codepoint list for the range @start..@end.
 *
 * Return value: the newly-created #MucharmapBlockCodepointList. Use
 * g_object_unref() to free the result.
 **/
MucharmapCodepointList *
mucharmap_block_codepoint_list_new (gunichar start,
	                                gunichar end)
{
  g_return_val_if_fail (start <= end, NULL);

  return g_object_new (MUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST,
	                   "first-codepoint", (guint) start,
	                   "last-codepoint", (guint) end,
	                   NULL);
}
