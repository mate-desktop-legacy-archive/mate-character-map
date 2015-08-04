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

static void
mucharmap_codepoint_list_init (MucharmapCodepointList *list)
{
}

static void
mucharmap_codepoint_list_class_init (MucharmapCodepointListClass *clazz)
{
}

G_DEFINE_ABSTRACT_TYPE (MucharmapCodepointList, mucharmap_codepoint_list, G_TYPE_OBJECT)

/**
 * mucharmap_codepoint_list_get_char:
 * @list: a #MucharmapCodepointList
 * @index: index indicating which character to get
 *
 * Return value: code point at index @index in the codepoint list, or
 *   (gunichar)(-1) if @index is beyond the last index.
 **/
gunichar
mucharmap_codepoint_list_get_char (MucharmapCodepointList *list,
	                               gint                    index)
{
  g_return_val_if_fail (MUCHARMAP_IS_CODEPOINT_LIST (list), (gunichar)(-1));

  return MUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_char (list, index);
}

/**
 * mucharmap_codepoint_list_get_index:
 * @list: a #MucharmapCodepointList
 * @wc: character for which to find the index
 *
 * Return value: index of @wc, or -1 if @wc is not in this
 * codepoint list.
 **/
gint
mucharmap_codepoint_list_get_index (MucharmapCodepointList *list,
	                                gunichar                wc)
{
  g_return_val_if_fail (MUCHARMAP_IS_CODEPOINT_LIST (list), -1);

  return MUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_index (list, wc);
}

/**
 * mucharmap_codepoint_list_get_last_index:
 * @list: a #MucharmapCodepointList
 *
 * Return value: last index in this codepoint list.
 **/
gint
mucharmap_codepoint_list_get_last_index (MucharmapCodepointList *list)
{
  g_return_val_if_fail (MUCHARMAP_IS_CODEPOINT_LIST (list), -1);

  return MUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_last_index (list);
}

