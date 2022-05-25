/*
 * Original work: Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019 Graeme Gott <graeme@gottcode.org>
 * Modified work: Copyright (C) 2017-2021 Zorin OS Technologies Ltd.
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

#include "plugin.h"

#include "applications-page.h"
#include "command.h"
#include "slot.h"
#include "window.h"

extern "C"
{
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
}

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

extern "C" void zorinmenulite_construct(XfcePanelPlugin* plugin)
{
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
	new Plugin(plugin);
}

static void zorinmenulite_free(XfcePanelPlugin*, Plugin* zorinmenulite)
{
	delete zorinmenulite;
}

// Wait for grab; allows modifier as shortcut
// Adapted from http://git.xfce.org/xfce/xfce4-panel/tree/common/panel-utils.c#n122
static bool panel_utils_grab_available()
{
	bool grab_succeed = false;

	GdkWindow* root = gdk_screen_get_root_window(xfce_gdk_screen_get_active(NULL));
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* seat = gdk_display_get_default_seat(display);

	// Don't try to get the grab for longer then 1/4 second
	for (guint i = 0; i < (G_USEC_PER_SEC / 400); ++i)
	{
		if (gdk_seat_grab(seat, root, GDK_SEAT_CAPABILITY_ALL, true, NULL, NULL, NULL, NULL))
		{
			gdk_seat_ungrab(seat);
			grab_succeed = true;
			break;
		}
		g_usleep(100);
	}

	if (!grab_succeed)
	{
		g_printerr("xfce4-zorinmenulite-plugin: Unable to get keyboard and mouse grab. Menu popup failed.\n");
	}

	return grab_succeed;
}

static void widget_add_css(GtkWidget* widget, const gchar* css)
{
	GtkCssProvider* provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, css, -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(widget),
			GTK_STYLE_PROVIDER(provider),
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(provider);
}

//-----------------------------------------------------------------------------

Plugin::Plugin(XfcePanelPlugin* plugin) :
	m_plugin(plugin),
	m_window(NULL)
{
	// Create toggle button
	m_button = xfce_panel_create_toggle_button();
	gtk_widget_set_name(m_button, "zorinmenulite-button");
#if !LIBXFCE4PANEL_CHECK_VERSION(4,13,0)
	widget_add_css(m_button, ".xfce4-panel button { padding: 1px; }");
#endif
	g_signal_connect_slot(m_button, "toggled", &Plugin::button_toggled, this);
	gtk_widget_show(m_button);

	m_button_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	gtk_container_add(GTK_CONTAINER(m_button), GTK_WIDGET(m_button_box));
	gtk_container_set_border_width(GTK_CONTAINER(m_button_box), 0);
	gtk_widget_show(GTK_WIDGET(m_button_box));

	m_button_icon = GTK_IMAGE(gtk_image_new());
	gtk_image_set_from_icon_name (m_button_icon,
                                  "start-here-symbolic", GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(m_button_box, GTK_WIDGET(m_button_icon), true, false, 0);
	gtk_widget_show(GTK_WIDGET(m_button_icon));
	gtk_widget_set_sensitive(GTK_WIDGET(m_button_icon), false);

	// Add plugin to panel
	gtk_container_add(GTK_CONTAINER(plugin), m_button);
	xfce_panel_plugin_add_action_widget(plugin, m_button);

	// Connect plugin signals to functions
	g_signal_connect(plugin, "free-data", G_CALLBACK(zorinmenulite_free), this);
	g_signal_connect_slot(plugin, "mode-changed", &Plugin::mode_changed, this);
	g_signal_connect_slot(plugin, "remote-event", &Plugin::remote_event, this);
	g_signal_connect_slot<XfcePanelPlugin*>(plugin, "about", &Plugin::show_about, this);
	g_signal_connect_slot(plugin, "size-changed", &Plugin::size_changed, this);

	xfce_panel_plugin_menu_show_about(plugin);

	m_menu_editor = new Command("xfce4-menueditor", _("_Edit Applications"), "menulibre", _("Failed to launch menu editor."), false, NULL);
	xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(m_menu_editor->get_menuitem()));

	mode_changed(m_plugin, xfce_panel_plugin_get_mode(m_plugin));

	g_signal_connect_slot<GtkWidget*>(plugin, "style-updated", &Plugin::update_size, this);
	g_signal_connect_slot<GtkWidget*,GdkScreen*>(m_button, "screen-changed", &Plugin::update_size, this);

	// Create menu window
	m_window = new Window(this);
	g_signal_connect_slot<GtkWidget*>(m_window->get_widget(), "unmap", &Plugin::menu_hidden, this);
}

//-----------------------------------------------------------------------------

Plugin::~Plugin()
{
	delete m_window;
	m_window = NULL;
	delete m_menu_editor;
	m_menu_editor = NULL;

	gtk_widget_destroy(m_button);
}

//-----------------------------------------------------------------------------

void Plugin::reload()
{
	m_window->hide();
	m_window->get_applications()->invalidate_applications();
}

//-----------------------------------------------------------------------------

void Plugin::set_loaded(bool loaded)
{
	gtk_widget_set_sensitive(GTK_WIDGET(m_button_icon), loaded);
}

//-----------------------------------------------------------------------------

void Plugin::button_toggled(GtkToggleButton* button)
{
	if (gtk_toggle_button_get_active(button) == false)
	{
		if (gtk_widget_get_visible(m_window->get_widget()))
		{
			m_window->hide();
		}
		xfce_panel_plugin_block_autohide(m_plugin, false);
	}
	else
	{
		xfce_panel_plugin_block_autohide(m_plugin, true);
		show_menu(false);
	}
}

//-----------------------------------------------------------------------------

void Plugin::menu_hidden()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button), false);
}

//-----------------------------------------------------------------------------

void Plugin::mode_changed(XfcePanelPlugin*, XfcePanelPluginMode)
{
	update_size();
}

//-----------------------------------------------------------------------------

gboolean Plugin::remote_event(XfcePanelPlugin*, gchar* name, GValue* value)
{
	if (strcmp(name, "popup") || !panel_utils_grab_available())
	{
		return false;
	}

	if (gtk_widget_get_visible(m_window->get_widget()))
	{
		m_window->hide();
	}
	else if (value && G_VALUE_HOLDS_BOOLEAN(value) && g_value_get_boolean(value))
	{
		show_menu(true);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button), true);
	}

	return true;
}

//-----------------------------------------------------------------------------

void Plugin::show_about()
{
	const gchar* authors[] = {
		"Kyrill Zorin <zorink@zoringroup.com>",
		"Graeme Gott <graeme@gottcode.org>",
		NULL };

	gtk_show_about_dialog
		(NULL,
		"authors", authors,
		"comments", _("The default application menu in Zorin OS Lite"),
		"copyright", _("Copyright \302\251 2013-2021 The Zorin Menu Lite authors"),
		"license", XFCE_LICENSE_GPL,
		"logo-icon-name", "xfce4-zorinmenulite",
		"program-name", "Zorin Menu Lite",
		"translator-credits", _("translator-credits"),
		"version", PACKAGE_VERSION,
		NULL);
}

//-----------------------------------------------------------------------------

gboolean Plugin::size_changed(XfcePanelPlugin*, gint size)
{
	GtkOrientation panel_orientation = xfce_panel_plugin_get_orientation(m_plugin);
	GtkOrientation orientation = panel_orientation;

	// Make icon expand to fill button
	gtk_box_set_child_packing(GTK_BOX(m_button_box), GTK_WIDGET(m_button_icon),
			true,
			true,
			0, GTK_PACK_START);

	// Resize icon
#if LIBXFCE4PANEL_CHECK_VERSION(4,13,0)
	gint icon_size;
	if (size <= 19)
		icon_size = 12;
	else if (size <= 27)
		icon_size = 16;
	else if (size <= 35)
		icon_size = 24;
	else if (size <= 41)
		icon_size = 32;
	else
		icon_size = size - 4;
#else
	GtkBorder padding, border;
	GtkStyleContext* context = gtk_widget_get_style_context(m_button);
	GtkStateFlags flags = gtk_widget_get_state_flags(m_button);
	gtk_style_context_get_padding(context, flags, &padding);
	gtk_style_context_get_border(context, flags, &border);
	gint xthickness = padding.left + padding.right + border.left + border.right;
	gint ythickness = padding.top + padding.bottom + border.top + border.bottom;
	gint icon_size = size - 2 * std::max(xthickness, ythickness);
#endif
	gtk_image_set_pixel_size(m_button_icon, icon_size);

	gtk_widget_set_size_request(m_button, size, size);

	xfce_panel_plugin_set_small(m_plugin, false);

	gtk_orientable_set_orientation(GTK_ORIENTABLE(m_button_box), orientation);

	return true;
}

//-----------------------------------------------------------------------------

void Plugin::update_size()
{
	size_changed(m_plugin, xfce_panel_plugin_get_size(m_plugin));
}

//-----------------------------------------------------------------------------

void Plugin::show_menu(bool at_cursor)
{
	m_window->show(at_cursor ? Window::PositionAtCursor : Window::Position(xfce_panel_plugin_get_orientation(m_plugin)));
}

//-----------------------------------------------------------------------------
