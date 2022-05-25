/*
 * Original work: Copyright (C) 2013, 2015 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_SEARCH_ACTION_H
#define ZORIN_MENU_LITE_SEARCH_ACTION_H

#include "element.h"

#include <string>

namespace ZorinMenuLite
{

class SearchAction : public Element
{
public:
	SearchAction();
	SearchAction(const gchar* name, const gchar* pattern, const gchar* command);
	~SearchAction();

	enum
	{
		Type = 3
	};
	int get_type() const
	{
		return Type;
	}

	void run(GdkScreen* screen) const;
	guint search(const Query& query);

private:
	guint match_regex(const gchar* haystack);

private:
	std::string m_name;
	std::string m_pattern;
	std::string m_command;

	std::string m_expanded_command;
	GRegex* m_regex;
};

}

#endif // ZORIN_MENU_LITE_SEARCH_ACTION_H
