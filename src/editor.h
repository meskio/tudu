
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
#include <string.h>
#include <list>
#include <set>
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
	bool result;

	virtual void initialize();
	virtual void left();
	virtual void right();
	virtual void up();
	virtual void down();
	virtual void home();
	virtual void end();
	virtual void backspace();
	virtual void supr();
	virtual void other();
	virtual void enter();
	virtual void esc();
};

class LineEditor: public Editor
{
protected:
	void left();
	void right();
	void home();
	void end();
	void backspace();
	void supr();
	void other();
};

class CategoryEditor: public LineEditor
{
protected:
	set<string>::iterator search;
	set<string>::iterator first;
	int length;

	void initialize();
	void other();
	void completion();
};

class HistoryEditor: public LineEditor
{
protected:
	list<string> history;
	list<string>::iterator shown;

	void initialize();
	void up();
	void down();
	void enter();
};

class CmdEditor: public HistoryEditor
{
protected:
	set<string>::iterator search;
	set<string>::iterator first;
	int length;

	void initialize();
	void tab();
	void back_tab();
};

class DateEditor: public Editor
{
protected:
	void left();
	void right();
	void home();
	void end();
	void other();
};

class PriorityEditor: public Editor
{
protected:
	void up();
	void down();
	void backspace();
	void supr();
	void other();
};

#endif
