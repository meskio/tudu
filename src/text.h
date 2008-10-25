
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

#ifndef TEXT_H
#define TEXT_H

#include <ncurses.h>
#include <list>
#include <iostream>
#include <string>
#include "window.h"

class Text
{
public:
	Text& operator=(const string& str);
	bool operator!=(const string& str);
	friend ostream& operator<<(ostream& os, Text& t);
	void print(Window& win);
	void edit(Window& win);
	string getStr();
	void scroll_up(Window& win);
	void scroll_down(Window& win);
private:
	list<string> text; /* list of lines */
	int cursor_col;
	int cursor_y;
	list<string>::iterator cursor_line;
	unsigned int offset;
	int lines, cols;

	/* begin=line for start, length=num of screen lines to fit*/
	string _getStr(unsigned int begin = 0, int length = 0);
	void _scroll_up();
	void _scroll_down();
	void left();
	void right();
	void up();
	void down();
	void backspace();
	void supr();
	void home();
	void end();
	void new_line();
};

ostream& operator<<(ostream& os, Text& t);

#endif
