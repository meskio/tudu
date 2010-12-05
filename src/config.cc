
/*************************************************************************
 * Copyright (C) 2007-2010 Ruben Pollan Bella <meskio@sindominio.net>    *
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

#include "config.h"

/*
 * Context in the config file
 * define with [ keys ]
 */
#define C_NULL 0
#define C_KEYS 1
#define C_GENERAL 2
#define C_THEME 3

Config::Config()
{
	collapse = false;
	hide_done = false;
}

bool Config::load(const char* path)
{
	int context = C_NULL;

	ifstream file(path);
	if (!file) return false;

	sort_order = "";
	editor[0] = '\0';
	while (!file.eof())
	{
		char line[256];
		string str = "", option, value;
		int pos;
		bool is_definition = false;
		bool quote = false;

		file.getline(line,256);

		// drop all the spaces
		for (unsigned int i = 0; i<strlen(line); i++)
		{
			if ('"' == line[i])
				quote = !quote;
			else if (quote)
				str += line[i];
			else if ('#' == line[i]) break; //is a coment
			else if ('=' == line[i])
			{
				is_definition = true;
				str += line[i];
			}
			else if ((line[i] != ' ') && (line[i] != '\t'))
				str += line[i];
		}

		/*
		 * Change of context
		 */
		if (('[' == str[0]) && (']' == str[str.length()-1]))
		{
			string aux = str.substr(1,str.length()-2);
			context = getContext(aux);
			continue;
		}
		if (!is_definition) continue; //is not a valid line

		pos = str.find("=");
		option = str.substr(0, pos);
		value = str.substr(pos+1);
		if (value.empty()) continue;
		
		if ('@' == option[0]) getOutContextOption(option, value);
		else switch (context)
		{
			case C_GENERAL:
				getGeneralOption(option, value);
				break;
			case C_KEYS:
				insertKeyMap(tree_keys, option, value);
				action_keys.erase(option);
				action_keys.insert(pair<string,string>(option,value));
				break;
			case C_THEME:
				getThemeOption(option, value);
				break;
		}
	}

	file.close();
	return true;
}

void Config::getOutContextOption(string& option, string& value)
{
	if ("@include" == option)
	{
		load(value.c_str());
	}
}

bool Config::isYes(string& value)
{
	if ("yes" == value)
		return true;

	return false;
}

void Config::getGeneralOption(string& option, string& value)
{
	if ("collapse" == option)
		collapse = isYes(value);

	if ("hide_done" == option)
		hide_done = isYes(value);

	if ("hide_percent" == option)
		hide_percent = isYes(value);

	if ("visual_tree" == option)
		visual_tree = isYes(value);

	if ("bold_parent" == option)
		bold_parent = isYes(value);

	if ("loop_move" == option)
		loop_move = isYes(value);

	if ("old_sched" == option)
		old_sched = isYes(value);

	if ("days_warn" == option)
		days_warn_deadline = atoi(value.c_str());

	if ("us_dates" == option)
		us_dates = isYes(value);

	if ("sort_order" == option)
		sort_order = value.c_str();

	if ("editor" == option)
		strncpy(editor, value.c_str(), 16);
}

void Config::insertKeyMap(key_map& k, string action, string keys)
{
	if ((keys.length() == 1) || (keys[1] == '#'))
	{
		key_action act;
		act.action = action;
		k.erase(keys[0]);
		k.insert(pair<char,key_action>(keys[0],act));
	}
	else
	{
		if (0 == k.count(keys[0]))
		{
			key_action act;
			act.action = "";
			k.insert(pair<char,key_action>(keys[0],act));
		}
		insertKeyMap(k[keys[0]].subkeys, action, keys.substr(1));
	}
}

bool Config::getAction(char key, string& action)
{
	static key_map* key_comb = NULL;

	if (key_comb)
	{
		if (key_comb->count(key) == 1)
		{
			key_action& act = (*key_comb)[key];
			if (!act.subkeys.empty())
			{
				key_comb = &act.subkeys;
			}
			else
			{
				key_comb = NULL;
			}
			if ( act.action != "")
			{
				action = act.action;
				return true;
			}
			else
			{
				action = "";
				return false;
			}
		}
		else
		{
			key_comb = NULL;
		}
	}
	if (tree_keys.count(key) == 1)
	{
		key_action& act = tree_keys[key];
		if (!act.subkeys.empty())
		{
			key_comb = &act.subkeys;
		}
		if ( act.action != "")
		{
			action = act.action;
			return true;
		}
	}

	action = "";
	return false;
}

void Config::getActionList(action_list& list)
{
	list = action_keys;
}

void Config::resetTheme()
{
	row_index = 0;
	tree_index = 0;
	num_colors = 0;
	for (int i=0; i<NUM_CT; ++i)
		color_win[i].exist = false;
}

void Config::getThemeOption(string& option, string& value)
{
	if ("row" == option)
	{
		getThemeRow(value);
	}
	else if ("columns" == option)
	{
		getThemeTree(value);
	}
	else
	{
		getThemeColors(option, value);
	}
}

void Config::getThemeRow(string& value)
{
	theme_row& row = rows[row_index];
	string::size_type i, j;
	string str;
	int win; 

	if (row_index == MAX_THEME_ROWS)
	{
		fprintf(stderr, "Error: too many rows in theme\n");
		exit(1);
	}

	/* get row height */
	if ((i=value.find("(", 0)) == string::npos)
	{
		fprintf(stderr, "Error: Bogus theme\n");
		exit(1);
	}
	if (i > 0)
	{
		if ('%' == value[i-1])
		{
			row.absolute_height = false;
			str = value.substr(0, i-1);
		}
		else
		{
			row.absolute_height = true;
			str = value.substr(0, i);
		}
		row.height = atoi(str.c_str());
	}
	else
		row.height = 0;

	/* windows */
	win = 0;
	while (i != string::npos) {
		string::size_type n;

		j=i+1;
		i=value.find(",", j);
		if (win == MAX_THEME_COLS)
		{
			fprintf(stderr, "Error: too many windows in a row for theme\n");
			exit(1);
		}

		if (i == string::npos)
		     n = value.length()-j-1;
		else n = i-j;
		getThemeWindow(value.substr(j,n), 
				row.windows[win]);
		win++;
	}
	row.num_windows = win;

	row_index++;
}

void Config::getThemeWindow(string fmt, theme_window& w)
{
	string::size_type i;

	if ((i=fmt.find("|", 0)) != string::npos)
	{
		if (fmt[i-1] == '%')
		{
			w.absolute_width = false;
			w.width = atoi(fmt.substr(0,i-1).c_str());
		}
		else
		{
			w.absolute_width = true;
			w.width = atoi(fmt.substr(0,i).c_str());
		}
		fmt = fmt.substr(i+1);
	}
	else
		w.width = 0;

	if ("" == fmt)
	{
		if (row_index == 0)
		{
			fprintf(stderr, "Error: null entry in theme too early\n");
			exit(1);
		}
		w.window = WNULL;
	}
	else if ("blank" == fmt)
		w.window = WBLANK;
	else if ("vpipe" == fmt)
		w.window = WVPIPE;
	else if ("hpipe" == fmt)
		w.window = WHPIPE;
	else if ("help" == fmt)
		w.window = WHELP;
	else if ("tree" == fmt)
		w.window = WTREE;
	else if ("text" == fmt)
		w.window = WTEXT;
	else if ("schedule" == fmt)
		w.window = WSCHEDULE;
	else if ("info" == fmt)
		w.window = WINFO;
	else
	{
		fprintf(stderr, "Error: unknown window %s\n", fmt.c_str());
		exit(1);
	}
}

void Config::getThemeTree(string& value)
{
	string::size_type i = 0, j = 0;
	string str;

	tree_index=0;
	while (i != string::npos)
	{
		if (tree_index>MAX_THEME_TREECOLS)
		{
			fprintf(stderr, "Error: too many cols in a tree for theme\n");
			exit(1);
		}
		i=value.find(",", j);
		str = value.substr(j,i-j);
		if ("title" == str)
		{
			tree_columns[tree_index] = WTREE;
		}
		else if ("priority" == str)
		{
			tree_columns[tree_index] = WPRIORITY;
		}
		else if ("category" == str)
		{
			tree_columns[tree_index] = WCATEGORY;
		}
		else if ("deadline" == str)
		{
			tree_columns[tree_index] = WDEADLINE;
		}
		else
		{
			tree_index--;
		}
		j=i+1;
		tree_index++;
	}
}

void Config::getThemeColors(string& option, string& value)
{
	short int color_index;
	string::size_type i,j;

	if ("color" == option)
		color_index = CT_DEFAULT;
	else if ("selected" == option)
		color_index = CT_SELECTED;
	else if ("warn" == option)
		color_index = CT_WARN;
	else if ("pipe" == option)
		color_index = CT_PIPE;
	else if ("help" == option)
		color_index = CT_HELP;
	else if ("tree" == option)
		color_index = CT_TREE;
	else if ("text" == option)
		color_index = CT_TEXT;
	else if ("schedule" == option)
		color_index = CT_SCHEDULE;
	else if ("info" == option)
		color_index = CT_INFO;
	else
	{
		fprintf(stderr, "Error: color theme %s undefined\n", option.c_str());
		exit(1);
	}

	j=value.find("(", 0);
	i=value.find(",", 0);
	if (j<i)
	{
		j=value.find(")", 0);
		i=value.find(",", j);
	}
	color_win[color_index].exist = true;
	color_win[color_index].foreground = getThemeColor(value.substr(0, i));
	color_win[color_index].background = getThemeColor(value.substr(i+1));
}

short int Config::getThemeColor(string color)
{
	static short int color_num = 8;
	short int color_id;
	string::size_type i,j;
	string str;

	/* color defined by RGB */
	if (color[0] == '(')
	{
		colors[num_colors].color = color_num;
		i = color.find(",", 0);
		str = color.substr(1, i);
		colors[num_colors].red = atoi(str.c_str());
		j = i+1;
		i = color.find(",", j);
		str = color.substr(j, i);
		colors[num_colors].green = atoi(str.c_str());
		j = i+1;
		i = color.find(",", j);
		str = color.substr(j, i);
		colors[num_colors].blue = atoi(str.c_str());
		color_id = color_num;
		color_num++;
		num_colors++;
	}
	/* color by name */
	else
	{
		if ("black" == color)
			color_id = COLOR_BLACK;
		else if ("red" == color)
			color_id = COLOR_RED;
		else if ("green" == color)
			color_id = COLOR_GREEN;
		else if ("yellow" == color)
			color_id = COLOR_YELLOW;
		else if ("blue" == color)
			color_id = COLOR_BLUE;
		else if ("magenta" == color)
			color_id = COLOR_MAGENTA;
		else if ("cyan" == color)
			color_id = COLOR_CYAN;
		else if ("white" == color)
			color_id = COLOR_WHITE;
		else if ("transparent" == color)
			color_id = -1;
		else
		{
			fprintf(stderr, "Error: color %s undefined\n", color.c_str());
			exit(1);
		}
	}

	return color_id;
}

int Config::getContext(string& str)
{
	int context;

	if ("keys" == str)
	{
		context = C_KEYS;
	}
	else if ("general" == str)
	{
		context = C_GENERAL;
	}
	else if ("theme" == str)
	{
		context = C_THEME;
		resetTheme();
	}
	else
	{
		context = C_NULL;
	}

	return context;
}

bool Config::getCollapse()
{
	return collapse;
}

bool& Config::getHideDone()
{
	return hide_done;
}

bool Config::getHidePercent()
{
	return hide_percent;
}

bool Config::getVisualTree()
{
	return visual_tree;
}

bool Config::getBoldParent()
{
	return bold_parent;
}

bool Config::getLoopMove()
{
	return loop_move;
}

bool Config::getOldSched()
{
	return old_sched;
}

int Config::getDaysWarn()
{
	return days_warn_deadline;
}

bool Config::useUSDates()
{
	return us_dates;
}

string& Config::getSortOrder()
{
	return sort_order;
}

char* Config::getEditor()
{
	return editor;
}

void Config::genWindowCoor(int lines, int cols, windows_defs& coor)
{
	/* if window don't fits will display only the tree */
	if (!_genWindowCoor(lines, cols, coor))
	{
		for (int i = 0; i < NUM_WINDOWS; ++i)
			coor.exist[i] = false;

		coor.exist[WTREE] = true;
		coor.coor[WTREE].x = 0;
		coor.coor[WTREE].y = 0;
		coor.coor[WTREE].lines = lines;
		coor.coor[WTREE].cols = cols;
	}
}

bool Config::genWindowHeights(int lines, int height[])
{
	int sum_height;
	int undefined_row;
	bool undefined_height;

	sum_height = 0;
	undefined_row = -1;
	undefined_height = false;
	for (int i=0; i<row_index; ++i)
	{
		theme_row& row = rows[i];
		if (row.height)
		{
			/* row height absolute */
			if (row.absolute_height)
			{
				sum_height  += row.height;
				height[i] =  row.height;
			}
			/* row height in percent */
			else
			{
				height[i] = lines*row.height/100;
				sum_height += height[i];
			}
		}
		/* row height not defined */
		else
		{
			height[i] = 0;
			for (int j=0; j<row.num_windows; ++j)
			{
				if ((row.windows[j].window == WHELP) ||
				    (row.windows[j].window == WINFO) ||
					(row.windows[j].window == WHPIPE))
				{
					height[i] = 1;
					sum_height++;
					break;
				}
			}
			if (!height[i])
			{
				undefined_row = i;
				if (undefined_height) return false;
				undefined_height = true;
			}
		}
	}
	if (sum_height > lines)
	{
		return false; /* don't fit in screen */
	}
	if (undefined_row != -1)
	{
		height[undefined_row] = lines-sum_height;
	}

	return true;
}

bool Config::genWindowWidths(int row_index, int cols, windows_defs& coor, int width[])
{
	int top_win;
	int undefined_col = -1;
	int sum_width = 0;
	bool undefined_width = false;
	theme_row& row = rows[row_index];

	for (int j=0; j<row.num_windows; ++j)
	{
		theme_window& win = row.windows[j];

		if (win.width)
		{
			/* column width absolute */
			if (win.absolute_width)
			{
				sum_width += win.width;
				width[j]  = win.width;
			}
			/* column width in percent */
			else
			{
				width[j]  = cols*win.width/100;
				sum_width += width[j];
			}
		}
		/* column width not defined */
		else if  (win.window == WNULL)
		{
			top_win = rows[row_index-1].windows[j].window;
			width[j] = coor.coor[top_win].cols;
			sum_width += width[j];
		}
		else if (win.window == WVPIPE)
		{
			width[j] = 1;
			sum_width++;
		}
		else
		{
			if (undefined_width) return false;
			undefined_width = true;
			undefined_col = j;
		}
	}
	if (sum_width > cols)
	{
		return false; /* don't fit in screen */
	}
	if (undefined_col != -1)
	{
		width[undefined_col] = cols-sum_width;
	}

	return true;
}

bool Config::genWindowTree(windows_defs& coor, int height, int x, int y)
{
	int x_tree;

	x_tree = x;
	coor.coor[WTREE].cols--;
	for (int k=0; k<tree_index; ++k)
	{
		if (tree_columns[k] == WPRIORITY)
			coor.coor[WTREE].cols -= PRIORITY_LENGTH+1;
		else if (tree_columns[k] == WCATEGORY)
			coor.coor[WTREE].cols -= CATEGORY_LENGTH+1;
		else if (tree_columns[k] == WDEADLINE)
			coor.coor[WTREE].cols -= DEADLINE_LENGTH+1;
	}

	for (int k=0; k<tree_index; ++k)
	{
		if (tree_columns[k] == WPRIORITY)
		{
			coor.exist[WPRIORITY] = true;
			coor.coor[WPRIORITY].lines = height;
			coor.coor[WPRIORITY].cols = PRIORITY_LENGTH;
			coor.coor[WPRIORITY].y = y;
			coor.coor[WPRIORITY].x = x_tree;
			x_tree += PRIORITY_LENGTH+1;
		}
		else if (tree_columns[k] == WCATEGORY)
		{
			coor.exist[WCATEGORY] = true;
			coor.coor[WCATEGORY].lines = height;
			coor.coor[WCATEGORY].cols = CATEGORY_LENGTH;
			coor.coor[WCATEGORY].y = y;
			coor.coor[WCATEGORY].x = x_tree;
			x_tree += CATEGORY_LENGTH+1;
		}
		else if (tree_columns[k] == WDEADLINE)
		{
			coor.exist[WDEADLINE] = true;
			coor.coor[WDEADLINE].lines = height;
			coor.coor[WDEADLINE].cols = DEADLINE_LENGTH;
			coor.coor[WDEADLINE].y = y;
			coor.coor[WDEADLINE].x = x_tree;
			x_tree += DEADLINE_LENGTH+1;
		}
		else if (tree_columns[k] == WTREE)
		{
			coor.coor[WTREE].x = x_tree;
			x_tree += coor.coor[WTREE].cols+1;
		}
	}

	if ((coor.coor[WTREE].cols < 20) || (coor.coor[WTREE].lines < 6))
		return false;

	return true;
}

bool Config::_genWindowCoor(int lines, int cols, windows_defs& coor)
{
	int height[MAX_THEME_ROWS];
	int y = 0,x = 0;

	/* check if there is enought rows */
	if (lines < row_index + 8) return false;

	/* initialice the windows as not present */
	for (int i=0; i<NUM_WINDOWS; ++i) coor.exist[i] = false;
	coor.vpipe.clear();
	coor.hpipe.clear();

	/* calculate the height of each row */
	if (!genWindowHeights(lines, height)) return false;

	/* calculate windows */
	for (int i=0; i<row_index; ++i)
	{
		theme_row& row = rows[i];
		int width[MAX_THEME_COLS];
		int top_win;
		window_coor c;

		/* calculate width */
		if (!genWindowWidths(i, cols, coor, width)) return false;

		/* generate the rest of the coordinates */
		if (i == 0) y = 0;
		else y += height[i-1];
		for (int j=0; j<row.num_windows; ++j)
		{
			theme_window& win = row.windows[j];

			if (j == 0) x = 0;
			else x += width[j-1];
			if (win.window < NUM_WINDOWS)
			{
				coor.exist[win.window] = true;
				coor.coor[win.window].cols = width[j];
				coor.coor[win.window].lines = height[i];
				coor.coor[win.window].y = y;
				coor.coor[win.window].x = x;
			}

			switch (win.window)
			{
			case WHELP:
				if ((coor.coor[WHELP].lines != 1) ||
				   (coor.coor[WHELP].cols < HELP_MIN_WIDTH))
					return false;
				break;
			case WINFO:
				if (coor.coor[WINFO].lines != 1)
					return false;
				break;
			case WSCHEDULE:
				if ((coor.coor[WSCHEDULE].lines < 5) ||
				   (coor.coor[WSCHEDULE].cols < 29))
					return false;
				break;
			case WTREE:
				if (!genWindowTree(coor, height[i], x, y)) return false;
				break;
			case WNULL:
				if (rows[i-1].num_windows <= j)
					return false;
				top_win = rows[i-1].windows[j].window;
				if (top_win == WTREE)
				{
					coor.coor[top_win].lines += height[i];
					if (coor.exist[WPRIORITY])
						coor.coor[WPRIORITY].lines += height[i];
					if (coor.exist[WCATEGORY])
						coor.coor[WCATEGORY].lines += height[i];
					if (coor.exist[WDEADLINE])
						coor.coor[WDEADLINE].lines += height[i];
				}
				else if (top_win != WBLANK)
					coor.coor[top_win].lines += height[i];
				break;
			case WVPIPE:
				c.cols = width[j];
				c.lines = height[i];
				c.y = y; c.x = x;
				if (c.cols != 1) return false;
				coor.vpipe.push_back(c);
				break;
			case WHPIPE:
				c.cols = width[j];
				c.lines = height[i];
				c.y = y;  c.x = x;
				if (c.lines != 1) return false;
				coor.hpipe.push_back(c);
				break;
			default:
				break;
			}
		}
	}

	if (!coor.exist[WTREE]) return false;
	return true;
}

void Config::getColorList(color_t* color_list[], short int& length)
{
	*color_list = colors;
	length = num_colors;
}

void Config::getColorPair(short int win, short int& foreground, short int& background)
{
	if (color_win[win].exist)
	{
		foreground = color_win[win].foreground;
		background = color_win[win].background;
	}
	else
	{
		foreground = color_win[CT_DEFAULT].foreground;
		background = color_win[CT_DEFAULT].background;
	}
}
