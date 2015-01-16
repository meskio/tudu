
/*************************************************************************
 * Copyright (C) 2007-2015 Ruben Pollan Bella <meskio@sindominio.net>    *
 *                                                                       *
 *  This file is part of TuDu.                                           *
 *                                                                       *
 *  TuDu is free software; you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation; version 3 of the License.       *
 *                                                                       *
 *  TuDu is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "screen.h"

#define COLOR_SELECTED COLOR_PAIR(CT_SELECTED)
#define COLOR_WARN     COLOR_PAIR(CT_WARN)
#define COLOR_HELP     COLOR_PAIR(CT_HELP)
#define COLOR_TREE     COLOR_PAIR(CT_TREE)
#define COLOR_TEXT     COLOR_PAIR(CT_TEXT)
#define COLOR_INFO     COLOR_PAIR(CT_INFO)
#define COLOR_SCHED    COLOR_PAIR(CT_SCHEDULE)
#define COLOR_PIPE     COLOR_PAIR(CT_PIPE)

#define PERCENT_COL (coor.coor[WINFO].cols-7)

#define deadline_close(deadline) ((deadline.valid()) && \
		(deadline.daysLeft() <= config.getDaysWarn()))
#define isCollapse(todo) ((todo.getCollapse()) && (!todo.actCollapse()))

Screen::Screen(Config &c): config(c)
{
	/*
	 * Initialize ncurses
	 */
	setlocale(LC_ALL, "");
	initscr ();
	if (has_colors())
	{
		start_color();
		if (can_change_color())
		{
			color_t* color_list;
			short int length;
			config.getColorList(&color_list, length);
			for (short int i=0; i<length; ++i)
			{
				color_t& c = color_list[i];
				init_color(c.color, c.red, c.green, c.blue);
			}
		}
		if (COLOR_PAIRS > NUM_CT)
		{
			short int foreground, background;

			use_default_colors();
			config.getColorPair(CT_DEFAULT, foreground, background);
			assume_default_colors(foreground, background);
			for (short int i=1; i<NUM_CT; ++i)
			{
				config.getColorPair(i, foreground, background);
				init_pair(i,foreground,background);
			}
		}
	}

	clear ();
	noecho();
	keypad(stdscr, TRUE);
	cbreak();
	curs_set(0);
	refresh();

	draw();
}

Screen::~Screen()
{
	delete whelp;
	delete wtree;
	delete wpriority;
	delete wcategory;
	delete wdeadline;
	delete wtext;
	delete winfo;
	delete wschedule;
	pipes.clear();
	clear ();
	refresh ();
	endwin ();
}

void Screen::draw_helpbar(window_coor c)
{
	for (int i = 0; i<c.cols; ++i)
		whelp->_addch(' ');
	action_list list;
	config.getActionList(list);
	wstring help_bar = L" " + list[L"quit"] + L":quit  " + \
		list[L"up"]   + L":up  "   + list[L"down"]    + L":down  " + \
		list[L"out"]  + L":out  "  + list[L"in"]      + L":in  " + \
		list[L"done"] + L":done  " + list[L"addTodo"] + L":add  " + \
		list[L"editTitle"] + L":modify";
	whelp->_move(0,0);
	whelp->_addstr(help_bar);
	whelp->_move(0, c.cols-8);
	wstring more_help = list[L"help"] + L":help";
	whelp->_addstr(more_help);
	whelp->_refresh();
}

void Screen::draw()
{
	int lines, cols;

	/* get size of windows */
	getmaxyx(stdscr, lines, cols); 
	config.genWindowCoor(lines, cols, coor);

	/* create windows */
	wtree = new Window(coor.coor[WTREE]);
	if (coor.exist[WHELP])
	{
		whelp = new Window(coor.coor[WHELP]);
		whelp->_attron(COLOR_HELP);
		draw_helpbar(coor.coor[WHELP]);
	}
	else
		whelp = NULL;

	if (coor.exist[WPRIORITY])
		wpriority = new Window(coor.coor[WPRIORITY]);
	else
		wpriority = NULL;
	if (coor.exist[WCATEGORY])
		wcategory = new Window(coor.coor[WCATEGORY]);
	else
		wcategory = NULL;
	if (coor.exist[WDEADLINE])
		wdeadline = new Window(coor.coor[WDEADLINE]);
	else
		wdeadline = NULL;
	if (coor.exist[WTEXT])
		wtext = new Window(coor.coor[WTEXT]);
	else
		wtext = NULL;
	if (coor.exist[WINFO])
		winfo = new Window(coor.coor[WINFO]);
	else
		winfo = NULL;
	if (coor.exist[WSCHEDULE])
		wschedule = new Window(coor.coor[WSCHEDULE]);
	else
		wschedule = NULL;

	/* draw pipes */
	for (vector<window_coor>::iterator i = coor.vpipe.begin();
		 i != coor.vpipe.end(); i++)
	{
		pipes.push_back(new Window(*i));
		pipes.back()->_attron(COLOR_PIPE);
		pipes.back()->_move(0,0);
		pipes.back()->_vline('|', i->lines);
		pipes.back()->_refresh();
	}
	for (vector<window_coor>::iterator i = coor.hpipe.begin();
		 i != coor.hpipe.end(); i++)
	{
		pipes.push_back(new Window(*i));
		pipes.back()->_attron(COLOR_PIPE);
		pipes.back()->_move(0,0);
		pipes.back()->_hline('-', i->cols);
		pipes.back()->_refresh();
	}
}

wstring Screen::date2str(Date& date)
{
	wchar_t str[11];

	if (config.useUSDates())
	{
		swprintf(str, 11, L"%2d/%2d/%4d", date.month(), date.day(), date.year());
	}
	else 
	{
		swprintf(str, 11, L"%2d/%2d/%4d", date.day(), date.month(), date.year());
	}
	
	return wstring(str);
}

Date Screen::str2date(wstring str)
{
	string date(str.length(), L' ');
	copy(str.begin(), str.end(), date.begin());

	int day, month;
	int year = atoi(date.substr(6,4).c_str());
	if (config.useUSDates())
	{
		month = atoi(date.substr(0,2).c_str());
		day = atoi(date.substr(3,2).c_str());
	}
	else
	{
		day = atoi(date.substr(0,2).c_str());
		month = atoi(date.substr(3,2).c_str());
	}
	Date d(day, month, year);

	return d;
}

void Screen::resizeTerm()
{
	int lines, cols;

	/* get new size of windows */
	endwin();
	refresh();
	getmaxyx(stdscr, lines, cols);
	config.genWindowCoor(lines, cols, coor);

	/* 
	 * resize windows
	 */
	/* clean stdscr */
	touchwin(stdscr);
	refresh();

	/* tree */
	window_coor c = coor.coor[WTREE];
	wtree->_resize(c.lines, c.cols);
	wtree->_mv(c.y, c.x);
	wtree->_erase();

	/* help */
	c = coor.coor[WHELP];
	if (coor.exist[WHELP])
	{
		whelp->_resize(c.lines, c.cols);
		whelp->_mv(c.y, c.x);
		whelp->_erase();
		whelp->_move(0,0);
		draw_helpbar(c);
	}

	/* priority */
	c = coor.coor[WPRIORITY];
	if (coor.exist[WPRIORITY])
	{
		wpriority->_resize(c.lines, c.cols);
		wpriority->_mv(c.y, c.x);
		wpriority->_erase();
	}

	/* category */
	c = coor.coor[WCATEGORY];
	if (coor.exist[WCATEGORY])
	{
		wcategory->_resize(c.lines, c.cols);
		wcategory->_mv(c.y, c.x);
		wcategory->_erase();
	}

	/* deadline */
	c = coor.coor[WDEADLINE];
	if (coor.exist[WDEADLINE])
	{
		wdeadline->_resize(c.lines, c.cols);
		wdeadline->_mv(c.y, c.x);
		wdeadline->_erase();
	}

	/* deadline */
	c = coor.coor[WTEXT];
	if (coor.exist[WTEXT])
	{
		wtext->_resize(c.lines, c.cols);
		wtext->_mv(c.y, c.x);
		wtext->_erase();
	}

	/* info */
	c = coor.coor[WINFO];
	if (coor.exist[WINFO])
	{
		winfo->_resize(c.lines, c.cols);
		winfo->_mv(c.y, c.x);
		winfo->_erase();
	}

	/* schedule */
	c = coor.coor[WSCHEDULE];
	if (coor.exist[WSCHEDULE])
	{
		wschedule->_resize(c.lines, c.cols);
		wschedule->_mv(c.y, c.x);
		wschedule->_erase();
	}

	/* draw pipes */
	vector<Window *>::iterator win = pipes.begin();
	for (vector<window_coor>::iterator i = coor.vpipe.begin();
		 i != coor.vpipe.end(); i++)
	{
		(*win)->_resize(i->lines, i->cols);
		(*win)->_mv(i->y, i->x);
		(*win)->_erase();
		(*win)->_move(0,0);
		(*win)->_vline('|', i->lines);
		(*win)->_refresh();

		win++;
	}
	for (vector<window_coor>::iterator i = coor.hpipe.begin();
		 i != coor.hpipe.end(); i++)
	{
		(*win)->_resize(i->lines, i->cols);
		(*win)->_mv(i->y, i->x);
		(*win)->_erase();
		(*win)->_move(0,0);
		(*win)->_hline('-', i->cols);
		(*win)->_refresh();

		win++;
	}
}

int Screen::startTitle(int depth)
{
	return depth * 4 + 7;
}

void Screen::drawTitle(int line, int depth, wstring& title, int startLine)
{
	int lines, cols;
	wtree->_getmaxyx(lines, cols);
	const unsigned int title_cols = wcswidth(title.c_str(), string::npos);
	const unsigned int line_cols = cols-startTitle(depth);
	const unsigned int title_lines = (title_cols && !(title_cols % line_cols)) ? (title_cols / line_cols)-1
					: (title_cols / line_cols);

	int c = 0;
	for (unsigned int i = 0; i <= title_lines; i++)
	{
		if ((int)(line + i) >= lines) break;

		if ((int)i < startLine) {
			for (unsigned int col = 0; col < line_cols; c++) {
				col += wcwidth(title[c]);
			}
			continue;
		}

		wstring titleLine = title.substr(c, line_cols);
		c += wtree->_addstr(line + i, startTitle(depth), titleLine, line_cols);
	}
}

void Screen::drawTask(int line, int depth, ToDo& t, bool isCursor)
{
	child_info chinf = t.getChildInfo();

	if (isCursor)
		wtree->_attron(COLOR_SELECTED);
	else
		wtree->_attron(COLOR_TREE);
	wtree->_move(line, 0);

	/* identation */
	for (int i = 0; i < depth; ++i) wtree->_addstr(L"    ");
	if (config.getVisualTree())
		if (t.haveChild())
		{
			if ((!t.actCollapse()) && (t.getCollapse()))
				wtree->_addstr("(+)");
			else
				wtree->_addstr("(-)");
		}
		else
			wtree->_addstr("   ");
	else
		wtree->_addstr("   ");

	if (t.done())
	{
		wtree->_addstr("[X] ");
	}
	else if (!t.haveChild() || config.getHidePercent())
	{
		wtree->_addstr("[ ] ");
	}
	else if (100 == chinf.percent)
	{
		wtree->_addstr("100 ");
	}
	else
	{
		char str[8];
		sprintf(str, "%2d%% ", chinf.percent);
		wtree->_addstr(str);
	}
	/* add the title split to the length of screen */
	if ((t.haveChild()) && (config.getBoldParent()))
		wtree->_attron(A_BOLD);
	wstring title = t.getTitle();
	drawTitle(line, depth, title);
	if ((t.haveChild()) && (config.getBoldParent()))
		wtree->_attroff(A_BOLD);
	if (isCursor)
		wtree->_attroff(COLOR_SELECTED);
	else
		wtree->_attroff(COLOR_TREE);
	wtree->_refresh();

	/* draw priority */
	if (coor.exist[WPRIORITY])
	{
		int priority = t.priority();
		if (isCursor)
			wpriority->_attron(COLOR_SELECTED);
		else
			wpriority->_attron(COLOR_TREE);
		if (priority)
		{
			char s[11];
			wpriority->_move(line, 0);
			sprintf(s, "%01d", priority);
			wpriority->_addstr(s);
		}
		if (isCursor)
			wpriority->_attroff(COLOR_SELECTED);
		else
			wpriority->_attroff(COLOR_TREE);
		wpriority->_refresh();
	}

	/* draw category */
	if (coor.exist[WCATEGORY])
	{
		wstring category = t.getCategoriesStr();
		if (isCursor)
			wcategory->_attron(COLOR_SELECTED);
		else
			wcategory->_attron(COLOR_TREE);
		if (!category.empty())
		{
			wcategory->_move(line, 0);
			wcategory->_addstr(category, config.getCategoryLength());
		}
		if (isCursor)
			wcategory->_attroff(COLOR_SELECTED);
		else
			wcategory->_attroff(COLOR_TREE);
		wcategory->_refresh();
	}

	/* draw date */
	if (coor.exist[WDEADLINE])
	{
		if (isCursor)
			wdeadline->_attron(COLOR_SELECTED);
		else
			wdeadline->_attron(COLOR_TREE);
		Date& deadline = t.deadline();
		if (deadline.valid())
		{
			wdeadline->_move(line, 0);
			wstring str = date2str(deadline);
			wdeadline->_addstr(str);
		}
		wdeadline->_move(line, 10);
		if ((!t.done() && deadline_close(deadline)) || (isCollapse(t) && deadline_close(chinf.deadline)))
		{
			wdeadline->_attron(COLOR_WARN);
			wdeadline->_addstr("<-");
			wdeadline->_attroff(COLOR_WARN);
		}
		else
		{
			wdeadline->_addstr("  ");
		}
		if (isCursor)
			wdeadline->_attroff(COLOR_SELECTED);
		else
			wdeadline->_attroff(COLOR_TREE);
		wdeadline->_refresh();
	}
}

void Screen::drawText(Text &t)
{
	if (!coor.exist[WTEXT])
		return;

	wtree->_attron(COLOR_TEXT);
	wtext->_erase();
	t.print(*wtext);
	wtree->_attroff(COLOR_TEXT);
}

void Screen::drawSched(Sched &sched, pToDo cursor)
{
	if (!coor.exist[WSCHEDULE])
		return;

	Date today;
	today.setToday();
	sched_l sched_list;
	if (config.getOldSched())
		sched.get(sched_list);
	else
		sched.get(today,sched_list);

	Date last;
	int line = 0;
	wschedule->_erase();
	wschedule->_move(0,0);
	for (sched_l::iterator i = sched_list.begin(); 
			(i != sched_list.end()) && (line < coor.coor[WSCHEDULE].lines); i++)
	{
		if (today > (*i)->sched())
			wschedule->_attron(COLOR_WARN);
		else
			wschedule->_attron(COLOR_SCHED);
		if ((*i)->done()) continue;
		if ((*i)->sched() != last)
		{
			if (line+2 > coor.coor[WSCHEDULE].lines) break;
			last = (*i)->sched();

			wostringstream ss;
			ss  << "  " << date2str(last) << endl;
			wstring str = ss.str();
			wschedule->_attron(A_BOLD);
			wschedule->_addstr(str);
			wschedule->_attroff(A_BOLD);
			line++;
		}
		if (cursor == (*i))
			wschedule->_attron(COLOR_SELECTED);
		wschedule->_addstr("    ");
		wstring title = (*i)->getTitle();
		int printed_chars = wschedule->_addstr(title, coor.coor[WSCHEDULE].cols-4);
		if ((int)title.length() == printed_chars)
			wschedule->_addstr("\n");
		line++;
	}
	wschedule->_refresh();
}

void Screen::scrollUpText(Text& t)
{
	if (!coor.exist[WTEXT])
		return;

	wtree->_attron(COLOR_TEXT);
	t.scroll_up(*wtext);
	wtree->_attroff(COLOR_TEXT);
}

void Screen::scrollDownText(Text& t)
{
	if (!coor.exist[WTEXT])
		return;

	wtree->_attron(COLOR_TEXT);
	t.scroll_down(*wtext);
	wtree->_attroff(COLOR_TEXT);
}

void Screen::deadlineClear(int line)
{
	if (!coor.exist[WDEADLINE])
		return;

	wdeadline->_attron(COLOR_TREE);
	wdeadline->_move(line, 0);
	wdeadline->_addstr("            ");
	wdeadline->_refresh();
	wdeadline->_attroff(COLOR_TREE);
}

void Screen::priorityClear(int line)
{
	if (coor.exist[WPRIORITY])
	{
		wpriority->_attron(COLOR_TREE);
		wpriority->_move(line, 0);
		wpriority->_addstr(" ");
		wpriority->_refresh();
		wpriority->_attroff(COLOR_TREE);
	}
}

Editor::return_t Screen::editTitle(int line, int depth, bool haveChild, wstring& str, int cursorPos)
{
	Editor::return_t save;

	wtree->_attron(COLOR_SELECTED);
	if ((haveChild) && (config.getBoldParent()))
		wtree->_attron(A_BOLD);
	titleEditor.getText() = str;
	if (cursorPos >= 0)
		titleEditor.cursorPos() = cursorPos;
	save = titleEditor.edit(*wtree, line, startTitle(depth),
			coor.coor[WTREE].cols-startTitle(depth));
	if (save == Editor::NOT_SAVED)
	{
		drawTitle(line, depth, str);
		for (int i = startTitle(depth) + (str.length()-1 % coor.coor[WTREE].cols);
		     i < coor.coor[WTREE].cols-1; i++) 
				wtree->_addch(' ');
		wtree->_refresh();
	}
	str = titleEditor.getText(); 
	if ((haveChild) && (config.getBoldParent()))
		wtree->_attroff(A_BOLD);
	wtree->_attroff(COLOR_SELECTED);
	return save;
}

void Screen::editText(Text& t)
{
	if (!coor.exist[WTEXT])
		return;

	wtree->_attron(COLOR_TEXT);
	t.edit(*wtext);
	wtree->_attroff(COLOR_TEXT);
}

Editor::return_t Screen::editDeadline(int line, Date& deadline, bool done, int cursorPos)
{
	if (!coor.exist[WDEADLINE])
		return Editor::NOT_SAVED;

	Editor::return_t save;
	wstring date;

	if (deadline.valid())
	{
		date = date2str(deadline);
	}
	else
	{
		Date d;
		d.setToday();
		date = date2str(d);
	}
	wdeadline->_attron(COLOR_SELECTED);
	dateEditor.getText() = date;
	if (cursorPos >= 0)
		dateEditor.cursorPos() = cursorPos;
	save = dateEditor.edit(*wdeadline, line, 0);
	deadline = str2date(dateEditor.getText());
	wdeadline->_attroff(COLOR_SELECTED);
	return save;
}

Editor::return_t Screen::editSched(Date& s, int cursorPos)
{
	if (!coor.exist[WSCHEDULE])
		return Editor::NOT_SAVED;

	wstring date;
	Editor::return_t save;

	wschedule->_attron(COLOR_SCHED);
	wschedule->_attron(A_BOLD);
	wschedule->_addstr(coor.coor[WSCHEDULE].lines-1, 0, "   Edit schedule: ");
	wschedule->_attroff(A_BOLD);
	wschedule->_refresh();

	/* if is not valid date use today date */
	if (s.valid())
	{
		date = date2str(s);
	}
	else
	{
		Date d;
		d.setToday();
		date = date2str(d);
	}

	/* edit and store */
	dateEditor.getText() = date;
	if (cursorPos >= 0)
		dateEditor.cursorPos() = cursorPos;
	save = dateEditor.edit(*wschedule, coor.coor[WSCHEDULE].lines-1, 18);
	s = str2date(dateEditor.getText());
	return save;
}

Editor::return_t Screen::setPriority(int line, int& priority)
{
	if (!coor.exist[WPRIORITY])
		return Editor::NOT_SAVED;

	wchar_t p[2] = L"N";
	if (priority)
		swprintf(p, 2, L"%01d", priority);

	Editor::return_t save;
	wpriority->_attron(COLOR_SELECTED);
	priorityEditor.getText() = p;
	save = priorityEditor.edit(*wpriority, line, 0);

	char num[2];
	wcstombs(num, priorityEditor.getText().c_str(), 2);
	priority = atoi(num);

	wpriority->_attroff(COLOR_SELECTED);
	return save;
}

Editor::return_t Screen::setCategory(int line, wstring& category, int cursorPos)
{
	if (!coor.exist[WCATEGORY])
		return Editor::NOT_SAVED;

	Editor::return_t save;
	wcategory->_attron(COLOR_SELECTED);
	categoryEditor.getText() = category;
	if (cursorPos >= 0)
		categoryEditor.cursorPos() = cursorPos;
	save = categoryEditor.edit(*wcategory, line, 0, config.getCategoryLength());
	category = categoryEditor.getText();
	wcategory->_attroff(COLOR_SELECTED);
	return save;
}

void Screen::treeClear()
{
	wtree->_erase();
	wtree->_move(0,0);
	wtree->_refresh();
	if (coor.exist[WPRIORITY])
	{
		wpriority->_move(0,0);
		wpriority->_erase();
		wpriority->_refresh();
	}
	if (coor.exist[WCATEGORY])
	{
		wcategory->_erase();
		wcategory->_move(0,0);
		wcategory->_refresh();
	}
	if (coor.exist[WDEADLINE])
	{
		wdeadline->_erase();
		wdeadline->_move(0,0);
		wdeadline->_refresh();
	}
}

int Screen::treeLines()
{
	return wtree->_lines();
}

int Screen::taskLines(int depth, ToDo &t)
{
	unsigned int title_cols = wcswidth(t.getTitle().c_str(), string::npos);
	if (title_cols==(unsigned int)-1) // nonprintable wide character occurs among these characters
			title_cols=0;
	int title_lines = (title_cols / (coor.coor[WTREE].cols-startTitle(depth))) + 1;
	
	if (title_cols && !(title_cols % (coor.coor[WTREE].cols-startTitle(depth))))
			title_lines -= 1;
	return title_lines;
}

Editor::return_t Screen::searchText(wstring& pattern, int cursorPos)
{
	if (!coor.exist[WINFO])
		return Editor::NOT_SAVED;

	Editor::return_t save;
	infoClear();
	winfo->_addch(0,0,'/');
	searchEditor.getText() = pattern;
	if (cursorPos >= 0)
		searchEditor.cursorPos() = cursorPos;
	save = searchEditor.edit(*winfo, 0, 1, 
			PERCENT_COL-2);
	pattern = searchEditor.getText();
	infoClear();
	return save;
}

Editor::return_t Screen::cmd(wstring& command, int cursorPos)
{
	if (!coor.exist[WINFO])
		return Editor::NOT_SAVED;

	Editor::return_t save;
	infoClear();
	winfo->_addch(0,0,':');
	cmdEditor.getText() = command;
	if (cursorPos >= 0)
		cmdEditor.cursorPos() = cursorPos;
	save = cmdEditor.edit(*winfo, 0, 1, 
			PERCENT_COL-2);
	command = cmdEditor.getText();
	infoClear();
	return save;
}

bool Screen::confirmQuit()
{
	infoClear();
	winfo->_addstr(0,0,"Close without save? (y/n) ");
	wint_t ch = 'N';
	winfo->_getch(ch);
	infoClear();
	if (('Y' == ch) || ('y' == ch))
		return true;
	else
		return false;
}

void Screen::infoMsg(const char str[])
{
	if (!coor.exist[WINFO])
		return;

	winfo->_attron(COLOR_INFO);
	infoClear();
	winfo->_addstr(0,0,str);
	winfo->_refresh();
	winfo->_attroff(COLOR_INFO);
}

void Screen::infoClear()
{
	if (!coor.exist[WINFO])
		return;

	winfo->_attron(COLOR_INFO);
	winfo->_move(0,0);
	for (int i = 0; i < COLS-15; ++i)
	{
		winfo->_addch(' ');
	}
	winfo->_refresh();
	winfo->_attroff(COLOR_INFO);
}

void Screen::infoPercent(int percent)
{
	if (!coor.exist[WINFO])
		return;

	winfo->_attron(COLOR_INFO);
	char str[8];
	sprintf(str, "(%3d%%)", percent);
	winfo->_addstr(0,PERCENT_COL,str);
	winfo->_refresh();
	winfo->_attroff(COLOR_INFO);
}

#define draw_help() \
	do { \
	h._erase(); \
	h._move(0,0); \
	for (int i = cursor; (i<=len) && (i-cursor<lines); ++i) \
		h._addstr(str[i]); \
	h._refresh(); \
	} while (0)
void Screen::helpPopUp(wstring str[], int len)
{
	int lines, cols;
	getmaxyx(stdscr, lines, cols); 
	Window help_box(lines-8, cols-16, 4, 8);
	help_box._box();
	wstring s = L"--- Press 'q' key to close, or space bar for next page ---";
	help_box._move(lines-10, (cols/2)-8-(s.length()/2));
	help_box._addstr(s);
	help_box._refresh();

	lines = lines-11;
	cols = cols-18;
	Window h(lines, cols, 5, 9);

	bool close = false;
	bool resized = false;
	int cursor = 0;
	draw_help();
	while (!close) {
		wint_t ch = 'q';

		h._getch(ch);
		switch (ch)
		{
			case KEY_RESIZE:
				resized = true;
				break;
			case KEY_DOWN: case 'j':
				if (cursor <= len-lines)
				{
					++cursor;
					draw_help();
				}
				break;
			case KEY_UP: case 'k':
				if (cursor > 0)
				{
					--cursor;
					draw_help();
				}
				break;
			case KEY_NPAGE: case ' ':
				cursor += lines;
				if (cursor > len-lines) {
					cursor = len + 1 - lines;
					if (cursor < 0)
						cursor = 0;
				}
				draw_help();
				break;
			case KEY_PPAGE: case 'b':
				cursor -= lines;
				if (cursor < 0)
					cursor = 0;
				draw_help();
				break;
			case KEY_HOME: case '<': case 'g':
				cursor = 0;
				draw_help();
				break;
			case KEY_END: case '>': case 'G':
				cursor = len + 1 - lines;
				draw_help();
				break;
			case 'q':
				close = true;
				break;
			default:
				;
		}
	}

	if (resized) ungetch(KEY_RESIZE);
	h._erase();

	/* redraw everything, removing the help window */
	redrawwin(stdscr);
	refresh();
	wtree->_redraw();
	wtree->_refresh();
	if (coor.exist[WHELP])
	{
		whelp->_redraw();
		whelp->_refresh();
	}
	if (coor.exist[WPRIORITY])
	{
		wpriority->_redraw();
		wpriority->_refresh();
	}
	if (coor.exist[WCATEGORY])
	{
		wcategory->_redraw();
		wcategory->_refresh();
	}
	if (coor.exist[WDEADLINE])
	{
		wdeadline->_redraw();
		wdeadline->_refresh();
	}
	if (coor.exist[WTEXT])
	{
		wtext->_redraw();
		wtext->_refresh();
	}
	if (coor.exist[WINFO])
	{
		winfo->_redraw();
		winfo->_refresh();
	}
	if (coor.exist[WSCHEDULE])
	{
		wschedule->_redraw();
		wschedule->_refresh();
	}
	for (vector<Window *>::iterator i = pipes.begin();
		 i != pipes.end(); i++)
	{
		(*i)->_redraw();
		(*i)->_refresh();
	}
}
