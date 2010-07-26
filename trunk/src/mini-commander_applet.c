 /*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>,
 *               2002 Sun Microsystems
 *
 * Authors: Oliver Maruhn <oliver@maruhn.com>,
 *          Mark McLoughlin <mark@skynet.ie>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>

#include <gtk/gtkaccessible.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkenums.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkvbox.h>

#include <panel-applet.h>
#include <gconf/gconf-client.h>
#include <libgnomeui-2.0/libgnomeui/gnome-window-icon.h>
#include "mini-commander_applet.h"
#include "preferences.h"
#include "command_line.h"
#include "about.h"
#include "help.h"

static const BonoboUIVerb mini_commander_menu_verbs [] = {
        BONOBO_UI_UNSAFE_VERB ("Props", mc_show_preferences),
        BONOBO_UI_UNSAFE_VERB ("Help",  show_help),
        BONOBO_UI_UNSAFE_VERB ("About", about_box),

        BONOBO_UI_VERB_END
};

void
set_atk_name_description (GtkWidget  *widget,
			  const char *name,
			  const char *description)
{	
    AtkObject *aobj;
	
    aobj = gtk_widget_get_accessible (widget);
    if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
        return;

    atk_object_set_name (aobj, name);
    atk_object_set_description (aobj, description);
}

/* Send button presses on the applet to the entry. This makes Fitts' law work (ie click on the bottom
** most pixel and the key press will be sent to the entry */
static gboolean
send_button_to_entry_event (GtkWidget *widget, GdkEventButton *event, MCData *mc)
{

	if (event->button == 1) {
		gtk_widget_grab_focus (mc->entry);
		return TRUE;
	}
	return FALSE;

}

static gboolean
key_press_cb (GtkWidget *widget, GdkEventKey *event, MCData *mc)
{
	switch (event->keyval) {	
	case GDK_b:
		if (event->state == GDK_CONTROL_MASK) {
			mc_show_file_browser (NULL, NULL, mc);
			return TRUE;
		}
		break;
	case GDK_h:
		if (event->state == GDK_CONTROL_MASK) {
			mc_show_history (NULL, NULL, mc);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;

}

void
mc_applet_draw (MCData *mc)
{

    gchar     *command_text = NULL;

    if (mc->entry != NULL)
	command_text = g_strdup (gtk_editable_get_chars (GTK_EDITABLE (mc->entry), 0, -1));

    if (mc->applet_box) {
        gtk_widget_destroy (mc->applet_box);	
    }

    mc_create_command_entry (mc);

    if (command_text != NULL) {
	gtk_entry_set_text (GTK_ENTRY (mc->entry), command_text);
	g_free (command_text);
    }
    mc->tooltips = gtk_tooltips_new ();
    g_signal_connect (G_OBJECT(mc->file_select_button), "button_release_event",
		      G_CALLBACK (mc_show_file_browser), mc);
    gtk_tooltips_set_tip (mc->tooltips, mc->file_select_button, _("Browser"), NULL);
    set_atk_name_description (mc->file_select_button,
			      _("Browser"),
			      _("Click this button to start the browser"));


    g_signal_connect (G_OBJECT(mc->history_button), "button_release_event",
		      G_CALLBACK (mc_show_history), mc);
    gtk_tooltips_set_tip (mc->tooltips, mc->history_button, _("History"), NULL);
    set_atk_name_description (mc->history_button,
			      _("History"),
			      _("Click this button for the list of previous commands"));

    gtk_container_add (GTK_CONTAINER (mc->applet), mc->applet_box);
    
    gtk_widget_show_all (mc->applet_box);
}

static void
mc_destroyed (GtkWidget *widget,
	      MCData    *mc)
{
    GConfClient *client;
    int          i;

    client = gconf_client_get_default ();
    for (i = 0; i < MC_NUM_LISTENERS; i++) {
	gconf_client_notify_remove (client, mc->listeners [i]);
	mc->listeners [i] = 0;
    }
    g_object_unref (client);

    mc_macros_free (mc->preferences.macros);

    if (mc->prefs_dialog.dialog)
        gtk_widget_destroy (mc->prefs_dialog.dialog);

    if (mc->prefs_dialog.dialog)
        g_object_unref (mc->prefs_dialog.macros_store);

    if (mc->file_select)
        gtk_widget_destroy (mc->file_select);
    
    g_free (mc);
}

/*
static void
mc_orient_changed (PanelApplet *applet,
		   PanelAppletOrient orient,
		   MCData *mc)
{
 
  mc->orient = orient;
  mc_applet_draw (mc);
}
*/

/*
static void
mc_pixel_size_changed (PanelApplet *applet,
		       GtkAllocation *allocation,
		       MCData      *mc)
{

  
  if ((mc->orient == PANEL_APPLET_ORIENT_LEFT) || (mc->orient == PANEL_APPLET_ORIENT_RIGHT)) {
    if (mc->preferences.panel_size_x == allocation->width)
      return;
    mc->preferences.panel_size_x = allocation->width;
  } else {
    if (mc->preferences.normal_size_y == allocation->height)
      return;
    mc->preferences.normal_size_y = allocation->height;
  }

  mc_applet_draw (mc);

}
*/

static void
reset_style (GtkWidget *widget)
{
    GtkRcStyle *rc_style;

    gtk_widget_set_style (widget, NULL);
    rc_style = gtk_rc_style_new ();
    gtk_widget_modify_style (widget, rc_style);
    gtk_rc_style_unref (rc_style);
}

static void
set_background_pixmap (GtkWidget *widget, GdkPixmap *pixmap)
{
    GtkStyle   *style;

    style = gtk_style_copy (widget->style);
    if (style->bg_pixmap[GTK_STATE_NORMAL])
        g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
     style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
    
    gtk_widget_set_style (widget, style);
    g_object_unref (style);
   
}

static void
prepare_pixmap_background (MCData *mc, GdkPixmap *pixmap)
{

    gint width=0, height=0, x, y;
    GdkPixmap *small_pix=NULL;
    GdkGC *gc;
    
    gdk_drawable_get_size (mc->entry->window, &width, &height);
    small_pix= gdk_pixmap_new (pixmap, width, height, -1);
    gc=gdk_gc_new (pixmap);
    gdk_window_get_position (mc->entry->window, &x, &y);
    gdk_draw_drawable (small_pix, gc, pixmap, x, y, 0, 0, width, height);
    set_background_pixmap (mc->applet_box, pixmap);
    set_background_pixmap (mc->entry, small_pix);

    g_object_unref (gc);
    g_object_unref (small_pix);

}

static void
applet_change_background (PanelApplet               *applet,
          PanelAppletBackgroundType  type,
          GdkColor                  *color,
          GdkPixmap                 *pixmap,
          MCData *mc)
{

    if (!mc->preferences.show_default_theme)
        return;
    
    reset_style (mc->entry);
    reset_style (mc->applet_box);
    mc->default_background=FALSE;
    
    switch (type) {
        case PANEL_NO_BACKGROUND:
            mc->default_background=TRUE;
        break;
        case PANEL_COLOR_BACKGROUND:
            gtk_widget_modify_base (mc->entry, GTK_STATE_NORMAL, color);
            gtk_widget_modify_bg (mc->applet_box, GTK_STATE_NORMAL, color);
        break;
        case PANEL_PIXMAP_BACKGROUND:
            prepare_pixmap_background (mc, pixmap);
        break;
    }

}

static gboolean
draw_background_timer (gpointer data)
{

    gint width=0, height=0;
    PanelAppletBackgroundType  type;
    GdkPixmap *pixmap=NULL;
    GdkColor color;
    MCData *mc= (MCData*) data;

     if (!GDK_IS_DRAWABLE (mc->entry->window))
         return TRUE;
  
     gdk_drawable_get_size (mc->entry->window, &width, &height);
     if (width <= 1 || height <= 1)
         return TRUE;

     type = panel_applet_get_background (mc->applet, &color, &pixmap);
        applet_change_background (mc->applet, type,  &color, pixmap, mc);

     g_signal_connect (mc->applet, "change_background",
          G_CALLBACK (applet_change_background), mc);

    return FALSE;
}

static void
on_applet_map (GtkWidget *widget,MCData *mc)
{
  g_timeout_add (100, draw_background_timer, mc);
}

static gboolean
mini_commander_applet_fill (PanelApplet *applet)
{
    MCData *mc;
    GConfClient *client;

    client = gconf_client_get_default ();
    if (gconf_client_get_bool (client, "/desktop/gnome/lockdown/inhibit_command_line", NULL)) {
	    GtkWidget *error_dialog;

	    error_dialog = gtk_message_dialog_new (NULL,
						   GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_ERROR,
						   GTK_BUTTONS_OK,
						   _("Command line has been disabled by your system administrator"));

	    gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
	    gtk_dialog_set_has_separator (GTK_DIALOG (error_dialog), FALSE);
	    gtk_window_set_screen (GTK_WINDOW (error_dialog),
				   gtk_widget_get_screen (GTK_WIDGET (applet)));
	    gtk_dialog_run (GTK_DIALOG (error_dialog));
	    gtk_widget_destroy (error_dialog);

	    /* Note that this is only kosher if this is an out of process thing,
	       which we really are.  We really don't need/want this applet when
	       command line is disabled */
	    exit (1);
    }

    g_set_application_name (_("Command Line"));
    
    gtk_window_set_default_icon_name ("gnome-mini-commander");
    
    mc = g_new0 (MCData, 1);
    mc->applet = applet;

    panel_applet_add_preferences (applet, "/schemas/apps/mini-commander/prefs", NULL);
    panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
    mc_load_preferences (mc);
  
    mc->file_select=NULL;

    mc->preferences.normal_size_y = panel_applet_get_size (applet);
    mc->orient = panel_applet_get_orient (applet);
    mc_applet_draw(mc);

    g_signal_connect ((gpointer) mc->applet, "map",
        G_CALLBACK (on_applet_map), mc);

    gtk_widget_show (GTK_WIDGET (mc->applet));
    
    g_signal_connect (mc->applet, "destroy", G_CALLBACK (mc_destroyed), mc); 
    g_signal_connect (mc->applet, "button_press_event",
		      G_CALLBACK (send_button_to_entry_event), mc);
    g_signal_connect (mc->applet, "key_press_event",
		      G_CALLBACK (key_press_cb), mc);

    panel_applet_setup_menu_from_file (mc->applet,
				       DATADIR,
				       "GNOME_MiniCommanderApplet.xml",
				       NULL,
				       mini_commander_menu_verbs,
				       mc);

    if (panel_applet_get_locked_down (mc->applet)) {
	    BonoboUIComponent *popup_component;

	    popup_component = panel_applet_get_popup_component (mc->applet);

	    bonobo_ui_component_set_prop (popup_component,
					  "/commands/Props",
					  "hidden", "1",
					  NULL);
    }

    set_atk_name_description (GTK_WIDGET (applet),
			      _("Mini-Commander applet"),
			      _("This applet adds a command line to the panel"));
    
    return TRUE;
}

static gboolean
mini_commander_applet_factory (PanelApplet *applet,
			       const gchar *iid,
			       gpointer     data)
{
        gboolean retval = FALSE;

        if (!strcmp (iid, "OAFIID:GNOME_MiniCommanderApplet"))
                retval = mini_commander_applet_fill(applet); 
    
        return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MiniCommanderApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "command-line",
                             "0",
                             mini_commander_applet_factory,
                             NULL)
