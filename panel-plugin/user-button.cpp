/*
 * Original work: Copyright (C) 2014, 2016, 2017 Graeme Gott <graeme@gottcode.org>
 * Modified work: Copyright (C) 2017-2018 Zorin OS Technologies Ltd.
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

#include "user-button.h"
#include "slot.h"
#include "window.h"

#include <libxfce4util/libxfce4util.h>

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

UserButton::UserButton(Window* window) :
	Command("avatar-default-symbolic", _("Edit _Profile"), "mugshot", _("Failed to edit profile."), true, window)
{
	get_button();
	update_username();

	gchar* path = g_build_filename(g_get_home_dir(), ".face", NULL);
	GFile* file = g_file_new_for_path(path);
	g_free(path);

	m_file_monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
	g_signal_connect_slot(m_file_monitor, "changed", &UserButton::update_user_image, this);
	update_user_image(m_file_monitor, file, NULL, G_FILE_MONITOR_EVENT_CHANGED);
	g_object_unref(file);
}

//-----------------------------------------------------------------------------

UserButton::~UserButton()
{
	g_file_monitor_cancel(m_file_monitor);
	g_object_unref(m_file_monitor);
}

//-----------------------------------------------------------------------------

void UserButton::update_username()
{
	// Create the username label
	const gchar* username = g_get_real_name();
	if (g_strcmp0(username, "Unknown") == 0)
	{
		username = g_get_user_name();
	}
	gtk_label_set_text(GTK_LABEL(get_label()), username);
}

//-----------------------------------------------------------------------------

void UserButton::update_user_image(GFileMonitor*, GFile* file, GFile*, GFileMonitorEvent)
{
	if (g_file_query_exists(file, NULL))
	{
		GIcon* icon = g_file_icon_new(file);
		gtk_image_set_from_gicon(GTK_IMAGE(get_image()), icon, GTK_ICON_SIZE_DND);
		g_object_unref(icon);
	}
	else
	{
		gtk_image_set_from_icon_name(GTK_IMAGE(get_image()), "avatar-default-symbolic", GTK_ICON_SIZE_DND);
	}
}

//-----------------------------------------------------------------------------
