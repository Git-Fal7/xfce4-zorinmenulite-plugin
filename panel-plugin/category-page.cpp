/*
 * Original work: Copyright (C) 2013, 2014, 2015, 2016, 2017 Graeme Gott <graeme@gottcode.org>
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

#include "category-page.h"

#include "category-view.h"
#include "category.h"
#include "slot.h"
#include "window.h"

#include <libxfce4ui/libxfce4ui.h>

#include <glib/gstdio.h>

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

CategoryPage::CategoryPage(Window* window) :
	m_window(window),
	m_selected_path(NULL)
{
	// Create view
	m_view = new CategoryView(window);
	g_signal_connect_slot(m_view->get_widget(), "row-activated", &CategoryPage::item_activated, this);
	g_signal_connect_swapped(m_view->get_widget(), "start-interactive-search", G_CALLBACK(gtk_widget_grab_focus), m_window->get_search_entry());

	// Add scrolling to view
	m_widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_widget), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_widget), m_view->get_widget());
	g_object_ref_sink(m_widget);
}

//-----------------------------------------------------------------------------

CategoryPage::~CategoryPage()
{
	if (m_selected_path)
	{
		gtk_tree_path_free(m_selected_path);
	}

	delete m_view;
	g_object_unref(m_widget);
}

//-----------------------------------------------------------------------------

void CategoryPage::set_categories(const std::vector<Category*>& categories)
{
	m_view->unset_model();
	GtkListStore* model = gtk_list_store_new(
			CategoryView::N_COLUMNS,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_POINTER);
	for (std::vector<Category*>::const_iterator i = categories.begin(), end = categories.end(); i != end; ++i)
	{
		gtk_list_store_insert_with_values(model,
				NULL, INT_MAX,
				CategoryView::COLUMN_ICON, (*i)->get_icon(),
				CategoryView::COLUMN_TEXT, (*i)->get_text(),
				CategoryView::COLUMN_CATEGORY, (*i),
				-1);
	}
	m_view->set_model(GTK_TREE_MODEL(model));
}
//-----------------------------------------------------------------------------

void CategoryPage::reset_selection()
{
	m_view->collapse_all();

	// Clear selection and scroll to top
	GtkTreeModel* model = m_view->get_model();
	GtkTreeIter iter;
	if (model && gtk_tree_model_get_iter_first(model, &iter))
	{
		GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
		get_view()->scroll_to_path(path);
		get_view()->set_cursor(path);
		gtk_tree_path_free(path);
	}
}

//-----------------------------------------------------------------------------

void CategoryPage::item_activated(GtkTreeView* view, GtkTreePath* path, GtkTreeViewColumn*)
{
	GtkTreeIter iter;
	GtkTreeModel* model = gtk_tree_view_get_model(view);
	gtk_tree_model_get_iter(model, &iter, path);

	// Find element
	Category* category = NULL;
	gtk_tree_model_get(model, &iter, CategoryView::COLUMN_CATEGORY, &category, -1);
	if (!category)
	{
		return;
	}

	// Open Category
	m_window->category_activated(category);
}

//-----------------------------------------------------------------------------
