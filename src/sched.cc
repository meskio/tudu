
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

#include "sched.h"

void Sched::add_recursive(pToDo todo)
{
	for (iToDo j(*todo) ; !j.end(); ++j)
		add_recursive(&(*j));

	if (todo->sched().valid())
	{
		sched_l::iterator i;
		for (i = sched.begin(); (i != sched.end()) && ((*i)->sched() < todo->sched()); i++);
		sched.insert(i, todo);
	}
}

void Sched::add(pToDo todo)
{
	sched_l::iterator i;

	for (i = sched.begin(); (i != sched.end()) && ((*i)->sched() < todo->sched()); i++);
	sched.insert(i, todo);
}

void Sched::del(pToDo todo)
{
	sched.remove(todo);
}

void Sched::del_recursive(pToDo todo)
{
	for (iToDo i(*todo) ; !i.end(); ++i)
		del_recursive(&(*i));
	sched.remove(todo);
}

int Sched::get(Date& from, Date& to, sched_l& list)
{
	sched_l::iterator i;
	int num_scheds = 0;

	list.clear();
	for (i = sched.begin(); (i != sched.end()) && ((*i)->sched() < from); i++);
	for (;  (i != sched.end()) && ((*i)->sched() < to); i++)
	{
		num_scheds++;
		list.push_back(*i);
	}

	return num_scheds;
}

int Sched::get(Date& from, sched_l& list)
{
	sched_l::iterator i;
	int num_scheds = 0;

	list.clear();
	for (i = sched.begin(); (i != sched.end()) && ((*i)->sched() < from); i++);
	for (;  i != sched.end(); i++)
	{
		num_scheds++;
		list.push_back(*i);
	}

	return num_scheds;
}
