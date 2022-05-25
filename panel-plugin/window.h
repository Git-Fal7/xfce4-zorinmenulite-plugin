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

#ifndef ZORIN_MENU_LITE_WINDOW_H
#define ZORIN_MENU_LITE_WINDOW_H

#include <vector>

#include <gtk/gtk.h>
#include <glib.h>

namespace ZorinMenuLite
{

class ApplicationsPage;
class CategoryPage;
class Page;
class Plugin;
class PlaceButton;
class UserButton;
class SearchPage;
class Category;
class Command;

class Window
{
public:
	explicit Window(Plugin* plugin);
	~Window();

	enum Position
	{
		PositionHorizontal = GTK_ORIENTATION_HORIZONTAL,
		PositionVertical = GTK_ORIENTATION_VERTICAL,
		PositionAtCursor
	};

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_window);
	}

	GtkEntry* get_search_entry() const
	{
		return m_search_entry;
	}

	ApplicationsPage* get_applications() const
	{
		return m_applications;
	}

	void hide();
	void show(const Position position);
	void on_context_menu_destroyed();
	void set_categories(const std::vector<Category*>& categories);
	void set_items();
	void set_loaded();
	void unset_items();
	void category_activated(Category* category);
	void back_button_activated();

private:
	gboolean on_enter_notify_event(GtkWidget*, GdkEvent* event);
	gboolean on_leave_notify_event(GtkWidget*, GdkEvent* event);
	gboolean on_button_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_key_press_event(GtkWidget* widget, GdkEvent* event);
	gboolean on_key_press_event_after(GtkWidget* widget, GdkEvent* event);
	gboolean on_map_event(GtkWidget*, GdkEvent*);
	gboolean on_state_flags_changed_event(GtkWidget*widget, GtkStateFlags);
	void on_screen_changed_event(GtkWidget* widget, GdkScreen* old_screen);
	gboolean on_draw_event(GtkWidget* widget, cairo_t* cr);
	void show_default_page();
	void search();

private:
	Plugin* m_plugin;

	GtkWindow* m_window;

	GtkStack* m_window_stack;
	GtkSpinner* m_window_load_spinner;

	GtkGrid* m_contents_grid;
	GtkBox* m_vbox;
	GtkBox* m_commands_box;
	GtkBox* m_places_box;
	GtkBox* m_quicklinks_box;
	GtkBox* m_panels_box;

	UserButton* m_user_button;
	GtkWidget* m_user_button_widget;
	gulong m_user_button_slot;

	GtkWidget* m_commands_spacer;
	GtkWidget* m_commands_button[2];
	gulong m_command_slots[2];
	Command* m_commands[2];

	GtkWidget* m_places_button[6];
	PlaceButton* m_places[6];

	GtkWidget* m_quicklinks_button[3];
	gulong m_quicklink_slots[3];
	Command* m_quicklinks[3];

	GtkEntry* m_search_entry;

	SearchPage* m_search_results;
	ApplicationsPage* m_applications;
	CategoryPage* m_categories;
	Page* m_default_page;

	GtkBox* m_sidebar;

	GdkRectangle m_geometry;
};



}

#endif // ZORIN_MENU_LITE_WINDOW_H
