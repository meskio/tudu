/**************************************************************************
 * Copyright (C) 2007-2009 Ruben Pollan Bella <meskio@amedias.org>        *
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
#include "data.h"

map<string,string> commands;

Cmd::Cmd()
{
	/* initialize commands */
	commands["hide"] = "category";
	commands["show"] = "category";
	commands["showall"] = "category";
	commands["showonly"] = "category";
}

void Cmd::get_interface(Interface *i)
{
	interface = i;
}

void Cmd::cmd(string command)
{
	vector<string> params;

	/* Get the command and params in text */
	for (size_t begin = 0, end = command.find(' ', 0); string::npos != end;
				begin = end+1, end = command.find(' ', begin))
	{
		params.push_back(command.substr(begin, end-begin));
	}

	/* Exec the command */
	if ("show" == params[0]) show(params);
	else if ("hide" == params[0]) hide(params);
}

void Cmd::hide(vector<string> &params)
{
	for (vector<string>::iterator p = params.begin(); p != params.end(); p++)
	{
		interface->hidden_categories.insert(*p);
	}
}

void Cmd::show(vector<string> &params)
{
	for (vector<string>::iterator p = params.begin(); p != params.end(); p++)
	{
		interface->hidden_categories.erase(*p);
	}
}

void Cmd::showall(vector<string> &params)
{
	interface->hidden_categories.clear();
}

void Cmd::showonly(vector<string> &params)
{
	/* add all the categories to the hidden list */
	interface->hidden_categories.insert("");
	for (set<string>::iterator c = categories.begin(); c != categories.end(); c++)
	{
		interface->hidden_categories.insert(*c);

	}
	/* remove the shown categories */
	for (vector<string>::iterator p = params.begin(); p != params.end(); p++)
	{
		interface->hidden_categories.erase(*p);
	}
}
