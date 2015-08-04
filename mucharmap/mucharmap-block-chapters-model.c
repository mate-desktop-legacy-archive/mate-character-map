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

#include <config.h>
#include <glib/gi18n-lib.h>

#include "mucharmap.h"
#include "mucharmap-private.h"

#include "unicode-blocks.h"

enum {
	BLOCK_CHAPTERS_MODEL_ID = MUCHARMAP_CHAPTERS_MODEL_COLUMN_ID,
	BLOCK_CHAPTERS_MODEL_LABEL = MUCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL,
	BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR = 2,
	BLOCK_CHAPTERS_MODEL_NUM_COLUMNS
};

static int
compare_iters (GtkTreeModel* model, GtkTreeIter* iter_a, GtkTreeIter* iter_b, gpointer user_data)
{
	UnicodeBlock *block_a, *block_b;
	char *label_a, *label_b;
	int ret;

	gtk_tree_model_get (model, iter_a, BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, &block_a, -1);
	gtk_tree_model_get (model, iter_b, BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, &block_b, -1);

	if (block_a == NULL && block_b != NULL)
		return -1;
	else if (block_a != NULL && block_b == NULL)
		return 1;
	else if (block_a == NULL && block_b == NULL)
		return 0;

	gtk_tree_model_get (model, iter_a, BLOCK_CHAPTERS_MODEL_LABEL, &label_a, -1);
	gtk_tree_model_get (model, iter_b, BLOCK_CHAPTERS_MODEL_LABEL, &label_b, -1);

	ret = g_utf8_collate (label_a, label_b);

	g_free (label_a);
	g_free (label_b);

	return ret;
}

static void
mucharmap_block_chapters_model_init (MucharmapBlockChaptersModel* model)
{
	GtkListStore *store = GTK_LIST_STORE (model);
	GtkTreeIter iter;
	guint i;

	GType types[] = {
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_POINTER
	};

	gtk_list_store_set_column_types(store, G_N_ELEMENTS (types), types);

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		BLOCK_CHAPTERS_MODEL_ID, "All",
		BLOCK_CHAPTERS_MODEL_LABEL, _("All"),
		BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, NULL,
		-1);

	for (i = 0;  i < G_N_ELEMENTS (unicode_blocks); i++)
	{
		const char *block_name;

		block_name = unicode_blocks_strings + unicode_blocks[i].block_name_index;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
			BLOCK_CHAPTERS_MODEL_ID, block_name,
			BLOCK_CHAPTERS_MODEL_LABEL, _(block_name),
			BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, unicode_blocks + i,
			-1);
	}

	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE (model),
		MUCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL,
		(GtkTreeIterCompareFunc) compare_iters,
		NULL, NULL);
}

static MucharmapCodepointList*
get_codepoint_list (MucharmapChaptersModel* chapters, GtkTreeIter* iter)
{
	GtkTreeModel* model = GTK_TREE_MODEL(chapters);
	UnicodeBlock* unicode_block;

	gtk_tree_model_get(model, iter, BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, &unicode_block, -1);

	/* special "All" block */
	if (unicode_block == NULL)
	{
		return mucharmap_block_codepoint_list_new(0, UNICHAR_MAX);
	}

	return mucharmap_block_codepoint_list_new(unicode_block->start, unicode_block->end);
}

static MucharmapCodepointList*
get_book_codepoint_list (MucharmapChaptersModel* model)
{
	MucharmapChaptersModelPrivate* model_priv = model->priv;

	if (model_priv->book_list == NULL)
	{
		model_priv->book_list = mucharmap_block_codepoint_list_new(0, UNICHAR_MAX);
	}

	return g_object_ref(model_priv->book_list);
}

/* XXX linear search */
static gboolean
character_to_iter (MucharmapChaptersModel* chapters, gunichar wc, GtkTreeIter* _iter)
{
	GtkTreeModel* model = GTK_TREE_MODEL(chapters);
	GtkTreeIter iter, all_iter;
	gboolean all_iter_set = TRUE;

	if (wc > UNICHAR_MAX)
		return FALSE;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return FALSE;

	do
	{
		UnicodeBlock* unicode_block;

		gtk_tree_model_get(model, &iter, BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, &unicode_block, -1);

		/* skip "All" block */
		if (unicode_block == NULL)
		{
			all_iter = iter;
			all_iter_set = TRUE;
			continue;
		}

		if (wc >= unicode_block->start && wc <= unicode_block->end)
		{
			*_iter = iter;
			return TRUE;
		}
	}
	while (gtk_tree_model_iter_next(model, &iter));

	/* "All" is the last resort */
	g_assert(all_iter_set);
	*_iter = all_iter;
	return TRUE;
}

static void
mucharmap_block_chapters_model_class_init (MucharmapBlockChaptersModelClass* clazz)
{
	MucharmapChaptersModelClass* chapters_class = MUCHARMAP_CHAPTERS_MODEL_CLASS(clazz);

	_mucharmap_intl_ensure_initialized();

	chapters_class->title = _("Unicode Block");
	chapters_class->character_to_iter = character_to_iter;
	chapters_class->get_codepoint_list = get_codepoint_list;
	chapters_class->get_book_codepoint_list = get_book_codepoint_list;
}

G_DEFINE_TYPE(MucharmapBlockChaptersModel, mucharmap_block_chapters_model, MUCHARMAP_TYPE_CHAPTERS_MODEL)

MucharmapChaptersModel*
mucharmap_block_chapters_model_new (void)
{
	return g_object_new(mucharmap_block_chapters_model_get_type(), NULL);
}
