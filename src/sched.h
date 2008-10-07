
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

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <list>
#include "data.h"
#include "date.h"

typedef list<pToDo> sched_l;

class Sched
{
public:
	void add(pToDo todo);
	void add_recursive(pToDo todo);
	void del(pToDo todo);
	void del_recursive(pToDo todo);
	int get(Date& from, Date& to, sched_l& list);
	int get(Date& from, sched_l& list);
private:
	list<pToDo> sched;
};

#endif
