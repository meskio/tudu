
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

#include "text.h"
#include "interface.h"

#define cursor_x (cursor_col % cols)
#define rows_in_line(line) ((line->length() / cols) + 1)

Text& Text::operator=(const string& str)
{
	unsigned int i = 0, size;
	
	/* build a list with each line of the text in an entry */
	text.clear();
	while (i < str.length())
	{
		for (size = 0; (i+size < str.length()) && 
				('\n' != str[i+size]); ++size);
		text.push_back(str.substr(i, size));
		i += size+1;
	}
	if (text.empty()) /* if is empty it makes a sigfault at edit */
		text.push_back("");
	
	/* set up the cursor */
	cursor_col = 0; cursor_y = 0;
	cursor_line = text.begin();

	offset = 0;

	return *this;
}

bool Text::operator!=(const string& str)
{
	return (str != getStr());
}

void Text::print(Window& win)
{
	lines = win._lines();
	cols = win._cols();
	string str = _getStr(offset, lines);

	win._erase();
	win._addstr(0,0,str);
	win._refresh();
}

void Text::edit(Window& win)
{
	bool resized = false;

	lines = win._lines();
	cols = win._cols();
	win._move(cursor_y, cursor_x);
	echo();
	curs_set(1);
	win._refresh();
	
	int key = win._getch();
	while ('\e' != key) {
		switch (key)
		{
			case KEY_RESIZE:
				resized = true;
				break;
			case KEY_LEFT: left();
				break;
			case KEY_RIGHT: right();
				break;
			case KEY_DOWN: down();
				break;
			case KEY_UP: up();
				break;
			case KEY_HOME: home();
				break;
			case KEY_END: end();
				break;
			case KEY_BACKSPACE: backspace();
				break;
			case KEY_DC: supr();
				break;
			case '\n':
			case KEY_ENTER: new_line();
				break;
			default: 
				if (key < 256)
				{
					cursor_line->insert(cursor_col,1,key);
					++cursor_col;
					if (0 == cursor_x) ++cursor_y;
				}
				break;
		}

		string str = _getStr(offset, lines);
		win._erase();
		win._addstr(0,0,str);
		win._move(cursor_y, cursor_x);
		win._refresh();
		key = win._getch();
	}
	noecho();
	curs_set(0);
	print(win);

	if (resized) ungetch(KEY_RESIZE);
	return;
}

string Text::getStr()
{
	string s = "";

	for (list<string>::iterator i = text.begin(); i != text.end(); ++i)
	{
		s += *i;
		s += '\n';
	}
	return s;
}

string Text::_getStr(unsigned int begin, int length)
{
	int rows = 0;
	string s = "";
	list<string>::iterator i = text.begin();

	if (begin >= text.size()) return s;
	for (unsigned int line = 0; line < begin; line++) ++i;
	for (;i != text.end(); ++i)
	{
		if (length)
		{
			int rows_line = rows_in_line(i);
			if (rows+rows_line > length)
			{
				for (int row = 0; row < length-rows; ++row)
				{
					s += i->substr(row*cols, cols);
				}
				s += '\n';
				break;
			}
			rows += rows_line;
		}
		s += *i;
		s += '\n';
		if ((length) && (rows == length)) break;
	}
	s.erase(s.length()-1); /* del the last '\n' */
	return s;
}

void Text::scroll_up(Window& win)
{
	lines = win._lines();
	cols = win._cols();
	_scroll_up();
	print(win);
}

void Text::scroll_down(Window& win)
{
	lines = win._lines();
	cols = win._cols();
	_scroll_down();
	print(win);
}

void Text::_scroll_up()
{
	if (0<offset)
	{
		list<string>::iterator i = text.begin();
		--offset;
		for (unsigned int line = 0; line < offset; ++line) ++i;

		cursor_y += rows_in_line(i);
		while(cursor_y >= lines)
		{
			cursor_y -= rows_in_line(cursor_line);
			--cursor_line;
			if ((int)cursor_line->length() < cursor_col)
				cursor_col = cursor_line->length();
		}
	}
}

void Text::_scroll_down()
{
	if (text.size()-1 > offset)
	{
		list<string>::iterator i = text.begin();
		for (unsigned int line = 0; line < offset; ++line) ++i;

		++offset;
		cursor_y -= rows_in_line(i);
		if (cursor_y < 0)
		{
			cursor_y = 0;
			++cursor_line;
			if (cursor_x < (int)cursor_line->length()) 
				cursor_col = cursor_x;
			else
				cursor_col = cursor_line->length();
		}
	}
}

void Text::left()
{
	if (cursor_col>0)
	{
		if (0 == cursor_x) --cursor_y;
		--cursor_col;
	}
	else if (cursor_line != text.begin())
	{
		--cursor_y;
		--cursor_line;
		cursor_col = cursor_line->length();
	}
	if (cursor_y < 0) _scroll_up();
}

void Text::right()
{
	if (cursor_col < (int)cursor_line->length())
	{
		++cursor_col;
		if (0 == cursor_x) ++cursor_y;
	}
	else if (cursor_line != --text.end())
	{
		cursor_col = 0;
		++cursor_y;
		++cursor_line;
	}
	if (cursor_y >= lines) _scroll_down();
}

#define last_line() ((int)rows_in_line(cursor_line)-1 == cursor_col / cols)
void Text::down()
{
	if (last_line())
	{
		if (cursor_line != --text.end())
		{
			++cursor_y;
			++cursor_line;
			cursor_col = cursor_x;
		}
	}
	else
	{
		++cursor_y;
		cursor_col += cols;
	}

	if (cursor_col > (int)cursor_line->length())
	{
		cursor_col = cursor_line->length();
	}
	if (cursor_y >= lines) _scroll_down();
}

void Text::up()
{
	if ((cursor_col > cols) || (cursor_line != text.begin()))
	{
		--cursor_y;

		if (cursor_col < cols)
		{
			--cursor_line;
			if (cursor_x < (int)cursor_line->length() % cols)
				cursor_col = cursor_x + (rows_in_line(cursor_line)-1) * cols;
			else
				cursor_col = cursor_line->length();
		}
		else
		{
			cursor_col -= cols;
		}

		if (cursor_y < 0) _scroll_up();
	}
}

void Text::backspace()
{
	if (cursor_col > 0) 
	{
		if (0 == cursor_x) --cursor_y;
		if (cursor_y < 0) _scroll_up();
		--cursor_col;
		cursor_line->erase(cursor_col, 1);
	}
	else if (cursor_line != text.begin()) /* delete line break */
	{
		list<string>::iterator i = cursor_line;

		--cursor_y;
		--cursor_line;
		cursor_col = cursor_line->length();
		if (cursor_y < 0) _scroll_up();
		*cursor_line += *i;
		text.erase(i);
	}
}

void Text::supr()
{
	if ((cursor_col == (int)cursor_line->length()) &&  (cursor_line != --text.end()))
	{
		list<string>::iterator i = cursor_line;

		++i;
		*cursor_line += *i;
		text.erase(i);
	}
	else
	{
		cursor_line->erase(cursor_col, 1);
	}
	if (cursor_y >= lines) _scroll_down();
}

void Text::home()
{
	for (int i = 0; i < cursor_col/cols; --cursor_y, ++i);
	cursor_col = 0;
	if (cursor_y >= lines) _scroll_down();
}

void Text::end()
{
	for (int i = 0; i < ((int)cursor_line->length()-cursor_col)/cols; 
			++cursor_y, ++i);
	cursor_col = cursor_line->length();
	if (cursor_y >= lines) _scroll_down();
}

void Text::new_line()
{
	list<string>::iterator i = cursor_line;

	++i;
	text.insert(i, cursor_line->substr(cursor_col));
	cursor_line->erase(cursor_col);
	++cursor_y;
	++cursor_line;
	cursor_col = 0;
	if (cursor_y >= lines) _scroll_down();
}

ostream& operator<<(ostream& os, Text& t)
{
	os << t.getStr();
	return os;
}
