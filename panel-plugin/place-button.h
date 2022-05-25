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

#ifndef ZORIN_MENU_LITE_PLACE_BUTTON_H
#define ZORIN_MENU_LITE_PLACE_BUTTON_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gio/gfile.h>
#include <gio/gfileinfo.h>
#include <gio/gicon.h>

namespace ZorinMenuLite
{

class Window;

class PlaceButton
{
public:
	PlaceButton(const gchar* path, const gchar* name, Window* window);
	~PlaceButton();

	GtkButton* get_button() const
	{
		return m_button;
	}

private:
	GIcon* get_icon();
	const char* get_name();
	void launch();

private:
	Window* m_window;
	GFile* m_file;
	GtkButton* m_button;
	GtkBox* m_box;
	GtkWidget* m_icon;
	GtkWidget* m_label;
};

}

#endif // ZORIN_MENU_LITE_PLACE_BUTTON_H
