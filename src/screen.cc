
/*************************************************************************
 * Copyright (C) 2007-2009 Ruben Pollan Bella <meskio@sindominio.net>    *
 *                                                                       *
 *  This file is part of TuDu.                                           *
 *                                                                       *
 *  TuDu is free software; you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation; either version 3 of the License.       *
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
#define COLOR_DEADLINE_MARK COLOR_PAIR(CT_DEADLINE_MARK)
#define COLOR_HELP     COLOR_PAIR(CT_HELP)
#define COLOR_TREE     COLOR_PAIR(CT_TREE)
#define COLOR_TEXT     COLOR_PAIR(CT_TEXT)
#define COLOR_SCHED    COLOR_PAIR(CT_SCHEDULE)
#define COLOR_INFO     COLOR_PAIR(CT_INFO)

#define PERCENT_COL (coor[WINFO].cols-7)

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
			config.getColorPair(0, foreground, background);
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
	clear ();
	refresh ();
	endwin ();
}

void Screen::draw()
{
	int lines, cols;

	/* get size of windows */
    getmaxyx(stdscr, lines, cols); 
	config.genWindowCoor(lines, cols, coor);

	/* create windows */
	window_coor c = coor[WTREE];
	wtree = new Window(c.lines, c.cols, c.y, c.x);
	c = coor[WHELP];
	if (c.exist)
	{
		whelp = new Window(c.lines, c.cols, c.y, c.x);
		whelp->_attron(COLOR_HELP);
		for (int i = 0; i<c.cols; ++i) whelp->_addch(' ');
		whelp->_move(0,0);
		whelp->_addstr(" q:quit  k:up  j:down  h:out  l:in  m:done  o:add  a:modify");
		whelp->_move(0, c.cols-8);
		whelp->_addstr("?:help");
		whelp->_refresh();
		whelp->_attroff(COLOR_HELP);
	}
	else
		whelp = NULL;
	c = coor[WPRIORITY];
	if (c.exist)
		wpriority = new Window(c.lines, c.cols, c.y, c.x);
	else
		wpriority = NULL;
	c = coor[WCATEGORY];
	if (c.exist)
		wcategory = new Window(c.lines, c.cols, c.y, c.x);
	else
		wcategory = NULL;
	c = coor[WDEADLINE];
	if (c.exist)
		wdeadline = new Window(c.lines, c.cols, c.y, c.x);
	else
		wdeadline = NULL;
	c = coor[WTEXT];
	if (c.exist)
		wtext = new Window(c.lines, c.cols, c.y, c.x);
	else
		wtext = NULL;
	c = coor[WINFO];
	if (c.exist)
		winfo = new Window(c.lines, c.cols, c.y, c.x);
	else
		winfo = NULL;
	c = coor[WSCHEDULE];
	if (c.exist)
		wschedule = new Window(c.lines, c.cols, c.y, c.x);
	else
		wschedule = NULL;
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
	/* tree */
	window_coor c = coor[WTREE];
	wtree->_resize(c.lines, c.cols);
	wtree->_mv(c.y, c.x);
	wtree->_erase();

	/* help */
	c = coor[WHELP];
	if (c.exist)
	{
		whelp->_resize(c.lines, c.cols);
		whelp->_mv(c.y, c.x);
		whelp->_erase();
		whelp->_attron(COLOR_HELP);
		whelp->_move(0,0);
		for (int i = 0; i<c.cols; ++i) whelp->_addch(' ');
		whelp->_move(0,0);
		whelp->_addstr(" q:quit  k:up  j:down  h:out  l:in  m:done  o:add  a:modify");
		whelp->_move(0, c.cols-8);
		whelp->_addstr("?:help");
		whelp->_refresh();
		whelp->_attroff(COLOR_HELP);
	}

	/* priority */
	c = coor[WPRIORITY];
	if (c.exist)
	{
		wpriority->_resize(c.lines, c.cols);
		wpriority->_mv(c.y, c.x);
		wpriority->_erase();
	}

	/* category */
	c = coor[WCATEGORY];
	if (c.exist)
	{
		wcategory->_resize(c.lines, c.cols);
		wcategory->_mv(c.y, c.x);
		wcategory->_erase();
	}

	/* deadline */
	c = coor[WDEADLINE];
	if (c.exist)
	{
		wdeadline->_resize(c.lines, c.cols);
		wdeadline->_mv(c.y, c.x);
		wdeadline->_erase();
	}

	/* deadline */
	c = coor[WTEXT];
	if (c.exist)
	{
		wtext->_resize(c.lines, c.cols);
		wtext->_mv(c.y, c.x);
		wtext->_erase();
	}

	/* info */
	c = coor[WINFO];
	if (c.exist)
	{
		winfo->_resize(c.lines, c.cols);
		winfo->_mv(c.y, c.x);
		winfo->_erase();
	}

	/* schedule */
	c = coor[WSCHEDULE];
	if (c.exist)
	{
		wschedule->_resize(c.lines, c.cols);
		wschedule->_mv(c.y, c.x);
		wschedule->_erase();
	}
}

#define startTitle (depth * 4 + 7)

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
	if (t.haveChild()) wtree->_attron(A_BOLD);
	STRING title = t.getTitle().substr(0,coor[WTREE].cols-startTitle);
	wtree->_addstr(title);
	if (t.haveChild()) wtree->_attroff(A_BOLD);
	if (isCursor)
		wtree->_attroff(COLOR_SELECTED);
	else
		wtree->_attroff(COLOR_TREE);
	wtree->_refresh();

	/* draw priority */
	if (coor[WPRIORITY].exist)
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
	if (coor[WCATEGORY].exist)
	{
		STRING category = t.getCategory();
		if (isCursor)
			wcategory->_attron(COLOR_SELECTED);
		else
			wcategory->_attron(COLOR_TREE);
		if (!category.empty())
		{
			wcategory->_move(line, 0);
			wcategory->_addstr(category);
		}
		if (isCursor)
			wcategory->_attroff(COLOR_SELECTED);
		else
			wcategory->_attroff(COLOR_TREE);
		wcategory->_refresh();
	}

	/* draw date */
	if (coor[WDEADLINE].exist)
	{
		if (isCursor)
			wdeadline->_attron(COLOR_SELECTED);
		else
			wdeadline->_attron(COLOR_TREE);
		Date& deadline = t.deadline();
		if (deadline.valid())
		{
			char s[11];
			wdeadline->_move(line, 0);
			sprintf(s, "%02d/%02d/%04d", deadline.day(), 
					deadline.month(), deadline.year());
			wdeadline->_addstr(s);

		}
		wdeadline->_move(line, 10);
		if ((!t.done() && deadline_close(deadline)) || (isCollapse(t) && deadline_close(chinf.deadline)))
		{
			wdeadline->_attron(COLOR_DEADLINE_MARK);
			wdeadline->_addstr("<-");
			wdeadline->_attroff(COLOR_DEADLINE_MARK);
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
	if (coor[WTEXT].exist)
	{
		wtree->_attron(COLOR_TEXT);
		wtext->_erase();
		t.print(*wtext);
		wtree->_attroff(COLOR_TEXT);
	}
}

void Screen::drawSched(Sched &sched, pToDo cursor)
{
	if (coor[WSCHEDULE].exist)
	{
		time_t t = time(NULL);
		struct tm* pt = localtime(&t);
		Date today(pt->tm_mday, pt->tm_mon+1, pt->tm_year+1900);
		sched_l sched_list;
		sched.get(today,sched_list);

		Date last;
		int line = 0;
		wschedule->_erase();
		wschedule->_move(0,0);
		wschedule->_attron(COLOR_SCHED);
		for (sched_l::iterator i = sched_list.begin(); 
				(i != sched_list.end()) && (line < coor[WSCHEDULE].lines); i++, line++)
		{
			if ((*i)->done()) continue;
			if ((*i)->sched() != last)
			{
				last = (*i)->sched();

				char str[32];
				sprintf(str, "  %d/%d/%d\n", last.day(), last.month(), last.year());
				wschedule->_attron(A_BOLD);
				wschedule->_addstr(str);
				wschedule->_attroff(A_BOLD);
				line++;
			}
			if (cursor == (*i))
				wschedule->_attron(COLOR_SELECTED);
			wschedule->_addstr("    ");
			STRING title = (*i)->getTitle().substr(0,coor[WSCHEDULE].cols-4);
			wschedule->_addstr(title);
			wschedule->_addstr("\n");
			if (cursor == (*i))
				wschedule->_attroff(COLOR_SELECTED);
		}
		wschedule->_attroff(COLOR_SCHED);
		wschedule->_refresh();
	}
}

void Screen::scrollUpText(Text& t)
{
	if (coor[WTEXT].exist)
	{
		wtree->_attron(COLOR_TEXT);
		t.scroll_up(*wtext);
		wtree->_attroff(COLOR_TEXT);
	}
}

void Screen::scrollDownText(Text& t)
{
	if (coor[WTEXT].exist)
	{
		wtree->_attron(COLOR_TEXT);
		t.scroll_down(*wtext);
		wtree->_attroff(COLOR_TEXT);
	}
}

void Screen::deadlineClear(int line)
{
	if (coor[WDEADLINE].exist)
	{
		wdeadline->_attron(COLOR_TREE);
		wdeadline->_move(line, 0);
		wdeadline->_addstr("            ");
		wdeadline->_refresh();
		wdeadline->_attroff(COLOR_TREE);
	}
}

void Screen::priorityClear(int line)
{
	if (coor[WPRIORITY].exist)
	{
		wpriority->_attron(COLOR_TREE);
		wpriority->_move(line, 0);
		wpriority->_addstr(" ");
		wpriority->_refresh();
		wpriority->_attroff(COLOR_TREE);
	}
}

bool Screen::editTitle(int line, int depth, bool haveChild, STRING& str)
{
	bool save;

	wtree->_attron(COLOR_SELECTED);
	if (haveChild) wtree->_attron(A_BOLD);
	lineEditor.getText() = str; 
	lineEditor.cursorPos() = str.length();
	save = lineEditor.edit(*wtree, line, startTitle, 
			coor[WTREE].cols-startTitle);
	if (!save)
	{
		wtree->_move(line, startTitle);
		wtree->_addstr(str);
		for (int i = startTitle + str.length(); i < coor[WTREE].cols; i++) 
				wtree->_addch(' ');
		wtree->_refresh();
	}
	str = lineEditor.getText(); 
	if (haveChild) wtree->_attroff(A_BOLD);
	wtree->_attroff(COLOR_SELECTED);
	return save;
}

void Screen::editText(Text& t)
{
	if (coor[WTEXT].exist)
	{
		wtree->_attron(COLOR_TEXT);
		t.edit(*wtext);
		wtree->_attroff(COLOR_TEXT);
	}
}

void Screen::editDeadline(int line, Date& deadline, bool done)
{
	if (coor[WDEADLINE].exist)
	{
		bool save;
		char date[11];
		CHAR wide_date[11];

		if (deadline.valid())
		{
			sprintf(date, "%02d/%02d/%04d", deadline.day(),
					deadline.month(), deadline.year());
		}
		else
		{
			time_t t = time(NULL);
			struct tm* pt = localtime(&t);
			strftime(date, 11, "%02d/%02m/%04Y", pt);
		}

		wdeadline->_attron(COLOR_SELECTED);
		mbstowcs(wide_date, date, 11);
		dateEditor.getText() = wide_date;
		dateEditor.cursorPos() = 0;
		save = dateEditor.edit(*wdeadline, line, 0, 11);

		/* store deadline */
		if (save)
		{
			wcstombs(date, dateEditor.getText().c_str(), 10);
			date[2] = '\0';
			date[5] = '\0';
			Date d(atoi(date), atoi(date+3), atoi(date+6));
			if (d.correct())
				deadline = d;
			else
				save = false;
		}

		/* if will not save redraw deadline */
		if (!save)
		{
			if (deadline.valid())
			{
				wdeadline->_move(line, 0);
				sprintf(date, "%02d/%02d/%04d", deadline.day(),
						deadline.month(), deadline.year());
				wdeadline->_addstr(date);
			}
			else
			{
				wdeadline->_move(line, 0);
				wdeadline->_addstr("          ");
			}
		}
		wdeadline->_attroff(COLOR_SELECTED);

		wdeadline->_move(line, 10);
		if ((!done) && (deadline_close(deadline)))
		{
			wdeadline->_attron(COLOR_DEADLINE_MARK);
			wdeadline->_addstr("<-");
			wdeadline->_attroff(COLOR_DEADLINE_MARK);
		}
		else
		{
			wdeadline->_addstr("  ");
		}
		wdeadline->_refresh();
	}
}

bool Screen::editSched(Date& s)
{
	if (coor[WSCHEDULE].exist)
	{
		char date[12];
		CHAR wide_date[12];
		bool save;

		wschedule->_attron(A_BOLD);
		wschedule->_addstr(coor[WSCHEDULE].lines-1, 0, "   Edit schedule: ");
		wschedule->_attroff(A_BOLD);
		wschedule->_refresh();

		/* if is not valid date use today date */
		if (s.valid())
		{
			sprintf(date, "%02d/%02d/%04d", s.day(),
					s.month(), s.year());
		}
		else
		{
			time_t t = time(NULL);
			struct tm* pt = localtime(&t);
			strftime(date, 11, "%02d/%02m/%04Y", pt);
		}

		/* edit and store */
		mbstowcs(wide_date, date, 11);
		dateEditor.getText() = wide_date;
		dateEditor.cursorPos() = 0;
		save = dateEditor.edit(*wschedule, coor[WSCHEDULE].lines-1, 18, 11);
		wschedule->_addstr(coor[WSCHEDULE].lines-1, 0, "                            ");
		wschedule->_refresh();
		if (save)
		{
			wcstombs(date, dateEditor.getText().c_str(), 10);
			date[2] = '\0';
			date[5] = '\0';
			Date d(atoi(date), atoi(date+3), atoi(date+6));
			if (d.correct())
			{
				s = d;
				return true;
			}
			else
				return false;
		}
	}
	return false;
}

void Screen::setPriority(int line, int& priority)
{
	if (coor[WPRIORITY].exist)
	{
		CHAR p[2] = L"N";
		char s[2];

		if (priority)
			swprintf(p, 2, L"%01d", priority);
		priorityEditor.getText() = p;
		if (priorityEditor.edit(*wpriority, line, 0, 1))
		{
			char num[2];
			wcstombs(num, priorityEditor.getText().c_str(), 2);
			priority = atoi(num);
		}

		wpriority->_move(line, 0);
		if (priority)
			sprintf(s, "%01d", priority);
		else
			strcpy(s, " ");

		wpriority->_attron(COLOR_SELECTED);
		wpriority->_addstr(s);
		wpriority->_attroff(COLOR_SELECTED);
		wpriority->_refresh();
	}
}

void Screen::setCategory(int line, ToDo& t)
{
	if (coor[WCATEGORY].exist)
	{
		STRING category = t.getCategory();
		categoryEditor.getText() = category;
		categoryEditor.cursorPos() = category.length();
		if (categoryEditor.edit(*wcategory, line, 0, CATEGORY_LENGTH))
		{
			category = categoryEditor.getText();
			t.setCategory(category);
		}

		wcategory->_move(line, 0);
		wcategory->_attron(COLOR_SELECTED);
		if (category.empty())
			for (int i=0; i<CATEGORY_LENGTH; i++) wcategory->_addch(' ');
		else
			wcategory->_addstr(category);
		wcategory->_attroff(COLOR_SELECTED);
		wcategory->_refresh();
	}
}

void Screen::treeClear()
{
	wtree->_erase();
	wtree->_move(0,0);
	wtree->_refresh();
	if (coor[WPRIORITY].exist)
	{
		wpriority->_move(0,0);
		wpriority->_erase();
		wpriority->_refresh();
	}
	if (coor[WCATEGORY].exist)
	{
		wcategory->_erase();
		wcategory->_move(0,0);
		wcategory->_refresh();
	}
	if (coor[WDEADLINE].exist)
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

bool Screen::searchText(STRING& pattern)
{
	if (coor[WINFO].exist)
	{
		bool save;

		infoClear();
		winfo->_addch(0,0,'/');
		save = searchEditor.edit(*winfo, 0, 1, 
				PERCENT_COL-2);
		pattern = searchEditor.getText();
		infoClear();
		return save;
	}
	else
	{
		return false;
	}
}

bool Screen::cmd(STRING& command)
{
	if (coor[WINFO].exist)
	{
		bool save;

		infoClear();
		winfo->_addch(0,0,':');
		save = cmdEditor.edit(*winfo, 0, 1, 
				PERCENT_COL-2);
		command = cmdEditor.getText();
		infoClear();
		return save;
	}
	else
	{
		return false;
	}
}

void Screen::infoMsg(const char str[])
{
	if (coor[WINFO].exist)
	{
		winfo->_attron(COLOR_INFO);
		infoClear();
		winfo->_addstr(0,0,str);
		winfo->_refresh();
		winfo->_attroff(COLOR_INFO);
	}
}

void Screen::infoClear()
{
	if (coor[WINFO].exist)
	{
		winfo->_attron(COLOR_INFO);
		winfo->_move(0,0);
		for (int i = 0; i < COLS-15; ++i)
		{
			winfo->_addch(' ');
		}
		winfo->_refresh();
		winfo->_attroff(COLOR_INFO);
	}
}

void Screen::infoPercent(int percent)
{
	if (coor[WINFO].exist)
	{
		winfo->_attron(COLOR_INFO);
		char str[8];
		sprintf(str, "(%3d%%)", percent);
		winfo->_addstr(0,PERCENT_COL,str);
		winfo->_refresh();
		winfo->_attroff(COLOR_INFO);
	}
}

#define draw_help() \
	do { \
	h._erase(); \
	h._move(0,0); \
	for (int i = cursor; (i<=len) && (i-cursor<lines); ++i) \
		h._addstr(str[i]); \
	h._refresh(); \
	} while (0)
void Screen::helpPopUp(string str[], int len)
{
	int lines, cols;
        getmaxyx(stdscr, lines, cols); 
	Window help_box(lines-8, cols-16, 4, 8);
	help_box._box();
	string s = "--- Press any key to close ---";
	help_box._move(lines-10, (cols/2)-8-(s.length()/2));
	help_box._addstr(s);
	help_box._refresh();

	lines = lines-11;
	cols = cols-18;
	Window h(lines, cols, 5, 9);

	bool close = false;
	bool resized = false;
	int cursor = 0;
	wint_t ch;
	draw_help();
	while (!close) {
		if (ERR == h._getch(&ch))
		{
			//TODO
		}
		else
		{
		switch (h._getch())
		{
			case KEY_RESIZE:
				resized = true;
				break;
			case KEY_DOWN:
				if (cursor <= len-lines)
				{
					++cursor;
					draw_help();
				}
				break;
			case KEY_UP:
				if (cursor > 0)
				{
					--cursor;
					draw_help();
				}
				break;
			case  KEY_NPAGE:
				cursor += lines;
				if (cursor > len-lines)
					cursor = len + 1 - lines;
				draw_help();
				break;
			case  KEY_PPAGE:
				cursor -= lines;
				if (cursor < 0)
					cursor = 0;
				draw_help();
				break;
			case KEY_HOME:
				cursor = 0;
				draw_help();
				break;
			case KEY_END:
				cursor = len + 1 - lines;
				draw_help();
				break;
			default: 
				close = true;
				break;
		}
		}
	}

	if (resized) ungetch(KEY_RESIZE);
	h._erase();

	/* redraw everything, removing the help window */
	redrawwin(stdscr);
	refresh();
	wtree->_redraw();
	wtree->_refresh();
	if (coor[WHELP].exist)
	{
		whelp->_redraw();
		whelp->_refresh();
	}
	if (coor[WPRIORITY].exist)
	{
		wpriority->_redraw();
		wpriority->_refresh();
	}
	if (coor[WCATEGORY].exist)
	{
		wcategory->_redraw();
		wcategory->_refresh();
	}
	if (coor[WDEADLINE].exist)
	{
		wdeadline->_redraw();
		wdeadline->_refresh();
	}
	if (coor[WTEXT].exist)
	{
		wtext->_redraw();
		wtext->_refresh();
	}
	if (coor[WINFO].exist)
	{
		winfo->_redraw();
		winfo->_refresh();
	}
	if (coor[WSCHEDULE].exist)
	{
		wschedule->_redraw();
		wschedule->_refresh();
	}
}
