/*
Copyright © 2011-2012 Clint Bellanger and morris989
Copyright © 2012 Stefan Beller
Copyright © 2013-2014 Henrik Andersson
Copyright © 2013 Kurt Rinnert
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
 * class ChatBox
 */

#include "Avatar.h"
#include "FileParser.h"
#include "FontEngine.h"
#include "InputState.h"
#include "Menu.h"
#include "MenuInventory.h"
#include "MenuManager.h"
#include "ChatBox.h"
#include "ChatClient.h"
#include "ChatManager.h"
#include "GetText.h"
#include "MenuVendor.h"
#include "MessageEngine.h"
#include "RenderDevice.h"
#include "SharedResources.h"
#include "SharedGameResources.h"
#include "StatBlock.h"
#include "UtilsParsing.h"
#include "WidgetButton.h"
#include "WidgetCheckBox.h"
#include "WidgetInput.h"
#include "WidgetLabel.h"
#include "WidgetListBox.h"

ChatBox::ChatBox()
	: Menu()
	, event_cursor(0)
	, font_who("font_regular")
	, font_dialog("font_regular")
	, topic_color_normal(font->getColor(FontEngine::COLOR_MENU_BONUS))
	, topic_color_hover(font->getColor(FontEngine::COLOR_WIDGET_NORMAL))
	, topic_color_pressed(font->getColor(FontEngine::COLOR_WIDGET_DISABLED))
	, trade_color_normal(font->getColor(FontEngine::COLOR_MENU_BONUS))
	, trade_color_hover(font->getColor(FontEngine::COLOR_WIDGET_NORMAL))
	, trade_color_pressed(font->getColor(FontEngine::COLOR_WIDGET_DISABLED))
 {

	setBackground("images/menus/dialog_box.png");
  visible = true;
	// Load config settings
	FileParser infile;
	// @CLASS ChatBox|Description of menus/talker.txt
	if(infile.open("menus/chat_box.txt", FileParser::MOD_FILE, FileParser::ERROR_NORMAL)) {
		while(infile.next()) {
			if (parseMenuKey(infile.key, infile.val))
				continue;

			// @ATTR dialogbox|rectangle|Position and dimensions of the text box graphics.
			else if (infile.key == "dialogbox") dialog_pos = Parse::toRect(infile.val);
			// @ATTR dialogtext|rectangle|Rectangle where the dialog text is placed.
			else if (infile.key == "dialogtext") text_pos = Parse::toRect(infile.val);
			// @ATTR text_offset|point|Margins for the left/right and top/bottom of the dialog text.
			else if (infile.key == "text_offset") text_offset = Parse::toPoint(infile.val);

			// @ATTR font_who|predefined_string|Font style to use for the name of the currently talking person.
			else if (infile.key == "font_who") font_who = infile.val;
			// @ATTR font_dialog|predefined_string|Font style to use for the dialog text.
			else if (infile.key == "font_dialog") font_dialog = infile.val;

			// @ATTR topic_color_normal|color|The normal color for topic text.
			else if (infile.key == "topic_color_normal") topic_color_normal = Parse::toRGB(infile.val);
			// @ATTR topic_color_hover|color|The color for topic text when highlighted.
			else if (infile.key == "topic_color_hover") topic_color_hover = Parse::toRGB(infile.val);
			// @ATTR topic_color_normal|color|The color for topic text when clicked.
			else if (infile.key == "topic_color_pressed") topic_color_pressed = Parse::toRGB(infile.val);

			// @ATTR trade_color_normal|color|The normal color for the "Trade" text.
			else if (infile.key == "trade_color_normal") trade_color_normal = Parse::toRGB(infile.val);
			// @ATTR trade_color_hover|color|The color for the "Trade" text when highlighted.
			else if (infile.key == "trade_color_hover") trade_color_hover = Parse::toRGB(infile.val);
			// @ATTR trade_color_normal|color|The color for the "Trade" text when clicked.
			else if (infile.key == "trade_color_pressed") trade_color_pressed = Parse::toRGB(infile.val);

			else infile.error("ChatBox: '%s' is not a valid key.", infile.key.c_str());
		}
		infile.close();
	}

	label_name = new WidgetLabel();
	label_name->setBasePos(text_pos.x + text_offset.x, text_pos.y + text_offset.y, Utils::ALIGN_TOPLEFT);
	label_name->setColor(font->getColor(FontEngine::COLOR_MENU_NORMAL));
	label_name->setText("CHAT");
	label_name->setFont(font_who);
	input_name = new WidgetInput(WidgetInput::DEFAULT_FILE);
	input_name->max_length = 20;
	input_name->setBasePos(text_pos.x, text_pos.y + text_offset.y, Utils::ALIGN_BOTTOMLEFT);
	textbox = new WidgetScrollBox(text_pos.w, 1000);//text_pos.h-(text_offset.y*2));
	textbox->setBasePos(text_pos.x, text_pos.y + text_offset.y, Utils::ALIGN_TOPLEFT);
	align();
}

void ChatBox::align() {
	Menu::align();

	label_name->setPos(window_area.x, window_area.y);
	//std::cout << window_area.x << " " << window_area.y << " " << window_area.h << " " << window_area.w << std::endl;
	textbox->setPos(window_area.x, window_area.y + label_name->getBounds()->h);
	textbox->pos.h = text_pos.h - (text_offset.y*2);
	input_name->setPos(window_area.x, window_area.y + label_name->getBounds()->h + text_pos.h);
}

/**
 * Menu interaction (enter/space/click to continue) -----------------------
 */
void ChatBox::logic() {
	// tablist.logic();
	if (!input_name->edit_mode){
		tablist.logic();
	}
 	input_name->logic();
	if (inpt->pressing[Input::ACCEPT] && !inpt->lock[Input::ACCEPT]){
		inpt->lock[Input::ACCEPT] = true;
		//std::cout << input_name->getText() << std::endl;
		input_lines.push_back(input_name->getText());
		//confirm_clicked = true;
	}
	chatrender();
}


void ChatBox::chatrender(){

	int y = 0;
	for (int i = 0; i < input_lines.size(); i++)
	{
		Point line_size = font->calc_size(input_lines[i],textbox->pos.w-(text_offset.x*2));
		font->render(
			input_lines[i],
			text_offset.x,
			y,
			FontEngine::JUSTIFY_LEFT,
			textbox->contents->getGraphics(),
			text_pos.w - text_offset.x*2,
			font->getColor(FontEngine::COLOR_MENU_NORMAL)
		);
		y = y + line_size.y;
  }
	align();

}
void ChatBox::render() {
	 if (!visible) return;
	Rect src;
	Rect dest;

	int offset_x = window_area.x;
	int offset_y = window_area.y;

	// dialog box --------------------------------------------------------

	src.x = 0;
	src.y = 0;
	dest.x = offset_x + dialog_pos.x;
	dest.y = offset_y + dialog_pos.y;
	src.w = dest.w = dialog_pos.w;
	src.h = dest.h = dialog_pos.h;

	setBackgroundClip(src);
	setBackgroundDest(dest);
	Menu::render();


	// name & dialog text
	label_name->render();
	input_name->render();
	if (textbox->update){
		textbox->refresh();
	}
	textbox->render();

//	WidgetInput.loadGraphics();
 }

ChatBox::~ChatBox() {
	delete label_name;
	delete input_name;
	delete textbox;
}
