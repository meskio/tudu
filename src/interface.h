
/**************************************************************************
 * Copyright (C) 2007-2012 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#ifndef INTERFACE_H
#define INTERFACE_H

#include "includes.h"
class Interface;
#include "data.h"
#include "sched.h"
#include "config.h"
#include "parser.h"
#include "editor.h"
#include "screen.h"
#include "cmd.h"

class Interface
{
public:
	Interface(Screen& s, iToDo &t, Sched& sch, Config &c, Writer &w, Cmd &com);
	~Interface();

	void main();
	friend class Cmd;
private:
	Screen &screen;
	iToDo &cursor;
	Sched &sched;
	Config &config;
	Writer &writer;
	pToDo copied;
	wstring sortOrder;
	wstring search_pattern;
	Cmd &cmd; /* command interface */
	set<wstring> hidden_categories;
	int cursor_line;

	void resizeTerm();
	void drawTodo();
	void eraseCursor();

	/*
	 *  calculate the position of the cursor on the screen
	 *  so if it was out of the screen it places it inside.
	 *  return true if the screen must be redrawn
	 */
	bool fitCursor();

	void drawCursor();
	bool isHide(iToDo& todo);
	void inherit();

	/* update cursor and cursor_line to previous entry on the screen */
	bool next();
	/* update cursor and cursor_line to next entry on the screen
	   always points to a valid line */
	bool prev();

	void left();
	void right();
	void up();
	void down();
	void prevPage();
	void nextPage();
	void home();
	void end();
	void move_up();
	void move_down();
	void done();
	void del();
	void delDeadline();
	void delPriority();
	void delSched();
	void paste();
	void pasteUp();
	void pasteChild();
	bool editLine(wstring& str);
	void editDeadline();
	void setPriority();
	void setCategory();
	void addLine();
	void addLineUp();
	void modifyLine();
	void editText();
	void editSched();
	void schedUp();
	void schedDown();
	void upText();
	void downText();
	void collapse();
	void hide_done();
	void command_line();
	bool _search();
	void search();
	void search_next();
	void search_prev();
	void sortByTitle();
	void sortByDone();
	void sortByDeadline();
	void sortByPriority();
	void sortByCategory();
	void sortByUser();
	void sortRevTitle();
	void sortRevDone();
	void sortRevDeadline();
	void sortRevPriority();
	void sortRevCategory();
	void sortRevUser();
	void save();
	void help();
};

#endif
