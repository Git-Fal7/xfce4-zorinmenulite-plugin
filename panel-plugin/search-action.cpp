/*
 * Original work: Copyright (C) 2013, 2015, 2016 Graeme Gott <graeme@gottcode.org>
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

#include "search-action.h"

#include "query.h"

#include <libxfce4ui/libxfce4ui.h>

using namespace ZorinMenuLite;

//-----------------------------------------------------------------------------

SearchAction::SearchAction(const gchar* name, const gchar* pattern, const gchar* command) :
	m_name(name ? name : ""),
	m_pattern(pattern ? pattern : ""),
	m_command(command ? command : ""),
	m_regex(NULL)
{
	set_icon("folder-saved-search");
	const gchar* direction = (gtk_widget_get_default_direction() != GTK_TEXT_DIR_RTL) ? "\342\200\216" : "\342\200\217";
	set_text(g_markup_printf_escaped("%s%s", direction, m_name.c_str()));
	set_tooltip(_("Search Action"));
}

//-----------------------------------------------------------------------------

SearchAction::~SearchAction()
{
	if (m_regex)
	{
		g_regex_unref(m_regex);
	}
}

//-----------------------------------------------------------------------------

guint SearchAction::search(const Query& query)
{
	if (m_pattern.empty() || m_command.empty())
	{
		return false;
	}

	m_expanded_command.clear();

	const gchar* haystack = query.raw_query().c_str();
	guint found = match_regex(haystack);

	return found;
}

//-----------------------------------------------------------------------------

guint SearchAction::match_regex(const gchar* haystack)
{
	guint found = G_MAXUINT;

	if (!m_regex)
	{
		m_regex = g_regex_new(m_pattern.c_str(), G_REGEX_OPTIMIZE, GRegexMatchFlags(0), NULL);
		if (!m_regex)
		{
			return found;
		}
	}
	GMatchInfo* match = NULL;
	if (g_regex_match(m_regex, haystack, GRegexMatchFlags(0), &match))
	{
		gchar* expanded = g_match_info_expand_references(match, m_command.c_str(), NULL);
		if (expanded)
		{
			m_expanded_command = expanded;
			g_free(expanded);
			found = m_pattern.length();
		}
	}
	if (match != NULL)
	{
		g_match_info_free(match);
	}

	return found;
}

//-----------------------------------------------------------------------------

void SearchAction::run(GdkScreen* screen) const
{
	GError* error = NULL;
	gboolean result = xfce_spawn_command_line_on_screen(screen, m_expanded_command.c_str(), FALSE, FALSE, &error);

	if (G_UNLIKELY(!result))
	{
		xfce_dialog_show_error(NULL, error, _("Failed to execute command \"%s\"."), m_expanded_command.c_str());
		g_error_free(error);
	}
}

//-----------------------------------------------------------------------------
