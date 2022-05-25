/*
 * Original work: Copyright (C) 2013, 2015, 2017, 2018 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_APPLICATIONS_PAGE_H
#define ZORIN_MENU_LITE_APPLICATIONS_PAGE_H

#include "page.h"

#include <map>
#include <string>
#include <vector>

#include <garcon/garcon.h>

namespace ZorinMenuLite
{

class Category;

class ApplicationsPage : public Page
{

public:
	explicit ApplicationsPage(Window* window);
	~ApplicationsPage();

	Launcher* get_application(const std::string& desktop_id) const;

	void invalidate_applications();
	bool load_applications();
	void apply_filter(Category* category);

private:
	void clear_applications();
	void load_garcon_menu();
	void load_contents();
	void load_menu(GarconMenu* menu, Category* parent_category);
	void load_menu_item(GarconMenuItem* menu_item, Category* category);

	static void load_garcon_menu_slot(GTask* task, gpointer, gpointer task_data, GCancellable*)
	{
		reinterpret_cast<ApplicationsPage*>(task_data)->load_garcon_menu();
		g_task_return_boolean(task, true);
	}

	static void load_contents_slot(GObject*, GAsyncResult*, gpointer user_data)
	{
		reinterpret_cast<ApplicationsPage*>(user_data)->load_contents();
	}

private:
	GarconMenu* m_garcon_menu;
	GarconMenu* m_garcon_settings_menu;
	std::vector<Category*> m_categories;
	Category* m_all_items_category;
	std::map<std::string, Launcher*> m_items;
	int m_load_status;
};

}

#endif // ZORIN_MENU_LITE_APPLICATIONS_PAGE_H
