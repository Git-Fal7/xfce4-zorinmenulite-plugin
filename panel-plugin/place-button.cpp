/*
 * Copyright (C) 2017 Zorin OS Technologies Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "place-button.h"
#include "slot.h"
#include "window.h"

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

PlaceButton::PlaceButton(const gchar* path, const gchar* name, Window* window) :
	m_window(window)
{
	m_file = g_file_new_for_path(path);

	m_button = GTK_BUTTON(gtk_button_new());
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	m_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8));
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(m_box));

	m_icon = gtk_image_new_from_gicon(get_icon(), GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_box_pack_start(m_box, m_icon, false, false, 0);

	if (name == NULL)
	{
		m_label = gtk_label_new(get_name());
	}
	else
	{
		m_label = gtk_label_new(name);
	}
	gtk_label_set_ellipsize(GTK_LABEL(m_label), PANGO_ELLIPSIZE_END);

	gtk_box_pack_start(m_box, m_label, false, true, 0);

	gtk_box_set_child_packing(m_box, m_icon, false, false, 0, GTK_PACK_START);
	gtk_widget_show(m_label);

	g_signal_connect_slot<GtkButton*>(m_button, "clicked", &PlaceButton::launch, this);
}

//-----------------------------------------------------------------------------

PlaceButton::~PlaceButton()
{
	gtk_widget_destroy(GTK_WIDGET(m_button));
}

//-----------------------------------------------------------------------------

GIcon* PlaceButton::get_icon()
{
	GError* error = NULL;
	GFileInfo* fileinfo = g_file_query_info(m_file,
			"standard::symbolic-icon",
			G_FILE_QUERY_INFO_NONE,
			NULL,
			&error);
	if (error != NULL)
	{
		g_error_free (error);
		return g_themed_icon_new("folder-symbolic");
	}
	else
	{
		return g_file_info_get_symbolic_icon (fileinfo);
	}
}

//-----------------------------------------------------------------------------

const char* PlaceButton::get_name()
{
	GError* error = NULL;
	GFileInfo* fileinfo = g_file_query_info (m_file,
			"standard::display-name",
			G_FILE_QUERY_INFO_NONE,
			NULL,
			&error);

	if (error != NULL)
	{
		g_error_free (error);
		return g_file_get_basename(m_file);
	}
	else
	{
		return g_file_info_get_display_name(fileinfo);
	}
}

//-----------------------------------------------------------------------------

void PlaceButton::launch()
{
	GdkDisplay* display = gdk_display_get_default();
	GdkAppLaunchContext* launch_context = gdk_display_get_app_launch_context (display);
	m_window->hide();
	g_app_info_launch_default_for_uri (g_file_get_uri(m_file), G_APP_LAUNCH_CONTEXT (launch_context), NULL);
}

//-----------------------------------------------------------------------------
