
/**************************************************************************
 * Copyright (C) 2007-2013 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#ifndef DEFS_H
#define DEFS_H

#include "defs.h"

/* include curses */
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_XCURSES_H
#include <xcurses.h>
#elif defined(HAVE_NCURSESW_CURSES_H)
#include <ncursesw/curses.h>
#elif defined(HAVE_NCURSES_CURSES_H)
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

/* system includes */
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <list>
#include <stack>
#include <algorithm>
#include <set>
#include <ctime>
#include <cstring>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <locale>
#include <cwchar>
#include <clocale>
#include <climits>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cerrno>

using namespace std;

#endif
