/*
 * Original work: Copyright (C) 2013, 2016, 2018 Graeme Gott <graeme@gottcode.org>
 * Modified work: Copyright (C) 2017, 2019 Zorin OS Technologies Ltd.
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

#include "command.h"
#include "slot.h"
#include "window.h"

#include <string>

#include <libxfce4ui/libxfce4ui.h>

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

enum
{
	ZORIN_MENU_LITE_COMMAND_UNCHECKED = -1,
	ZORIN_MENU_LITE_COMMAND_INVALID,
	ZORIN_MENU_LITE_COMMAND_VALID
};

//-----------------------------------------------------------------------------

Command::Command(const gchar* icon, const gchar* text, const gchar* command, const gchar* error_text, const bool show_label, Window* window, const gchar* confirm_question, const gchar* confirm_status) :
	m_button(NULL),
	m_image(NULL),
	m_menuitem(NULL),
	m_icon(g_strdup(icon)),
	m_mnemonic(g_strdup(text)),
	m_command(g_strdup(command)),
	m_error_text(g_strdup(error_text)),
	m_status(ZORIN_MENU_LITE_COMMAND_UNCHECKED),
	m_shown(true),
	m_show_label(show_label),
	m_window(window),
	m_timeout_details({NULL, g_strdup(confirm_question), g_strdup(confirm_status), 0})
{
	std::string mnemonic(text ? text : "");
	for (std::string::size_type i = 0, length = mnemonic.length(); i < length; ++i)
	{
		if (mnemonic[i] == '_')
		{
			mnemonic.erase(i, 1);
			--length;
			--i;
		}
	}
	m_text = g_strdup(mnemonic.c_str());

	check();
}

//-----------------------------------------------------------------------------

Command::~Command()
{
	if (m_button)
	{
		gtk_widget_destroy(m_button);
		g_object_unref(m_button);
	}
	if (m_image)
	{
		gtk_widget_destroy(m_image);
		g_object_unref(m_image);
	}
	if (m_menuitem)
	{
		gtk_widget_destroy(m_menuitem);
		g_object_unref(m_menuitem);
	}

	g_free(m_icon);
	g_free(m_mnemonic);
	g_free(m_text);
	g_free(m_command);
	g_free(m_error_text);
	g_free(m_timeout_details.question);
	g_free(m_timeout_details.status);
}

//-----------------------------------------------------------------------------

GtkWidget* Command::get_button()
{
	if (m_button)
	{
		return m_button;
	}

	m_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(m_button), GTK_RELIEF_NONE);
	gtk_button_set_always_show_image(GTK_BUTTON(m_button), true);

	m_button_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8));
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(m_button_box));

	m_image = gtk_image_new_from_icon_name(m_icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_box_pack_start(m_button_box, m_image, false, false, 0);

	if (m_show_label)
	{
		m_label = gtk_label_new(m_text);
		gtk_label_set_ellipsize(GTK_LABEL(m_label), PANGO_ELLIPSIZE_END);
		gtk_box_pack_start(m_button_box, m_label, false, true, 0);
	}
	else
	{
		gtk_widget_set_tooltip_text(m_button, m_text);
	}

	g_signal_connect_slot<GtkButton*>(m_button, "clicked", &Command::activate, this, true);

	gtk_widget_set_visible(m_button, m_shown);
	gtk_widget_set_sensitive(m_button, m_status == ZORIN_MENU_LITE_COMMAND_VALID);

	g_object_ref_sink(m_button);

	return m_button;
}

//-----------------------------------------------------------------------------

GtkWidget* Command::get_menuitem()
{
	if (m_menuitem)
	{
		return m_menuitem;
	}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	m_menuitem = gtk_image_menu_item_new_with_mnemonic(m_mnemonic);
	GtkWidget* image = gtk_image_new_from_icon_name(m_icon, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(m_menuitem), image);
	g_signal_connect_slot<GtkMenuItem*>(m_menuitem, "activate", &Command::activate, this);
G_GNUC_END_IGNORE_DEPRECATIONS

	gtk_widget_set_visible(m_menuitem, m_shown);
	gtk_widget_set_sensitive(m_menuitem, m_status == ZORIN_MENU_LITE_COMMAND_VALID);

	g_object_ref_sink(m_menuitem);

	return m_menuitem;
}

//-----------------------------------------------------------------------------

void Command::set(const gchar* command)
{
	if (command == m_command)
	{
		return;
	}

	g_free(m_command);
	m_command = g_strdup(command);
	m_status = ZORIN_MENU_LITE_COMMAND_UNCHECKED;
}

//-----------------------------------------------------------------------------

void Command::set_shown(bool shown)
{
	if (shown == m_shown)
	{
		return;
	}

	m_shown = shown;

	if (m_button)
	{
		gtk_widget_set_visible(m_button, m_shown);
	}
	if (m_menuitem)
	{
		gtk_widget_set_visible(m_menuitem, m_shown);
	}
}

//-----------------------------------------------------------------------------

void Command::check()
{
	if (m_status == ZORIN_MENU_LITE_COMMAND_UNCHECKED)
	{
		gchar** argv;
		if (g_shell_parse_argv(m_command, NULL, &argv, NULL))
		{
			gchar* path = g_find_program_in_path(argv[0]);
			m_status = path ? ZORIN_MENU_LITE_COMMAND_VALID : ZORIN_MENU_LITE_COMMAND_INVALID;
			g_free(path);
			g_strfreev(argv);
		}
		else
		{
			m_status = ZORIN_MENU_LITE_COMMAND_INVALID;
		}
	}

	if (m_button)
	{
		gtk_widget_set_visible(m_button, m_shown);
		gtk_widget_set_sensitive(m_button, m_status == ZORIN_MENU_LITE_COMMAND_VALID);
	}
	if (m_menuitem)
	{
		gtk_widget_set_visible(m_menuitem, m_shown);
		gtk_widget_set_sensitive(m_menuitem, m_status == ZORIN_MENU_LITE_COMMAND_VALID);
	}
}

//-----------------------------------------------------------------------------

void Command::activate()
{
	if (m_timeout_details.question
			&& m_timeout_details.status
			&& !confirm())
	{
		return;
	}

	GError* error = NULL;
	if (g_spawn_command_line_async(m_command, &error) == false)
	{
		xfce_dialog_show_error(NULL, error, m_error_text, NULL);
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------

// Adapted from https://git.xfce.org/xfce/xfce4-panel/tree/plugins/actions/actions.c
bool Command::confirm()
{
	// Create dialog
	m_timeout_details.dialog = gtk_message_dialog_new(NULL, GtkDialogFlags(0),
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_CANCEL,
			"%s", m_timeout_details.question);
	GtkDialog* dialog = GTK_DIALOG(m_timeout_details.dialog);

	GtkWindow* window = GTK_WINDOW(m_timeout_details.dialog);
	gtk_window_set_keep_above(window, true);
	gtk_window_stick(window);
	gtk_window_set_skip_taskbar_hint(window, true);
	gtk_window_set_title(window, m_text);

	// Add icon
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	GtkWidget* image = gtk_image_new_from_icon_name(m_icon, GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);
	gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), image);
G_GNUC_END_IGNORE_DEPRECATIONS

	// Create accept button
	gtk_dialog_add_button(dialog, m_mnemonic, GTK_RESPONSE_ACCEPT);
	gtk_dialog_set_default_response(dialog, GTK_RESPONSE_ACCEPT);

	// Run dialog
	m_timeout_details.time_left = 60;
	guint timeout_id = g_timeout_add(1000, &Command::confirm_countdown, &m_timeout_details);
	confirm_countdown(&m_timeout_details);

	gint result = gtk_dialog_run(dialog);

	g_source_remove(timeout_id);
	gtk_widget_destroy(m_timeout_details.dialog);
	m_timeout_details.dialog = NULL;

	return result == GTK_RESPONSE_ACCEPT;
}

//-----------------------------------------------------------------------------

gboolean Command::confirm_countdown(gpointer data)
{
	TimeoutDetails* details = static_cast<TimeoutDetails*>(data);

	if (details->time_left == 0)
	{
		gtk_dialog_response(GTK_DIALOG(details->dialog), GTK_RESPONSE_ACCEPT);
	}
	else
	{
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(details->dialog),
				details->status, details->time_left);
	}

	return --details->time_left >= 0;
}

//-----------------------------------------------------------------------------
