
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

#ifndef DATA_H
#define DATA_H

#include <string.h>
#include <list>
#include <stack>
#include <string>
#include <algorithm>
#include "text.h"
#include "date.h"
using namespace std;

struct child_info {
	int percent;
	Date deadline;
};
	

class ToDo;
class iToDo;
typedef ToDo* pToDo;
typedef list<pToDo>::iterator cursor_t;

/*
 * Compare function for sort
 */
bool cmp(pToDo t1, pToDo t2);

class ToDo
{
public:
	ToDo(string t = "", bool isDone = false, string txt = "");
	~ToDo();

	string& getTitle();
	Text& getText();
	Date& deadline();
	Date& sched();
	int& schedPosition();
	bool& done();
	int& priority();
	string& category();
	bool haveChild();
	child_info getChildInfo();
	int line;
	bool& getCollapse(); // real collapse
	bool& actCollapse(); // if must be collapse now
protected:
	int  _order; // unic number that shows the order of the todo
	string title;
	bool _done;
	Date _deadline;
	Date _sched;
	int sched_position; // position on the sched list, use for sort the list
	int  _priority;
	string _category;
	Text text;
	list<pToDo> childs;
	bool collapse; //real param
	bool cursor_in; //if the cursor is inside, use for actCollapse

	void _sort(); // sort the children with given order

	friend class iToDo;
	friend bool cmp(pToDo , pToDo );
};

/*
 * Iterator for ToDo class
 */
class iToDo 
{
public:
	iToDo(ToDo& t);

	iToDo& operator=(const iToDo& t);
	bool operator++();
	bool operator--();
	bool in();
	bool out();
	ToDo &operator*();
	ToDo *operator->();
	bool end();
	bool begin();
	int percentUp();
	void addChild(pToDo p);
	void addChildUp(pToDo p);
	void del();
	int depth();
	/*
	 * sort all the todo with given order
	 * t = title
	 * d = done
	 * l = deadline
	 * p = priority
	 * c = category
	 * e = percent
	 * u = user
	 * the same leters in capital are for inverse order
	 */
	void sort(char order[]);
protected:
	pToDo root;
	stack<list<pToDo>::iterator> path;
	stack<pToDo> parents;
	cursor_t cursor;
};

#endif
