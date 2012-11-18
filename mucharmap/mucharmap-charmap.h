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

#ifndef GUCHARMAP_CHARMAP_H
#define GUCHARMAP_CHARMAP_H

#include <gtk/gtk.h>

#include <mucharmap/mucharmap-chapters-model.h>
#include <mucharmap/mucharmap-chapters-view.h>
#include <mucharmap/mucharmap-chartable.h>

G_BEGIN_DECLS

//class MucharmapCharmap extends GtkPaned
//{
	#define GUCHARMAP_TYPE_CHARMAP             (mucharmap_charmap_get_type ())
	#define GUCHARMAP_CHARMAP(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_CHARMAP, MucharmapCharmap))
	#define GUCHARMAP_CHARMAP_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_CHARMAP, MucharmapCharmapClass))
	#define GUCHARMAP_IS_CHARMAP(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_CHARMAP))
	#define GUCHARMAP_IS_CHARMAP_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_CHARMAP))
	#define GUCHARMAP_CHARMAP_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_CHARMAP, MucharmapCharmapClass))

	typedef struct _MucharmapCharmap        MucharmapCharmap;
	typedef struct _MucharmapCharmapPrivate MucharmapCharmapPrivate;
	typedef struct _MucharmapCharmapClass   MucharmapCharmapClass;

	struct _MucharmapCharmap
	{
	  GtkPaned parent;

	  /*< private >*/
	  MucharmapCharmapPrivate *priv;
	};

	struct _MucharmapCharmapClass
	{
	  GtkPanedClass parent_class;

	  void (* status_message) (MucharmapCharmap *charmap, const gchar *message);
	  void (* link_clicked) (MucharmapCharmap *charmap, 
		                     gunichar old_character,
		                     gunichar new_character);
	  void (* _mucharmap_reserved0) (void);
	  void (* _mucharmap_reserved1) (void);
	  void (* _mucharmap_reserved2) (void);
	  void (* _mucharmap_reserved3) (void);
	};

	GType                 mucharmap_charmap_get_type           (void);

	GtkWidget *           mucharmap_charmap_new                (void);

	#ifndef GUCHARMAP_DISABLE_DEPRECATED
	void           mucharmap_charmap_set_orientation (MucharmapCharmap *charmap,
		                                              GtkOrientation orientation);
	GtkOrientation mucharmap_charmap_get_orientation (MucharmapCharmap *charmap);
	#endif

	void      mucharmap_charmap_set_active_character (MucharmapCharmap *charmap,
		                                              gunichar           uc);
	gunichar  mucharmap_charmap_get_active_character (MucharmapCharmap *charmap);

	void      mucharmap_charmap_set_active_chapter (MucharmapCharmap *charmap,
		                                            const gchar *chapter);
	char *    mucharmap_charmap_get_active_chapter (MucharmapCharmap *charmap);

	void mucharmap_charmap_next_chapter     (MucharmapCharmap *charmap);
	void mucharmap_charmap_previous_chapter (MucharmapCharmap *charmap);

	void                     mucharmap_charmap_set_font_desc      (MucharmapCharmap  *charmap,
		                                                           PangoFontDescription *font_desc);

	PangoFontDescription *   mucharmap_charmap_get_font_desc      (MucharmapCharmap  *charmap);

	MucharmapChaptersView *  mucharmap_charmap_get_chapters_view  (MucharmapCharmap       *charmap);

	void                     mucharmap_charmap_set_chapters_model (MucharmapCharmap       *charmap,
		                                                           MucharmapChaptersModel *model);

	MucharmapChaptersModel * mucharmap_charmap_get_chapters_model (MucharmapCharmap       *charmap);

	MucharmapCodepointList * mucharmap_charmap_get_active_codepoint_list (MucharmapCharmap *charmap);

	MucharmapCodepointList * mucharmap_charmap_get_book_codepoint_list (MucharmapCharmap *charmap);

	void     mucharmap_charmap_set_chapters_visible (MucharmapCharmap *charmap,
		                                             gboolean visible);

	gboolean mucharmap_charmap_get_chapters_visible (MucharmapCharmap *charmap);

	typedef enum {
	  GUCHARMAP_CHARMAP_PAGE_CHARTABLE,
	  GUCHARMAP_CHARMAP_PAGE_DETAILS
	} MucharmapCharmapPageType;

	void     mucharmap_charmap_set_page_visible (MucharmapCharmap *charmap,
		                                         int page,
		                                         gboolean visible);

	gboolean mucharmap_charmap_get_page_visible (MucharmapCharmap *charmap,
		                                         int page);

	void mucharmap_charmap_set_active_page (MucharmapCharmap *charmap,
		                                    int page);

	int  mucharmap_charmap_get_active_page (MucharmapCharmap *charmap);

	void mucharmap_charmap_set_snap_pow2 (MucharmapCharmap *charmap,
		                                  gboolean snap);
	gboolean mucharmap_charmap_get_snap_pow2 (MucharmapCharmap *charmap);

	/* private; FIXMEchpe remove */
	MucharmapChartable *     mucharmap_charmap_get_chartable      (MucharmapCharmap  *charmap);
//}

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARMAP_H */
