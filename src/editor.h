
/**************************************************************************
 * Copyright (C) 2007-2010 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#ifndef EDITOR_H
#define EDITOR_H

#include "defs.h"
#include "data.h"
#include "window.h"

class Editor
{
public:
	/* return values for the editor */
	enum return_t {
		NOT_SAVED = 0,
		SAVED,
		/* In case of changes that need to redraw or resize the screen.
		 * After update the screen the function edit should be call again. */
		RESIZE,
		REDRAW
	};

	Editor();

	wstring& getText();
	int& cursorPos();
	return_t edit(Window& win, int begin_y, int begin_x, int ncols);
protected:
	Window *window;
	int y;
	int x;
	unsigned int cols;
	wstring text;
	int cursor;
	wint_t key;
	bool exit;
	return_t result;

	virtual void initialize();
	virtual void updateText();
	virtual void left();
	virtual void right();
	virtual void up();
	virtual void down();
	virtual void home();
	virtual void end();
	virtual void backspace();
	virtual void supr();
	virtual void tab();
	virtual void other();
	virtual void enter();
	virtual void esc();
};

class LineEditor: public Editor
{
protected:
	void updateText();
	void left();
	void right();
	void home();
	void end();
	void backspace();
	void supr();
	void tab();
	void other();
};

class TitleEditor: public LineEditor
{
protected:
	int textLines;

	void initialize();
	void updateText();
	void up();
	void down();

	unsigned int cursorLine();
	unsigned int cursorCol();
};

class CategoryEditor: public LineEditor
{
public:
	CategoryEditor();
protected:
	set<wstring>::iterator search;
	set<wstring>::iterator first;
	int length;

	void tab();
};

class HistoryEditor: public LineEditor
{
protected:
	list<wstring> history;
	list<wstring>::iterator shown;

	void initialize();
	void up();
	void down();
	void enter();
	void backspace();
};

class CmdEditor: public HistoryEditor
{
protected:
	map<wstring,wstring>::iterator com_search;
	map<wstring,wstring>::iterator com_first;
	set<wstring>::iterator search;
	set<wstring>::iterator first;
	int length;
	int param;

	void initialize();
	void tab();
	void command_completion(wstring& com);
	void category_completion(wstring& cat, int num_param);
};

class DateEditor: public Editor
{
public:
	return_t edit(Window& win, int begin_y, int begin_x);
protected:
	void updateText();
	void left();
	void right();
	void home();
	void end();
	void other();
};

class PriorityEditor: public Editor
{
public:
	return_t edit(Window& win, int begin_y, int begin_x);
protected:
	void updateText();
	void up();
	void down();
	void backspace();
	void supr();
	void other();
};

#endif
