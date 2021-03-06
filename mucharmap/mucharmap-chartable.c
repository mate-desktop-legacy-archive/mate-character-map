/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2007, 2008 Christian Persch
 *
 * Some code copied from gtk+/gtk/gtkiconview:
 * Copyright © 2002, 2004  Anders Carlsson <andersca@gnu.org>
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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "mucharmap-marshal.h"
#include "mucharmap-chartable.h"
#include "mucharmap-unicode-info.h"
#include "mucharmap-private.h"

#define ENABLE_ACCESSIBLE

#ifdef ENABLE_ACCESSIBLE
#include "mucharmap-chartable-accessible.h"
#endif

enum
{
  ACTIVATE,
  STATUS_MESSAGE,
  MOVE_CURSOR,
  COPY_CLIPBOARD,
  PASTE_CLIPBOARD,
  NUM_SIGNALS
};

enum
{
  PROP_0,
#if GTK_CHECK_VERSION (3, 0, 0)
  PROP_HADJUSTMENT,
  PROP_VADJUSTMENT,
  PROP_HSCROLL_POLICY,
  PROP_VSCROLL_POLICY,
#endif
  PROP_ACTIVE_CHAR,
  PROP_CODEPOINT_LIST,
  PROP_FONT_DESC,
  PROP_FONT_FALLBACK,
  PROP_SNAP_POW2,
  PROP_ZOOM_ENABLED,
  PROP_ZOOM_SHOWING
};

static void mucharmap_chartable_class_init (MucharmapChartableClass *klass);
static void mucharmap_chartable_finalize   (GObject *object);
static void mucharmap_chartable_set_active_cell (MucharmapChartable *chartable,
                                                         int cell);

static guint signals[NUM_SIGNALS] = { 0 };

#define DEFAULT_FONT_SIZE (20.0 * (double) PANGO_SCALE)

/* These are chosen for compatibility with the older code that
 * didn't scale the font size by resolution and used 3 and 2.5 here, resp.
 * Where exactly these factors came from, I don't know.
 */
#define FACTOR_WIDTH (2.25) /* 3 / (96 / 72) */
#define FACTOR_HEIGHT (1.875) /* 2.5 / (96 / 72) */

/** Notes
 *
 * 1. Table geometry
 * The allocated rectangle is divided into ::rows rows and ::col columns,
 * numbered 0..rows-1 and 0..cols-1.
 * The available width (height) is divided evenly between all columns (rows).
 * The remaining space is distributed among the columns (rows) so that
 * columns cols-n_padded_columns .. cols-1 (rows rows-n_padded_rows .. rows)
 * are 1px wider (taller) than the others.
 */

/* ATK factory */

#ifdef ENABLE_ACCESSIBLE

typedef AtkObjectFactory      MucharmapChartableAccessibleFactory;
typedef AtkObjectFactoryClass MucharmapChartableAccessibleFactoryClass;

static void
mucharmap_chartable_accessible_factory_init (MucharmapChartableAccessibleFactory *factory)
{
}

static AtkObject*
mucharmap_chartable_accessible_factory_create_accessible (GObject *obj)
{
  return mucharmap_chartable_accessible_new (MUCHARMAP_CHARTABLE (obj));
}

static GType
mucharmap_chartable_accessible_factory_get_accessible_type (void)
{
  return mucharmap_chartable_accessible_get_type ();
}

static void
mucharmap_chartable_accessible_factory_class_init (AtkObjectFactoryClass *klass)
{
  klass->create_accessible = mucharmap_chartable_accessible_factory_create_accessible;
  klass->get_accessible_type = mucharmap_chartable_accessible_factory_get_accessible_type;
}

static GType mucharmap_chartable_accessible_factory_get_type (void);
G_DEFINE_TYPE (MucharmapChartableAccessibleFactory, mucharmap_chartable_accessible_factory, ATK_TYPE_OBJECT_FACTORY)

#endif

/* Type definition */

#if GTK_CHECK_VERSION (3, 0, 0)
G_DEFINE_TYPE_WITH_CODE (MucharmapChartable, mucharmap_chartable, GTK_TYPE_DRAWING_AREA,
                         G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, NULL))
#else
G_DEFINE_TYPE (MucharmapChartable, mucharmap_chartable, GTK_TYPE_DRAWING_AREA)
#endif

/* utility functions */

static void
mucharmap_chartable_clear_pango_layout (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (priv->pango_layout == NULL)
    return;
  g_object_unref (priv->pango_layout);
  priv->pango_layout = NULL;
}

static void
mucharmap_chartable_ensure_pango_layout (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (priv->pango_layout != NULL)
    return;

  priv->pango_layout = gtk_widget_create_pango_layout (GTK_WIDGET (chartable), NULL);
  pango_layout_set_font_description (priv->pango_layout,
                                     priv->font_desc);

  if (priv->font_fallback == FALSE)
    {
      PangoAttrList *list;

      list = pango_attr_list_new ();
      pango_attr_list_insert (list, pango_attr_fallback_new (FALSE));
      pango_layout_set_attributes (priv->pango_layout, list);
      pango_attr_list_unref (list);
    }
}

static void
mucharmap_chartable_set_font_desc_internal (MucharmapChartable *chartable,
	                                        PangoFontDescription *font_desc /* adopting */)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget;

  if (priv->font_desc)
	pango_font_description_free (priv->font_desc);

  priv->font_desc = font_desc;

  mucharmap_chartable_clear_pango_layout (chartable);

  widget = GTK_WIDGET (chartable);
  if (gtk_widget_get_realized (widget))
    gtk_widget_queue_resize (widget);

  g_object_notify (G_OBJECT (chartable), "font-desc");
}

static void
mucharmap_chartable_emit_status_message (MucharmapChartable *chartable,
	                                     const char *message)
{
  g_signal_emit (chartable, signals[STATUS_MESSAGE], 0, message);
}

typedef enum {
  POSITION_DOWN_ALIGN_LEFT,
  POSITION_DOWN_ALIGN_RIGHT,
  POSITION_RIGHT_ALIGN_TOP,
  POSITION_RIGHT_ALIGN_BOTTOM,
  POSITION_TOP_ALIGN_LEFT,
  POSITION_TOP_ALIGN_RIGHT,
  POSITION_LEFT_ALIGN_TOP,
  POSITION_LEFT_ALIGN_BOTTOM
} PositionType;

static const PositionType rtl_position[] = {
  POSITION_DOWN_ALIGN_RIGHT,
  POSITION_DOWN_ALIGN_LEFT,
  POSITION_LEFT_ALIGN_TOP,
  POSITION_LEFT_ALIGN_BOTTOM,
  POSITION_TOP_ALIGN_RIGHT,
  POSITION_TOP_ALIGN_LEFT,
  POSITION_RIGHT_ALIGN_TOP,
  POSITION_RIGHT_ALIGN_BOTTOM
};

/**
 * position_rectangle:
 * @rect: the rectangle to position. Inout; width and height must be initialised
 * @target_rect: the rectangle to position @rect on
 * @bounding_rect: the bounding rectangle
 * @position: how to position the rectangle
 * @direction: the text direction
 *
 * Returns: %TRUE if @rect could be positioned on @reference_point
 * with positioning according to @gravity inside @bounding_rect
 */
static gboolean
position_rectangle (GdkRectangle *position_rect,
	                GdkRectangle *target_rect,
	                GdkRectangle *bounding_rect,
	                PositionType position,
	                GtkTextDirection direction)
{
  GdkRectangle rect;

  if (direction == GTK_TEXT_DIR_RTL) {
	position = rtl_position[position];
  }

  rect.x = target_rect->x;
  rect.y = target_rect->y;
  rect.width = position_rect->width;
  rect.height = position_rect->height;

  switch (position) {
	case POSITION_DOWN_ALIGN_RIGHT:
	  rect.x -= rect.width - target_rect->width;
	  /* fall-through */
	case POSITION_DOWN_ALIGN_LEFT:
	  rect.y += target_rect->height;
	  break;

	case POSITION_RIGHT_ALIGN_BOTTOM:
	  rect.y -= rect.height - target_rect->height;
	  /* fall-through */
	case POSITION_RIGHT_ALIGN_TOP:
	  rect.x += target_rect->width;
	  break;

	case POSITION_TOP_ALIGN_RIGHT:
	  rect.x -= rect.width - target_rect->width;
	  /* fall-through */
	case POSITION_TOP_ALIGN_LEFT:
	  rect.y -= rect.height;
	  break;

	case POSITION_LEFT_ALIGN_BOTTOM:
	  rect.y -= rect.height - target_rect->height;
	  /* fall-through */
	case POSITION_LEFT_ALIGN_TOP:
	  rect.x -= rect.width;
	  break;
  }

  *position_rect = rect;

  return rect.x >= bounding_rect->x &&
	     rect.y >= bounding_rect->y &&
	     rect.x + rect.width <= bounding_rect->x + bounding_rect->width &&
	     rect.y + rect.height <= bounding_rect->y + bounding_rect->height;
}

static gboolean
position_rectangle_on_screen (GtkWidget *widget,
	                          GdkRectangle *rectangle,
	                          GdkRectangle *target_rect)
{
  GtkTextDirection direction;
  GdkRectangle monitor;
  int monitor_num;
  GdkScreen *screen;
  static const PositionType positions[] = {
	POSITION_DOWN_ALIGN_LEFT,
	POSITION_TOP_ALIGN_LEFT,
	POSITION_RIGHT_ALIGN_TOP,
	POSITION_LEFT_ALIGN_TOP,
	POSITION_DOWN_ALIGN_RIGHT,
	POSITION_TOP_ALIGN_RIGHT,
	POSITION_RIGHT_ALIGN_BOTTOM,
	POSITION_LEFT_ALIGN_BOTTOM
  };
  guint i;

  direction = gtk_widget_get_direction (widget);
  screen = gtk_widget_get_screen (widget);
  monitor_num = gdk_screen_get_monitor_at_window (screen, gtk_widget_get_window (widget));
  if (monitor_num < 0)
	monitor_num = 0;
  gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

  for (i = 0; i < G_N_ELEMENTS (positions); ++i) {
	if (position_rectangle (rectangle, target_rect, &monitor, positions[i], direction))
	  return TRUE;
  }

  return FALSE;
}

static void
get_root_coords_at_active_char (MucharmapChartable *chartable,
	                            gint *x_root,
	                            gint *y_root)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  gint x, y;
  gint row, col;

  gdk_window_get_origin (gtk_widget_get_window (widget), &x, &y);

  row = (priv->active_cell - priv->page_first_cell) / priv->cols;
  col = _mucharmap_chartable_cell_column (chartable, priv->active_cell);

  *x_root = x + _mucharmap_chartable_x_offset (chartable, col);
  *y_root = y + _mucharmap_chartable_y_offset (chartable, row);
}

static void
get_active_cell_rect (MucharmapChartable *chartable, GdkRectangle *rect)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  int row, col;

  get_root_coords_at_active_char (chartable, &rect->x, &rect->y);

  row = (priv->active_cell - priv->page_first_cell) / priv->cols;
  col = _mucharmap_chartable_cell_column (chartable, priv->active_cell);

  rect->width = _mucharmap_chartable_column_width (chartable, col);
  rect->height = _mucharmap_chartable_row_height (chartable, row);
}

static void
get_appropriate_upper_left_xy (MucharmapChartable *chartable,
	                           gint width,  gint height,
	                           gint x_root, gint y_root,
	                           gint *x,     gint *y)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  gint row, col;

  row = (priv->active_cell - priv->page_first_cell) / priv->cols;
  col = _mucharmap_chartable_cell_column (chartable, priv->active_cell);

  *x = x_root;
  *y = y_root;

  if (row >= priv->rows / 2)
	*y -= height;

  if (col >= priv->cols / 2)
	*x -= width;
}

/* depends on directionality */
static guint
get_cell_at_rowcol (MucharmapChartable *chartable,
	                gint            row,
	                gint            col)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
	return priv->page_first_cell + row * priv->cols + (priv->cols - col - 1);
  else
	return priv->page_first_cell + row * priv->cols + col;
}

/* Depends on directionality. Column 0 is the furthest left.  */
gint
_mucharmap_chartable_cell_column (MucharmapChartable *chartable,
	                          guint cell)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
	return priv->cols - (cell - priv->page_first_cell) % priv->cols - 1;
  else
	return (cell - priv->page_first_cell) % priv->cols;
}

/* not all columns are necessarily the same width because of padding */
gint
_mucharmap_chartable_column_width (MucharmapChartable *chartable, gint col)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  int num_padded_columns = priv->n_padded_columns;
  int min_col_w = priv->minimal_column_width;

  if (priv->cols - col <= num_padded_columns)
	return min_col_w + 1;
  else
	return min_col_w;
}

/* calculates the position of the left end of the column (just to the right
 * of the left border) */
/* XXX: calling this repeatedly is not the most efficient, but it probably
 * is the most readable */
gint
_mucharmap_chartable_x_offset (MucharmapChartable *chartable, gint col)
{
  gint c, x;

  for (c = 0, x = 1;  c < col;  c++)
	x += _mucharmap_chartable_column_width (chartable, c);

  return x;
}

/* not all rows are necessarily the same height because of padding */
gint
_mucharmap_chartable_row_height (MucharmapChartable *chartable, gint row)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  int num_padded_rows = priv->n_padded_rows;
  int min_row_h = priv->minimal_row_height;

  if (priv->rows - row <= num_padded_rows)
	return min_row_h + 1;
  else
	return min_row_h;
}

/* calculates the position of the top end of the row (just below the top
 * border) */
/* XXX: calling this repeatedly is not the most efficient, but it probably
 * is the most readable */
gint
_mucharmap_chartable_y_offset (MucharmapChartable *chartable, gint row)
{
  gint r, y;

  for (r = 0, y = 1;  r < row;  r++)
	y += _mucharmap_chartable_row_height (chartable, r);

  return y;
}

/* returns the font family of the last glyph item in the first line of the
 * layout; should be freed by caller */
static gchar *
get_font (PangoLayout *layout)
{
  PangoLayoutLine *line;
  PangoGlyphItem *glyph_item;
  PangoFont *font;
  GSList *run_node;
  gchar *family;
  PangoFontDescription *font_desc;

  line = pango_layout_get_line (layout, 0);

  /* get to the last glyph_item (the one with the character we're drawing */
  for (run_node = line->runs;
	   run_node && run_node->next;
	   run_node = run_node->next);

  if (run_node)
	{
	  glyph_item = run_node->data;
	  font = glyph_item->item->analysis.font;
	  font_desc = pango_font_describe (font);

	  family = g_strdup (pango_font_description_get_family (font_desc));

	  pango_font_description_free (font_desc);
	}
  else
	family = NULL;

  return family;
}

/* font_family (if not null) gets filled in with the actual font family
 * used to draw the character */
static PangoLayout *
layout_scaled_glyph (MucharmapChartable *chartable,
	                 gunichar uc,
	                 double font_factor,
	                 char **font_family)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  PangoFontDescription *font_desc;
  PangoLayout *layout;
  gchar buf[11];

  font_desc = pango_font_description_copy (priv->font_desc);

  if (pango_font_description_get_size_is_absolute (priv->font_desc))
	pango_font_description_set_absolute_size (font_desc,
	                                          font_factor * pango_font_description_get_size (priv->font_desc));
  else
	pango_font_description_set_size (font_desc,
	                                 font_factor * pango_font_description_get_size (priv->font_desc));

  mucharmap_chartable_ensure_pango_layout (chartable);
  layout = pango_layout_new (pango_layout_get_context (priv->pango_layout));

  pango_layout_set_font_description (layout, font_desc);

  buf[mucharmap_unichar_to_printable_utf8 (uc, buf)] = '\0';
  pango_layout_set_text (layout, buf, -1);

  if (priv->font_fallback == FALSE)
    {
      PangoAttrList *list;

      list = pango_attr_list_new ();
      pango_attr_list_insert (list, pango_attr_fallback_new (FALSE));
      pango_layout_set_attributes (layout, list);
      pango_attr_list_unref (list);
    }

  if (font_family)
	*font_family = get_font (layout);

  pango_font_description_free (font_desc);

  return layout;
}

#if GTK_CHECK_VERSION (3, 0, 0)
static cairo_surface_t *
create_glyph_surface (MucharmapChartable *chartable,
                      gunichar wc,
                      double font_factor,
                      gboolean draw_font_family,
                      int *zoom_surface_width,
                      int *zoom_surface_height)
{
  GtkWidget *widget = GTK_WIDGET (chartable);
  enum { PADDING = 8 };

  PangoLayout *pango_layout, *pango_layout2 = NULL;
  PangoRectangle char_rect, family_rect;
  gint width, height;
  GtkStyle *style;
  char *family;
  cairo_surface_t *surface;
  cairo_t *cr;

  /* Apply the scaling.  Unfortunately not all fonts seem to be scalable.
   * We could fall back to GdkPixbuf scaling, but that looks butt ugly :-/
   */
  pango_layout = layout_scaled_glyph (chartable, wc,
                                      font_factor, &family);
  pango_layout_get_pixel_extents (pango_layout, &char_rect, NULL);

  if (draw_font_family)
    {
      if (family == NULL)
        family = g_strdup (_("[not a printable character]"));

      pango_layout2 = gtk_widget_create_pango_layout (GTK_WIDGET (chartable), family);
      pango_layout_get_pixel_extents (pango_layout2, NULL, &family_rect);

      /* Make the GdkPixmap large enough to account for possible offsets in the
       * ink extents of the glyph. */
      width  = MAX (char_rect.width, family_rect.width)  + 2 * PADDING;
      height = family_rect.height + char_rect.height + 4 * PADDING;
    }
  else
    {
      width  = char_rect.width + 2 * PADDING;
      height = char_rect.height + 2 * PADDING;
    }

  style = gtk_widget_get_style (widget);

  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               width, height);
  cr = cairo_create (surface);

  gdk_cairo_set_source_color (cr, &style->base[GTK_STATE_NORMAL]);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_fill (cr);

  gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_INSENSITIVE]);
  cairo_set_line_width (cr, 1);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
  cairo_rectangle (cr, 1.5, 1.5, width - 3, height - 3);
  cairo_stroke (cr);

  /* Now draw the glyph.  The coordinates are adapted
   * in order to compensate negative char_rect offsets.
   */
  gdk_cairo_set_source_color (cr, &style->text[GTK_STATE_NORMAL]);
  cairo_move_to (cr, -char_rect.x + PADDING, -char_rect.y + PADDING);
  pango_cairo_show_layout (cr, pango_layout);
  g_object_unref (pango_layout);

  if (draw_font_family)
    {
      cairo_set_line_width (cr, 1);
      cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
      gdk_cairo_set_source_color (cr, &style->dark[GTK_STATE_NORMAL]);
      cairo_move_to (cr, 6 + 1 + .5, char_rect.height + 2 * PADDING + .5);
      cairo_line_to (cr, width - 3 - 6 - .5, char_rect.height + 2 * PADDING + .5);
      cairo_stroke (cr);

      gdk_cairo_set_source_color (cr, &style->text[GTK_STATE_NORMAL]);
      cairo_move_to (cr, PADDING, height - PADDING - family_rect.height);
      /* FIXME: clip here? */
      pango_cairo_show_layout (cr, pango_layout2);

      g_object_unref (pango_layout2);
    }

  g_free (family);

  cairo_destroy (cr);

  if (zoom_surface_width)
    *zoom_surface_width = width;
  if (zoom_surface_height)
    *zoom_surface_height = height;

  return surface;
}

static GdkPixbuf *
create_glyph_pixbuf (MucharmapChartable *chartable,
                     gunichar wc,
                     double font_factor,
                     gboolean draw_font_family,
                     int *zoom_surface_width,
                     int *zoom_surface_height)
{
  cairo_surface_t *surface;
  int width, height;
  GdkPixbuf *pixbuf;

  surface = create_glyph_surface (chartable, wc, font_factor, draw_font_family,
                                  &width, &height);
#if GTK_CHECK_VERSION (3, 0, 0)
  pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, width, height);
#else
  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
  gdk_pixbuf_get_from_surface (pixbuf, surface, 0, 0, 0, 0, width, height);
#endif
  cairo_surface_destroy (surface);

  if (zoom_surface_width)
    *zoom_surface_width = width;
  if (zoom_surface_height)
    *zoom_surface_height = height;

  return pixbuf;
}
#else
static GdkPixmap *
create_glyph_pixmap (MucharmapChartable *chartable,
                     gunichar wc,
                     double font_factor,
	             gboolean draw_font_family,
                     int *width,
                     int *height)
{
  GtkWidget *widget = GTK_WIDGET (chartable);
  enum { PADDING = 8 };

  PangoLayout *pango_layout, *pango_layout2 = NULL;
  PangoRectangle char_rect, family_rect;
  gint pixmap_width, pixmap_height;
  GtkStyle *style;
  GdkPixmap *pixmap;
  char *family;
  cairo_t *cr;

  /* Apply the scaling.  Unfortunately not all fonts seem to be scalable.
   * We could fall back to GdkPixbuf scaling, but that looks butt ugly :-/
   */
  pango_layout = layout_scaled_glyph (chartable, wc,
	                                  font_factor, &family);
  pango_layout_get_pixel_extents (pango_layout, &char_rect, NULL);

  if (draw_font_family)
	{
	  if (family == NULL)
	    family = g_strdup (_("[not a printable character]"));

	  pango_layout2 = gtk_widget_create_pango_layout (GTK_WIDGET (chartable), family);
	  pango_layout_get_pixel_extents (pango_layout2, NULL, &family_rect);

	  /* Make the GdkPixmap large enough to account for possible offsets in the
	   * ink extents of the glyph. */
	  pixmap_width  = MAX (char_rect.width, family_rect.width)  + 2 * PADDING;
	  pixmap_height = family_rect.height + char_rect.height + 4 * PADDING;
	}
  else
	{
	  pixmap_width  = char_rect.width + 2 * PADDING;
	  pixmap_height = char_rect.height + 2 * PADDING;
	}

  style = gtk_widget_get_style (widget);

  pixmap = gdk_pixmap_new (gtk_widget_get_window (widget),
	                       pixmap_width, pixmap_height, -1);

  cr = gdk_cairo_create (pixmap);

  gdk_cairo_set_source_color (cr, &style->base[GTK_STATE_NORMAL]);
  cairo_rectangle (cr, 0, 0, pixmap_width, pixmap_height);
  cairo_fill (cr);

  gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_INSENSITIVE]);
  cairo_set_line_width (cr, 1);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
  cairo_rectangle (cr, 1.5, 1.5, pixmap_width - 3, pixmap_height - 3);
  cairo_stroke (cr);

  /* Now draw the glyph.  The coordinates are adapted
   * in order to compensate negative char_rect offsets.
   */
  gdk_cairo_set_source_color (cr, &style->text[GTK_STATE_NORMAL]);
  cairo_move_to (cr, -char_rect.x + PADDING, -char_rect.y + PADDING);
  pango_cairo_show_layout (cr, pango_layout);
  g_object_unref (pango_layout);

  if (draw_font_family)
	{
	  cairo_set_line_width (cr, 1);
	  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
	  gdk_cairo_set_source_color (cr, &style->dark[GTK_STATE_NORMAL]);
	  cairo_move_to (cr, 6 + 1 + .5, char_rect.height + 2 * PADDING + .5);
	  cairo_line_to (cr, pixmap_width - 3 - 6 - .5, char_rect.height + 2 * PADDING + .5);
	  cairo_stroke (cr);

	  gdk_cairo_set_source_color (cr, &style->text[GTK_STATE_NORMAL]);
	  cairo_move_to (cr, PADDING, pixmap_height - PADDING - family_rect.height);
	  /* FIXME: clip here? */
	  pango_cairo_show_layout (cr, pango_layout2);

	  g_object_unref (pango_layout2);
	}

  g_free (family);

  cairo_destroy (cr);

  if (width)
    *width = pixmap_width;
  if (height)
    *height = pixmap_height;

  return pixmap;
}
#endif

/* places the zoom window toward the inside of the coordinates */
static void
place_zoom_window (MucharmapChartable *chartable, gint x_root, gint y_root)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  int x, y;

  if (!priv->zoom_window)
	return;

  get_appropriate_upper_left_xy (chartable,
                                 priv->zoom_image_width,
                                 priv->zoom_image_height,
                                 x_root, y_root, &x, &y);
  gtk_window_move (GTK_WINDOW (priv->zoom_window), x, y);
}

static void
place_zoom_window_on_active_cell (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GdkRectangle rect, keepout_rect;

  if (!priv->zoom_window)
	return;

  get_active_cell_rect (chartable, &keepout_rect);

  rect.x = rect.y = 0;
  rect.width = priv->zoom_image_width;
  rect.height = priv->zoom_image_height;

  position_rectangle_on_screen (GTK_WIDGET (chartable),
	                            &rect,
	                            &keepout_rect);
  gtk_window_move (GTK_WINDOW (priv->zoom_window), rect.x, rect.y);
}

static int
get_font_size_px (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  GdkScreen *screen;
  double resolution;
  int font_size;

  g_assert (priv->font_desc != NULL);

  screen = gtk_widget_get_screen (widget);
  resolution = gdk_screen_get_resolution (screen);
  if (resolution < 0.0) /* will be -1 if the resolution is not defined in the GdkScreen */
	resolution = 96.0;

  if (pango_font_description_get_size_is_absolute (priv->font_desc))
	font_size = pango_font_description_get_size (priv->font_desc);
  else
	font_size = ((double) pango_font_description_get_size (priv->font_desc)) * resolution / 72.0;

  if (PANGO_PIXELS (font_size) <= 0)
	font_size = DEFAULT_FONT_SIZE * resolution / 72.0;

  return PANGO_PIXELS (font_size);
}

static void
update_zoom_window (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  double scale;
  int font_size_px, screen_height;

  if (priv->zoom_window == NULL)
    return;

  font_size_px = get_font_size_px (chartable);
  screen_height = gdk_screen_get_height (gtk_widget_get_screen (widget));

  scale = (0.3 * screen_height) / (FACTOR_WIDTH * font_size_px);
  scale = CLAMP (scale, 1.0, 12.0);

#if GTK_CHECK_VERSION (2, 90, 8)
  GdkPixbuf *pixbuf;

  pixbuf = create_glyph_pixbuf (chartable,
                                mucharmap_chartable_get_active_character (chartable),
                                scale,
                                TRUE,
                                &priv->zoom_image_width,
                                &priv->zoom_image_height);
  gtk_image_set_from_pixbuf (GTK_IMAGE (gtk_bin_get_child (GTK_BIN (priv->zoom_window))),
                             pixbuf);
  g_object_unref (pixbuf);
#else
  GdkPixmap *pixmap;

  pixmap = create_glyph_pixmap (chartable,
                                mucharmap_chartable_get_active_character (chartable),
                                scale,
                                TRUE,
                                &priv->zoom_image_width,
                                &priv->zoom_image_height);
  gtk_image_set_from_pixmap (GTK_IMAGE (gtk_bin_get_child (GTK_BIN (priv->zoom_window))),
                             pixmap, NULL);
  g_object_unref (pixmap);
#endif

  gtk_window_resize (GTK_WINDOW (priv->zoom_window),
                     priv->zoom_image_width, priv->zoom_image_height);
}

static void
make_zoom_window (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  GtkWidget *image;

  /* if there is already a zoom window, do nothing */
  if (priv->zoom_window || !priv->zoom_mode_enabled)
	return;

  priv->zoom_window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_resizable (GTK_WINDOW (priv->zoom_window), FALSE);
  gtk_window_set_screen (GTK_WINDOW (priv->zoom_window),
	                     gtk_widget_get_screen (widget));

  image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (priv->zoom_window), image);
  gtk_widget_show (image);
}

static void
destroy_zoom_window (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (priv->zoom_window)
	{
	  GtkWidget *widget = GTK_WIDGET (chartable);
	  GtkWidget *zoom_window;

	  zoom_window = priv->zoom_window;
	  priv->zoom_window = NULL;

	  gdk_window_set_cursor (gtk_widget_get_window (widget), NULL);
	  gtk_widget_destroy (zoom_window);
	}
}

static void
mucharmap_chartable_show_zoom (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (!priv->zoom_mode_enabled)
	return;

  make_zoom_window (chartable);
  update_zoom_window (chartable);

  place_zoom_window_on_active_cell (chartable);

  gtk_widget_show (priv->zoom_window);

  g_object_notify (G_OBJECT (chartable), "zoom-showing");
}

static void
mucharmap_chartable_hide_zoom (MucharmapChartable *chartable)
{
  destroy_zoom_window (chartable);

  g_object_notify (G_OBJECT (chartable), "zoom-showing");
}

static gunichar
get_cell_at_xy (MucharmapChartable *chartable,
	            gint            x,
	            gint            y)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  gint r, c, x0, y0;
  guint cell;

  for (c = 0, x0 = 0;  x0 <= x && c < priv->cols;  c++)
	x0 += _mucharmap_chartable_column_width (chartable, c);

  for (r = 0, y0 = 0;  y0 <= y && r < priv->rows;  r++)
	y0 += _mucharmap_chartable_row_height (chartable, r);

  /* cell = rowcol_to_unichar (chartable, r-1, c-1); */
  cell = get_cell_at_rowcol (chartable, r-1, c-1);

  /* XXX: check this somewhere else? */
  if (cell > priv->last_cell)
	return priv->last_cell;

  return cell;
}

static void
draw_character (MucharmapChartable *chartable,
		cairo_t            *cr,
#if GTK_CHECK_VERSION (3, 0, 0)
		cairo_rectangle_int_t  *rect,
#else
		GdkRectangle *rect,
#endif
		gint            row,
		gint            col)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  int n, char_width, char_height;
  gunichar wc;
  guint cell;
  GtkStyle *style;
  GdkColor *color;
  gchar buf[10];

  cell = get_cell_at_rowcol (chartable, row, col);
  wc = mucharmap_codepoint_list_get_char (priv->codepoint_list, cell);

  if (wc > UNICHAR_MAX || !mucharmap_unichar_validate (wc) || !mucharmap_unichar_isdefined (wc))
	return;

  n = mucharmap_unichar_to_printable_utf8 (wc, buf);
  pango_layout_set_text (priv->pango_layout, buf, n);

  /* Keep the square empty if font fallback is disabled and the
   * font has no glyph for this cell.
   */
  if (!priv->font_fallback &&
      pango_layout_get_unknown_glyphs_count (priv->pango_layout) > 0)
    return;

  style = gtk_widget_get_style (widget);

  cairo_save (cr);

  if (gtk_widget_has_focus (widget) && (gint)cell == priv->active_cell)
	color = &style->text[GTK_STATE_SELECTED];
  else if ((gint)cell == priv->active_cell)
	color = &style->text[GTK_STATE_ACTIVE];
  else
	color = &style->text[GTK_STATE_NORMAL];

  gdk_cairo_set_source_color (cr, color);

  cairo_rectangle (cr,
                   rect->x + 1, rect->y + 1,
                   rect->width - 2, rect->height - 2);
  cairo_clip (cr);

  pango_layout_get_pixel_size (priv->pango_layout, &char_width, &char_height);

  cairo_move_to (cr,
                 rect->x + (rect->width - char_width - 2 + 1) / 2,
                 rect->y + (rect->height - char_height - 2 + 1) / 2);
  pango_cairo_show_layout (cr, priv->pango_layout);

  cairo_restore (cr);
}

static void
expose_square (MucharmapChartable *chartable, gint row, gint col)
{
  GtkWidget *widget = GTK_WIDGET (chartable);

  gtk_widget_queue_draw_area (widget,
	                          _mucharmap_chartable_x_offset (chartable, col),
	                          _mucharmap_chartable_y_offset (chartable, row),
	                          _mucharmap_chartable_column_width (chartable, col) - 1,
	                          _mucharmap_chartable_row_height (chartable, row) - 1);
}

static void
expose_cell (MucharmapChartable *chartable,
	     guint cell)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  gint row = (cell - priv->page_first_cell) / priv->cols;
  gint col = _mucharmap_chartable_cell_column (chartable, cell);

  if (row >= 0 && row < priv->rows && col >= 0 && col < priv->cols)
    expose_square (chartable, row, col);
}

static void
draw_square_bg (MucharmapChartable *chartable,
		cairo_t *cr,
#if GTK_CHECK_VERSION (2, 90, 5)
                cairo_rectangle_int_t  *rect,
#else
                GdkRectangle *rect,
#endif
		gint row,
		gint col)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  GdkColor *untinted;
  GtkStyle *style;
  guint cell;
  gunichar wc;

  cairo_save (cr);

  cell = get_cell_at_rowcol (chartable, row, col);
  wc = mucharmap_codepoint_list_get_char (priv->codepoint_list, cell);

  style = gtk_widget_get_style (widget);

  if (gtk_widget_has_focus (widget) && (gint)cell == priv->active_cell)
    untinted = &style->base[GTK_STATE_SELECTED];
  else if ((gint)cell == priv->active_cell)
    untinted = &style->base[GTK_STATE_ACTIVE];
  else if ((gint)cell > priv->last_cell)
    untinted = &style->dark[GTK_STATE_NORMAL];
  else if (! mucharmap_unichar_validate (wc))
    untinted = &style->fg[GTK_STATE_INSENSITIVE];
  else if (! mucharmap_unichar_isdefined (wc))
    untinted = &style->bg[GTK_STATE_INSENSITIVE];
  else
    untinted = &style->base[GTK_STATE_NORMAL];

  gdk_cairo_set_source_color (cr, untinted);
  cairo_set_line_width (cr, 1);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

  cairo_rectangle (cr, rect->x, rect->y, rect->width, rect->height);
  cairo_fill (cr);

  cairo_restore (cr);
}

static void
draw_borders (MucharmapChartable *chartable,
	      cairo_t *cr)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  GtkAllocation *allocation;
  GtkStyle *style;
  gint x, y, col, row;
  GtkAllocation widget_allocation;

  gtk_widget_get_allocation (widget, &widget_allocation);
  allocation = &widget_allocation;

  cairo_save (cr);

  /* dark_gc[GTK_STATE_NORMAL] seems to be what is used to draw the borders
   * around widgets, so we use it for the lines */

  style = gtk_widget_get_style (widget);
  gdk_cairo_set_source_color (cr, &style->dark[GTK_STATE_NORMAL]);

  cairo_set_line_width (cr, 1); /* FIXME themeable? */
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

  /* vertical lines */
  cairo_move_to (cr, .5, .5);
  cairo_line_to (cr, .5, allocation->height - .5);

  for (col = 0, x = 0;  col < priv->cols;  col++)
  {
    x += _mucharmap_chartable_column_width (chartable, col);
    cairo_move_to (cr, x + .5, .5);
    cairo_line_to (cr, x + .5, allocation->height - .5);
  }

  /* horizontal lines */
  cairo_move_to (cr, .5, .5);
  cairo_line_to (cr, allocation->width - .5, .5);

  for (row = 0, y = 0;  row < priv->rows;  row++)
  {
    y += _mucharmap_chartable_row_height (chartable, row);

    cairo_move_to (cr, .5, y + .5);
    cairo_line_to (cr, allocation->width - .5, y + .5);
  }

  cairo_stroke (cr);
  cairo_restore (cr);
}

static void
update_scrollbar_adjustment (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkAdjustment *vadjustment = priv->vadjustment;

  if (!vadjustment)
    return;

  gtk_adjustment_configure (vadjustment,
                            priv->page_first_cell / priv->cols,
                            0 /* lower */,
                            priv->last_cell / priv->cols + 1 /* upper */,
                            3 /* step increment */,
                            priv->rows /* page increment */,
			    priv->rows);
}

static void
mucharmap_chartable_set_active_cell (MucharmapChartable *chartable,
				     int cell)
{
  GtkWidget *widget = GTK_WIDGET (chartable);
  MucharmapChartablePrivate *priv = chartable->priv;
  int old_active_cell, old_page_first_cell;

  if (cell == priv->active_cell)
    return;

  if (cell < 0)
    cell = 0;
  else if (cell > priv->last_cell)
    cell = priv->last_cell;

  old_active_cell = priv->active_cell;
  old_page_first_cell = priv->page_first_cell;

  priv->active_cell = cell;

  if (cell < priv->page_first_cell || cell >= priv->page_first_cell + priv->page_size)
  {
    int old_row = old_active_cell / priv->cols;
    int new_row = cell / priv->cols;
    int new_page_first_cell = old_page_first_cell + (new_row - old_row) * priv->cols;
    int last_page_first_cell = (priv->last_cell / priv->cols - priv->rows + 1) * priv->cols;

    priv->page_first_cell = CLAMP (new_page_first_cell, 0, last_page_first_cell);

    if (priv->vadjustment)
      gtk_adjustment_set_value (priv->vadjustment, priv->page_first_cell / priv->cols);
    }

  else if (gtk_widget_get_realized (widget)) {
    expose_cell (chartable, old_active_cell);
    expose_cell (chartable, cell);
  }

  g_object_notify (G_OBJECT (chartable), "active-character");

  update_zoom_window (chartable);
  place_zoom_window_on_active_cell (chartable);
}

static void
set_active_char (MucharmapChartable *chartable,
		 gunichar wc)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  guint cell = mucharmap_codepoint_list_get_index (priv->codepoint_list, wc);
  if (cell == -1) {
    gtk_widget_error_bell (GTK_WIDGET (chartable));
    return;
  }

  mucharmap_chartable_set_active_cell (chartable, cell);
}

static void
vadjustment_value_changed_cb (GtkAdjustment *vadjustment, MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  int row, r, c, old_page_first_cell, old_active_cell, first_cell;

  row = (int) gtk_adjustment_get_value (vadjustment);

  if (row < 0 || row > priv->last_cell / priv->cols)
    row = 0;

  first_cell = row * priv->cols;

  gtk_widget_queue_draw (GTK_WIDGET (chartable));

  old_page_first_cell = priv->page_first_cell;
  old_active_cell = priv->active_cell;

  priv->page_first_cell = first_cell;

  /* character is still on the visible page */
  if (priv->active_cell - priv->page_first_cell >= 0
      && priv->active_cell - priv->page_first_cell < priv->page_size)
    return;

  c = old_active_cell % priv->cols;

  if (priv->page_first_cell < old_page_first_cell)
    r = priv->rows - 1;
  else
    r = 0;

  mucharmap_chartable_set_active_cell (chartable, priv->page_first_cell + r * priv->cols + c);
}

/* GtkWidget class methods */

/*  - single click with left button: activate character under pointer
 *  - double-click with left button: add active character to text_to_copy
 *  - single-click with middle button: jump to selection_primary
 */
static gboolean
mucharmap_chartable_button_press (GtkWidget *widget,
	                              GdkEventButton *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;

  /* in case we lost keyboard focus and are clicking to get it back */
  gtk_widget_grab_focus (widget);

  if (event->button == 1)
	{
	  priv->click_x = event->x;
	  priv->click_y = event->y;
	}

  /* double-click */
  if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
	{
	  g_signal_emit (chartable, signals[ACTIVATE], 0);
	}
  /* single-click */
  else if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
	{
	  mucharmap_chartable_set_active_cell (chartable, get_cell_at_xy (chartable, event->x, event->y));
	}
  else if (event->button == 3)
	{
	  mucharmap_chartable_set_active_cell (chartable, get_cell_at_xy (chartable, event->x, event->y));
	  mucharmap_chartable_show_zoom (chartable);
	}

  /* XXX: [need to return false so it gets drag events] */
  /* actually return true because we handle drag_begin because of
   * http://bugzilla.mate.org/show_bug.cgi?id=114534 */
  return TRUE;
}

static gboolean
mucharmap_chartable_button_release (GtkWidget *widget,
	                                GdkEventButton *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  gboolean (* button_press_event) (GtkWidget *, GdkEventButton *) =
	GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->button_release_event;

  if (event->button == 3)
	mucharmap_chartable_hide_zoom (chartable);

  if (button_press_event)
	return button_press_event (widget, event);
  return FALSE;
}

static void
mucharmap_chartable_drag_begin (GtkWidget *widget,
	                            GdkDragContext *context)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  double scale;
  int font_size_px, screen_height;
#if GTK_CHECK_VERSION (3, 0, 0)
  cairo_surface_t *drag_surface;
#else
  GdkPixmap *drag_icon;
#endif

  font_size_px = get_font_size_px (chartable);
  screen_height = gdk_screen_get_height (gtk_widget_get_screen (widget));

  scale = (0.3 * screen_height) / (FACTOR_WIDTH * font_size_px);
  scale = CLAMP (scale, 1.0, 5.0);

#if GTK_CHECK_VERSION (2, 90, 8)
  drag_surface = create_glyph_surface (chartable,
                                       mucharmap_chartable_get_active_character (chartable),
                                       scale,
                                       FALSE, NULL, NULL);
  gtk_drag_set_icon_surface (context, drag_surface);
  cairo_surface_destroy (drag_surface);
#else
  drag_icon = create_glyph_pixmap (chartable,
	                               mucharmap_chartable_get_active_character (chartable),
	                               scale,
	                               FALSE, NULL, NULL);
  gtk_drag_set_icon_pixmap (context, gtk_widget_get_colormap (widget),
	                        drag_icon, NULL, -8, -8);
  g_object_unref (drag_icon);
#endif

  /* no need to chain up */
}

static void
mucharmap_chartable_drag_data_get (GtkWidget *widget,
	                               GdkDragContext *context,
	                               GtkSelectionData *selection_data,
	                               guint info,
	                               guint time)

{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;
  gchar buf[7];
  gint n;

  n = g_unichar_to_utf8 (mucharmap_codepoint_list_get_char (priv->codepoint_list, priv->active_cell), buf);
  gtk_selection_data_set_text (selection_data, buf, n);

  /* no need to chain up */
}

static void
mucharmap_chartable_drag_data_received (GtkWidget *widget,
	                                    GdkDragContext *context,
	                                    gint x, gint y,
	                                    GtkSelectionData *selection_data,
	                                    guint info,
	                                    guint time)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;
  gchar *text;
  gunichar wc;

  if (gtk_selection_data_get_length (selection_data) <= 0 ||
	  gtk_selection_data_get_data (selection_data) == NULL)
	return;

  text = (gchar *) gtk_selection_data_get_text (selection_data);

  if (text == NULL) /* XXX: say something in the statusbar? */
	return;

  wc = g_utf8_get_char_validated (text, -1);

  if (wc == (gunichar)(-2) || wc == (gunichar)(-1) || wc > UNICHAR_MAX)
	mucharmap_chartable_emit_status_message (chartable, _("Unknown character, unable to identify."));
  else if (mucharmap_codepoint_list_get_index (priv->codepoint_list, wc) == (guint)(-1))
	mucharmap_chartable_emit_status_message (chartable, _("Not found."));
  else
	{
	  mucharmap_chartable_emit_status_message (chartable, _("Character found."));
	  set_active_char (chartable, wc);
	  place_zoom_window_on_active_cell (chartable);
	}

  g_free (text);

  /* no need to chain up */
}

#if GTK_CHECK_VERSION (3, 0, 0)
static gboolean
mucharmap_chartable_draw (GtkWidget *widget,
                          cairo_t *cr)
#else
static gboolean
mucharmap_chartable_expose_event (GtkWidget *widget,
                                  GdkEventExpose *event)
#endif
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkStyle *style;
  int row, col;
#if GTK_CHECK_VERSION (3, 0, 0)
  cairo_rectangle_int_t clip_rect;
  cairo_region_t *region;
#else
  GdkRegion *region = event->region;
  cairo_t *cr;
#endif

#if !GTK_CHECK_VERSION (3, 0, 0)
  /* Don't draw anything if we haven't set a codepoint list yet */
  if (event->window != gtk_widget_get_window (widget))
	return FALSE;
#endif

#if GTK_CHECK_VERSION (3, 0, 0)
  if (!gdk_cairo_get_clip_rectangle (cr, &clip_rect))
	return FALSE;

  region = cairo_region_create_rectangle (&clip_rect);

  if (cairo_region_is_empty (region)) {
    cairo_region_destroy (region);
    return FALSE;
  }
#else
  if (gdk_region_empty (region))
	return FALSE;
#endif

#if 0
  {
	int i, n_rects;

	n_rects = cairo_region_num_rectangles (event->region);

	g_print ("Exposing area %d:%d@(%d,%d) with %d rects ", event->area.width, event->area.height,
	         event->area.x, event->area.y, n_rects);
	for (i = 0; i < n_rects; ++i) {
	  g_print ("[Rect %d:%d@(%d,%d)] ", rects[i].width, rects[i].height, rects[i].x, rects[i].y);
	}
	g_print ("\n");
  }
#endif

#if !GTK_CHECK_VERSION (3, 0, 0)
  cr = gdk_cairo_create (event->window);
  gdk_cairo_region (cr, region);
  cairo_clip (cr);
#endif

  style = gtk_widget_get_style (widget);
  gdk_cairo_set_source_color (cr, &style->bg[GTK_STATE_NORMAL]);
  gdk_cairo_region (cr, region);
  cairo_fill (cr);

  if (priv->codepoint_list == NULL)
    goto expose_done;

  mucharmap_chartable_ensure_pango_layout (chartable);

  for (row = priv->rows - 1; row >= 0; --row)
    {
      for (col = priv->cols - 1; col >= 0; --col)
        {
#if GTK_CHECK_VERSION (2, 90, 5)
          cairo_rectangle_int_t rect;
#else
          GdkRectangle rect;
#endif

          rect.x = _mucharmap_chartable_x_offset (chartable, col);
          rect.y = _mucharmap_chartable_y_offset (chartable, row);
          rect.width = _mucharmap_chartable_column_width (chartable, col);
          rect.height = _mucharmap_chartable_row_height (chartable, row);

#if GTK_CHECK_VERSION (3, 0, 0)
          if (cairo_region_contains_rectangle (region, &rect) == CAIRO_REGION_OVERLAP_OUT)
            continue;
#else
          if (gdk_region_rect_in (region, &rect) == GDK_OVERLAP_RECTANGLE_OUT)
            continue;
#endif

          draw_square_bg (chartable, cr, &rect, row, col);
          draw_character (chartable, cr, &rect, row, col);
        }
    }

  draw_borders (chartable, cr);

expose_done:

#if GTK_CHECK_VERSION (3, 0, 0)
  cairo_region_destroy (region);
#else
  cairo_destroy (cr);
#endif

  /* no need to chain up */
  return FALSE;
}

static gboolean
mucharmap_chartable_focus_in_event (GtkWidget *widget,
	                                GdkEventFocus *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;

  expose_cell (chartable, priv->active_cell);

  return GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->focus_in_event (widget, event);
}

static gboolean
mucharmap_chartable_focus_out_event (GtkWidget *widget,
	                                 GdkEventFocus *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;

  mucharmap_chartable_hide_zoom (chartable);

  expose_cell (chartable, priv->active_cell);

  /* FIXME: the parent's handler already does a draw... */

  return GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->focus_out_event (widget, event);
}

static gboolean
mucharmap_chartable_key_press_event (GtkWidget *widget,
	                                 GdkEventKey *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);

  if (event->state & (GDK_MOD1_MASK | GDK_CONTROL_MASK))
	return GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->key_press_event (widget, event);

  switch (event->keyval)
	{
	  case GDK_KEY_Shift_L: case GDK_KEY_Shift_R:
	    mucharmap_chartable_show_zoom (chartable);
	    break;

	  /* pass on other keys, like tab and stuff that shifts focus */
	  default:
	    break;
	}

  return GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->key_press_event (widget, event);
}

static gboolean
mucharmap_chartable_key_release_event (GtkWidget *widget,
	                                   GdkEventKey *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);

  switch (event->keyval)
	{
	  /* XXX: If the group(shift_toggle) Xkb option is set, then releasing
	   * the shift key gives either ISO_Next_Group or ISO_Prev_Group. Is
	   * there a better way to handle this case? */
	  case GDK_KEY_Shift_L:
	  case GDK_KEY_Shift_R:
	  case GDK_KEY_ISO_Next_Group:
	  case GDK_KEY_ISO_Prev_Group:
	    mucharmap_chartable_hide_zoom (chartable);
	    break;
	}

  return GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->key_release_event (widget, event);
}

static gboolean
mucharmap_chartable_motion_notify (GtkWidget *widget,
	                               GdkEventMotion *event)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;
  gboolean (* motion_notify_event) (GtkWidget *, GdkEventMotion *) =
	GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->motion_notify_event;

  if ((event->state & GDK_BUTTON1_MASK) &&
	  gtk_drag_check_threshold (widget,
	                            priv->click_x,
	                            priv->click_y,
	                            event->x,
	                            event->y) &&
	  mucharmap_unichar_validate (mucharmap_chartable_get_active_character (chartable)))
	{
	  gtk_drag_begin (widget, priv->target_list,
	                  GDK_ACTION_COPY, 1, (GdkEvent *) event);
	}

  if ((event->state & GDK_BUTTON3_MASK) != 0 &&
	  priv->zoom_window)
	{
	  guint cell = get_cell_at_xy (chartable, MAX (0, event->x), MAX (0, event->y));

	  if ((gint)cell != priv->active_cell)
	    {
	      gtk_widget_hide (priv->zoom_window);
	      mucharmap_chartable_set_active_cell (chartable, cell);
	    }

	  place_zoom_window (chartable, event->x_root, event->y_root);
	  gtk_widget_show (priv->zoom_window);
	}

  if (motion_notify_event)
	motion_notify_event (widget, event);
  return FALSE;
}

#define FIRST_CELL_IN_SAME_ROW(x) ((x) - ((x) % priv->cols))

static void
mucharmap_chartable_size_allocate (GtkWidget *widget,
	                               GtkAllocation *allocation)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;
  int old_rows, old_cols;
  int total_extra_pixels;
  int new_first_cell;
  int bare_minimal_column_width, bare_minimal_row_height;
  int font_size_px;
  GtkAllocation widget_allocation;

  GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->size_allocate (widget, allocation);

  gtk_widget_get_allocation (widget, &widget_allocation);
  allocation = &widget_allocation;

  old_rows = priv->rows;
  old_cols = priv->cols;

  font_size_px = get_font_size_px (chartable);

  /* FIXMEchpe bug 329481 */
  bare_minimal_column_width = FACTOR_WIDTH * font_size_px;
  bare_minimal_row_height = FACTOR_HEIGHT * font_size_px;

  if (priv->snap_pow2_enabled)
	priv->cols = (1 << g_bit_nth_msf ((allocation->width - 1) / bare_minimal_column_width, -1));
  else
	priv->cols = (allocation->width - 1) / bare_minimal_column_width;

  priv->rows = (allocation->height - 1) / bare_minimal_row_height;

  /* avoid a horrible floating point exception crash */
  if (priv->rows < 1)
	priv->rows = 1;
  if (priv->cols < 1)
	priv->cols = 1;

  priv->page_size = priv->rows * priv->cols;

  total_extra_pixels = allocation->width - (priv->cols * bare_minimal_column_width + 1);
  priv->minimal_column_width = bare_minimal_column_width + total_extra_pixels / priv->cols;
  priv->n_padded_columns = allocation->width - (priv->minimal_column_width * priv->cols + 1);

  total_extra_pixels = allocation->height - (priv->rows * bare_minimal_row_height + 1);
  priv->minimal_row_height = bare_minimal_row_height + total_extra_pixels / priv->rows;
  priv->n_padded_rows = allocation->height - (priv->minimal_row_height * priv->rows + 1);

  if (priv->rows == old_rows && priv->cols == old_cols)
	return;

  /* Need to recalculate the first cell, see bug #517188 */
  new_first_cell = FIRST_CELL_IN_SAME_ROW (priv->active_cell);
  if ((new_first_cell + priv->rows*priv->cols) > (priv->last_cell))
	{
	  /* last cell is visible, so make sure it is in the last row */
	  new_first_cell = FIRST_CELL_IN_SAME_ROW (priv->last_cell) - priv->page_size + priv->cols;

	  if (new_first_cell < 0)
	    new_first_cell = 0;
	}
  priv->page_first_cell = new_first_cell;

  update_scrollbar_adjustment (chartable);
}

#if GTK_CHECK_VERSION (2, 91, 0)
static void
mucharmap_chartable_get_preferred_width (GtkWidget *widget,
                                         gint      *minimum,
                                         gint      *natural)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  int font_size_px;

  font_size_px = get_font_size_px (chartable);

  *minimum = *natural = FACTOR_WIDTH * font_size_px;
}

static void
mucharmap_chartable_get_preferred_height (GtkWidget *widget,
                                          gint      *minimum,
                                          gint      *natural)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  int font_size_px;

  font_size_px = get_font_size_px (chartable);

  *minimum = *natural = FACTOR_HEIGHT * font_size_px;
}

#else
static void
mucharmap_chartable_size_request (GtkWidget *widget,
	                              GtkRequisition *requisition)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  int font_size_px;

  font_size_px = get_font_size_px (chartable);

  requisition->width = FACTOR_WIDTH * font_size_px;
  requisition->height = FACTOR_HEIGHT * font_size_px;
}
#endif

static void
mucharmap_chartable_style_set (GtkWidget *widget,
	                           GtkStyle *previous_style)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (widget);
  MucharmapChartablePrivate *priv = chartable->priv;

  GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->style_set (widget, previous_style);

  mucharmap_chartable_clear_pango_layout (chartable);

  if (priv->font_desc == NULL) {
	GtkStyle *style;
	PangoFontDescription *font_desc;

	style = gtk_widget_get_style (widget);
	font_desc = pango_font_description_copy (style->font_desc);

	/* Use twice the size of the style's font */
	if (pango_font_description_get_size_is_absolute (font_desc))
	  pango_font_description_set_absolute_size (font_desc,
	                                            2 * pango_font_description_get_size (font_desc));
	else
	  pango_font_description_set_size (font_desc,
	                                   2 * pango_font_description_get_size (font_desc));

	mucharmap_chartable_set_font_desc_internal (chartable, font_desc /* adopts */);
	g_assert (priv->font_desc != NULL);
  }

  /* FIXME: necessary? */
  /* gtk_widget_queue_draw (widget); */
  gtk_widget_queue_resize (widget);
}

#ifdef ENABLE_ACCESSIBLE

static AtkObject *
mucharmap_chartable_get_accessible (GtkWidget *widget)
{
  static gboolean first_time = TRUE;

  if (first_time)
	{
	  AtkObjectFactory *factory;
	  AtkRegistry *registry;
	  GType derived_type;
	  GType derived_atk_type;

	  /*
	   * Figure out whether accessibility is enabled by looking at the
	   * type of the accessible object which would be created for
	   * the parent type of MucharmapChartable.
	   */
	  derived_type = g_type_parent (MUCHARMAP_TYPE_CHARTABLE);

	  registry = atk_get_default_registry ();
	  factory = atk_registry_get_factory (registry,
	                                      derived_type);
	  derived_atk_type = atk_object_factory_get_accessible_type (factory);
	  if (g_type_is_a (derived_atk_type, GTK_TYPE_ACCESSIBLE))
	atk_registry_set_factory_type (registry,
					   MUCHARMAP_TYPE_CHARTABLE,
					   mucharmap_chartable_accessible_factory_get_type ());
	  first_time = FALSE;
	}

  return GTK_WIDGET_CLASS (mucharmap_chartable_parent_class)->get_accessible (widget);
}

#endif

/* MucharmapChartable class methods */

#if GTK_CHECK_VERSION (3, 0, 0)
static void
mucharmap_chartable_set_hadjustment (MucharmapChartable *chartable,
                                     GtkAdjustment *hadjustment)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (hadjustment == priv->hadjustment)
    return;

  if (priv->hadjustment)
    g_object_unref (priv->hadjustment);

  priv->hadjustment = hadjustment ? g_object_ref_sink (hadjustment) : NULL;
}
#endif

static void
mucharmap_chartable_set_vadjustment (MucharmapChartable *chartable,
                                     GtkAdjustment *vadjustment)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (vadjustment)
	g_return_if_fail (GTK_IS_ADJUSTMENT (vadjustment));
  else
	vadjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  if (priv->vadjustment)
	{
	  g_signal_handler_disconnect (priv->vadjustment,
	                               priv->vadjustment_changed_handler_id);
	  priv->vadjustment_changed_handler_id = 0;
	  g_object_unref (priv->vadjustment);
	  priv->vadjustment = NULL;
	}

  if (vadjustment)
	{
	  priv->vadjustment = g_object_ref_sink (vadjustment);
	  priv->vadjustment_changed_handler_id =
	      g_signal_connect (vadjustment, "value-changed",
	                        G_CALLBACK (vadjustment_value_changed_cb),
	                        chartable);
	}

  update_scrollbar_adjustment (chartable);
}

#if !GTK_CHECK_VERSION (2, 91, 2)
static void
mucharmap_chartable_set_adjustments (MucharmapChartable *chartable,
                                     GtkAdjustment *hadjustment,
                                     GtkAdjustment *vadjustment)
{
  mucharmap_chartable_set_vadjustment (chartable, vadjustment);
}
#endif

static void
mucharmap_chartable_add_move_binding (GtkBindingSet  *binding_set,
	                                  guint           keyval,
	                                  guint           modmask,
	                                  GtkMovementStep step,
	                                  gint            count)
{
  gtk_binding_entry_add_signal (binding_set, keyval, modmask,
	                            "move-cursor", 2,
	                            G_TYPE_ENUM, step,
	                            G_TYPE_INT, count);

  gtk_binding_entry_add_signal (binding_set, keyval, GDK_SHIFT_MASK,
	                            "move-cursor", 2,
	                            G_TYPE_ENUM, step,
	                            G_TYPE_INT, count);

  if ((modmask & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
   return;

  gtk_binding_entry_add_signal (binding_set, keyval, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
	                            "move-cursor", 2,
	                            G_TYPE_ENUM, step,
	                            G_TYPE_INT, count);

  gtk_binding_entry_add_signal (binding_set, keyval, GDK_CONTROL_MASK,
	                            "move-cursor", 2,
	                            G_TYPE_ENUM, step,
	                            G_TYPE_INT, count);
}

static void
mucharmap_chartable_move_cursor_left_right (MucharmapChartable *chartable,
	                                        int count)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GtkWidget *widget = GTK_WIDGET (chartable);
  gboolean is_rtl;
  int offset;

  is_rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);
  offset = is_rtl ? -count : count;
  mucharmap_chartable_set_active_cell (chartable, priv->active_cell + offset);
}

static void
mucharmap_chartable_move_cursor_up_down (MucharmapChartable *chartable,
	                                     int count)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  mucharmap_chartable_set_active_cell (chartable,
				       priv->active_cell + priv->cols * count);
}

static void
mucharmap_chartable_move_cursor_page_up_down (MucharmapChartable *chartable,
	                                          int count)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  mucharmap_chartable_set_active_cell (chartable,
				       priv->active_cell + priv->page_size * count);
}

static void
mucharmap_chartable_move_cursor_start_end (MucharmapChartable *chartable,
	                                       int count)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  int new_cell;

  if (count > 0)
	new_cell = priv->last_cell;
  else
	new_cell = 0;

  mucharmap_chartable_set_active_cell (chartable, new_cell);
}

static gboolean
mucharmap_chartable_move_cursor (MucharmapChartable *chartable,
	                             GtkMovementStep     step,
	                             int                 count)
{
  g_return_val_if_fail (step == GTK_MOVEMENT_LOGICAL_POSITIONS ||
	                    step == GTK_MOVEMENT_VISUAL_POSITIONS ||
	                    step == GTK_MOVEMENT_DISPLAY_LINES ||
	                    step == GTK_MOVEMENT_PAGES ||
	                    step == GTK_MOVEMENT_BUFFER_ENDS, FALSE);

  switch (step)
	{
	case GTK_MOVEMENT_LOGICAL_POSITIONS:
	case GTK_MOVEMENT_VISUAL_POSITIONS:
	  mucharmap_chartable_move_cursor_left_right (chartable, count);
	  break;
	case GTK_MOVEMENT_DISPLAY_LINES:
	  mucharmap_chartable_move_cursor_up_down (chartable, count);
	  break;
	case GTK_MOVEMENT_PAGES:
	  mucharmap_chartable_move_cursor_page_up_down (chartable, count);
	  break;
	case GTK_MOVEMENT_BUFFER_ENDS:
	  mucharmap_chartable_move_cursor_start_end (chartable, count);
	  break;
	default:
	  g_assert_not_reached ();
	}

  return TRUE;
}

static void
mucharmap_chartable_copy_clipboard (MucharmapChartable *chartable)
{
  GtkClipboard *clipboard;
  gunichar wc;
  gchar utf8[7];
  gsize len;

  wc = mucharmap_chartable_get_active_character (chartable);
  if (!mucharmap_unichar_validate (wc))
	return;

  len = g_unichar_to_utf8 (wc, utf8);

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (chartable),
	                                    GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text (clipboard, utf8, len);
}

static void
mucharmap_chartable_paste_received_cb (GtkClipboard *clipboard,
	                                   const char *text,
	                                   gpointer user_data)
{
  gpointer *data = (gpointer *) user_data;
  MucharmapChartable *chartable = *data;
  gunichar wc;

  g_slice_free (gpointer, data);

  if (!chartable)
	return;

  g_object_remove_weak_pointer (G_OBJECT (chartable), data);

  if (!text)
	return;

  wc = g_utf8_get_char_validated (text, -1);
  if (wc == 0 ||
	  !mucharmap_unichar_validate (wc)) {
	gtk_widget_error_bell (GTK_WIDGET (chartable));
	return;
  }

  mucharmap_chartable_set_active_character (chartable, wc);
}

static void
mucharmap_chartable_paste_clipboard (MucharmapChartable *chartable)
{
  GtkClipboard *clipboard;
  gpointer *data;

  if (!gtk_widget_get_realized (GTK_WIDGET (chartable)))
	return;

  data = g_slice_new (gpointer);
  *data = chartable;
  g_object_add_weak_pointer (G_OBJECT (chartable), data);

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (chartable),
	                                    GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_request_text (clipboard,
	                          mucharmap_chartable_paste_received_cb,
	                          data);
}

/* does all the initial construction */
static void
mucharmap_chartable_init (MucharmapChartable *chartable)
{
  GtkWidget *widget = GTK_WIDGET (chartable);
  MucharmapChartablePrivate *priv;

  priv = chartable->priv = G_TYPE_INSTANCE_GET_PRIVATE (chartable, MUCHARMAP_TYPE_CHARTABLE, MucharmapChartablePrivate);

  priv->page_first_cell = 0;
  priv->active_cell = 0;
  priv->rows = 1;
  priv->cols = 1;
  priv->zoom_mode_enabled = TRUE;
  priv->zoom_window = NULL;
  priv->snap_pow2_enabled = FALSE;
  priv->font_fallback = TRUE;

  priv->vadjustment = NULL;
#if GTK_CHECK_VERSION (3, 0, 0)
  priv->hadjustment = NULL;
  priv->hscroll_policy = GTK_SCROLL_NATURAL;
  priv->vscroll_policy = GTK_SCROLL_NATURAL;
#endif

/* This didn't fix the slow expose events either: */
/*  gtk_widget_set_double_buffered (widget, FALSE); */

  gtk_widget_set_events (widget,
	      GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK |
	      GDK_BUTTON_RELEASE_MASK | GDK_BUTTON3_MOTION_MASK |
	      GDK_BUTTON1_MOTION_MASK | GDK_FOCUS_CHANGE_MASK | GDK_SCROLL_MASK);

  priv->target_list = gtk_target_list_new (NULL, 0);
  gtk_target_list_add_text_targets (priv->target_list, 0);

  gtk_drag_dest_set (widget, GTK_DEST_DEFAULT_ALL,
	                 NULL, 0,
	                 GDK_ACTION_COPY);
  gtk_drag_dest_add_text_targets (widget);

  /* this is required to get key_press events */
  gtk_widget_set_can_focus (widget, TRUE);

  mucharmap_chartable_set_codepoint_list (chartable, NULL);

  gtk_widget_show_all (GTK_WIDGET (chartable));
}

static void
mucharmap_chartable_finalize (GObject *object)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (object);
  MucharmapChartablePrivate *priv = chartable->priv;

  if (priv->font_desc)
	pango_font_description_free (priv->font_desc);

  mucharmap_chartable_clear_pango_layout (chartable);

  gtk_target_list_unref (priv->target_list);

  if (priv->codepoint_list)
	g_object_unref (priv->codepoint_list);

  destroy_zoom_window (chartable);

  G_OBJECT_CLASS (mucharmap_chartable_parent_class)->finalize (object);
}

static void
mucharmap_chartable_set_property (GObject *object,
	                              guint prop_id,
	                              const GValue *value,
	                              GParamSpec *pspec)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (object);
  MucharmapChartablePrivate *priv = chartable->priv;
  switch (prop_id) {
#if GTK_CHECK_VERSION (3, 0, 0)
	case PROP_HADJUSTMENT:
	  mucharmap_chartable_set_hadjustment (chartable, g_value_get_object (value));
	  break;
	case PROP_VADJUSTMENT:
	  mucharmap_chartable_set_vadjustment (chartable, g_value_get_object (value));
	  break;
	case PROP_HSCROLL_POLICY:
	  priv->hscroll_policy = g_value_get_enum (value);
	  gtk_widget_queue_resize_no_redraw (GTK_WIDGET (chartable));
	  break;
	case PROP_VSCROLL_POLICY:
	  priv->vscroll_policy = g_value_get_enum (value);
	  gtk_widget_queue_resize_no_redraw (GTK_WIDGET (chartable));
	  break;
#endif
	case PROP_ACTIVE_CHAR:
	  mucharmap_chartable_set_active_character (chartable, g_value_get_uint (value));
	  break;
	case PROP_CODEPOINT_LIST:
	  mucharmap_chartable_set_codepoint_list (chartable, g_value_get_object (value));
	  break;
	case PROP_FONT_DESC:
	  mucharmap_chartable_set_font_desc (chartable, g_value_get_boxed (value));
	  break;
	case PROP_FONT_FALLBACK:
	  mucharmap_chartable_set_font_fallback (chartable, g_value_get_boolean (value));
	  break;
	case PROP_SNAP_POW2:
	  mucharmap_chartable_set_snap_pow2 (chartable, g_value_get_boolean (value));
	  break;
	case PROP_ZOOM_ENABLED:
	  mucharmap_chartable_set_zoom_enabled (chartable, g_value_get_boolean (value));
	  break;
	case PROP_ZOOM_SHOWING:
	  /* not writable */
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
mucharmap_chartable_get_property (GObject *object,
	                              guint prop_id,
	                              GValue *value,
	                              GParamSpec *pspec)
{
  MucharmapChartable *chartable = MUCHARMAP_CHARTABLE (object);
  MucharmapChartablePrivate *priv = chartable->priv;

  switch (prop_id) {
#if GTK_CHECK_VERSION (2, 91, 2)
	case PROP_HADJUSTMENT:
	  g_value_set_object (value, NULL);
	  break;
	case PROP_VADJUSTMENT:
	  g_value_set_object (value, priv->vadjustment);
	  break;
	case PROP_HSCROLL_POLICY:
	  g_value_set_enum (value, priv->hscroll_policy);
	  break;
	case PROP_VSCROLL_POLICY:
	  g_value_set_enum (value, priv->vscroll_policy);
	  break;
#endif
	case PROP_ACTIVE_CHAR:
	  g_value_set_uint (value, mucharmap_chartable_get_active_character (chartable));
	  break;
	case PROP_CODEPOINT_LIST:
	  g_value_set_object (value, mucharmap_chartable_get_codepoint_list (chartable));
	  break;
	case PROP_FONT_DESC:
	  g_value_set_boxed (value, mucharmap_chartable_get_font_desc (chartable));
	  break;
	case PROP_SNAP_POW2:
	  g_value_set_boolean (value, priv->snap_pow2_enabled);
	  break;
	case PROP_FONT_FALLBACK:
	  g_value_set_boolean (value, mucharmap_chartable_get_font_fallback (chartable));
	  break;
	case PROP_ZOOM_ENABLED:
	  g_value_set_boolean (value, priv->zoom_mode_enabled);
	  break;
	case PROP_ZOOM_SHOWING:
	  g_value_set_boolean (value, priv->zoom_window != NULL);
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
mucharmap_chartable_class_init (MucharmapChartableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkBindingSet *binding_set;

  g_type_class_add_private (object_class, sizeof (MucharmapChartablePrivate));

  object_class->finalize = mucharmap_chartable_finalize;
  object_class->get_property = mucharmap_chartable_get_property;
  object_class->set_property = mucharmap_chartable_set_property;

  widget_class->drag_begin = mucharmap_chartable_drag_begin;
  widget_class->drag_data_get = mucharmap_chartable_drag_data_get;
  widget_class->drag_data_received = mucharmap_chartable_drag_data_received;
  widget_class->button_press_event = mucharmap_chartable_button_press;
  widget_class->button_release_event = mucharmap_chartable_button_release;
#if GTK_CHECK_VERSION (3, 0, 0)
  widget_class->get_preferred_width = mucharmap_chartable_get_preferred_width;
  widget_class->get_preferred_height = mucharmap_chartable_get_preferred_height;
  widget_class->draw = mucharmap_chartable_draw;
#else
  widget_class->size_request = mucharmap_chartable_size_request;
  widget_class->expose_event = mucharmap_chartable_expose_event;
#endif
  widget_class->focus_in_event = mucharmap_chartable_focus_in_event;
  widget_class->focus_out_event = mucharmap_chartable_focus_out_event;
  widget_class->key_press_event = mucharmap_chartable_key_press_event;
  widget_class->key_release_event = mucharmap_chartable_key_release_event;
  widget_class->motion_notify_event = mucharmap_chartable_motion_notify;
  widget_class->size_allocate = mucharmap_chartable_size_allocate;
  widget_class->style_set = mucharmap_chartable_style_set;
#ifdef ENABLE_ACCESSIBLE
  widget_class->get_accessible = mucharmap_chartable_get_accessible;
#endif

  klass->move_cursor = mucharmap_chartable_move_cursor;
  klass->activate = NULL;
  klass->copy_clipboard = mucharmap_chartable_copy_clipboard;
  klass->paste_clipboard = mucharmap_chartable_paste_clipboard;
  klass->set_active_char = NULL;

  widget_class->activate_signal = signals[ACTIVATE] =
	g_signal_new (I_("activate"),
	              G_TYPE_FROM_CLASS (object_class),
	              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	              G_STRUCT_OFFSET (MucharmapChartableClass, activate),
	              NULL, NULL,
	              g_cclosure_marshal_VOID__VOID,
	              G_TYPE_NONE,
	              0);

#if GTK_CHECK_VERSION (3, 0, 0)
  /* GtkScrollable interface properties */
  g_object_class_override_property (object_class, PROP_HADJUSTMENT, "hadjustment");
  g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
  g_object_class_override_property (object_class, PROP_HSCROLL_POLICY, "hscroll-policy");
  g_object_class_override_property (object_class, PROP_VSCROLL_POLICY, "vscroll-policy");
#else
  klass->set_scroll_adjustments = mucharmap_chartable_set_adjustments;

  /**
   * MucharmapChartable::set-scroll-adjustments
   * @horizontal: the horizontal #GtkAdjustment
   * @vertical: the vertical #GtkAdjustment
   *
   * Set the scroll adjustments for the text view. Usually scrolled containers
   * like #GtkScrolledWindow will emit this signal to connect two instances
   * of #GtkScrollbar to the scroll directions of the #MucharmapChartable.
   */
  widget_class->set_scroll_adjustments_signal =
	g_signal_new (I_("set-scroll-adjustments"),
	              G_TYPE_FROM_CLASS (object_class),
	              G_SIGNAL_RUN_LAST,
	              G_STRUCT_OFFSET (MucharmapChartableClass, set_scroll_adjustments),
	              NULL, NULL,
	              _mucharmap_marshal_VOID__OBJECT_OBJECT,
	              G_TYPE_NONE, 2,
	              GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);
#endif

  signals[STATUS_MESSAGE] =
	g_signal_new (I_("status-message"), mucharmap_chartable_get_type (), G_SIGNAL_RUN_FIRST,
	              G_STRUCT_OFFSET (MucharmapChartableClass, status_message),
	              NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE,
	              1, G_TYPE_STRING);

  signals[MOVE_CURSOR] =
	g_signal_new (I_("move-cursor"),
	              G_TYPE_FROM_CLASS (object_class),
	              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	              G_STRUCT_OFFSET (MucharmapChartableClass, move_cursor),
	              NULL, NULL,
	              _mucharmap_marshal_BOOLEAN__ENUM_INT,
	              G_TYPE_BOOLEAN, 2,
	              GTK_TYPE_MOVEMENT_STEP,
	              G_TYPE_INT);

  signals[COPY_CLIPBOARD] =
	g_signal_new (I_("copy-clipboard"),
	              G_OBJECT_CLASS_TYPE (object_class),
	              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	              G_STRUCT_OFFSET (MucharmapChartableClass, copy_clipboard),
	              NULL, NULL,
	              g_cclosure_marshal_VOID__VOID,
	              G_TYPE_NONE, 0);

  signals[PASTE_CLIPBOARD] =
	g_signal_new (I_("paste-clipboard"),
	              G_OBJECT_CLASS_TYPE (object_class),
	              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	              G_STRUCT_OFFSET (MucharmapChartableClass, paste_clipboard),
	              NULL, NULL,
	              g_cclosure_marshal_VOID__VOID,
	              G_TYPE_NONE, 0);

  /* Not using g_param_spec_unichar on purpose, since it disallows certain values
   * we want (it's performing a g_unichar_validate).
   */
  g_object_class_install_property
	(object_class,
	 PROP_ACTIVE_CHAR,
	 g_param_spec_uint ("active-character", NULL, NULL,
	                    0,
	                    UNICHAR_MAX,
	                    0,
	                    G_PARAM_READWRITE |
	                    G_PARAM_STATIC_NAME |
	                    G_PARAM_STATIC_NICK |
	                    G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_CODEPOINT_LIST,
	 g_param_spec_object ("codepoint-list", NULL, NULL,
	                      mucharmap_codepoint_list_get_type (),
	                      G_PARAM_READWRITE |
	                      G_PARAM_STATIC_NAME |
	                      G_PARAM_STATIC_NICK |
	                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_FONT_DESC,
	 g_param_spec_boxed ("font-desc", NULL, NULL,
	                     PANGO_TYPE_FONT_DESCRIPTION,
	                     G_PARAM_READWRITE |
	                     G_PARAM_STATIC_NAME |
	                     G_PARAM_STATIC_NICK |
	                     G_PARAM_STATIC_BLURB));

  /**
   * MucharmapChartable:font-fallback:
   *
   * Whether font fallback is enabled.
   *
   */
  g_object_class_install_property
    (object_class,
     PROP_FONT_FALLBACK,
     g_param_spec_boolean ("font-fallback", NULL, NULL,
                           TRUE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property
	(object_class,
	 PROP_SNAP_POW2,
	 g_param_spec_boolean ("snap-power-2", NULL, NULL,
	                       FALSE,
	                       G_PARAM_READWRITE |
	                       G_PARAM_STATIC_NAME |
	                       G_PARAM_STATIC_NICK |
	                       G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_ZOOM_ENABLED,
	 g_param_spec_boolean ("zoom-enabled", NULL, NULL,
	                       FALSE,
	                       G_PARAM_READWRITE |
	                       G_PARAM_STATIC_NAME |
	                       G_PARAM_STATIC_NICK |
	                       G_PARAM_STATIC_BLURB));

  g_object_class_install_property
	(object_class,
	 PROP_ZOOM_SHOWING,
	 g_param_spec_boolean ("zoom-showing", NULL, NULL,
	                       FALSE,
	                       G_PARAM_READABLE |
	                       G_PARAM_STATIC_NAME |
	                       G_PARAM_STATIC_NICK |
	                       G_PARAM_STATIC_BLURB));

  /* Keybindings */
  binding_set = gtk_binding_set_by_class (klass);

  /* Cursor movement */
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Up, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Up, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Down, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, 1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Down, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, 1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_p, GDK_CONTROL_MASK,
	                                    GTK_MOVEMENT_DISPLAY_LINES, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_n, GDK_CONTROL_MASK,
	                                    GTK_MOVEMENT_DISPLAY_LINES, 1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Home, 0,
	                                    GTK_MOVEMENT_BUFFER_ENDS, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Home, 0,
	                                    GTK_MOVEMENT_BUFFER_ENDS, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_End, 0,
	                                    GTK_MOVEMENT_BUFFER_ENDS, 1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_End, 0,
	                                    GTK_MOVEMENT_BUFFER_ENDS, 1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Page_Up, 0,
	                                    GTK_MOVEMENT_PAGES, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Page_Up, 0,
	                                    GTK_MOVEMENT_PAGES, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Page_Down, 0,
	                                    GTK_MOVEMENT_PAGES, 1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Page_Down, 0,
	                                    GTK_MOVEMENT_PAGES, 1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Left, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Left, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_Right, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_KP_Right, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, 1);

  /* Activate */
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Return, 0,
	                            "activate", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_ISO_Enter, 0,
	                            "activate", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Enter, 0,
	                            "activate", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_space, 0,
	                            "activate", 0);

  /* Clipboard actions */
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_c, GDK_CONTROL_MASK,
	                            "copy-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Insert, GDK_CONTROL_MASK,
	                            "copy-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_v, GDK_CONTROL_MASK,
	                            "paste-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Insert, GDK_SHIFT_MASK,
	                            "paste-clipboard", 0);

#if 0
  /* VI keybindings */
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_k, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_K, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_j, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, 1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_J, 0,
	                                    GTK_MOVEMENT_DISPLAY_LINES, 1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_b, 0,
	                                    GTK_MOVEMENT_PAGES, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_B, 0,
	                                    GTK_MOVEMENT_PAGES, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_h, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, -1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_H, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, -1);

  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_l, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  mucharmap_chartable_add_move_binding (binding_set, GDK_KEY_L, 0,
	                                    GTK_MOVEMENT_VISUAL_POSITIONS, 1);
#endif
}

/* public API */

/**
 * mucharmap_chartable_new:
 *
 * Returns: a new #MucharmapChartable
 */
GtkWidget *
mucharmap_chartable_new (void)
{
  return GTK_WIDGET (g_object_new (mucharmap_chartable_get_type (), NULL));
}

/**
 * mucharmap_chartable_set_zoom_enabled:
 * @chartable: a #MucharmapChartable
 * @enabled: whether to enable zoom mode
 *
 * Enables or disables the zoom popup.
 */
void
mucharmap_chartable_set_zoom_enabled (MucharmapChartable *chartable,
	                                  gboolean enabled)
{
  MucharmapChartablePrivate *priv;
  GObject *object;

  g_return_if_fail (MUCHARMAP_IS_CHARTABLE (chartable));

  priv = chartable->priv;

  enabled = enabled != FALSE;
  if (priv->zoom_mode_enabled == enabled)
	return;

  object = G_OBJECT (chartable);
  g_object_freeze_notify (object);

  priv->zoom_mode_enabled = enabled;
  if (!enabled)
	mucharmap_chartable_hide_zoom (chartable);

  g_object_notify (object, "zoom-enabled");
  g_object_thaw_notify (object);
}

/**
 * mucharmap_chartable_get_zoom_enabled:
 * @chartable: a #MucharmapChartable
 *
 * Returns: whether zooming is enabled
 */
gboolean
mucharmap_chartable_get_zoom_enabled (MucharmapChartable *chartable)
{
  g_return_val_if_fail (MUCHARMAP_IS_CHARTABLE (chartable), FALSE);

  return chartable->priv->zoom_mode_enabled;
}

/**
 * mucharmap_chartable_set_font_desc:
 * @chartable: a #MucharmapChartable
 * @font_desc: a #PangoFontDescription
 *
 * Sets @font_desc as the font to use to display the character table.
 */
void
mucharmap_chartable_set_font_desc (MucharmapChartable *chartable,
	                               PangoFontDescription *font_desc)
{
  MucharmapChartablePrivate *priv;

  g_return_if_fail (MUCHARMAP_IS_CHARTABLE (chartable));
  g_return_if_fail (font_desc != NULL);

  priv = chartable->priv;

  if (priv->font_desc &&
	  pango_font_description_equal (font_desc, priv->font_desc))
	return;

  mucharmap_chartable_set_font_desc_internal (chartable,
	                                          pango_font_description_copy (font_desc));
}

/**
 * mucharmap_chartable_get_font_desc:
 * @chartable: a #MucharmapChartable
 *
 * Returns: the #PangoFontDescription used to display the character table.
 *   The returned object is owned by @chartable and must not be modified or freed.
 */
PangoFontDescription *
mucharmap_chartable_get_font_desc (MucharmapChartable *chartable)
{
  g_return_val_if_fail (MUCHARMAP_IS_CHARTABLE (chartable), NULL);

  return chartable->priv->font_desc;
}

/**
 * mucharmap_chartable_set_font_fallback:
 * @chartable: a #MucharmapChartable
 * @enable_font_fallback: whether to enable font fallback
 *
 */
void
mucharmap_chartable_set_font_fallback (MucharmapChartable *chartable,
                                       gboolean enable_font_fallback)
{
  MucharmapChartablePrivate *priv;
  GtkWidget *widget;

  g_return_if_fail (MUCHARMAP_IS_CHARTABLE (chartable));

  priv = chartable->priv;
  enable_font_fallback = enable_font_fallback != FALSE;
  if (enable_font_fallback == priv->font_fallback)
    return;

  priv->font_fallback = enable_font_fallback;
  g_object_notify (G_OBJECT (chartable), "font-fallback");

  mucharmap_chartable_clear_pango_layout (chartable);

  widget = GTK_WIDGET (chartable);
  if (gtk_widget_get_realized (widget))
    gtk_widget_queue_draw (widget);
}

/**
 * mucharmap_chartable_get_font_fallback:
 * @chartable: a #MucharmapChartable
 *
 * Returns: whether font fallback is enabled
 *
 */
gboolean
mucharmap_chartable_get_font_fallback (MucharmapChartable *chartable)
{
  g_return_val_if_fail (MUCHARMAP_IS_CHARTABLE (chartable), FALSE);

  return chartable->priv->font_fallback;
}

/**
 * mucharmap_chartable_get_active_character:
 * @chartable: a #MucharmapChartable
 *
 * Returns: the currently selected character
 */
gunichar
mucharmap_chartable_get_active_character (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  if (!priv->codepoint_list)
	return 0;

  return mucharmap_codepoint_list_get_char (priv->codepoint_list, priv->active_cell);
}

/**
 * mucharmap_chartable_get_active_character:
 * @chartable: a #MucharmapChartable
 * @wc: a unicode character (UTF-32)
 *
 * Sets @wc as the selected character.
 */
void
mucharmap_chartable_set_active_character (MucharmapChartable *chartable,
	                                      gunichar wc)
{
  set_active_char (chartable, wc);
}

/**
 * mucharmap_chartable_set_snap_pow2:
 * @chartable: a #MucharmapChartable
 * @snap: whether to enable or disable snapping
 *
 * Sets whether the number columns the character table shows should
 * always be a power of 2.
 */
void
mucharmap_chartable_set_snap_pow2 (MucharmapChartable *chartable,
	                               gboolean snap)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  snap = snap != FALSE;

  if (snap != priv->snap_pow2_enabled)
	{
	  priv->snap_pow2_enabled = snap;

	  gtk_widget_queue_resize (GTK_WIDGET (chartable));

	  g_object_notify (G_OBJECT (chartable), "snap-power-2");
	}
}

/**
 * mucharmap_chartable_get_snap_pow2:
 * @chartable: a #MucharmapChartable
 *
 * Returns whether the number of columns the character table shows is
 * always a power of 2.
 */
gboolean
mucharmap_chartable_get_snap_pow2 (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  return priv->snap_pow2_enabled;
}

/**
 * mucharmap_chartable_get_active_character:
 * @chartable: a #MucharmapChartable
 * @list: a #MucharmapCodepointList
 *
 * Sets the codepoint list to show in the character table.
 */
void
mucharmap_chartable_set_codepoint_list (MucharmapChartable     *chartable,
	                                    MucharmapCodepointList *codepoint_list)
{
  MucharmapChartablePrivate *priv = chartable->priv;
  GObject *object = G_OBJECT (chartable);
  GtkWidget *widget = GTK_WIDGET (chartable);

  g_object_freeze_notify (object);

  if (codepoint_list)
	g_object_ref (codepoint_list);
  if (priv->codepoint_list)
	g_object_unref (priv->codepoint_list);
  priv->codepoint_list = codepoint_list;
  priv->codepoint_list_changed = TRUE;

  priv->active_cell = 0;
  priv->page_first_cell = 0;
  if (codepoint_list)
	priv->last_cell = mucharmap_codepoint_list_get_last_index (codepoint_list);
  else
	priv->last_cell = 0;

  g_object_notify (object, "codepoint-list");
  g_object_notify (object, "active-character");

  update_scrollbar_adjustment (chartable);

  gtk_widget_queue_draw (widget);

  g_object_thaw_notify (object);
}

/**
 * mucharmap_chartable_get_codepoint_list:
 * @chartable: a #MucharmapChartable
 *
 * Returns: the current codepoint list
 */
MucharmapCodepointList *
mucharmap_chartable_get_codepoint_list (MucharmapChartable *chartable)
{
  MucharmapChartablePrivate *priv = chartable->priv;

  return priv->codepoint_list;
}
