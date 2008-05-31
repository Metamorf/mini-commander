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

#ifndef ICON_ENTRY_H
#define ICON_ENTRY_H

#include <gtk/gtkbin.h>

G_BEGIN_DECLS

#define TYPE_ICON_ENTRY		(icon_entry_get_type())
#define ICON_ENTRY(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_ICON_ENTRY, IconEntry))
#define ICON_ENTRY_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ICON_ENTRY, IconEntryClass))
#define IS_ICON_ENTRY(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_ICON_ENTRY))
#define IS_ICON_ENTRY_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ICON_ENTRY))
#define ICON_ENTRY_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ICON_ENTRY, IconEntryClass))

typedef struct _IconEntryClass	IconEntryClass;
typedef struct _IconEntry		IconEntry;
typedef struct _IconEntryPrivate	IconEntryPrivate;

struct _IconEntryClass
{
	GtkBinClass parent_class;
};

struct _IconEntry
{
	GtkBin parent_object;

	/*< public >*/
	GtkWidget *entry;

	/*< private >*/
	IconEntryPrivate *priv;
};

GType		icon_entry_get_type	(void);

GtkWidget      *icon_entry_new		(void);

void		icon_entry_pack_widget	(IconEntry *entry,
						 GtkWidget *widget,
						 gboolean start);

GtkWidget      *icon_entry_get_entry	(IconEntry *entry);

void
icon_entry_paint (GtkWidget *widget, GdkEventExpose *event);


G_END_DECLS

#endif
