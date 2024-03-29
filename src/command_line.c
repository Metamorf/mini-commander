/*
 * Mini-Commander Applet
 * Copyright (C) 1998, 1999 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
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
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkfilechooser.h>
#include <gtk/gtkfilechooserdialog.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkimage.h>

#include <panel-applet.h>

#include "mini-commander_applet.h"
#include "command_line.h"
#include "preferences.h"
#include "exec.h"
#include "cmd_completion.h"
#include "history.h"
#include "icon-entry.h"

static gint file_browser_response_signal(GtkWidget *widget, gint response, gpointer mc_data);
static gint history_popup_clicked_cb(GtkWidget *widget, gpointer data);
static gint history_popup_clicked_inside_cb(GtkWidget *widget, gpointer data);
static gchar* history_auto_complete(GtkWidget *widget, GdkEventKey *event);
gboolean history_window_expose_event (GtkWidget *widget,  GdkEventExpose  *event,
    gpointer user_data);

static int history_position = MC_HISTORY_LIST_LENGTH;
static gchar *browsed_folder = NULL;

static gboolean
button_press_cb (GtkEntry       *entry,
		 GdkEventButton *event,
		 MCData         *mc)
{
    const gchar *str;

    panel_applet_request_focus (mc->applet, event->time);

    if (mc->error) { 
	   mc->error = FALSE; 
	   str = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	   gtk_entry_set_text (entry, (gchar *) str + 3);
	}
    return FALSE;
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean
button_press_hack (GtkWidget      *widget,
		   GdkEventButton *event,
		   MCData         *mc)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (GTK_WIDGET (mc->applet), (GdkEvent *) event);
	return TRUE;
    }

    return FALSE;
}

static gboolean
command_key_event (GtkEntry   *entry,
		   GdkEventKey *event,
		   MCData      *mc)
{
    guint key = event->keyval;
    char *command;
    static char current_command[MC_MAX_COMMAND_LENGTH];
    char buffer[MC_MAX_COMMAND_LENGTH];
    gboolean propagate_event = TRUE;
    const gchar *str;

    if (mc->error) { 
	   mc->error = FALSE; 
	   str = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	   gtk_entry_set_text (entry, (gchar *) str + 3);
	   gtk_editable_set_position (GTK_EDITABLE (entry), strlen (str));
	}
    if(key == GDK_Tab
       || key == GDK_KP_Tab
       || key == GDK_ISO_Left_Tab)
        {
            if(event->state == GDK_CONTROL_MASK)
                {
                    /*
                     * Move focus to the next applet.
                     */
                    gtk_widget_child_focus (gtk_widget_get_toplevel(GTK_WIDGET(mc->applet)),
                        GTK_DIR_TAB_FORWARD);
	            propagate_event = FALSE;
                }
            else if(event->state != GDK_SHIFT_MASK)
	        {	
	                    /* tab key pressed */
	            strcpy(buffer, (char *) gtk_entry_get_text(GTK_ENTRY(entry)));
	            mc_cmd_completion (mc, buffer);
	            gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);

	            propagate_event = FALSE;
	        }
	}
    else if(key == GDK_Up
	    || key == GDK_KP_Up
	    || key == GDK_ISO_Move_Line_Up
	    || key == GDK_Pointer_Up)
	{
	    /* up key pressed */
	    if(history_position == MC_HISTORY_LIST_LENGTH)
		{	    
		    /* store current command line */
		    strcpy(current_command, (char *) gtk_entry_get_text(entry));
		}
	    if(history_position > 0 && exists_history_entry(history_position - 1))
		{
		    gtk_entry_set_text(entry, (gchar *) get_history_entry(--history_position));
		}

	    propagate_event = FALSE;
	}
    else if(key == GDK_Down
	    || key == GDK_KP_Down
	    || key == GDK_ISO_Move_Line_Down
	    || key == GDK_Pointer_Down)
	{
	    /* down key pressed */
	    if(history_position <  MC_HISTORY_LIST_LENGTH - 1)
		{
		    gtk_entry_set_text(entry, (gchar *) get_history_entry(++history_position));
		}
	    else if(history_position == MC_HISTORY_LIST_LENGTH - 1)
		{	    
		    gtk_entry_set_text(entry, (gchar *) current_command);
		    ++history_position;
		}

	    propagate_event = FALSE;
	}
    else if(key == GDK_Return
	    || key == GDK_KP_Enter
	    || key == GDK_ISO_Enter
	    || key == GDK_3270_Enter)
	{
	    /* enter pressed -> exec command */
	    command = (char *) malloc(sizeof(char) * MC_MAX_COMMAND_LENGTH);
	    strcpy(command, (char *) gtk_entry_get_text(entry));
	    mc_exec_command(mc, command);

	    history_position = MC_HISTORY_LIST_LENGTH;		   
	    free(command);

	    strcpy(current_command, "");
	    propagate_event = FALSE;
	}
    else if (mc->preferences.auto_complete_history && key >= GDK_space && key <= GDK_asciitilde )
	{
            char *completed_command;
	    gint current_position = gtk_editable_get_position(GTK_EDITABLE(entry));
	    
	    if(current_position != 0)
		{
		    gtk_editable_delete_text( GTK_EDITABLE(entry), current_position, -1 );
		    completed_command = history_auto_complete(GTK_WIDGET (entry), event);
		    
		    if(completed_command != NULL)
			{
			    gtk_entry_set_text(entry, completed_command);
			    gtk_editable_set_position(GTK_EDITABLE(entry), current_position + 1);
			    propagate_event = FALSE;
			}
		}	    
	}
    
    return !propagate_event;
}

static gint
history_popup_clicked_cb(GtkWidget *widget, gpointer data)
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    gdk_keyboard_ungrab(GDK_CURRENT_TIME);
    gtk_grab_remove(GTK_WIDGET(widget));
    gtk_widget_destroy(GTK_WIDGET(widget));
    widget = NULL;
     
    /* go on */
    return (FALSE);
}

static gint
history_popup_clicked_inside_cb(GtkWidget *widget, gpointer data)
{
    /* eat signal (prevent that popup will be destroyed) */
    return(TRUE);
}

static gboolean
history_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval == GDK_Escape) {
        gdk_pointer_ungrab(GDK_CURRENT_TIME);
        gdk_keyboard_ungrab(GDK_CURRENT_TIME);
        gtk_grab_remove(GTK_WIDGET(widget));
        gtk_widget_destroy(GTK_WIDGET(widget));
        widget = NULL;
    }

    return FALSE;
    
}

static gboolean
history_list_key_press_cb (GtkWidget   *widget,
			   GdkEventKey *event,
			   MCData      *mc)
{
    GtkTreeView *tree = g_object_get_data (G_OBJECT (mc->applet), "tree");
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *command;
    
    switch (event->keyval) {
    case GDK_KP_Enter:
    case GDK_ISO_Enter:
    case GDK_3270_Enter:
    case GDK_Return:
    case GDK_space:
    case GDK_KP_Space:
        if ((event->state & GDK_CONTROL_MASK) &&
            (event->keyval == GDK_space || event->keyval == GDK_KP_Space))
             break;

        if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (tree),
        				      &model,
        				      &iter))        				    
            return FALSE;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                            0, &command, -1);
        mc_exec_command (mc, command);
        g_free (command);
        gtk_widget_destroy(GTK_WIDGET(widget->parent->parent->parent));

	return TRUE;

    default:
        break;
    }
    
    return FALSE;
}

static gboolean 
history_list_button_press_cb (GtkWidget      *widget,
			      GdkEventButton *event,
			      MCData         *mc)
{
    GtkTreeView *tree = g_object_get_data (G_OBJECT (mc->applet), "tree");
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *command;
    
    if (event->type == GDK_2BUTTON_PRESS) {
    	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (tree),
        				      &model,
        				      &iter))        				    
            return FALSE;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                            0, &command, -1);
        mc_exec_command(mc, command);
        g_free (command);
        gtk_widget_destroy(GTK_WIDGET(widget->parent->parent));

	return TRUE;
    }

    return FALSE;
}

gboolean
history_window_expose_event (GtkWidget       *widget,
    GdkEventExpose  *event,
    gpointer         user_data)
{

  gint width, height;

  gdk_drawable_get_size (widget->window, &width, &height);
  gdk_draw_rectangle (widget->window, widget->style->black_gc, FALSE,
    0, 0, width-1, height-1);
  return FALSE;
}

gboolean
mc_show_history (GtkWidget      *event_box, 
                         GdkEventButton *event,
		 MCData    *mc)
{
     GtkWidget *window;
     GtkWidget *scrolled_window;
     GtkListStore *store;
     GtkTreeIter iter;
     GtkTreeModel *model;
     GtkWidget    *treeview;
     GtkCellRenderer *cell_renderer;
     GtkTreeViewColumn *column;
     GtkRequisition  req;
     gchar *command_list[1];
     int i, j;
     gint win_x, win_y, width, height;
     gint entry_h, entry_x, entry_y, applet_x, applet_y;

     /* count commands stored in history list */
     for(i = 0, j = 0; i < MC_HISTORY_LIST_LENGTH; i++)
	 if(exists_history_entry(i))
	     j++;

     window = gtk_window_new(GTK_WINDOW_POPUP); 
     gtk_window_set_screen (GTK_WINDOW (window),
			    gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
     gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 1);
     gtk_widget_set_app_paintable (window, TRUE);
     gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_COMBO);
     /* cb */
     g_signal_connect_after(GTK_OBJECT(window),
			      "button_press_event",
			      GTK_SIGNAL_FUNC(history_popup_clicked_cb),
			      NULL);
     g_signal_connect_after (G_OBJECT (window), "key_press_event",
     		       G_CALLBACK (history_key_press_cb), NULL);

     gdk_window_get_geometry (GTK_WIDGET (mc->applet_box)->window, NULL, NULL,
         &width, &height, NULL);
     gdk_window_get_origin (mc->applet_box->window, &applet_x, &applet_y);
     gdk_window_get_position (mc->entry->window, &entry_x, &entry_y);
     gdk_drawable_get_size (mc->entry->window, NULL, &entry_h);

      win_x=applet_x + entry_x-1;
      win_y=applet_y + entry_y;

      /* size */
      gtk_widget_set_usize(GTK_WIDGET(window), width-2*(entry_x-1), 350);
     /* scrollbars */
     /* create scrolled window to put the Gtk_list widget inside */
     scrolled_window=gtk_scrolled_window_new(NULL, NULL);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
     g_signal_connect(GTK_OBJECT(scrolled_window),
			"button_release_event",
			GTK_SIGNAL_FUNC(history_popup_clicked_inside_cb),
			NULL);
     gtk_container_add(GTK_CONTAINER(window), scrolled_window);
     gtk_container_set_border_width (GTK_CONTAINER(scrolled_window), 1);
     gtk_widget_show(scrolled_window);
          
     store = gtk_list_store_new (1, G_TYPE_STRING);

     /* add history entries to list */
     if (j == 0) {
          gtk_list_store_append (store, &iter);
          gtk_list_store_set (store, &iter,0, _("No items in history"), -1);
     }
     else {	
          for(i = 0; i < MC_HISTORY_LIST_LENGTH; i++)
	      {
     	     if(exists_history_entry(i))
	     	 {
     		      command_list[0] = get_history_entry(i);
                      gtk_list_store_prepend (store, &iter);
                      gtk_list_store_set (store, &iter,0,command_list[0],-1);
		 }
	      }
     } 
     model = GTK_TREE_MODEL(store);
     treeview = gtk_tree_view_new_with_model (model);
     g_object_set_data (G_OBJECT (mc->applet), "tree", treeview);
     cell_renderer = gtk_cell_renderer_text_new ();
     column = gtk_tree_view_column_new_with_attributes (NULL, cell_renderer,
                                                       "text", 0, NULL);
     gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
     gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
     if (j == 0) {
          gtk_tree_selection_set_mode( (GtkTreeSelection *)gtk_tree_view_get_selection
                                (GTK_TREE_VIEW (treeview)),
                                 GTK_SELECTION_NONE);
     }
     else {
          gtk_tree_selection_set_mode( (GtkTreeSelection *)gtk_tree_view_get_selection
                                (GTK_TREE_VIEW (treeview)),
                                 GTK_SELECTION_SINGLE);
          g_signal_connect (G_OBJECT (treeview), "button_press_event",
     		       G_CALLBACK (history_list_button_press_cb), mc);
          g_signal_connect (G_OBJECT (treeview), "key_press_event",
     		       G_CALLBACK (history_list_key_press_cb), mc);
     }
   
     g_object_unref (G_OBJECT (model));
     gtk_container_add(GTK_CONTAINER(scrolled_window),treeview);
     gtk_widget_show (treeview); 
     
     gtk_widget_size_request (window, &req);

   
     switch (panel_applet_get_orient (mc->applet)) {
        case PANEL_APPLET_ORIENT_RIGHT:
        case PANEL_APPLET_ORIENT_LEFT:
     case PANEL_APPLET_ORIENT_DOWN:
            win_y += (entry_h+BORDER-1);
     	break;
        
     case PANEL_APPLET_ORIENT_UP:
            win_y -= (req.height+BORDER-1);
	break;
     }

     gtk_window_move (GTK_WINDOW (window), win_x, win_y);
     g_signal_connect ((gpointer) window, "expose_event",
                    G_CALLBACK (history_window_expose_event),
                    NULL);
     gtk_widget_show(window);

     /* grab focus */
     gdk_pointer_grab (window->window,
		       TRUE,
		       GDK_BUTTON_PRESS_MASK
		       | GDK_BUTTON_RELEASE_MASK
		       | GDK_ENTER_NOTIFY_MASK
		       | GDK_LEAVE_NOTIFY_MASK 
		       | GDK_POINTER_MOTION_MASK,
		       NULL,
		       NULL,
		       GDK_CURRENT_TIME); 
     gdk_keyboard_grab (window->window, TRUE, GDK_CURRENT_TIME);
     gtk_grab_add(window);
     gtk_widget_grab_focus (treeview);
 
     return FALSE;
}

static gint 
file_browser_response_signal(GtkWidget *widget, gint response, gpointer mc_data)
{
    MCData *mc = mc_data;
    gchar *filename;

    if (response == GTK_RESPONSE_OK) {
    
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(mc->file_select));
        
        if (filename != NULL) {
            if (browsed_folder)
                g_free (browsed_folder);
            
            browsed_folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(mc->file_select));
            
            mc_exec_command(mc, filename);
            g_free(filename);
       }
    }

    /* destroy file select dialog */
    gtk_widget_destroy (mc->file_select);
    mc->file_select = NULL;

     /* go on */
    return FALSE;  
}

gboolean
mc_show_file_browser (GtkWidget *event_box, 
                         GdkEventButton *event,
		      MCData    *mc)
{
    if(mc->file_select && GTK_WIDGET_VISIBLE(mc->file_select)) {
        gtk_window_present (GTK_WINDOW (mc->file_select));
        return TRUE;
    }

    /* build file select dialog */
    mc->file_select = gtk_file_chooser_dialog_new((gchar *) _("Start program"),
						  NULL,
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						  GTK_STOCK_EXECUTE, GTK_RESPONSE_OK,
						  NULL);

    g_signal_connect(G_OBJECT(mc->file_select),
		     "response",
		     G_CALLBACK(file_browser_response_signal),
		     mc);

    /* set path to last selected path */
    if (browsed_folder)
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(mc->file_select),
			          (gchar *) browsed_folder);

    /* Set as modal */
    gtk_window_set_modal(GTK_WINDOW(mc->file_select),TRUE);

    gtk_window_set_screen (GTK_WINDOW (mc->file_select), 
			   gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
    gtk_window_set_position (GTK_WINDOW (mc->file_select), GTK_WIN_POS_CENTER);
    
    gtk_widget_show(mc->file_select);

    return FALSE;
}

void
mc_create_command_entry (MCData *mc)
{

    mc->applet_box = icon_entry_new ();
    mc->entry= ICON_ENTRY(mc->applet_box)->entry;
    g_object_set_data (G_OBJECT (mc->applet_box), "mcdata", mc);
    gtk_entry_set_max_length (GTK_ENTRY (mc->entry), MC_MAX_COMMAND_LENGTH);
    
    mc->history_button = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (mc->history_button), FALSE);
    mc->history_image = gtk_image_new_from_stock ("gtk-go-down", 
        GTK_ICON_SIZE_MENU);
    gtk_widget_show (mc->history_image);
    gtk_container_add (GTK_CONTAINER (mc->history_button), mc->history_image);
    icon_entry_pack_widget (ICON_ENTRY(mc->applet_box), mc->history_button, FALSE);
    
    mc->file_select_button = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (mc->file_select_button), FALSE);
    mc->file_select_image = gtk_image_new_from_stock ("gtk-open", 
        GTK_ICON_SIZE_MENU);
    gtk_widget_show (mc->file_select_image);
    gtk_container_add (GTK_CONTAINER (mc->file_select_button), mc->file_select_image);
    icon_entry_pack_widget (ICON_ENTRY(mc->applet_box), 
        mc->file_select_button, FALSE);
    
    g_signal_connect (mc->entry, "key_press_event",
		      G_CALLBACK (command_key_event), mc);
   
    g_signal_connect (mc->entry, "button_press_event",
		      G_CALLBACK (button_press_cb), mc);

    g_signal_connect (mc->file_select_button, "button_press_event",
		      G_CALLBACK (button_press_hack), mc);
          
    g_signal_connect (mc->history_button, "button_press_event",
		      G_CALLBACK (button_press_hack), mc);

    if (!mc->preferences.show_default_theme)
    {
	    gtk_widget_set_name (mc->entry, "minicommander-applet-entry");
	    mc_command_update_entry_color (mc); 
    }
    else
	    gtk_widget_set_name (mc->entry,
			    "minicommander-applet-entry-default");

    mc_command_update_entry_size (mc);

    set_atk_name_description (mc->entry,
			      _("Command line"), 
			      _("Type a command here and Gnome will execute it for you"));
}

void
mc_command_update_entry_color (MCData *mc)
{
    GdkColor fg;
    GdkColor bg;
    char *rc_string;

    fg.red   = mc->preferences.cmd_line_color_fg_r;
    fg.green = mc->preferences.cmd_line_color_fg_g;
    fg.blue  = mc->preferences.cmd_line_color_fg_b;

    /* FIXME: wish we had an API for this, see bug #79585 */
    rc_string = g_strdup_printf (
		    "\n"
		    " style \"minicommander-applet-entry-style\"\n"
		    " {\n"
		    "  GtkWidget::cursor-color=\"#%04x%04x%04x\"\n"
		    " }\n"
		    " widget \"*.minicommander-applet-entry\" "
		    "style \"minicommander-applet-entry-style\"\n"
		    "\n",
		    fg.red, fg.green, fg.blue);
    gtk_rc_parse_string (rc_string);
    g_free (rc_string);

    gtk_widget_modify_text (mc->entry, GTK_STATE_NORMAL, &fg);
    gtk_widget_modify_text (mc->entry, GTK_STATE_PRELIGHT, &fg);
   
    bg.red   = mc->preferences.cmd_line_color_bg_r;
    bg.green = mc->preferences.cmd_line_color_bg_g;
    bg.blue  = mc->preferences.cmd_line_color_bg_b;

    gtk_widget_modify_base (mc->entry, GTK_STATE_NORMAL, &bg);
    
    gtk_widget_queue_draw (mc->applet_box);
   
}

void
mc_command_update_entry_size (MCData *mc)
{
    int size_x = -1;
    
    size_x = mc->preferences.normal_size_x - 17;

   gtk_widget_set_size_request (GTK_WIDGET (mc->applet_box), size_x, -1);

}


/* Thanks to Halfline <halfline@hawaii.rr.com> for his initial version
   of history_auto_complete */
gchar *
history_auto_complete(GtkWidget *widget, GdkEventKey *event)
{
    gchar current_command[MC_MAX_COMMAND_LENGTH];
    gchar* completed_command;
    int i;

    g_snprintf(current_command, sizeof(current_command), "%s%s", 
	       gtk_entry_get_text(GTK_ENTRY(widget)), event->string); 
    for(i = MC_HISTORY_LIST_LENGTH - 1; i >= 0; i--) 
  	{
	    if(!exists_history_entry(i))
		break;
  	    completed_command = get_history_entry(i); 
  	    if(!strncmp(completed_command, current_command, strlen(current_command))) 
		return completed_command; 
  	} 
    
    return NULL;
}
