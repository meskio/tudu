
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

#ifndef SCREEN_H
#define SCREEN_H

#include "includes.h"
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
	void drawTitle(int line, int depth, wstring& title, int startLine=0);
	void drawText(Text &t);
	void drawSched(Sched &sched, pToDo cursor = NULL);
	void scrollUpText(Text &t);
	void scrollDownText(Text &t);
	void deadlineClear(int line);
	void priorityClear(int line);
	Editor::return_t editTitle(int line, int depth, bool haveChild, wstring& str, int cursorPos = -1);
	void editText(Text& t);
	Editor::return_t editDeadline(int line, Date& deadline, bool done, int cursorPos = -1);
	Editor::return_t editSched(Date& s, int cursorPos = -1);
	Editor::return_t setPriority(int line, int& priority);
	Editor::return_t setCategory(int line, wstring& category, int cursorPos = -1);
	void treeClear();
	int treeLines();
	/* number of lines the task needs on the screen */
	int taskLines(int depth, ToDo &t);
	Editor::return_t searchText(wstring& pattern, int cursorPos = -1);
	Editor::return_t cmd(wstring& command, int cursorPos = -1);
	bool confirmQuit();
	void infoMsg(const char str[]);
	void infoClear();
	void infoPercent(int percent);
	void helpPopUp(wstring str[], int len);
private:
	Window *whelp;
	Window *wtree;
	Window *wpriority;
	Window *wcategory;
	Window *wdeadline;
	Window *wtext;
	Window *winfo;
	Window *wschedule;
	vector<Window *> pipes;
	Config &config;
	TitleEditor titleEditor;
	CategoryEditor categoryEditor;
	DateEditor dateEditor;
	PriorityEditor priorityEditor;
	HistoryEditor searchEditor;
	CmdEditor cmdEditor;
	windows_defs coor;

	void draw_helpbar(window_coor c);
	void draw();
	wstring date2str(Date& date);
	Date str2date(wstring str);

	/* col where the title text starts */
	int startTitle(int depth);
};

#endif
