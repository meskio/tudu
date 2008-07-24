
/**************************************************************************
 * Copyright (C) 2007 Ruben Pollan Bella <meskio@amedias.org>             *
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

	exit = false;
	win._move(y, x);
	win._addstr(text);
	win._move(y, x+cursor);
	echo();
	curs_set(1);
	win._refresh();
	key = win._getch();
	while ('\n' != key)
	{
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
			case KEY_BACKSPACE: backspace();
				break;
			case KEY_DC: supr();
				break;
			case '\e': esc();
				break;
			default: other();
				break;
		}
		if (max_length <= text.length())
			text.erase(max_length);
		if (cursor >= max_length)
			cursor = max_length-1;
		win._move(y, x);
		win._addstr(text);
		for (unsigned int i = text.length(); i < max_length; i++) 
				win._addch(' ');
		win._move(y, x+cursor);
		win._refresh();
		if (exit) break;
		key = win._getch();
	}
	noecho();
	curs_set(0);
	win._refresh();

	if (resized) ungetch(KEY_RESIZE);
	return !exit;
}

void Editor::left() {}
void Editor::right() {}
void Editor::up() {}
void Editor::down() {}
void Editor::backspace() {}
void Editor::supr() {}
void Editor::esc() {}
void Editor::other() {}



void LineEditor::left()
{
	if (cursor>0) --cursor;
}

void LineEditor::right()
{
	if (cursor<(int)text.length()) ++cursor;
}

void LineEditor::backspace()
{
	if (cursor>0) text.erase(--cursor, 1);
}

void LineEditor::supr()
{
	if (cursor<(int)text.length()) text.erase(cursor, 1);
}

void LineEditor::esc()
{
	exit = true;
}

void LineEditor::other()
{
	if (key < 256)
	{
		text.insert(cursor,1,key);
		++cursor;
	}
}

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

void DateEditor::esc()
{
	exit = true;
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

void PriorityEditor::esc()
{
	exit = true;
}

void PriorityEditor::other()
{
	/* if is a number between 1 and 5 */
	if ((key >= 49) && (key <= 53))
	{
		text[cursor] = key;
	}
}
