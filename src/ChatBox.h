/*
Copyright © 2011-2012 Clint Bellanger and morris989
Copyright © 2013-2014 Henrik Andersson
Copyright © 2012-2016 Justin Jacobs

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

/**
 * class MenuTalker
 */

#ifndef CHATBOX_H
#define CHATBOX_H

#include "CommonIncludes.h"
#include "Utils.h"
#include "Widget.h"
#include "Menu.h"
#include "NetClient.h"
#include "WidgetScrollBar.h"
#include "WidgetScrollBox.h"
#include "WidgetInput.h"

class WidgetButton;
class WidgetLabel;
class WidgetScrollBox;
class WidgetInput;

class ChatBox : public Menu {
private:

	unsigned int event_cursor;

	Rect dialog_pos;
	Rect text_pos;
	Point text_offset;

	std::string font_who;
	std::string font_dialog;

	WidgetLabel *label_name;
	WidgetScrollBox *textbox;
	WidgetInput *input_name;

	Color topic_color_normal;
	Color topic_color_hover;
	Color topic_color_pressed;

	Color trade_color_normal;
	Color trade_color_hover;
	Color trade_color_pressed;

	std::vector<std::string> input_lines;
	NetClient client1;
	
public:
	explicit ChatBox();
	~ChatBox();
	void align();


	void logic();
	void render();
	void chatrender();
};

#endif
