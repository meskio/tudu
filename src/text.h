
/**************************************************************************
 * Copyright (C) 2007-2011 Ruben Pollan Bella <meskio@sindominio.net>     *
 *                                                                        *
 *  This file is part of TuDu.                                            *
 *                                                                        *
 *  TuDu is free software; you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; version 3 of the License.        *
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

#include "includes.h"
#include "window.h"

class Text
{
public:
	Text& operator=(const wstring& str);
	bool operator!=(const wstring& str);
	void print(Window& win);
	void edit(Window& win);
	wstring getStr();
	void scroll_up(Window& win);
	void scroll_down(Window& win);
private:
	list<wstring> text; /* list of lines */
	int cursor_col;
	int cursor_y; /* = INT_MIN when is no editing */
	list<wstring>::iterator cursor_line;
	list<wstring>::iterator offset; /* line for start to display */
	int lines, cols;

	/* begin=line for start, length=num of screen lines to fit*/
	wstring _getStr(list<wstring>::iterator begin, int length = 0);
	bool _scroll_up();
	bool _scroll_down();
	void left();
	void right();
	void up();
	void down();
	void backspace();
	void supr();
	void home();
	void end();
	void next_page();
	void prev_page();
	void new_line();
	void tab();
};

wostream& operator<<(wostream& os, Text& t);
wistream& operator>>(wistream& is, Text& t);

#endif
