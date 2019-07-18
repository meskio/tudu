
/**************************************************************************
 * Copyright (C) 2007-2019 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#include "scheduler.h"

/*
 * when del don't update the position of the neighbours
 * because when paste it use the last position
 */

void Sched::add_recursive(pToDo todo)
{
	for (iToDo j(*todo) ; !j.end(); ++j)
		add_recursive(&(*j));

	if (todo->sched().valid())
		add(todo);
}

void Sched::add(pToDo todo)
{
	sched_l::iterator i;

	for (i = sched.begin(); (i != sched.end()) && ((*i)->sched() < todo->sched()); i++);
	if (todo->schedPosition() == 0)
	{
		if ((i != sched.end()) && ((*i)->sched() == todo->sched()))
		{
			for (; ((*i)->sched() == todo->sched()); i++);
			i--;
			todo->schedPosition() = (*i)->schedPosition() + 1;
			i++;
		}
		else
		{
			todo->schedPosition() = 1;
		}
	}
	else
	{
		for (; (i != sched.end()) && ((*i)->sched() == todo->sched()) &&
				((*i)->schedPosition() < todo->schedPosition()); i++);
	}
	sched.insert(i, todo);
}

void Sched::up(pToDo todo)
{
	sched_l::iterator i,j;

	for (i = sched.begin(); (i != sched.end()) && ((*i) != todo); i++);
	j = i; j--;

	/* if there is a task before swap them */
	if ((i != sched.begin()) && ((*i) == todo) && ((*j)->sched() == todo->sched()))
	{
		int aux = (*j)->schedPosition();
		(*j)->schedPosition() = todo->schedPosition();
		todo->schedPosition() = aux;
		sched.insert(j, todo);
		j = i; i++;
		sched.erase(j,i);
	}
}

void Sched::down(pToDo todo)
{
	sched_l::iterator i,j;

	for (i = sched.begin(); (i != sched.end()) && ((*i) != todo); i++);
	j = i; j++;

	/* if there is a task before swap them */
	if ((j != sched.end()) && ((*i) == todo) && ((*j)->sched() == todo->sched()))
	{
		sched.erase(i,j);
		int aux = (*j)->schedPosition();
		(*j)->schedPosition() = todo->schedPosition();
		todo->schedPosition() = aux;
		j++;
		sched.insert(j, todo);
	}
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

int Sched::get(sched_l& list)
{
	sched_l::iterator i;
	int num_scheds = 0;

	list.clear();
	for (i = sched.begin();  (i != sched.end()); i++)
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
