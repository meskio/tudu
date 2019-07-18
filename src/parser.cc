
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

#include "parser.h"

Parser::Parser(const char* path)
{
	file.imbue(locale(""));
	file.open(path);
}

Parser::~Parser()
{
	file.close();
}

bool Parser::parse(ToDo& todo, Sched& sched)
{
	wchar_t ch;
	bool tag = false, att = false;
	iToDo iterator(todo);

	if (!file) return false; //if the file is not open

	str = L"";
	collect_text = false;
	deadline = false;
	scheduled = false;
	while (file.get(ch))
	{
		switch (ch)
		{
			case L'<':
				if (tag) return false;
				tag = true;
				if (collect_text)
				{
					txt = str;
					str = L"";
				}
				else collect_text = true;
				break;
			case L'>':
				if (!tag) return false;
				tag = false;
				collect_text = false;
				if (!att) ptag(iterator,sched);
				else {
					patt(iterator);
					att = false;
				}
				str = L"";
				break;
			case L'\t':
			case L' ':
				if (tag)
				{
					if (!att)
					{
						ptag(iterator,sched);
						att = true;
						str = L"";
					} else if (str.length()) {
						patt(iterator);
						str = L"";
					}
				} else
					if (collect_text) str += ch;
				break;
			case L'&':
				if (collect_text) str += amp();
				break;
			default:
				if (collect_text) str += ch;
		}
	}

	// in case of empty file create an empty task
	if (!todo.haveChild())
		iterator.addChild(new ToDo());

	return true;
}

wchar_t Parser::amp()
{
	wchar_t ch;
	wstring str = L"";

	file.get(ch);
	while (L';' != ch)
	{
		str += ch;
		file.get(ch);
	}
	if (L"amp" == str) return L'&';
	if (L"lt" == str) return L'<';
	if (L"gt" == str) return L'>';
	else return L' ';
}

void Parser::ptag(iToDo& iterator, Sched& sched)
{
	static bool first_todo = true;

	if (L"todo" == str)
	{
		if (first_todo) first_todo = false;
		else {
			iterator.addChild(new ToDo());
			iterator.in();
		}
	}
	if (L"/todo" == str)
	{
		iterator.out();
	}
	if (L"title" == str)
	{
		collect_text = true;
	}
	if (L"/title" == str)
	{
		iterator->getTitle() = txt;
	}
	if (L"deadline" == str)
	{
		deadline = true;
	}
	if (L"/deadline" == str)
	{
		deadline = false;
	}
	if (L"day" == str)
	{
		collect_text = true;
	}
	if (L"/day" == str)
	{
		char num[3];
		wcstombs(num, txt.c_str(), 3);
		int day = atoi(num);
		if (deadline)
		{
			iterator->deadline().day() = day;
		}
		else if (scheduled)
			iterator->sched().day() = day;
	}
	if (L"month" == str)
	{
		collect_text = true;
	}
	if (L"/month" == str)
	{
		char num[3];
		wcstombs(num, txt.c_str(), 3);
		int month = atoi(num);
		if (deadline)
		{
			iterator->deadline().month() = month;
		}
		else if (scheduled)
			iterator->sched().month() = month;
	}
	if (L"year" == str)
	{
		collect_text = true;
	}
	if (L"/year" == str)
	{
		char num[5];
		wcstombs(num, txt.c_str(), 5);
		int year = atoi(num);
		if (deadline)
		{
			iterator->deadline().year() = year;
		}
		else if (scheduled)
			iterator->sched().year() = year;
	}
	if (L"position" == str)
	{
		collect_text = true;
	}
	if (L"/position" == str)
	{
		char num[8];
		wcstombs(num, txt.c_str(), 8);
		iterator->schedPosition() = atoi(num);
	}
	if (L"priority" == str)
	{
		collect_text = true;
	}
	if (L"/priority" == str)
	{
		char num[2];
		wcstombs(num, txt.c_str(), 2);
		iterator->priority() = atoi(num);
	}
	if (L"category" == str)
	{
		collect_text = true;
	}
	if (L"/category" == str)
	{
		iterator->addCategory(txt);
	}
	if (L"text" == str)
	{
		collect_text = true;
	}
	if (L"/text" == str)
	{
		if (L'\n' == txt[0]) txt.erase(0,1);
		iterator->getText() = txt;
	}
	if (L"scheduled" == str)
	{
		scheduled = true;
	}
	if (L"/scheduled" == str)
	{
		if (iterator->sched().valid())
			sched.add(&(*iterator));
		scheduled = false;
	}
}

void Parser::patt(iToDo& iterator)
{
	wstring name, data;
	int eq_pos = str.find(L"=");

	name = str.substr(0, eq_pos);
	eq_pos++;
	data = str.substr(eq_pos, str.length()-eq_pos);
	if (L"done" == name)
	{
		if (L"\"yes\"" == data) iterator->done() = true;
		else iterator->done() = false;
	}
	else if (L"collapse" == name)
	{
		if (L"\"yes\"" == data) iterator->getCollapse() = true;
		else iterator->getCollapse() = false;
	}
}


Writer::Writer(const char* pathToSave, ToDo& t): todo(t) 
{
	strncpy(path, pathToSave, 255);
	path[255] = '\0';
	file.imbue(locale(""));
}

Writer::~Writer() {}

bool Writer::save()
{
	file.open(path);
	file << "<?xml version=\"1.0\"?>" << endl;

	/* there was an error writing */
	if (file.fail()) return false;

	file << "<!DOCTYPE tudu SYSTEM \"" << PATH_DTD << "\">" << endl;
	file << "<todo>" << endl;
	i = new iToDo(todo);
	i->sort(L"");
	while(--(*i));
	_save();
	delete i;
	file << "</todo>" << endl;
	file.close();

	return true;
}

#define putTabs(num) \
	for (int j = 0; j < num+1; j++) file << "\t"

void Writer::_save()
{
	for (; !((*i).end()); ++(*i))
	{
		putTabs((*i).depth());
		file << "<todo done=\"";
		if ((*i)->done()) file << "yes";
		else file << "no";
		file << "\" collapse=\"";
		if ((*i)->getCollapse()) file << "yes";
		else file << "no";
		file << "\">" << endl;
		if (L"" != (*i)->getTitle())
		{
			wstring str = (*i)->getTitle();

			putTabs((*i).depth()+1);
			amp(str);
			file << "<title>" <<  str
					<< "</title>" << endl;
		}
		if ((*i)->deadline().valid())
		{
			Date &date = (*i)->deadline();

			putTabs((*i).depth()+1);
			file << "<deadline>" << endl;
			putTabs((*i).depth()+2);
			file << "<day>" << date.day() << "</day>" << endl;
			putTabs((*i).depth()+2);
			file << "<month>" << date.month() << "</month>" << endl;
			putTabs((*i).depth()+2);
			char str[10]; /* fixing problem with locales that prints 2,010 */
			sprintf(str,"%i",date.year());
			file << "<year>" << str << "</year>" << endl;
			putTabs((*i).depth()+1);
			file << "</deadline>" << endl;
		}
		if ((*i)->priority())
		{
			putTabs((*i).depth()+1);
			file << "<priority>" << (*i)->priority() << 
					"</priority>" << endl;
		}
		if (!(*i)->getCategories().empty())
		{
			for (set<wstring>::iterator it = (*i)->getCategories().begin();
			     it != (*i)->getCategories().end(); it++)
			{
				putTabs((*i).depth()+1);
				file << "<category>" << *it << 
						"</category>" << endl;
			}
		}
		if ((*i)->getText() != L"")
		{
			wstring str = (*i)->getText().getStr();

			putTabs((*i).depth()+1);
			amp(str);
			file << "<text>" << str;
			file << "</text>" << endl;
		}
		if ((*i)->sched().valid())
		{
			Date &date = (*i)->sched();

			putTabs((*i).depth()+1);
			file << "<scheduled>" << endl;
			putTabs((*i).depth()+2);
			file << "<day>" << date.day() << "</day>" << endl;
			putTabs((*i).depth()+2);
			file << "<month>" << date.month() << "</month>" << endl;
			putTabs((*i).depth()+2);
			char str[10]; /* fixing problem with locales that prints 2,010 */
			sprintf(str,"%i",date.year());
			file << "<year>" << str << "</year>" << endl;
			putTabs((*i).depth()+2);
			file << "<position>" << (*i)->schedPosition() << "</position>" << endl;
			putTabs((*i).depth()+1);
			file << "</scheduled>" << endl;
		}
		(*i).in();
		_save();
		(*i).out();
		putTabs((*i).depth());
		file << "</todo>" << endl;
	}
}

#define replace(orig, alt) \
	{ \
	index = 0; \
	while ((index = str.find(orig,index)) != wstring::npos) \
	{ \
		str.erase(index,1); \
		str.insert(index, alt); \
		index += 4; \
	} \
	} while (0)

void Writer::amp(wstring& str)
{
	wstring::size_type index;
	replace(L'&', L"&amp;");
	replace(L'<', L"&lt;");
	replace(L'>', L"&gt;");
}
