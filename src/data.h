
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

#ifndef DATA_H
#define DATA_H

#include "includes.h"
class ToDo;
class iToDo;
typedef ToDo* pToDo;
typedef list<pToDo>::iterator cursor_t;
#include "text.h"
#include "date.h"

extern set<wstring> categories;
#define NONE_CATEGORY L"NONE"

struct child_info {
	int percent;
	Date deadline;
};


/*
 * Compare function for sort
 */
bool cmp(pToDo t1, pToDo t2);

class ToDo
{
public:
	ToDo();
	~ToDo();

	wstring& getTitle();
	Text& getText();
	Date& deadline();
	Date& sched();
	int& schedPosition();
	bool& done();
	int& priority();
	set<wstring>& getCategories();
	wstring getCategoriesStr();
	void addCategory(const wstring& c);
	void setCategories(set<wstring>& c);
	void setCategoriesStr(wstring& c);
	bool haveChild();
	child_info getChildInfo();
	bool& getCollapse(); // real collapse
	bool& actCollapse(); // if must be collapse now
protected:
	int  _order; // unic number that shows the order of the todo
	wstring title;
	bool _done;
	Date _deadline;
	Date _sched;
	int sched_position; // position on the sched list, use for sort the list
	int  _priority;
	set<wstring> _category;
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
	bool operator==(const iToDo& t);
	bool operator!=(const iToDo& t) { return !operator==(t); }
	bool operator++();
	bool operator--();
	bool in();
	bool out();
	void next();
	void prev();
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
	void sort(wstring order);
	bool search(wstring& str);
	bool searchUp(wstring& str);
protected:
	pToDo root;
	stack<list<pToDo>::iterator> path;
	stack<pToDo> parents;
	cursor_t cursor;
};

#endif
