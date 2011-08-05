
/**************************************************************************
 * Copyright (C) 2007-2011 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#include "data.h"

set<wstring> categories;

ToDo::ToDo() 
{
	collapse = false;
	cursor_in = false;
	title = L"";
	text = L"";
	_done = false;
	_priority = 0;
	sched_position = 0;
}

ToDo::~ToDo()
{
	list<pToDo>::iterator it;
	for (it = childs.begin(); it != childs.end(); it++)
	{
		delete (*it);
	}
}


wstring& ToDo::getTitle()
{
	return title;
}

Text& ToDo::getText()
{
	return text;
}

Date& ToDo::deadline()
{
	return _deadline;
}

Date& ToDo::sched()
{
	return _sched;
}

int& ToDo::schedPosition()
{
	return sched_position;
}

bool& ToDo::done()
{
	return _done;
}

int& ToDo::priority()
{
	return _priority;
}

set<wstring>& ToDo::getCategories()
{
	return _category;
}

wstring ToDo::getCategoriesStr()
{
	wstring cat(L"");
	for (set<wstring>::iterator it = _category.begin();
	     it != _category.end(); it++)
	{
		cat += (*it);
		if (it != --(_category.end()))
		   	cat += L',';
	}

	return cat;
}

void ToDo::addCategory(const wstring& c)
{
	_category.insert(c);
	if (c != NONE_CATEGORY) categories.insert(c);
}

void ToDo::setCategories(set<wstring>& c)
{
	_category = c;
	for (set<wstring>::iterator it = c.begin(); it != c.end(); it++)
		if (*it != NONE_CATEGORY) categories.insert(*it);
}

void ToDo::setCategoriesStr(wstring& c)
{
	_category.clear();
	if (c.empty()) return;

	size_t posEnd, posStart = 0;
	while ((posEnd = c.find(L',', posStart)) != wstring::npos)
	{
		_category.insert(c.substr(posStart, posEnd-posStart));
		posStart = posEnd+1;
	}
	_category.insert(c.substr(posStart));
}

bool ToDo::haveChild()
{
	return !childs.empty();
}

child_info ToDo::getChildInfo()
{
	child_info chinf;
	if (!haveChild())
	{
		chinf.percent = _done? 100: 0;
		if (!_done)
			chinf.deadline = _deadline;
	}
	else
	{
		iToDo i(*this);
		int sum_percent = 0, j;
		child_info aux;
		if (!_done) chinf.deadline = _deadline;
		for (j = 0; !(i.end()); ++i, ++j)
		{
			aux = i->getChildInfo();
			sum_percent += aux.percent;
			if (aux.deadline.valid() && 
					((aux.deadline < chinf.deadline)
					|| !chinf.deadline.valid()))
				chinf.deadline = aux.deadline;
		}
		chinf.percent = sum_percent/j;
	}

	return chinf;
}

bool& ToDo::getCollapse()
{
	return collapse;
}

bool& ToDo::actCollapse()
{
	return cursor_in;
}

wstring cmpOrder;
bool cmp(pToDo t1, pToDo t2)
{
	bool res = true;
	bool out = false;
	child_info chinf1, chinf2;

	chinf1 = t1->getChildInfo();
	chinf2 = t2->getChildInfo();
	for (int i = 0; i<16 && !out; i++)
	{
		switch (cmpOrder[i])
		{
			/* Title */
			case 't':
				if (t1->title != t2->title)
				{
					res = (t1->title < t2->title);
					out = true;
				}
				break;
			case 'T':
				if (t1->title != t2->title)
				{
					res = (t1->title > t2->title);
					out = true;
				}
				break;
			/* Done */
			case 'd':
				if (t1->_done != t2->_done)
				{
					res = t2->_done;
					out = true;
				}
				else if (!t1->_done && (chinf1.percent == 100))
				{
					res = false;
					out = true;
				}
				else if (!t2->_done && (chinf2.percent == 100))
				{
					res = true;
					out = true;
				}
				break;
			case 'D':
				if (t1->_done != t2->_done)
				{
					res = t1->_done;
					out = true;
				}
				else if (!t1->_done && (chinf1.percent == 100))
				{
					res = true;
					out = true;
				}
				else if (!t2->_done && (chinf2.percent == 100))
				{
					res = false;
					out = true;
				}
				break;
			/* Deadline */
			case 'l':
				if (chinf1.deadline != chinf2.deadline)
				{
					if (chinf1.deadline.valid())
						res = (chinf1.deadline < chinf2.deadline);
					else
						res = false;
					out = true;
				}
				break;
			case 'L':
				if (chinf1.deadline != chinf2.deadline)
				{
					res = (chinf1.deadline > chinf2.deadline);
					out = true;
				}
				break;
			/* Priority */
			case 'p':
				if (t1->_priority != t2->_priority)
				{
					int p1, p2;
					p1 = t1->_priority;
					p2 = t2->_priority;
					if (!p1) p1 = 6;
					if (!p2) p2 = 6;
					res=(p1  < p2);
					out = true;
				}
				break;
			case 'P':
				if (t1->_priority != t2->_priority)
				{
					res=(t1->_priority > t2->_priority);
					out = true;
				}
				break;
			/* Category */
			case 'c':
				if (t1->_category != t2->_category)
				{
					if (t1->_category.empty())
						res = false;
					else if (t2->_category.empty())
						res = true;
					else
						res = (t1->getCategoriesStr()<t2->getCategoriesStr());
					out = true;
				}
				break;
			case 'C':
				if (t1->_category != t2->_category)
				{
					if (t1->_category.empty())
						res = true;
					else if (t2->_category.empty())
						res = false;
					else
						res = (t1->getCategoriesStr()>t2->getCategoriesStr());
					out = true;
				}
				break;
			/* Percent */
			case 'e':
				if (chinf1.percent != chinf2.percent)
				{
					res = (chinf1.percent < chinf2.percent);
					out = true;
				}
				break;
			case 'E':
				if (chinf1.percent != chinf2.percent)
				{
					res = (chinf1.percent > chinf2.percent);
					out = true;
				}
				break;
			/* User */
			case 'u':
				if (t1->_order != t2->_order)
				{
					res = (t1->_order < t2->_order);
					out = true;
				}
				break;
			case 'U':
				if (t1->_order != t2->_order)
				{
					res = (t1->_order > t2->_order);
					out = true;
				}
				break;
		}
	}

	return res;
}

void ToDo::_sort()
{
	childs.sort(cmp);
	for (cursor_t i = childs.begin(); i != childs.end(); i++)
	{
		(*i)->_sort();
	}
}


#define parent parents.top()

iToDo::iToDo(ToDo& t)
{
	root = &t;
	cursor = t.childs.begin();
	parents.push(&t);
}

iToDo& iToDo::operator=(const iToDo& t)
{
	path = t.path;
	parents = t.parents;
	cursor = t.cursor;
	return *this;
}

bool iToDo::operator==(const iToDo& t)
{
	return (*t.cursor) == (*cursor);
}

bool iToDo::operator++()
{
	if (!end())
	{
		++cursor;
		return true;
	}
	else
	{
		return false;
	}
}

bool iToDo::operator--()
{
	if (!begin())
	{
		--cursor;
		return true;
	}
	else
	{
		return false;
	}
}

bool iToDo::in()
{
	if (!end())
	{
		parents.push(*cursor);
		path.push(cursor);
		cursor = (*cursor)->childs.begin();
		return true;
	}
	else
	{
		return false;
	}
}

bool iToDo::out()
{
	if (!path.empty())
	{
		cursor = path.top();
		path.pop();
		parents.pop();
		return true;
	}
	else
	{
		return false;
	}
}

void iToDo::next()
{
	in();
	/* if there is no childs */
	if (end())
	{
		/* while is the end of the childs go to the parent and move forward */
		while (end() && !path.empty()) {
			out();
			operator++();
		}

		/* if is the last task continue from the beginning */
		if (end() && path.empty())
			cursor = parent->childs.begin();
	}
}

void iToDo::prev()
{
	/* if is not the first child move up and go as deep as possible */
	if (!begin())
	{
		operator--();
		while ((*cursor)->haveChild())
		{
			in();
			while (operator++());
			operator--();
		}
	}
	/* if is the first child go to the parent */
	else
	{
		if (!out())
		{
			cursor = parent->childs.end();
			cursor--;
			while ((*cursor)->haveChild())
			{
				in();
				while (operator++());
				operator--();
			}
		}
	}
}

ToDo &iToDo::operator*()
{
	if (!end()) return *(*cursor);
	else 
	{
		return *(parent);
	}
}

ToDo *iToDo::operator->()
{
	if (!end()) return *cursor;
	else 
	{
		return parent;
	}
}

bool iToDo::end()
{
	return ((parent->childs.end() == cursor) && (parent->childs.end()-- == cursor));
}

bool iToDo::begin()
{
	return (parent->childs.begin() == cursor);
}

int iToDo::percentUp()
{
	return parent->getChildInfo().percent;
}

void iToDo::addChild(pToDo p)
{
	if (!end())
	{
		p->_order = (*cursor)->_order+1;
		++cursor;
		for (cursor_t c = cursor; c != parent->childs.end(); c++)
			(*c)->_order++;
	}
	else
	{
		if (!begin())
		{
			--cursor;
			p->_order = (*cursor)->_order+1;
			++cursor;
		}
		else
		{
			p->_order = 0;
		}
	}
	cursor = parent->childs.insert(cursor, p);
}

void iToDo::addChildUp(pToDo p)
{
	if (end() && begin())
	{
		p->_order = 0;
	}
	else
	{
		p->_order = (*cursor)->_order;
		for (cursor_t c = cursor; c != parent->childs.end(); c++)
			(*c)->_order++;
	}
	cursor = parent->childs.insert(cursor, p);
}

void iToDo::del()
{
	cursor = parent->childs.erase(cursor);
	
	/* if is the last task create an empty one */
	if (end() && begin() && path.empty())
	{
		addChild(new ToDo());
	}
}

int iToDo::depth()
{
	return path.size();
}

void iToDo::sort(wstring order)
{
	cmpOrder = order;
	if (cmpOrder.find(L'u') == wstring::npos)
		cmpOrder += L'u';
	root->_sort();
}

bool iToDo::search(wstring& str)
{
	pToDo begin = *cursor;

	while (1)
	{
		next();
		/* if the title contains str */
		if ((*cursor)->title.find(str, 0) != wstring::npos)
			return true;
		/* if is the same as the beginning */
		if (*cursor == begin)
			return false;
	}
}

bool iToDo::searchUp(wstring& str)
{
	pToDo begin = *cursor;

	while (1)
	{
		prev();
		/* if the title contains str */
		if ((*cursor)->title.find(str, 0) != wstring::npos)
			return true;
		/* if is the same as the beginning */
		if (*cursor == begin)
			return false;
	}
}
