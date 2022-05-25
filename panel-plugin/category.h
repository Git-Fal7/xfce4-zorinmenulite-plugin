/*
 * Original work: Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_CATEGORY_H
#define ZORIN_MENU_LITE_CATEGORY_H

#include "launcher.h"

#include <vector>

#include <garcon/garcon.h>
#include <gtk/gtk.h>

namespace ZorinMenuLite
{

class Category : public Element
{
public:
	explicit Category(GarconMenuDirectory* directory);
	~Category();

	enum
	{
		Type = 1
	};
	int get_type() const
	{
		return Type;
	}

	GtkTreeModel* get_model();

	bool empty() const;

	bool has_separators() const
	{
		return m_has_separators;
	}

	void append_item(Launcher* launcher)
	{
		unset_model();
		m_items.push_back(launcher);
	}

	Category* append_menu(GarconMenuDirectory* directory);

	void append_separator();

	void sort();

private:
	void insert_items(GtkTreeStore* model, GtkTreeIter* parent, const gchar* fallback_icon);
	void insert_items(GtkListStore* model);
	void merge();
	void unset_model();

private:
	std::vector<Element*> m_items;
	GtkTreeModel* m_model;
	bool m_has_separators;
	bool m_has_subcategories;
};

}

#endif // ZORIN_MENU_LITE_CATEGORY_H
