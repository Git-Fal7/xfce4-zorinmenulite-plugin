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

#include "category-view.h"

#include "category.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

#include <exo/exo.h>
#include <gdk/gdkkeysyms.h>

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

static gboolean is_separator(GtkTreeModel* model, GtkTreeIter* iter, gpointer)
{
	const gchar* text;
	gtk_tree_model_get(model, iter, CategoryView::COLUMN_TEXT, &text, -1);
	return exo_str_is_empty(text);
}

//-----------------------------------------------------------------------------

CategoryView::CategoryView(Window* window) :
	m_window(window),
	m_model(NULL),
	m_icon_size(0)
{
	// Create the view
	m_view = GTK_TREE_VIEW(exo_tree_view_new());
	gtk_tree_view_set_headers_visible(m_view, false);
	gtk_tree_view_set_enable_tree_lines(m_view, false);
	gtk_tree_view_set_hover_selection(m_view, true);
	gtk_tree_view_set_enable_search(m_view, false);
	gtk_tree_view_set_fixed_height_mode(m_view, true);
	gtk_tree_view_set_row_separator_func(m_view, &is_separator, NULL, NULL);
	create_column();
	g_signal_connect_slot(m_view, "key-press-event", &CategoryView::on_key_press_event, this);
	g_signal_connect_slot(m_view, "key-release-event", &CategoryView::on_key_release_event, this);

	// Use single clicks to activate items
	exo_tree_view_set_single_click(EXO_TREE_VIEW(m_view), true);

	// Only allow up to one selected item
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	g_object_ref_sink(m_view);
}

//-----------------------------------------------------------------------------

CategoryView::~CategoryView()
{
	m_model = NULL;

	g_object_unref(m_view);
}

//-----------------------------------------------------------------------------

void CategoryView::scroll_to_path(GtkTreePath* path)
{
	gtk_tree_view_scroll_to_cell(m_view, path, NULL, true, 0.5f, 0.5f);
}

//-----------------------------------------------------------------------------

void CategoryView::set_cursor(GtkTreePath* path)
{
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_view);
	GtkSelectionMode mode = gtk_tree_selection_get_mode(selection);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);
	gtk_tree_view_set_cursor(m_view, path, NULL, false);
	gtk_tree_selection_set_mode(selection, mode);
}

//-----------------------------------------------------------------------------

void CategoryView::set_fixed_height_mode(bool fixed_height)
{
	gtk_tree_view_set_fixed_height_mode(m_view, fixed_height);
}

//-----------------------------------------------------------------------------

void CategoryView::collapse_all()
{
	gtk_tree_view_collapse_all(m_view);
}

//-----------------------------------------------------------------------------

void CategoryView::set_model(GtkTreeModel* model)
{
	m_model = model;
	gtk_tree_view_set_model(m_view, model);
}

//-----------------------------------------------------------------------------

void CategoryView::unset_model()
{
	m_model = NULL;
	gtk_tree_view_set_model(m_view, NULL);
}

//-----------------------------------------------------------------------------

void CategoryView::create_column()
{
	m_icon_size = 32;

	m_column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_expand(m_column, true);
	gtk_tree_view_column_set_visible(m_column, true);

	if (m_icon_size > 1)
	{
		GtkCellRenderer* icon_renderer = exo_cell_renderer_icon_new();
		g_object_set(icon_renderer, "follow-state", false, NULL);
		g_object_set(icon_renderer, "size", m_icon_size, NULL);
		gtk_tree_view_column_pack_start(m_column, icon_renderer, false);
		gtk_tree_view_column_add_attribute(m_column, icon_renderer, "icon", CategoryView::COLUMN_ICON);
	}

	GtkCellRenderer* text_renderer = gtk_cell_renderer_text_new();
	g_object_set(text_renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_pack_start(m_column, text_renderer, true);
	gtk_tree_view_column_add_attribute(m_column, text_renderer, "markup", CategoryView::COLUMN_TEXT);

	gtk_tree_view_column_set_sizing(m_column, GTK_TREE_VIEW_COLUMN_FIXED);

	gtk_tree_view_append_column(m_view, m_column);
}

//-----------------------------------------------------------------------------

gboolean CategoryView::on_key_press_event(GtkWidget*, GdkEvent* event)
{
	GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);
	if ((key_event->keyval == GDK_KEY_Up) || (key_event->keyval == GDK_KEY_Down))
	{
		gtk_tree_view_set_hover_selection(m_view, false);
	}
	return false;
}

//-----------------------------------------------------------------------------

gboolean CategoryView::on_key_release_event(GtkWidget*, GdkEvent* event)
{
	GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);
	if ((key_event->keyval == GDK_KEY_Up) || (key_event->keyval == GDK_KEY_Down))
	{
		gtk_tree_view_set_hover_selection(m_view, true);
	}
	return false;
}

//-----------------------------------------------------------------------------
