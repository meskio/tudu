
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

#ifndef SCREEN_H
#define SCREEN_H

#include "defs.h"
#include "window.h"
#include "text.h"
#include "data.h"
#include "config.h"
#include "editor.h"
#include "sched.h"

class Screen
{
public:
	Screen(Config& c);
	~Screen();

	void resizeTerm();
	void drawTask(int line, int depth, ToDo& t, bool isCursor=false);
	void drawText(Text &t);
	void drawSched(Sched &sched, pToDo cursor = NULL);
	void scrollUpText(Text &t);
	void scrollDownText(Text &t);
	void deadlineClear(int line);
	void priorityClear(int line);
	bool editTitle(int line, int depth, bool haveChild, wstring& str);
	void editText(Text& t);
	void editDeadline(int line, Date& deadline, bool done);
	bool editSched(Date& s);
	void setPriority(int line, int& priority);
	void setCategory(int line, ToDo& t);
	void treeClear();
	int treeLines();
	bool searchText(wstring& pattern);
	bool cmd(wstring& command);
	bool confirmQuit();
	void infoMsg(const char str[]);
	void infoClear();
	void infoPercent(int percent);
	void helpPopUp(string str[], int len);
private:
	Window *whelp;
	Window *wtree;
	Window *wpriority;
	Window *wcategory;
	Window *wdeadline;
	Window *wtext;
	Window *winfo;
	Window *wschedule;
	Config &config;
	LineEditor lineEditor;
	CategoryEditor categoryEditor;
	DateEditor dateEditor;
	PriorityEditor priorityEditor;
	HistoryEditor searchEditor;
	CmdEditor cmdEditor;
	window_coor coor[NUM_WINDOWS];

	void draw();
};

#endif
