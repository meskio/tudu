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

#ifndef CMD_H
#define CMD_H

#include <string>
#include <vector>
#include <map>

class Cmd;
#include "interface.h"

/* comands definition with it's params types */
extern map<string,string> commands;

class Cmd
{
public:
	Cmd();

	void get_interface(Interface *i);
	bool cmd(string command); /* return if should be redraw the screen */
private:
	Interface *interface;

	void hide(vector<string> &params);
	void show(vector<string> &params);
	void showall(vector<string> &params);
	void showonly(vector<string> &params);
};
#endif

