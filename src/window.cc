
/**************************************************************************
 * Copyright (C) 2007-2020 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#include "window.h"

Window::Window(window_coor coor)
{
	win = newwin(coor.lines, coor.cols, coor.y, coor.x);
	keypad(win, TRUE);
}

Window::Window(int lines, int cols, int y, int x)
{
	win = newwin(lines, cols, y, x);
	keypad(win, TRUE);
}

Window::Window()
{
	int lines, cols;
        getmaxyx(stdscr, lines, cols);
	win = newwin(lines, cols, 0, 0);
	keypad(win, TRUE);
}

Window::~Window()
{
	delwin(win);
}

/* int Window::printw(const char *fmt, ...)
{
	wprintw(win, fmt, ...);
	wrefresh(win);
} */

int Window::_addstr(const char *str)
{
	return waddstr(win, str);
}

int Window::_addstr(const wchar_t *str)
{
	return waddwstr(win, str);
}

int Window::_addstr(int y, int x, const char *str)
{
	return mvwaddstr(win, y, x, str);
}

int Window::_addstr(int y, int x, const wchar_t *str)
{
	return mvwaddwstr(win, y, x, str);
}

int Window::_addstr(const string &str)
{
	return waddstr(win, str.c_str());
}

int Window::_addstr(const wstring &str)
{
	return waddwstr(win, str.c_str());
}

int Window::_addstr(int y, int x, const string &str)
{
	return mvwaddstr(win, y, x,  str.c_str());
}

int Window::_addstr(int y, int x, const wstring &str)
{
	return mvwaddwstr(win, y, x,  str.c_str());
}

int Window::_addstr(const wstring &str, const unsigned int cols)
{
    unsigned int i = 0, c = 0;
    while (i < str.length()) {
        unsigned int width = wcwidth(str[i]);
        if (c+width > cols) {
            break;
        }
        c+=width;
        i++;
    }

    waddwstr(win, str.substr(0,i).c_str());
    if(cols>c) {
        waddwstr(win, wstring(cols-c,' ').c_str());
    }

    return i;
}

int Window::_addstr(int y, int x, const wstring &str, const unsigned int cols)
{
	wmove(win, y, x);
	return _addstr(str, cols);
}

int Window::_addch(const char ch)
{
	return waddch(win, ch);
}

int Window::_addch(int y, int x, const char ch)
{
	return mvwaddch(win, y, x, ch);
}

int Window::_refresh()
{
	return wrefresh(win);
}

int Window::_redraw()
{
	return redrawwin(win);
}

int Window::_move(int y, int x)
{
	return wmove(win, y, x);
}

int Window::_attron(int attrs)
{
	return wattron(win, attrs);
}

int Window::_attroff(int attrs)
{
	return wattroff(win, attrs);
}

int Window::_erase()
{
	return werase(win);
}

int Window::_getch(wint_t& ch)
{
	return wget_wch(win, &ch);
}

void Window::_getmaxyx(int& y, int& x)
{
	getmaxyx(win, y, x);
}

int Window::_box()
{
	return box(win, 0, 0);
}

int Window::_lines()
{
	int lines, cols;
	getmaxyx(win, lines, cols);
	(void)cols;
	return lines;
}

int Window::_cols()
{
	int lines, cols;
	getmaxyx(win, lines, cols);
	(void)lines;
	return cols;
}

int Window::_resize(int lines, int columns)
{
	return wresize(win, lines, columns);
}

int Window::_mv(int y, int x)
{
	return mvwin(win, y, x);
}

int Window::_vline(chtype ch, int n)
{
	return wvline(win, ch, n);
}

int Window::_hline(chtype ch, int n)
{
	return whline(win, ch, n);
}

int Window::_delwin()
{
	return delwin(win);
}

