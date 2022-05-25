/*
 * Original work: Copyright (C) 2013, 2018 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_COMMAND_H
#define ZORIN_MENU_LITE_COMMAND_H

#include <gtk/gtk.h>

namespace ZorinMenuLite
{

class Window;

class Command
{
public:
	Command(const gchar* icon, const gchar* text, const gchar* command, const gchar* error_text, const bool show_label, Window* window, const gchar* confirm_question = NULL, const gchar* confirm_status = NULL);
	~Command();

	GtkWidget* get_button();
	GtkWidget* get_menuitem();

	const gchar* get() const
	{
		return m_command;
	}

	bool get_shown() const
	{
		return m_shown;
	}

	GtkWidget* get_image() const
	{
		return m_image;
	}

	GtkWidget* get_label() const
	{
		return m_label;
	}

	const gchar* get_text() const
	{
		return m_mnemonic;
	}

	void set(const gchar* command);

	void set_shown(bool shown);

	void check();

	void activate();

private:
	bool confirm();
	static gboolean confirm_countdown(gpointer data);

private:
	GtkWidget* m_button;
	GtkBox* m_button_box;
	GtkWidget* m_image;
	GtkWidget* m_label;
	GtkWidget* m_menuitem;
	gchar* m_icon;
	gchar* m_mnemonic;
	gchar* m_text;
	gchar* m_command;
	gchar* m_error_text;
	int m_status;
	bool m_shown;
	bool m_show_label;
	Window* m_window;

	struct TimeoutDetails
	{
		GtkWidget* dialog;
		gchar* question;
		gchar* status;
		gint time_left;
	}
	m_timeout_details;
};

}

#endif // ZORIN_MENU_LITE_COMMAND_H
