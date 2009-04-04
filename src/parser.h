
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

#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include "data.h"
#include "sched.h"
#include "text.h"

#ifndef SHARE_DIR
#define SHARE_DIR "/usr/local/share/tudu"
#endif
#define PATH_DTD SHARE_DIR"/tudu.dtd"

class Parser
{
public:
	Parser(const char* path);
	~Parser();

	bool parse(ToDo& todo, Sched& sched);
private:
	Sched *sched;
	ifstream file;
	string str;
	string txt;
	bool collect_text;
	bool deadline;
	bool scheduled;

	void ptag(iToDo& iterator, Sched& sched);
	void patt(iToDo& iterator);
	char amp();
};

class Writer
{
public:
	Writer(const char* pathToSave, ToDo& t);
	~Writer();

	void save();
private:
	ofstream file;
	ToDo& todo;
	iToDo* i;
	char path[128];

	void _save();
	void amp(string& str);
};

#endif
