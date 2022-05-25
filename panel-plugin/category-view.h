/*
 * Original work: Copyright (C) 2013, 2016 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_CATEGORY_VIEW_H
#define ZORIN_MENU_LITE_CATEGORY_VIEW_H

#include <gtk/gtk.h>

namespace ZorinMenuLite
{

class Category;
class Window;

class CategoryView
{
public:
	CategoryView(Window* window);
	~CategoryView();

	GtkWidget* get_widget() const
	{
		return GTK_WIDGET(m_view);
	}

	void scroll_to_path(GtkTreePath* path);
	void set_cursor(GtkTreePath* path);

	void set_fixed_height_mode(bool fixed_height);

	void collapse_all();

	GtkTreeModel* get_model() const
	{
		return m_model;
	}

	void set_model(GtkTreeModel* model);
	void unset_model();

	enum Columns
	{
		COLUMN_ICON = 0,
		COLUMN_TEXT,
		COLUMN_CATEGORY,
		N_COLUMNS
	};

private:
	void create_column();
	gboolean on_key_press_event(GtkWidget*, GdkEvent* event);
	gboolean on_key_release_event(GtkWidget*, GdkEvent* event);

private:
	Window* m_window;

	GtkTreeModel* m_model;
	GtkTreeView* m_view;
	GtkTreeViewColumn* m_column;
	int m_icon_size;
};

}

#endif // ZORIN_MENU_LITE_CATEGORY_VIEW_H
