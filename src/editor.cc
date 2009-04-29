
/**************************************************************************
 * Copyright (C) 2007-2008 Ruben Pollan Bella <meskio@amedias.org>        *
 *                                                                        *
 *  This file is part of TuDu.                                            *
 *                                                                        *
 *  TuDu is free software; you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 3 of the License.        *
 *                                                                        *
 *  TuDu is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include "editor.h"
#include "interface.h"
#include "data.h"

#define cmp(str) text.compare(0, length, str, 0, length)

Editor::Editor()
{
	text = "";
	cursor = 0;
	key = '\0';
}

string& Editor::getText()
{
	return text;
}

int& Editor::cursorPos()
{
	return cursor;
}

bool Editor::edit(Window& win, int y, int x, unsigned int max_length)
{
	bool resized = false;

	initialize();
	exit = false;
	result = true;
	win._move(y, x);
	win._addstr(text);
	win._move(y, x+cursor);
	echo();
	curs_set(1);
	win._refresh();
	while (!exit)
	{
		key = win._getch();
		switch (key)
		{
			case KEY_RESIZE:
				resized = true;
				break;
			case KEY_LEFT: left();
				break;
			case KEY_RIGHT: right();
				break;
			case KEY_UP: up();
				break;
			case KEY_DOWN: down();
				break;
			case KEY_HOME: home();
				break;
			case KEY_END: end();
				break;
			case KEY_BACKSPACE: backspace();
				break;
			case KEY_DC: supr();
				break;
			case '\e': esc();
				break;
			case '\n':
			case KEY_ENTER: enter();
				break;
			default: other();
				break;
		}
		if (max_length <= text.length())
			text.erase(max_length);
		if (cursor >= (int) max_length)
			cursor = max_length-1;
		win._move(y, x);
		win._addstr(text);
		for (unsigned int i = text.length(); i < max_length; i++) 
				win._addch(' ');
		win._move(y, x+cursor);
		win._refresh();
	}
	noecho();
	curs_set(0);
	win._refresh();

	if (resized) ungetch(KEY_RESIZE);
	return result;
}

void Editor::initialize() {}
void Editor::left() {}
void Editor::right() {}
void Editor::up() {}
void Editor::down() {}
void Editor::home() {}
void Editor::end() {}
void Editor::backspace() {}
void Editor::supr() {}
void Editor::other() {}

void Editor::esc()
{ 
	exit = true;
	result = false;
}

void Editor::enter()
{ 
	exit = true;
}


void LineEditor::left()
{
	if (cursor>0) --cursor;
	else if (text.length() == 0)
	{
		exit = true;
		result = false;
	}
}

void LineEditor::right()
{
	if (cursor<(int)text.length()) ++cursor;
}

void LineEditor::home()
{
	cursor = 0;
}

void LineEditor::end()
{
	cursor = text.length();
}

void LineEditor::backspace()
{
	if (cursor>0) text.erase(--cursor, 1);
}

void LineEditor::supr()
{
	if (cursor<(int)text.length()) text.erase(cursor, 1);
}

void LineEditor::other()
{
	if (key < 256)
	{
		text.insert(cursor,1,key);
		++cursor;
	}
}

void CategoryEditor::initialize()
{
	search = categories.end();
}

void CategoryEditor::other()
{
	if (key == '\t')
	{
		completion();
	}
	else if (key < 256)
	{
		//FIXME: call the parent method
		text.insert(cursor,1,key);
		++cursor;
	}
}

void CategoryEditor::completion()
{
	/* if it is no the first time */
	if ((cursor == (int)text.length()) &&
	    (search != categories.end()) &&
	    (text == *search))
	{
		search++;
		if ((search != categories.end()) && 
		   (!cmp(*search)))
		{
			text = *search;
			cursor = text.length();
		}
		else
		{
			text = text.substr(0, length);
			search = first;
			cursor = length;
		}
	}
	/* if it is the first time */
	else
	{
		length = text.length();
		for (search = categories.begin(); 
		    (search != categories.end()) && 
		    (cmp(*search)); 
		    search++);
		if ((search != categories.end()) && 
		    (!cmp(*search)))
		{
			text = *search;
			first = search;
			cursor = text.length();
		}
	}
}

void HistoryEditor::initialize()
{
	shown = history.begin();
	cursor = 0;
	text = "";
}

void HistoryEditor::up()
{
	if (shown != history.end())
	{
		text = *shown;
		cursor = text.length();
		shown++;
	}
}

void HistoryEditor::down()
{
	if (shown != history.begin())
	{
		shown--;
		text = *shown;
		cursor = text.length();
	}
}

void HistoryEditor::enter()
{
	exit = true;
	history.push_front(text);
}

void CmdEditor::initialize()
{
	search = categories.end();
	com_search = commands.end();
	param = 0;

	//FIXME: call the parent method
	shown = history.begin();
	cursor = 0;
	text = "";
}

void CmdEditor::other()
{
	if (key == '\t')
	{
		completion();
	}
	else if (key < 256)
	{
		//FIXME: call the parent method
		text.insert(cursor,1,key);
		++cursor;
	}
}

void CmdEditor::completion()
{
	vector<string> params;
	size_t begin, end;
	string rest_params = "";

	/* Get the command and params in text */
	for (begin = 0, end = text.find(' ', 0); (string::npos != end) && (cursor > (int)end);
		       	begin = end+1, end = text.find(' ', begin))
	{
		params.push_back(text.substr(begin, end-begin));
	}
	if (string::npos != end) 
	{
		params.push_back(text.substr(begin, end-begin));
		rest_params = text.substr(end+1);
	}
	else
	{
		params.push_back(text.substr(begin));
	}

	/* if is completing the command */
	if (params.size() == 1)
	{
		command_completion(params[0]);
	}
	/* if is completing the param */
	else
	{
		//TODO
		if (commands[params[0]] == "category")
			category_completion(params.back(), params.size()-1);
	}

	/* regenerate the text string */
	text = "";
	for (vector<string>::iterator p = params.begin(); p != params.end(); p++)
	{
		text += *p + " ";
	}
	cursor = text.length()-1;
	if (rest_params.empty())
		text.erase(text.length()-1, 1);
	else
		text += rest_params;
	param = params.size()-1;
}

void CmdEditor::command_completion(string& com)
{
	/* if it is no the first time */
	if ((param == 0) &&
	    (com_search != commands.end()) &&
	    (com == com_search->first))
	{
		com_search++;
		if ((com_search != commands.end()) && 
		   (!cmp(com_search->first)))
		{
			com = com_search->first;
			cursor = com.length();
		}
		else
		{
			com = com.substr(0, length);
			com_search = com_first;
			cursor = length;
		}
	}
	/* if it is the first time */
	else
	{
		length = com.length();
		//TODO: try upper_bound
		for (com_search = commands.begin(); 
		    (com_search != commands.end()) && 
		    (cmp(com_search->first)); 
		    com_search++);
		if ((com_search != commands.end()) && 
		    (!cmp(com_search->first)))
		{
			com = com_search->first;
			com_first = com_search;
			cursor = com.length();
		}
	}
}

#define param_cmp(str) cat.compare(0, length, str, 0, length)
void CmdEditor::category_completion(string& cat, int num_param)
{

	/* if it is no the first time */
	if ((param == num_param) &&
	    (search != categories.end()) &&
	    (cat == *search))
	{
			search++;
			if ((search != categories.end()) && 
			   (!param_cmp(*search)))
			{
				cat = *search;
			}
			else
			{
				cat = cat.substr(0, length);
				search = first;
			}
	}
	/* if it is the first time */
	else
	{
		length = cat.length();
		for (search = categories.begin(); 
		    (search != categories.end()) && 
		    (param_cmp(*search)); 
		    search++);
		if ((search != categories.end()) && 
		    (!param_cmp(*search)))
		{
			cat = *search;
			first = search;
		}
	}
}

#define compare_command() text.compare(0, length, commands[command], length)
/*void CmdEditor::tab()
{
	//TODO: rewrite it
	string params("");
	size_t length;

	/* Get the command and params in text 
	if (string::npos == (length = text.find(' ', 0)))
		length = text.length();
	else
		params = text.substr(length);
	/* if it's exactly I gess is not the first press on tab 
	if ((command != -1) && (strlen(commands[command]) == length) && (!compare_command()))
		length = letters;

	/* Search for a command with the tiped letters 
	if ((command != -1) && (compare_command() > 0))
	{
		//for (command--; (command != -1) && (compare_command() <= 0); command--);
		//if (command < commands_length) command++;
	}
	else if (command < commands_length-1)
	{
		
		for (command++; (command < commands_length) && (compare_command() > 0); command++);
	}

	/* if exist a command display it 
	if (!compare_command())
	{
		text = commands[command];
		text += params;
		letters = length;
	}
}*/

void DateEditor::left()
{
	if (cursor>0) --cursor;
	if ((2==cursor) || (5==cursor)) --cursor;
}

void DateEditor::right()
{
	if (cursor<9) ++cursor;
	if ((2==cursor) || (5==cursor)) ++cursor;
}

void DateEditor::home()
{
	cursor = 0;
}

void DateEditor::end()
{
	cursor = 9;
}

void DateEditor::other()
{
	/* if is a number */
	if ((key >= 48) && (key <= 57))
	{
		text[cursor] = key;
		if (cursor<9) ++cursor;
		if ((2==cursor) || (5==cursor)) ++cursor;
	}
}


void PriorityEditor::up()
{
	if ((text != "N") && (text[cursor] > 49))
	{
		text[cursor]--;
	}
	else
	{
		text = "1";
	}
}

void PriorityEditor::down()
{
	if ((text != "N") && (text[cursor] < 53))
	{
		text[cursor]++;
	}
	else
	{
		text = "5";
	}
}

void PriorityEditor::backspace()
{
	text = "N";
}

void PriorityEditor::supr()
{
	text = "N";
}

void PriorityEditor::other()
{
	/* if is a number between 1 and 5 */
	if ((key >= 49) && (key <= 53))
	{
		text[cursor] = key;
	}
}
