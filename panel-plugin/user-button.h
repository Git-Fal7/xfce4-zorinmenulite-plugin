/*
 * Original work: Copyright (C) 2014, 2016 Graeme Gott <graeme@gottcode.org>
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

#ifndef ZORIN_MENU_LITE_USER_BUTTON_H
#define ZORIN_MENU_LITE_USER_BUTTON_H

#include <gtk/gtk.h>
#include "command.h"

namespace ZorinMenuLite
{

class Window;

class UserButton : public Command
{
public:
	explicit UserButton(Window* window);
	~UserButton();

	void update_username();

private:
	void update_user_image(GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type);

private:
	GFileMonitor* m_file_monitor;
};

}

#endif // ZORIN_MENU_LITE_USER_BUTTON_H
