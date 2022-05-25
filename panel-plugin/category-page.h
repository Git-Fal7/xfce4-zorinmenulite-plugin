/*
 * Original work: Copyright (C) 2013, 2017 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_CATEGORY_PAGE_H
#define ZORIN_MENU_LITE_CATEGORY_PAGE_H

#include <gtk/gtk.h>
#include <vector>

namespace ZorinMenuLite
{

class CategoryView;
class Window;
class Category;

class CategoryPage
{
public:
	explicit CategoryPage(ZorinMenuLite::Window *window);
	virtual ~CategoryPage();

	GtkWidget* get_widget() const
	{
		return m_widget;
	}

	CategoryView* get_view() const
	{
		return m_view;
	}

	void set_categories(const std::vector<Category*>& categories);
	void reset_selection();

protected:
	Window* get_window() const
	{
		return m_window;
	}

private:
	void item_activated(GtkTreeView* view, GtkTreePath* path, GtkTreeViewColumn*);

private:
	Window* m_window;
	GtkWidget* m_widget;
	CategoryView* m_view;
	GtkTreePath* m_selected_path;
};

}

#endif // ZORIN_MENU_LITE_CATEGORY_PAGE_H
