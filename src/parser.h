
/**************************************************************************
 * Copyright (C) 2007-2015 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#ifndef PARSER_H
#define PARSER_H

#include "includes.h"
#include "data.h"
#include "sched.h"
#include "text.h"


class Parser
{
public:
	Parser(const char* path);
	~Parser();

	bool parse(ToDo& todo, Sched& sched);
private:
	wifstream file;
	wstring str;
	wstring txt;
	bool collect_text;
	bool deadline;
	bool scheduled;

	void ptag(iToDo& iterator, Sched& sched);
	void patt(iToDo& iterator);
	wchar_t amp();
};

class Writer
{
public:
	Writer(const char* pathToSave, ToDo& t);
	~Writer();

	bool save(); /* return if was succesfull */
private:
	wofstream file;
	ToDo& todo;
	iToDo* i;
	char path[256];

	void _save();
	void amp(wstring& str);
};

#endif
