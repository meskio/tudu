/**************************************************************************
 * Copyright (C) 2007-2009 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#include "cmd.h"

map<STRING,STRING> commands;

Cmd::Cmd()
{
	/* initialize commands */
	commands[L"hide"] = L"category";
	commands[L"show"] = L"category";
	commands[L"showall"] = L"category";
	commands[L"showonly"] = L"category";
}

void Cmd::get_interface(Interface *i)
{
	interface = i;
}

bool Cmd::cmd(STRING command)
{
	vector<STRING> params;
	STRING com;
	size_t begin, end;

	/* Get the command and params in text */
	for (begin = 0, end = command.find(' ', 0); string::npos != end;
				begin = end+1, end = command.find(' ', begin))
	{
		if (begin == 0)
			com = command.substr(begin, end-begin);
		else
			params.push_back(command.substr(begin, end-begin));
	}
	if (com == L"")
		com = command.substr(begin);
	else
		params.push_back(command.substr(begin));


	/* Exec the command */
	if (L"show" == com) { show(params); return true; }
	else if (L"hide" == com) { hide(params); return true; }
	else if (L"showall" == com) { showall(params); return true; }
	else if (L"showonly" == com) { showonly(params); return true; }
	else return false;
}

void Cmd::hide(vector<STRING> &params)
{
	for (vector<STRING>::iterator p = params.begin(); p != params.end(); p++)
	{
		if (*p != NONE_CATEGORY) interface->hidden_categories.insert(*p);
	}
}

void Cmd::show(vector<STRING> &params)
{
	for (vector<STRING>::iterator p = params.begin(); p != params.end(); p++)
	{
		interface->hidden_categories.erase(*p);
	}
}

void Cmd::showall(vector<STRING> &params)
{
	interface->hidden_categories.clear();
}

void Cmd::showonly(vector<STRING> &params)
{
	/* add all the categories to the hidden list */
	interface->hidden_categories.insert(L"");
	for (set<STRING>::iterator c = categories.begin(); c != categories.end(); c++)
	{
		interface->hidden_categories.insert(*c);
	}
	/* remove the shown categories */
	for (vector<STRING>::iterator p = params.begin(); p != params.end(); p++)
	{
		interface->hidden_categories.erase(*p);
	}
	interface->hidden_categories.erase(NONE_CATEGORY);
}
