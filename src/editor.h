
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

#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "window.h"

class Editor
{
public:
	Editor();

	string& getText();
	int& cursorPos();
	bool edit(Window& win, int y, int x, unsigned int max_length);
protected:
	string text;
	int cursor;
	int key;
	bool exit;

	virtual void left();
	virtual void right();
	virtual void up();
	virtual void down();
	virtual void backspace();
	virtual void supr();
	virtual void esc();
	virtual void other();
};

class LineEditor: public Editor
{
protected:
	void left();
	void right();
	void backspace();
	void supr();
	void esc();
	void other();
};

class DateEditor: public Editor
{
protected:
	void left();
	void right();
	void esc();
	void other();
};

class PriorityEditor: public Editor
{
protected:
	void up();
	void down();
	void backspace();
	void supr();
	void esc();
	void other();
};

#endif
