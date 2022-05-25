/*
 * Original work: Copyright (C) 2013, 2014, 2015, 2016, 2018 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_PLUGIN_H
#define ZORIN_MENU_LITE_PLUGIN_H

#define PLUGIN_WEBSITE "https://zorinos.com"

#include <string>

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

namespace ZorinMenuLite
{

class Window;
class Command;

class Plugin
{
public:
	explicit Plugin(XfcePanelPlugin* plugin);
	~Plugin();

	GtkWidget* get_button() const
	{
		return m_button;
	}

	void reload();
	void set_loaded(bool loaded);

private:
	void button_toggled(GtkToggleButton* button);
	void menu_hidden();
	void configure();
	void mode_changed(XfcePanelPlugin*, XfcePanelPluginMode);
	gboolean remote_event(XfcePanelPlugin*, gchar* name, GValue* value);
	void show_about();
	gboolean size_changed(XfcePanelPlugin*, gint size);
	void update_size();
	void show_menu(bool at_cursor);

private:
	XfcePanelPlugin* m_plugin;
	Window* m_window;
	Command* m_menu_editor;

	GtkWidget* m_button;
	GtkBox* m_button_box;
	GtkLabel* m_button_label;
	GtkImage* m_button_icon;
};

}

#endif // ZORIN_MENU_LITE_PLUGIN_H
