/*
 *  Copyright (C) 2005 Jochen Baier
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  Adapted and modified from ephy-icon-entry (epiphany) (C) Christian Persch
 *
 * 
 */

#include "config.h"

#include "icon-entry.h"
#include "mini-commander_applet.h"

#include <gtk/gtkentry.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkhbox.h>


#define ICON_ENTRY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_ICON_ENTRY,IconEntryPrivate))

struct _IconEntryPrivate
{
  GtkWidget *hbox;
};

static GtkWidgetClass *parent_class = NULL;


void
icon_entry_paint (GtkWidget *widget, GdkEventExpose *event)
{

  IconEntry *entry = ICON_ENTRY (widget);
  GtkWidget *entry_widget = entry->entry;
  gint entry_w, entry_h, entry_x, entry_y, applet_w, applet_h;
  gint rec_x, rec_y, rec_w, rec_h;
  GdkColor bg;
  MCData *mc;
  GdkGC *gc;

  mc=g_object_get_data  (G_OBJECT (widget), "mcdata");

  if (!GDK_IS_DRAWABLE (widget->window) || !GDK_IS_DRAWABLE (entry_widget->window))
      return;
  
  gdk_drawable_get_size (entry_widget->window, &entry_w, &entry_h);
  gdk_drawable_get_size (widget->window, &applet_w, &applet_h);
  gdk_window_get_position (entry_widget->window, &entry_x, &entry_y);
  
  if ((entry_h + 2 *BORDER) >= applet_h) {
    rec_x=entry_x-1; rec_y=entry_y-1; rec_w=applet_w- (2*(entry_x-1))-1;
    rec_h=entry_h+2;
  } else {
    rec_x=entry_x-1; rec_y=entry_y-BORDER; rec_w=applet_w- (2*(entry_x-1))-1;
    rec_h=entry_h + 2*BORDER-1;
  }
  
   if (!mc->preferences.show_default_theme) {

    bg.red   = mc->preferences.cmd_line_color_bg_r;
    bg.green = mc->preferences.cmd_line_color_bg_g;
    bg.blue  = mc->preferences.cmd_line_color_bg_b;
    
    gc = gdk_gc_new (widget->window);
  
    gdk_gc_set_rgb_fg_color (gc, &bg);
    gdk_draw_rectangle (widget->window, gc, TRUE,
      rec_x, rec_y, rec_w, rec_h);
    g_object_unref (gc);
   
  } else if (mc->default_background) {
    gdk_draw_rectangle (widget->window, mc->entry->style->base_gc[GTK_STATE_NORMAL], TRUE,
      rec_x, rec_y, rec_w, rec_h);
  }    

  gdk_draw_rectangle (widget->window, widget->style->black_gc, FALSE,
  rec_x, rec_y, rec_w, rec_h);

}

static void
icon_entry_init (IconEntry *entry)
{
  IconEntryPrivate *priv;
  GtkWidget *widget = (GtkWidget *) entry;

  priv = entry->priv = ICON_ENTRY_GET_PRIVATE (entry);
  
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_NO_WINDOW);
  
  priv->hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (entry), priv->hbox);
  
  entry->entry = gtk_entry_new ();
  gtk_entry_set_has_frame (GTK_ENTRY (entry->entry), FALSE);
  gtk_box_pack_start (GTK_BOX (priv->hbox), entry->entry, TRUE, TRUE, 0);
 
}

static void
icon_entry_realize (GtkWidget *widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget)
  | GDK_EXPOSURE_MASK;
  
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
      &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
icon_entry_size_request (GtkWidget *widget,
     GtkRequisition *requisition)
{
  IconEntry *entry = ICON_ENTRY (widget);
  GtkContainer *container = GTK_CONTAINER (widget);
  GtkBin *bin = GTK_BIN (widget);
  
  requisition->width = requisition->height = container->border_width * 2;
  requisition->width = requisition->height = 0;
  
  gtk_widget_ensure_style (entry->entry);
  
  if (GTK_WIDGET_VISIBLE (bin->child))
  {
    GtkRequisition child_requisition;
    
    gtk_widget_size_request (bin->child, &child_requisition);
    requisition->width += child_requisition.width;
    requisition->height += child_requisition.height;
  }
  
  requisition->width += 2 * BORDER;
  requisition->height += 2 * BORDER;
}

static void
icon_entry_size_allocate (GtkWidget *widget,
       GtkAllocation *allocation)
{
  GtkContainer *container = GTK_CONTAINER (widget);
  GtkBin *bin = GTK_BIN (widget);
  GtkAllocation child_allocation;
  
  widget->allocation = *allocation;
  
  if (GTK_WIDGET_REALIZED (widget))
  {
    child_allocation.x = container->border_width;
    child_allocation.y = container->border_width;
    child_allocation.width = MAX (allocation->width - container->border_width * 2, 0);
    child_allocation.height = MAX (allocation->height - container->border_width * 2, 0);
    
    gdk_window_move_resize (widget->window,
    allocation->x + child_allocation.x,
    allocation->y + child_allocation.y,
    child_allocation.width,
    child_allocation.height);
  }
  
  child_allocation.x = container->border_width + BORDER;
  child_allocation.y = container->border_width + BORDER;
  child_allocation.width = MAX (allocation->width - (container->border_width + BORDER) * 2, 0);
  child_allocation.height = MAX (allocation->height - (container->border_width + BORDER) * 2, 0);
  
  gtk_widget_size_allocate (bin->child, &child_allocation);
}

static gboolean
icon_entry_expose (GtkWidget *widget,
    GdkEventExpose *event)
{
  if (GTK_WIDGET_DRAWABLE (widget) &&
  event->window == widget->window) {
    icon_entry_paint (widget, event);
  }

  return parent_class->expose_event (widget, event);
}

static void
icon_entry_class_init (IconEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  parent_class = GTK_WIDGET_CLASS (g_type_class_peek_parent (klass));
  
  widget_class->realize = icon_entry_realize;
  widget_class->size_request = icon_entry_size_request;
  widget_class->size_allocate = icon_entry_size_allocate;
  widget_class->expose_event = icon_entry_expose;
  
  g_type_class_add_private (object_class, sizeof (IconEntryPrivate));
}

GType
icon_entry_get_type (void)
{
  static GType type = 0;
  
  if (G_UNLIKELY (type == 0)) {
    static const GTypeInfo our_info =
    {
      sizeof (IconEntryClass), NULL, NULL, (GClassInitFunc) icon_entry_class_init,
      NULL, NULL, sizeof (IconEntry), 0, (GInstanceInitFunc) icon_entry_init
    };
    
    type = g_type_register_static (GTK_TYPE_BIN, "IconEntry", &our_info, 0);
  }
  
  return type;
}

GtkWidget *
icon_entry_new (void)
{
  return GTK_WIDGET (g_object_new (TYPE_ICON_ENTRY, NULL));
}

void
icon_entry_pack_widget (IconEntry *entry,
			     GtkWidget *widget,
			     gboolean start)
{
  IconEntryPrivate *priv;
  
  g_return_if_fail (IS_ICON_ENTRY (entry));
  
  priv = entry->priv;
  
  if (start) {
   gtk_box_pack_start (GTK_BOX (priv->hbox), widget, FALSE, FALSE, 2);
   gtk_box_reorder_child (GTK_BOX (priv->hbox), widget, 0);
  } else {
   gtk_box_pack_end (GTK_BOX (priv->hbox), widget, FALSE, FALSE, 2);
  }
}

GtkWidget *
icon_entry_get_entry (IconEntry *entry)
{
 g_return_val_if_fail (IS_ICON_ENTRY (entry), NULL);
 return entry->entry;
}
