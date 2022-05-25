/*
 * Original work: Copyright (C) 2013, 2015, 2016, 2017, 2018 Graeme Gott <graeme@gottcode.org>
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

#include "applications-page.h"

#include "category.h"
#include "launcher.h"
#include "launcher-view.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

extern "C"
{
#include <libxfce4util/libxfce4util.h>
}

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

enum
{
	STATUS_INVALID,
	STATUS_LOADING,
	STATUS_LOADING_RELOAD,
	STATUS_LOADED
};

//-----------------------------------------------------------------------------

ApplicationsPage::ApplicationsPage(Window* window) :
	Page(window),
	m_garcon_menu(NULL),
	m_garcon_settings_menu(NULL),
	m_all_items_category(NULL),
	m_load_status(STATUS_INVALID)
{
	// Set desktop environment for applications
	const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
	if (G_LIKELY(!desktop))
	{
		desktop = "XFCE";
	}
	else if (*desktop == '\0')
	{
		desktop = NULL;
	}
	garcon_set_environment(desktop);
}

//-----------------------------------------------------------------------------

ApplicationsPage::~ApplicationsPage()
{
	clear_applications();
}

//-----------------------------------------------------------------------------

Launcher* ApplicationsPage::get_application(const std::string& desktop_id) const
{
	std::map<std::string, Launcher*>::const_iterator i = m_items.find(desktop_id);
	return (i != m_items.end()) ? i->second : NULL;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::apply_filter(Category* category)
{
	if (!category)
	{
		return;
	}

	// Apply filter
	get_view()->unset_model();
	get_view()->set_fixed_height_mode(!category->has_separators());
	get_view()->set_model(category->get_model());
}

//-----------------------------------------------------------------------------

void ApplicationsPage::invalidate_applications()
{
	if (m_load_status == STATUS_LOADED)
	{
		m_load_status = STATUS_INVALID;
	}
	else if (m_load_status == STATUS_LOADING)
	{
		m_load_status = STATUS_LOADING_RELOAD;
	}
}

//-----------------------------------------------------------------------------

bool ApplicationsPage::load_applications()
{
	// Check if already loaded
	if (m_load_status == STATUS_LOADED)
	{
		return true;
	}
	// Check if currently loading
	else if ((m_load_status == STATUS_LOADING) || (m_load_status == STATUS_LOADING_RELOAD))
	{
		return false;
	}
	m_load_status = STATUS_LOADING;

	// Load menu
	clear_applications();

	// Load contents in thread if possible
	GTask* task = g_task_new(NULL, NULL, &ApplicationsPage::load_contents_slot, this);
	g_task_set_task_data(task, this, NULL);
	g_task_run_in_thread(task, &ApplicationsPage::load_garcon_menu_slot);
	g_object_unref(task);

	return false;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::clear_applications()
{
	// Free categories
	for (std::vector<Category*>::iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		delete *i;
	}
	m_categories.clear();

	if (m_all_items_category)
	{
		delete m_all_items_category;
		m_all_items_category = NULL;
	}

	// Free menu items
	get_window()->unset_items();
	get_view()->unset_model();

	for (std::map<std::string, Launcher*>::iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		delete i->second;
	}
	m_items.clear();

	// Free menu
	if (G_LIKELY(m_garcon_menu))
	{
		g_object_unref(m_garcon_menu);
		m_garcon_menu = NULL;
	}

	// Free settings menu
	if (G_LIKELY(m_garcon_settings_menu))
	{
		g_object_unref(m_garcon_settings_menu);
		m_garcon_settings_menu = NULL;
	}
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_garcon_menu()
{
	// Create menu
	m_garcon_menu = garcon_menu_new_applications();

	// Load menu
	if (m_garcon_menu && !garcon_menu_load(m_garcon_menu, NULL, NULL))
	{
		g_object_unref(m_garcon_menu);
		m_garcon_menu = NULL;
	}

	if (!m_garcon_menu)
	{
		return;
	}

	g_signal_connect_slot<GarconMenu*>(m_garcon_menu, "reload-required", &ApplicationsPage::invalidate_applications, this);
	load_menu(m_garcon_menu, NULL);

	// Create settings menu
	gchar* path = xfce_resource_lookup(XFCE_RESOURCE_CONFIG, "menus/xfce-settings-manager.menu");
	m_garcon_settings_menu = garcon_menu_new_for_path(path != NULL ? path : SETTINGS_MENUFILE);
	g_free(path);

	if (m_garcon_settings_menu)
	{
		g_signal_connect_slot<GarconMenu*>(m_garcon_settings_menu, "reload-required", &ApplicationsPage::invalidate_applications, this);
	}

	// Load settings menu
	if (m_garcon_settings_menu && garcon_menu_load(m_garcon_settings_menu, NULL, NULL))
	{
		load_menu(m_garcon_settings_menu, NULL);
	}

	// Sort items and categories
	for (std::vector<Category*>::const_iterator i = m_categories.begin(), end = m_categories.end(); i != end; ++i)
	{
		(*i)->sort();
	}
	std::sort(m_categories.begin(), m_categories.end(), &Element::less_than);

	// Create all items category
	Category* category = new Category(NULL);
	for (std::map<std::string, Launcher*>::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i)
	{
		category->append_item(i->second);
	}
	category->sort();
	m_all_items_category = category;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_contents()
{
	if (!m_garcon_menu)
	{
		get_window()->set_loaded();

		m_load_status = STATUS_INVALID;

		return;
	}

	// Set all applications category
	get_view()->set_fixed_height_mode(true);
	get_view()->set_model(m_all_items_category->get_model());

	// Add category buttons to window
	get_window()->set_categories(m_categories);

	// Update menu items of other panels
	get_window()->set_items();
	get_window()->set_loaded();

	m_load_status = (m_load_status == STATUS_LOADING) ? STATUS_LOADED : STATUS_INVALID;
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_menu(GarconMenu* menu, Category* parent_category)
{
	GarconMenuDirectory* directory = garcon_menu_get_directory(menu);

	// Skip hidden categories
	if (directory && !garcon_menu_directory_get_visible(directory))
	{
		return;
	}

	// Track categories
	bool first_level = directory && (garcon_menu_get_parent(menu) == m_garcon_menu);
	Category* category = NULL;
	if (directory)
	{
		if (first_level)
		{
			category = new Category(directory);
			m_categories.push_back(category);
		}
		else
		{
			category = parent_category;
		}
	}

	// Add menu elements
	GList* elements = garcon_menu_get_elements(menu);
	for (GList* li = elements; li != NULL; li = li->next)
	{
		if (GARCON_IS_MENU_ITEM(li->data))
		{
			load_menu_item(GARCON_MENU_ITEM(li->data), category);
		}
		else if (GARCON_IS_MENU(li->data))
		{
			load_menu(GARCON_MENU(li->data), category);
		}
	}
	g_list_free(elements);

	// Free unused top-level categories
	if (first_level && category->empty())
	{
		m_categories.erase(std::find(m_categories.begin(), m_categories.end(), category));
		delete category;
		category = NULL;
	}

	// Listen for menu changes
	g_signal_connect_slot<GarconMenu*,GarconMenuDirectory*,GarconMenuDirectory*>(menu, "directory-changed", &ApplicationsPage::invalidate_applications, this);
}

//-----------------------------------------------------------------------------

void ApplicationsPage::load_menu_item(GarconMenuItem* menu_item, Category* category)
{
	// Skip hidden items
	if (!garcon_menu_element_get_visible(GARCON_MENU_ELEMENT(menu_item)))
	{
		return;
	}

	// Add to map
	std::string desktop_id(garcon_menu_item_get_desktop_id(menu_item));
	std::map<std::string, Launcher*>::iterator iter = m_items.find(desktop_id);
	if (iter == m_items.end())
	{
		iter = m_items.insert(std::make_pair(desktop_id, new Launcher(menu_item))).first;
	}

	// Add menu item to current category
	if (category)
	{
		category->append_item(iter->second);
	}

	// Listen for menu changes
	g_signal_connect_slot<GarconMenuItem*>(menu_item, "changed", &ApplicationsPage::invalidate_applications, this);
}

//-----------------------------------------------------------------------------
