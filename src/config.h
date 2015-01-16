
/**************************************************************************
 * Copyright (C) 2007-2015 Ruben Pollan Bella <meskio@sindominio.net>     *
 *                                                                        *
 *  This file is part of TuDu.                                            *
 *                                                                        *
 *  TuDu is free software; you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; version 3 of the License.               *
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

#include "includes.h"
#include "window.h"

struct key_action;
typedef map<wchar_t,key_action> key_map;
struct key_action {
	wstring action;
	key_map subkeys;
};
typedef map<wstring,wstring> action_list;

#define PRIORITY_LENGTH 1
#define DEADLINE_LENGTH 12
#define HELP_MIN_WIDTH 67

/* information about window position */
#define MAX_THEME_COLS 16
#define MAX_THEME_ROWS 16
#define MAX_THEME_TREECOLS 4
enum window_type {
	WHELP,
	WTREE,
	WTEXT,
	WINFO,
	WPRIORITY,
	WCATEGORY,
	WDEADLINE,
	WSCHEDULE,
	NUM_WINDOWS,
	WNULL,
	WBLANK,
	WVPIPE,
	WHPIPE
};
typedef struct {
	bool exist[NUM_WINDOWS];
	window_coor coor[NUM_WINDOWS];
	vector<window_coor> vpipe;
	vector<window_coor> hpipe;
} windows_defs;

/* color theme */
enum color_theme {
	CT_DEFAULT,
	CT_SELECTED,
	CT_WARN,
	CT_HELP,
	CT_TREE,
	CT_TEXT,
	CT_INFO,
	CT_SCHEDULE,
	CT_PIPE,
	NUM_CT
};
typedef struct {
	short int color;
	short int red;
	short int green;
	short int blue;
} color_t;


class Config
{
public:
	Config();

	bool load(const char* path);
	bool getAction(wchar_t key, wstring& action);
	void getActionList(action_list& list);
	bool getCollapse();
	bool& getHideDone();
	bool getHidePercent();
	bool getVisualTree();
	bool getBoldParent();
	bool getLoopMove();
	bool getOldSched();
	int getDaysWarn();
	bool useUSDates();
	wstring& getTuduFile();
	wstring& getSortOrder();
	char* getEditor();
	int getCategoryLength();
	void genWindowCoor(int lines, int cols, windows_defs& coor);
	void getColorList(color_t* color_list[], short int& length);
	void getColorPair(short int win, short int& foreground, short int& background);
private:
	typedef struct {
		window_type window;
		int width;
		bool absolute_width;
	} theme_window;
	typedef struct {
		theme_window windows[MAX_THEME_COLS];
		int num_windows;
		int height;
		bool absolute_height;
	} theme_row;
	typedef struct {
		bool exist;
		short int foreground;
		short int background;
	} color_pair_t;

	key_map tree_keys;
	action_list action_keys;
	bool collapse;
	bool hide_done;
	bool hide_percent;
	bool visual_tree;
	bool bold_parent;
	bool loop_move;
	int  days_warn_deadline;
	bool us_dates;
	bool old_sched;
	wstring tudu_file;
	wstring sort_order;
	char editor[64];
	/* themes */
	int row_index;
	theme_row rows[MAX_THEME_ROWS];
	int tree_columns[MAX_THEME_TREECOLS];
	int tree_index;
	int category_length;
	color_pair_t color_win[NUM_CT];
	color_t colors[NUM_CT*2];
	short int num_colors;

	void getOutContextOption(wstring& option, wstring& value);
	bool isYes(wstring& value);
	void getGeneralOption(wstring& option, wstring& value);
	void insertKeyMap(key_map& k, wstring action, wstring keys);
	void resetTheme();
	void getThemeOption(wstring& option, wstring& value);
	void getThemeRow(wstring& value);
	void getThemeWindow(wstring fmt, theme_window& w);
	void getThemeTree(wstring& value);
	void getThemeCategoryLength(wstring& value);
	void getThemeColors(wstring& option, wstring& value);
	short int getThemeColor(wstring color);
	int getContext(wstring& str);
	bool genWindowHeights(int lines, int height[]);
	bool genWindowWidths(int row_index, int cols, windows_defs& coor, int width[]);
	bool genWindowTree(windows_defs& coor, int height, int x, int y);
	bool _genWindowCoor(int lines, int cols, windows_defs& coor);
};

#endif
