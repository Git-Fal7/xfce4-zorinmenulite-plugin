/*
 * Original work: Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018 Graeme Gott <graeme@gottcode.org>
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

#include "window.h"

#include "applications-page.h"
#include "category-page.h"
#include "category.h"
#include "category-view.h"
#include "command.h"
#include "launcher-view.h"
#include "plugin.h"
#include "place-button.h"
#include "user-button.h"
#include "search-page.h"
#include "slot.h"

#include <exo/exo.h>
#include <gdk/gdkkeysyms.h>
#include <libxfce4ui/libxfce4ui.h>

#include <ctime>

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

static void grab_pointer(GtkWidget* widget)
{
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* seat = gdk_display_get_default_seat(display);
	GdkWindow* window = gtk_widget_get_window(widget);
	gdk_seat_grab(seat, window, GDK_SEAT_CAPABILITY_ALL_POINTING, true, NULL, NULL, NULL, NULL);
}

static void ungrab_pointer()
{
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* seat = gdk_display_get_default_seat(display);
	gdk_seat_ungrab(seat);
}

//-----------------------------------------------------------------------------

ZorinMenuLite::Window::Window(Plugin* plugin) :
	m_plugin(plugin),
	m_window(NULL)
{
	m_geometry.x = 0;
	m_geometry.y = 0;
	m_geometry.width = 400;
	m_geometry.height = 447;

	// Create the window
	m_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_widget_set_name(GTK_WIDGET(m_window), "zorinmenulite-window");
	// Untranslated window title to allow window managers to identify it; not visible to users.
	gtk_window_set_title(m_window, "Zorin Menu Lite");
	gtk_window_set_modal(m_window, true);
	gtk_window_set_decorated(m_window, false);
	gtk_window_set_skip_taskbar_hint(m_window, true);
	gtk_window_set_skip_pager_hint(m_window, true);
	gtk_window_set_type_hint(m_window, GDK_WINDOW_TYPE_HINT_POPUP_MENU);
	gtk_window_stick(m_window);
	gtk_widget_add_events(GTK_WIDGET(m_window), GDK_BUTTON_PRESS_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_STRUCTURE_MASK);
	g_signal_connect_slot(m_window, "enter-notify-event", &Window::on_enter_notify_event, this);
	g_signal_connect_slot(m_window, "leave-notify-event", &Window::on_leave_notify_event, this);
	g_signal_connect_slot(m_window, "button-press-event", &Window::on_button_press_event, this);
	g_signal_connect_slot(m_window, "key-press-event", &Window::on_key_press_event, this);
	g_signal_connect_slot(m_window, "key-press-event", &Window::on_key_press_event_after, this, true);
	g_signal_connect_slot(m_window, "map-event", &Window::on_map_event, this);
	g_signal_connect_slot(m_window, "state-flags-changed", &Window::on_state_flags_changed_event, this);
	g_signal_connect(m_window, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	// Create the border of the window
	GtkWidget* frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_add(GTK_CONTAINER(m_window), frame);

	// Create window contents stack
	m_window_stack = GTK_STACK(gtk_stack_new());
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(m_window_stack));

	// Create loading message
	m_window_load_spinner = GTK_SPINNER(gtk_spinner_new());
	gtk_widget_set_halign(GTK_WIDGET(m_window_load_spinner), GTK_ALIGN_CENTER);
	gtk_widget_set_valign(GTK_WIDGET(m_window_load_spinner), GTK_ALIGN_CENTER);
	gtk_stack_add_named(m_window_stack, GTK_WIDGET(m_window_load_spinner), "load");

	// Create the user button
	m_user_button = new UserButton(this);
	m_user_button_widget = m_user_button->get_button();
	m_user_button_slot = g_signal_connect_slot<GtkButton*>(m_user_button_widget, "clicked", &Window::hide, this);

	//Create commands
	m_commands[0] = new Command("changes-prevent-symbolic", _("_Lock Screen"), "xflock4", _("Failed to lock screen."), false, this);
	m_commands[1] = new Command("system-shutdown-symbolic", _("Shut _Down"), "xfce4-session-logout", _("Failed to shut down."), false, this);

	// Create action buttons
	for (int i = 0; i < 2; ++i)
	{
		m_commands_button[i] = m_commands[i]->get_button();
		m_command_slots[i] = g_signal_connect_slot<GtkButton*>(m_commands_button[i], "clicked", &Window::hide, this);
		GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(m_commands_button[i]));
		gtk_style_context_add_class(context, "zorinmenulite-circular");
		GtkCssProvider* css_provider = gtk_css_provider_new();
		gtk_css_provider_load_from_data (css_provider, ".zorinmenulite-circular { border-radius: 50%; padding: 4px 8px; }", -1, NULL);
		gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}

	// Create quicklink commands
	m_quicklinks[0] = new Command("gnome-software-symbolic",_("Software"), "gnome-software", _("Failed to open Software."), true, this);
	m_quicklinks[1] = new Command("preferences-system-symbolic",_("Settings"), "xfce4-settings-manager", _("Failed to open settings manager."), true, this);
	m_quicklinks[2] = new Command("zorin-appearance-symbolic",_("Zorin Appearance"), "zorin-appearance", _("Failed to open Zorin Appearance."), true, this);

	// Create quicklink buttons
	for (int i = 0; i < 3; ++i)
	{
		m_quicklinks_button[i] = m_quicklinks[i]->get_button();
		m_quicklink_slots[i] = g_signal_connect_slot<GtkButton*>(m_quicklinks_button[i], "clicked", &Window::hide, this);
	}

	// Create places buttons
	const GUserDirectory DEFAULT_DIRECTORIES[5] = {
		G_USER_DIRECTORY_DOCUMENTS,
		G_USER_DIRECTORY_DOWNLOAD,
		G_USER_DIRECTORY_MUSIC,
		G_USER_DIRECTORY_PICTURES,
		G_USER_DIRECTORY_VIDEOS
	};

	const gchar* home_path = g_get_home_dir ();
	m_places[0] = new PlaceButton(home_path, _("Home"), this);
	m_places_button[0] = GTK_WIDGET(m_places[0]->get_button());
	for (int i = 1; i < 6; ++i)
	{
		const gchar* path = g_get_user_special_dir(DEFAULT_DIRECTORIES[i - 1]);
		if (path == NULL || path == home_path)
		{
			m_places[i] = NULL;
			m_places_button[i] = NULL;
			continue;
		}

		m_places[i] = new PlaceButton(path, NULL, this);
		m_places_button[i] = GTK_WIDGET(m_places[i]->get_button());
	}

	// Create search entry
	m_search_entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_icon_from_icon_name(m_search_entry, GTK_ENTRY_ICON_PRIMARY, "edit-find");
	gtk_entry_set_icon_activatable(m_search_entry, GTK_ENTRY_ICON_PRIMARY, false);
	g_signal_connect_slot<GtkEditable*>(m_search_entry, "changed", &Window::search, this);

	// Create applications
	m_applications = new ApplicationsPage(this);

	// Create categories
	m_categories = new CategoryPage(this);

	// Create search results
	m_search_results = new SearchPage(this);

	// Create box for packing children
	m_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 8));
	gtk_container_set_border_width(GTK_CONTAINER(m_vbox), 8);
	gtk_stack_add_named(m_window_stack, GTK_WIDGET(m_vbox), "contents");

	// Create box for packing commands
	m_commands_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	m_commands_spacer = gtk_label_new(NULL);
	gtk_box_pack_start(m_commands_box, m_commands_spacer, true, true, 0);
	for (int i = 0; i < 2; ++i)
	{
		gtk_box_pack_start(m_commands_box, m_commands_button[i], true, false, 0);
	}

	// Create box for packing launcher pages and sidebar
	m_contents_grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_homogeneous(m_contents_grid, true);
	gtk_grid_set_row_homogeneous(m_contents_grid, true);
	gtk_grid_set_column_spacing(m_contents_grid, 8);
	gtk_box_pack_start(m_vbox, GTK_WIDGET(m_contents_grid), true, true, 0);

	// Create box for packing launcher pages
	m_panels_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_box_set_spacing(m_panels_box, 8);
	gtk_grid_attach(m_contents_grid, GTK_WIDGET(m_panels_box), 0, 0, 20, 1);
	gtk_box_pack_start(m_panels_box, m_categories->get_widget(), true, true, 0);
	gtk_box_pack_start(m_panels_box, m_applications->get_widget(), true, true, 0);
	gtk_box_pack_start(m_panels_box, m_search_results->get_widget(), true, true, 0);

	// Create box for packing sidebar
	m_sidebar = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_grid_attach(m_contents_grid, GTK_WIDGET(m_sidebar), 20, 0, 15, 1);

	// Add user button to sidebar
	gtk_box_pack_start(m_sidebar, GTK_WIDGET(m_user_button->get_button()), false, false, 0);
	gtk_box_pack_start(m_sidebar, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), false, false, 8);

	// Create box for places
	m_places_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	for (int i = 0; i < 6; ++i)
	{
		if (m_places_button[i] != NULL) {
			gtk_box_pack_start(m_places_box, m_places_button[i], true, true, 0);
		}
	}
	gtk_box_pack_start(m_sidebar, GTK_WIDGET(m_places_box), false, false, 0);
	gtk_box_pack_start(m_sidebar, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), false, false, 8);

	// Create box for sidebar quicklinks
	m_quicklinks_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	for (int i = 0; i < 3; ++i)
	{
		gtk_box_pack_start(m_quicklinks_box, m_quicklinks_button[i], true, true, 0);
	}
	gtk_box_pack_start(m_sidebar, GTK_WIDGET(m_quicklinks_box), false, false, 0);

	// Add search entry and commands box
	gtk_box_pack_end(m_panels_box, GTK_WIDGET(m_search_entry), false, false, 0);
	gtk_box_pack_end(m_sidebar, GTK_WIDGET(m_commands_box), false, false, 0);

	// Show widgets
	gtk_widget_show_all(frame);
	gtk_widget_hide(m_categories->get_widget());
	gtk_widget_hide(m_applications->get_widget());
	gtk_widget_hide(m_search_results->get_widget());
	gtk_widget_show(frame);

	// Resize to last known size
	gtk_window_set_default_size(m_window, m_geometry.width, m_geometry.height);

	// Handle transparency
	gtk_widget_set_app_paintable(GTK_WIDGET(m_window), true);
	g_signal_connect_slot(m_window, "draw", &Window::on_draw_event, this);
	g_signal_connect_slot(m_window, "screen-changed", &Window::on_screen_changed_event, this);
	on_screen_changed_event(GTK_WIDGET(m_window), NULL);

	// Load applications
	m_applications->load_applications();

	g_object_ref_sink(m_window);
}

//-----------------------------------------------------------------------------

ZorinMenuLite::Window::~Window()
{
	for (int i = 0; i < 2; ++i)
	{
		g_signal_handler_disconnect(m_commands_button[i], m_command_slots[i]);
		gtk_container_remove(GTK_CONTAINER(m_commands_box), m_commands_button[i]);
		delete m_commands[i];
	}

	for (int i = 0; i < 6; ++i)
	{
		if (m_places_button[i] != NULL) {
			gtk_container_remove(GTK_CONTAINER(m_places_box), m_places_button[i]);
			delete m_places[i];
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		g_signal_handler_disconnect(m_quicklinks_button[i], m_quicklink_slots[i]);
		gtk_container_remove(GTK_CONTAINER(m_quicklinks_box), m_quicklinks_button[i]);
		delete m_quicklinks[i];
	}

	g_signal_handler_disconnect(m_user_button_widget, m_user_button_slot);

	delete m_applications;
	delete m_categories;
	delete m_search_results;

	delete m_user_button;

	gtk_widget_destroy(GTK_WIDGET(m_window));
	g_object_unref(m_window);
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::hide()
{
	ungrab_pointer();

	// Hide command buttons to remove active border
	for (int i = 0; i < 2; ++i)
	{
		gtk_widget_set_visible(m_commands_button[i], false);
	}

	// Hide place buttons to remove active border
	for (int i = 0; i < 6; ++i)
	{
		if (m_places_button[i] != NULL) {
			gtk_widget_set_visible(m_places_button[i], false);
		}
	}

	// Hide quicklink buttons to remove active border
	for (int i = 0; i < 3; ++i)
	{
		gtk_widget_set_visible(m_quicklinks_button[i], false);
	}

	// Hide window
	gtk_widget_hide(GTK_WIDGET(m_window));

	// Reset mouse cursor by forcing default page to hide
	gtk_widget_hide(m_categories->get_widget());

	// Switch back to default page
	show_default_page();
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::show(const Position position)
{
	//Make sure User Button is valid and visible
	m_user_button->update_username();
	m_user_button->check();

	// Make sure commands are valid and visible
	for (int i = 0; i < 2; ++i)
	{
		m_commands[i]->check();
	}

	// Make sure quicklinks are valid and visible
	for (int i = 0; i < 3; ++i)
	{
		m_quicklinks[i]->check();
	}

	// Make sure place buttons are visible
	for (int i = 0; i < 6; ++i)
	{
		if (m_places_button[i] != NULL) {
			gtk_widget_set_visible(m_places_button[i], true);
		}
	}

	if (m_applications->load_applications())
	{
		set_loaded();
	}
	else
	{
		m_plugin->set_loaded(false);
		gtk_stack_set_visible_child_name(m_window_stack, "load");
		gtk_spinner_start(m_window_load_spinner);
	}

	// Reset mouse cursor by forcing default page to hide
	gtk_widget_show(m_categories->get_widget());

	show_default_page();

	GdkScreen* screen = NULL;
	int parent_x = 0, parent_y = 0, parent_w = 0, parent_h = 0;
	if (position != PositionAtCursor)
	{
		// Wait up to half a second for auto-hidden panels to be shown
		clock_t end = clock() + (CLOCKS_PER_SEC / 2);
		GtkWidget* parent = m_plugin->get_button();
		GtkWindow* parent_window = GTK_WINDOW(gtk_widget_get_toplevel(parent));
		gtk_window_get_position(parent_window, &parent_x, &parent_y);
		while ((parent_x == -9999) && (parent_y == -9999) && (clock() < end))
		{
			while (gtk_events_pending())
			{
				gtk_main_iteration();
			}
			gtk_window_get_position(parent_window, &parent_x, &parent_y);
		}

		// Fetch parent geometry
		if (!gtk_widget_get_realized(parent))
		{
			gtk_widget_realize(parent);
		}
		GdkWindow* window = gtk_widget_get_window(parent);
		gdk_window_get_origin(window, &parent_x, &parent_y);
		screen = gdk_window_get_screen(window);
		parent_w = gdk_window_get_width(window);
		parent_h = gdk_window_get_height(window);
	}
	else
	{
		GdkDisplay* display = gdk_display_get_default();
		GdkSeat* seat = gdk_display_get_default_seat(display);
		GdkDevice* device = gdk_seat_get_pointer(seat);
		gdk_device_get_position(device, &screen, &parent_x, &parent_y);
	}

	// Fetch screen geomtry
	GdkRectangle monitor;
	GdkMonitor* monitor_gdk = gdk_display_get_monitor_at_point(gdk_display_get_default(), parent_x, parent_y);
	gdk_monitor_get_geometry(monitor_gdk, &monitor);

	// Prevent window from being larger than screen
	if (m_geometry.width > monitor.width)
	{
		m_geometry.width = monitor.width;
		gtk_window_resize(m_window, m_geometry.width, m_geometry.height);
	}
	if (m_geometry.height > monitor.height)
	{
		m_geometry.height = monitor.height;
		gtk_window_resize(m_window, m_geometry.width, m_geometry.height);
	}

	// Find window position
	bool layout_left = ((2 * (parent_x - monitor.x)) + parent_w) < monitor.width;
	bool layout_bottom = ((2 * (parent_y - monitor.y)) + (parent_h / 2)) > monitor.height;
	if (position != PositionVertical)
	{
		m_geometry.x = layout_left ? parent_x : (parent_x + parent_w - m_geometry.width);
		m_geometry.y = layout_bottom ? (parent_y - m_geometry.height) : (parent_y + parent_h);
	}
	else
	{
		m_geometry.x = layout_left ? (parent_x + parent_w) : (parent_x - m_geometry.width);
		m_geometry.y = layout_bottom ? (parent_y + parent_h - m_geometry.height) : parent_y;
	}

	// Prevent window from leaving screen
	m_geometry.x = CLAMP(m_geometry.x, monitor.x, monitor.x + monitor.width - m_geometry.width);
	m_geometry.y = CLAMP(m_geometry.y, monitor.y, monitor.y + monitor.height - m_geometry.height);

	// Move window
	gtk_window_move(m_window, m_geometry.x, m_geometry.y);

	// Show window
	gtk_widget_show(GTK_WIDGET(m_window));
	gtk_window_move(m_window, m_geometry.x, m_geometry.y);
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::on_context_menu_destroyed()
{
	grab_pointer(GTK_WIDGET(m_window));
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::set_categories(const std::vector<Category*>& categories)
{
	m_categories->set_categories(categories);

	show_default_page();
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::set_items()
{
	m_search_results->set_menu_items(m_applications->get_view()->get_model());
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::set_loaded()
{
	// Hide loading spinner
	gtk_spinner_stop(m_window_load_spinner);
	gtk_stack_set_visible_child_name(m_window_stack, "contents");

	// Focus search entry
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));

	// Show panel button
	m_plugin->set_loaded(true);
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::unset_items()
{
	m_search_results->unset_menu_items();
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_enter_notify_event(GtkWidget*, GdkEvent* event)
{
	GdkEventCrossing* crossing_event = reinterpret_cast<GdkEventCrossing*>(event);
	if ((crossing_event->detail == GDK_NOTIFY_INFERIOR)
			|| (crossing_event->mode == GDK_CROSSING_GRAB)
			|| (crossing_event->mode == GDK_CROSSING_GTK_GRAB))
	{
		return false;
	}

	grab_pointer(GTK_WIDGET(m_window));

	return false;
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_leave_notify_event(GtkWidget*, GdkEvent* event)
{
	GdkEventCrossing* crossing_event = reinterpret_cast<GdkEventCrossing*>(event);
	if ((crossing_event->detail == GDK_NOTIFY_INFERIOR)
			|| (crossing_event->mode != GDK_CROSSING_NORMAL))
	{
		return false;
	}

	grab_pointer(GTK_WIDGET(m_window));

	return false;
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_button_press_event(GtkWidget*, GdkEvent* event)
{
	// Hide menu if user clicks outside
	GdkEventButton* button_event = reinterpret_cast<GdkEventButton*>(event);
	if ((button_event->x_root <= m_geometry.x)
			|| (button_event->x_root >= m_geometry.x + m_geometry.width)
			|| (button_event->y_root <= m_geometry.y)
			|| (button_event->y_root >= m_geometry.y + m_geometry.height))
	{
		hide();
	}
	return false;
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_key_press_event(GtkWidget* widget, GdkEvent* event)
{
	GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);

	// Hide if escape is pressed and there is no text in search entry
	if ( (key_event->keyval == GDK_KEY_Escape) && exo_str_is_empty(gtk_entry_get_text(m_search_entry)) )
	{
		hide();
		return true;
	}

	Page* page = NULL;
	GtkWidget* view = NULL;
	bool page_is_category = false;
	if (gtk_widget_get_visible(m_categories->get_widget()))
	{
		view = m_categories->get_view()->get_widget();
		page_is_category = true;
	}
	else
	{
		if (gtk_widget_get_visible(m_search_results->get_widget()))
		{
			page = m_search_results;
		}
		else
		{
			page = m_applications;
		}
		view = page->get_view()->get_widget();
	}

	// Allow keyboard navigation out of treeview
	if ((key_event->keyval == GDK_KEY_Left) || (key_event->keyval == GDK_KEY_Right))
	{
		if ((widget == view) || (gtk_window_get_focus(m_window) == view))
		{
			if (page_is_category)
			{
				m_categories->reset_selection();
			}
			else
			{
				page->reset_selection();
			}
		}
	}

	// Make up and down keys scroll current list of applications from search
	if ((key_event->keyval == GDK_KEY_Up) || (key_event->keyval == GDK_KEY_Down))
	{
		GtkWidget* search = GTK_WIDGET(m_search_entry);
		if ((widget == search) || (gtk_window_get_focus(m_window) == search))
		{
			gtk_widget_grab_focus(view);
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_key_press_event_after(GtkWidget* widget, GdkEvent* event)
{
	// Pass unhandled key presses to search entry
	GtkWidget* search_entry = GTK_WIDGET(m_search_entry);
	if ((widget != search_entry) && (gtk_window_get_focus(m_window) != search_entry))
	{
		GdkEventKey* key_event = reinterpret_cast<GdkEventKey*>(event);
		if (key_event->is_modifier)
		{
			return false;
		}
		gtk_widget_grab_focus(search_entry);
		gtk_window_propagate_key_event(m_window, key_event);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_map_event(GtkWidget*, GdkEvent*)
{
	gtk_window_set_keep_above(m_window, true);

	// Track mouse clicks outside of menu
	grab_pointer(GTK_WIDGET(m_window));

	// Focus search entry
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));

	return false;
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_state_flags_changed_event(GtkWidget* widget, GtkStateFlags)
{
	// Refocus and raise window if visible
	if (gtk_widget_get_visible(widget))
	{
		gtk_window_present(m_window);
	}

	return false;
}

//-----------------------------------------------------------------------------


void ZorinMenuLite::Window::on_screen_changed_event(GtkWidget* widget, GdkScreen*)
{
	GdkScreen* screen = gtk_widget_get_screen(widget);
	GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
	visual = gdk_screen_get_system_visual(screen);
	gtk_widget_set_visual(widget, visual);
}

//-----------------------------------------------------------------------------

gboolean ZorinMenuLite::Window::on_draw_event(GtkWidget* widget, cairo_t* cr)
{
	if (!gtk_widget_get_realized(widget))
	{
		gtk_widget_realize(widget);
	}

	GtkStyleContext* context = gtk_widget_get_style_context(widget);
	const double width = gtk_widget_get_allocated_width(widget);
	const double height = gtk_widget_get_allocated_height(widget);
	gtk_render_background(context, cr, 0.0, 0.0, width, height);

	return false;
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::category_activated(Category* category)
{
	m_applications->reset_selection();
	m_applications->apply_filter(category);
	gtk_widget_hide(m_categories->get_widget());
	gtk_widget_show_all(m_applications->get_widget());
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

void ZorinMenuLite::Window::back_button_activated()
{
	m_categories->reset_selection();
	gtk_widget_hide(m_applications->get_widget());
	gtk_widget_hide(m_search_results->get_widget());
	gtk_widget_show_all(m_categories->get_widget());

	// Clear search entry
	gtk_entry_set_text(m_search_entry, "");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::show_default_page()
{
	gtk_widget_show_all(m_categories->get_widget());
	gtk_widget_hide(m_applications->get_widget());
	
	// Clear search entry
	gtk_entry_set_text(m_search_entry, "");
	gtk_widget_grab_focus(GTK_WIDGET(m_search_entry));
}

//-----------------------------------------------------------------------------

void ZorinMenuLite::Window::search()
{
	// Fetch search string
	const gchar* text = gtk_entry_get_text(m_search_entry);
	if (exo_str_is_empty(text))
	{
		text = NULL;
	}

	// Update search entry icon
	bool visible = text != NULL;

	if (visible)
	{
		gtk_entry_set_icon_from_icon_name(m_search_entry, GTK_ENTRY_ICON_SECONDARY, "edit-clear");
		gtk_entry_set_icon_activatable(m_search_entry, GTK_ENTRY_ICON_SECONDARY, true);

		// Show search results
		gtk_widget_hide(m_categories->get_widget());
		gtk_widget_hide(m_applications->get_widget());
		gtk_widget_show(m_search_results->get_widget());
	}
	else
	{
		gtk_entry_set_icon_from_icon_name(m_search_entry, GTK_ENTRY_ICON_SECONDARY, NULL);
		gtk_entry_set_icon_activatable(m_search_entry, GTK_ENTRY_ICON_SECONDARY, false);

		// Show active panel
		gtk_widget_hide(m_search_results->get_widget());
		gtk_widget_show(m_categories->get_widget());
	}

	// Apply filter
	m_search_results->set_filter(text);
}

//-----------------------------------------------------------------------------
