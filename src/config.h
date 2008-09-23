
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

#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>
#include <ncurses.h>
#include <fstream>
#include <map>
#include <string>
#include <cstdlib>
using namespace std;

struct key_action;
typedef map<char,key_action> key_map;
struct key_action {
	string action;
	key_map subkeys;
};
typedef map<string,string> action_list;

#define PRIORITY_LENGTH 1
#define CATEGORY_LENGTH 7
#define DEADLINE_LENGTH 12
#define HELP_MIN_WIDTH 67

/* information about window position */
#define MAX_THEME_WINDOWS 8
#define MAX_THEME_ROWS 16
#define MAX_THEME_TREECOLS 4
#define NUM_WINDOWS 8
#define WNULL 10
#define WBLANK 11
#define WHELP 0
#define WTREE 1
#define WTEXT 2
#define WINFO 3
#define WPRIORITY 4
#define WCATEGORY 5
#define WDEADLINE 6
#define WSCHEDULE 7
typedef struct {
	int window;
	int width;
	bool absolute_width;
} theme_window;
typedef struct {
	theme_window windows[MAX_THEME_WINDOWS];
	int num_windows;
	int height;
	bool absolute_height;
} theme_row;
typedef struct {
	bool exist;
	int y, x;
	int lines, cols;
} window_coor;

/* color theme */
#define NUM_CT 8
#define CT_DEFAULT 0
#define CT_SELECTED 1
#define CT_DEADLINE_MARK 2
#define CT_HELP 3
#define CT_TREE 4
#define CT_TEXT 5
#define CT_INFO 6
#define CT_SCHEDULE 7
typedef struct {
	bool exist;
	short int foreground;
	short int background;
} color_pair_t;
typedef struct {
	short int color;
	short int red;
	short int green;
	short int blue;
} color_t;

class Config
{
public:
	bool load(const char* path);
	bool getAction(char key, string& action);
	void getActionList(action_list& list);
	bool getCollapse();
	int getDaysWarn();
	int getDaysSched();
	char* getSortOrder();
	char* getEditor();
	bool genWindowCoor(int lines, int cols, window_coor coor[]);
	void getColorList(color_t* color_list[], short int& length);
	void getColorPair(short int win, short int& foreground, short int& background);
private:
	key_map tree_keys;
	bool collapse;
	int  days_warn_deadline;
	int  days_sched;
	char sort_order[16];
	char editor[64];
	/* themes */
	int row_index;
	theme_row rows[MAX_THEME_ROWS];
	int tree_columns[MAX_THEME_TREECOLS];
	int tree_index;
	color_pair_t color_win[NUM_CT];
	color_t colors[NUM_CT*2];
	short int num_colors;

	void getOutContextOption(string& option, string& value);
	void getGeneralOption(string& option, string& value);
	void insertKeyMap(key_map& k, string action, string keys);
	void _getActionList(action_list& list, key_map& k, string key);
	void resetTheme();
	void getThemeOption(string& option, string& value);
	void getThemeRow(string& value);
	void getThemeWindow(string fmt, theme_window& w);
	void getThemeTree(string& value);
	void getThemeColors(string& option, string& value);
	short int getThemeColor(string color);
	int getContext(string& str);
};

#endif
