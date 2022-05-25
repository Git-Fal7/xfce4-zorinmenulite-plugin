/*
 * Original work: Copyright (C) 2013, 2017, 2018 Graeme Gott <graeme@gottcode.org>
 * Modified work: Copyright (C) 2017 Zorin OS Technologies Ltd.
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

#ifndef ZORIN_MENU_LITE_PAGE_H
#define ZORIN_MENU_LITE_PAGE_H

#include <gtk/gtk.h>

namespace ZorinMenuLite
{

class DesktopAction;
class Launcher;
class LauncherView;
class Window;

class Page
{
public:
	explicit Page(ZorinMenuLite::Window *window);
	virtual ~Page();

	GtkWidget* get_widget() const
	{
		return m_widget;
	}

	LauncherView* get_view() const
	{
		return m_view;
	}

	void reset_selection();

protected:
	Window* get_window() const
	{
		return m_window;
	}

private:
	virtual bool remember_launcher(Launcher* launcher);
	void item_activated(GtkTreeView* view, GtkTreePath* path, GtkTreeViewColumn*);
	void item_action_activated(GtkMenuItem* menuitem, DesktopAction* action);
	void back_button_activated();
	gboolean view_button_press_event(GtkWidget* view, GdkEvent* event);
	gboolean view_popup_menu_event(GtkWidget* view);
	void destroy_context_menu(GtkMenuShell* menu);
	void add_selected_to_desktop();
	void add_selected_to_panel();
	void edit_selected();
	Launcher* get_selected_launcher() const;
	void create_context_menu(GtkTreeIter* iter, GdkEvent* event);
	virtual void extend_context_menu(GtkWidget* menu);

private:
	Window* m_window;
	GtkWidget* m_widget;
	GtkWidget* m_scroll;
	GtkButton* m_back_button;
	GtkBox* m_button_box;
	GtkWidget* m_back_icon;
	GtkWidget* m_back_label;
	LauncherView* m_view;
	GtkTreePath* m_selected_path;
};

}

#endif // ZORIN_MENU_LITE_PAGE_H
