
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

	sort_order[0] = '\0';
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

void Config::getGeneralOption(string& option, string& value)
{
	if ("collapse" == option)
	{
		if ("yes" == value)
		{
			collapse = true;
		}
		if ("no" == value)
		{
			collapse = false;
		}
	}
	if ("hide_done" == option)
	{
		if ("yes" == value)
		{
			hide_done = true;
		}
		if ("no" == value)
		{
			hide_done = false;
		}
	}
	if ("hide_percent" == option)
	{
		if ("yes" == value)
		{
			hide_percent = true;
		}
		if ("no" == value)
		{
			hide_percent = false;
		}
	}
	if ("visual_tree" == option)
	{
		if ("yes" == value)
		{
			visual_tree = true;
		}
		if ("no" == value)
		{
			visual_tree = false;
		}
	}
	if ("loop_move" == option)
	{
		if ("yes" == value)
		{
			loop_move = true;
		}
		if ("no" == value)
		{
			loop_move = false;
		}
	}
	if ("days_warn" == option)
	{
		days_warn_deadline = atoi(value.c_str());
	}
	if ("sort_order" == option)
	{
		strncpy(sort_order, value.c_str(), 16);
	}
	if ("editor" == option)
	{
		strncpy(editor, value.c_str(), 16);
	}
}

void Config::insertKeyMap(key_map& k, string action, string keys)
{
	if ((keys.length() == 1) || (keys[1] == '#'))
	{
		key_action act;
		act.action = action;
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
	_getActionList(list,tree_keys,"");
}

void Config::_getActionList(action_list& list, key_map& k, string key)
{
	key_map::iterator it;

	for (it  = k.begin(); it != k.end(); ++it)
	{
		if (it->second.action != "")
		{
			list[it->second.action] = key + it->first;
		}
		if (!(it->second.subkeys.empty()))
		{
			_getActionList(list, it->second.subkeys, key+it->first);
		}
	}
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
		if (win == MAX_THEME_WINDOWS)
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
	else if ("deadlineMark" == option)
		color_index = CT_DEADLINE_MARK;
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

bool Config::getLoopMove()
{
	return loop_move;
}

int Config::getDaysWarn()
{
	return days_warn_deadline;
}

char* Config::getSortOrder()
{
	return sort_order;
}

char* Config::getEditor()
{
	return editor;
}

void Config::genWindowCoor(int lines, int cols, window_coor coor[])
{
	/* if window don't fits will display only the tree */
	if (!_genWindowCoor(lines, cols, coor))
	{
		for (int i = 0; i < NUM_WINDOWS; ++i)
			coor[i].exist = false;

		coor[WTREE].exist = true;
		coor[WTREE].x = 0;
		coor[WTREE].y = 0;
		coor[WTREE].lines = lines;
		coor[WTREE].cols = cols;
	}
}

bool Config::_genWindowCoor(int lines, int cols, window_coor coor[])
{
	int height[MAX_THEME_ROWS];
	int undefined_row;
	int sum_height;
	int y = 0,x = 0;
	bool undefined_height = false;

	/* check if there is enought rows */
	if (lines < row_index + 8) return false;

	/* initialice the windows as not present */
	for (int i=0; i<NUM_WINDOWS; ++i) coor[i].exist = false;

	/* calculate the height of each row */
	sum_height = 0;
	undefined_row = -1;
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
				    (row.windows[j].window == WINFO))
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

	/* calculate windows */
	for (int i=0; i<row_index; ++i)
	{
		theme_row& row = rows[i];
		int width[MAX_THEME_WINDOWS];
		int top_win;

		/* calculate width */
		int undefined_col = -1;
		int sum_width = 0;
		bool undefined_width = false;
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
				top_win = rows[i-1].windows[j].window;
				width[i] = coor[top_win].cols;
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

		/* generate the rest of the coordinates */
		if (i == 0) y = 0;
		else y += height[i-1];
		for (int j=0; j<row.num_windows; ++j)
		{
			theme_window& win = row.windows[j];
			int x_tree;

			if (j == 0) x = 0;
			else x += width[j-1];
			if (win.window < NUM_WINDOWS)
			{
				coor[win.window].exist = true;
				coor[win.window].cols = width[j];
				coor[win.window].lines = height[i];
				coor[win.window].y = y;
				coor[win.window].x = x;
			}

			switch (win.window)
			{
			case WHELP:
				if ((coor[WHELP].lines != 1) ||
				   (coor[WHELP].cols < HELP_MIN_WIDTH))
					return false;
				break;
			case WINFO:
				if (coor[WINFO].lines != 1)
					return false;
				break;
			case WSCHEDULE:
				if ((coor[WSCHEDULE].lines < 5) ||
				   (coor[WSCHEDULE].cols < 29))
					return false;
				break;
			case WTREE:
				x_tree = x;
				coor[WTREE].cols--;
				for (int k=0; k<tree_index; ++k)
				{
					if (tree_columns[k] == WPRIORITY)
						coor[WTREE].cols -= PRIORITY_LENGTH+1;
					else if (tree_columns[k] == WCATEGORY)
						coor[WTREE].cols -= CATEGORY_LENGTH+1;
					else if (tree_columns[k] == WDEADLINE)
						coor[WTREE].cols -= DEADLINE_LENGTH+1;
				}
				for (int k=0; k<tree_index; ++k)
				{
					if (tree_columns[k] == WPRIORITY)
					{
						coor[WPRIORITY].exist = true;
						coor[WPRIORITY].lines = height[i];
						coor[WPRIORITY].cols = PRIORITY_LENGTH;
						coor[WPRIORITY].y = y;
						coor[WPRIORITY].x = x_tree;
						x_tree += PRIORITY_LENGTH+1;
					}
					else if (tree_columns[k] == WCATEGORY)
					{
						coor[WCATEGORY].exist = true;
						coor[WCATEGORY].lines = height[i];
						coor[WCATEGORY].cols = CATEGORY_LENGTH;
						coor[WCATEGORY].y = y;
						coor[WCATEGORY].x = x_tree;
						x_tree += CATEGORY_LENGTH+1;
					}
					else if (tree_columns[k] == WDEADLINE)
					{
						coor[WDEADLINE].exist = true;
						coor[WDEADLINE].lines = height[i];
						coor[WDEADLINE].cols = DEADLINE_LENGTH;
						coor[WDEADLINE].y = y;
						coor[WDEADLINE].x = x_tree;
						x_tree += DEADLINE_LENGTH+1;
					}
					else if (tree_columns[k] == WTREE)
					{
						coor[WTREE].x = x_tree;
						x_tree += coor[WTREE].cols;
					}
				}
				if ((coor[WTREE].cols < 20) || (coor[WTREE].lines < 6))
					return false;
				break;
			case WNULL:
				if (rows[i-1].num_windows <= j)
					return false;
				top_win = rows[i-1].windows[j].window;
				if (top_win == WTREE)
				{
					coor[top_win].lines += height[i];
					if (coor[WPRIORITY].exist)
						coor[WPRIORITY].lines += height[i];
					if (coor[WCATEGORY].exist)
						coor[WCATEGORY].lines += height[i];
					if (coor[WDEADLINE].exist)
						coor[WDEADLINE].lines += height[i];
				}
				else if (top_win != WBLANK)
					coor[top_win].lines += height[i];
				break;
			}
		}
	}

	if (!coor[WTREE].exist) return false;
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
