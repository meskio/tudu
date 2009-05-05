
/**************************************************************************
 * Copyright (C) 2007-2009 Ruben Pollan Bella <meskio@amedias.org>        *
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

#include "parser.h"

Parser::Parser(const char* path)
{
	file.open(path);
}

Parser::~Parser()
{
	file.close();
}

bool Parser::parse(ToDo& todo, Sched& sched)
{
	char ch;
	bool tag = false, att = false;
	iToDo iterator(todo);

	if (!file) return false; //if the file is not open

	str = "";
	collect_text = false;
	deadline = false;
	scheduled = false;
	while (file.get(ch))
	{
		switch (ch)
		{
			case '<':
				if (tag) return false;
				tag = true;
				if (collect_text)
				{
					txt = str;
					str = "";
				}
				else collect_text = true;
				break;
			case '>':
				if (!tag) return false;
				tag = false;
				collect_text = false;
				if (!att) ptag(iterator,sched);
				else {
					patt(iterator);
					att = false;
				}
				str = "";
				break;
			case '\t':
			case ' ':
				if (tag)
				{
					if (!att)
					{
						ptag(iterator,sched);
						att = true;
						str = "";
					} else if (str.length()) {
						patt(iterator);
						str = "";
					}
				} else
					if (collect_text) str += ch;
				break;
			case '&':
				if (collect_text) str += amp();
				break;
			default:
				if (collect_text) str += ch;
		}
	}

	return true;
}

char Parser::amp()
{
	char ch;
	string str = "";

	file.get(ch);
	while (';' != ch)
	{
		str += ch;
		file.get(ch);
	}
	if ("amp" == str) return '&';
	if ("lt" == str) return '<';
	if ("gt" == str) return '>';
	else return ' ';
}

void Parser::ptag(iToDo& iterator, Sched& sched)
{
	static bool first_todo = true;

	if ("todo" == str)
	{
		if (first_todo) first_todo = false;
		else {
			iterator.addChild(new ToDo());
			iterator.in();
		}
	}
	if ("/todo" == str)
	{
		iterator.out();
	}
	if ("title" == str)
	{
		collect_text = true;
	}
	if ("/title" == str)
	{
		iterator->getTitle() = txt;
	}
	if ("deadline" == str)
	{
		deadline = true;
	}
	if ("/deadline" == str)
	{
		deadline = false;
	}
	if ("day" == str)
	{
		collect_text = true;
	}
	if ("/day" == str)
	{
		if (deadline)
			iterator->deadline().day() = atoi(txt.c_str());
		else if (scheduled)
			iterator->sched().day() = atoi(txt.c_str());
	}
	if ("month" == str)
	{
		collect_text = true;
	}
	if ("/month" == str)
	{
		if (deadline)
			iterator->deadline().month() = atoi(txt.c_str());
		else if (scheduled)
			iterator->sched().month() = atoi(txt.c_str());
	}
	if ("year" == str)
	{
		collect_text = true;
	}
	if ("/year" == str)
	{
		if (deadline)
			iterator->deadline().year() = atoi(txt.c_str());
		else if (scheduled)
			iterator->sched().year() = atoi(txt.c_str());
	}
	if ("position" == str)
	{
		collect_text = true;
	}
	if ("/position" == str)
	{
		if (scheduled)
			iterator->schedPosition() = atoi(txt.c_str());
	}
	if ("priority" == str)
	{
		collect_text = true;
	}
	if ("/priority" == str)
	{
		iterator->priority() = atoi(txt.c_str());
	}
	if ("category" == str)
	{
		collect_text = true;
	}
	if ("/category" == str)
	{
		iterator->setCategory(txt);
	}
	if ("text" == str)
	{
		collect_text = true;
	}
	if ("/text" == str)
	{
		if ('\n' == txt[0]) txt.erase(0,1);
		iterator->getText() = txt;
	}
	if ("scheduled" == str)
	{
		scheduled = true;
	}
	if ("/scheduled" == str)
	{
		if (iterator->sched().valid())
			sched.add(&(*iterator));
		scheduled = false;
	}
}

void Parser::patt(iToDo& iterator)
{
	string name, data;
	int eq_pos = str.find("=");

	name = str.substr(0, eq_pos);
	eq_pos++;
	data = str.substr(eq_pos, str.length()-eq_pos);
	if ("done" == name)
	{
		if ("\"yes\"" == data) iterator->done() = true;
		else iterator->done() = false;
	}
	else if ("collapse" == name)
	{
		if ("\"yes\"" == data) iterator->getCollapse() = true;
		else iterator->getCollapse() = false;
	}
}


Writer::Writer(const char* pathToSave, ToDo& t): todo(t) 
{
	strncpy(path, pathToSave, 128);
}

Writer::~Writer() {}

void Writer::save()
{
	file.open(path);
	file << "<?xml version=\"1.0\"?>" << endl;
	file << "<!DOCTYPE tudu SYSTEM \"" << PATH_DTD << "\">" << endl;
	file << "<todo>" << endl;
	i = new iToDo(todo);
	i->sort((char*)"");
	while(--(*i));
	_save();
	delete i;
	file << "</todo>" << endl;
	file.close();
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
		if ("" != (*i)->getTitle())
		{
			string str = (*i)->getTitle();

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
			file << "<year>" << date.year() << "</year>" << endl;
			putTabs((*i).depth()+1);
			file << "</deadline>" << endl;
		}
		if ((*i)->priority())
		{
			putTabs((*i).depth()+1);
			file << "<priority>" << (*i)->priority() << 
					"</priority>" << endl;
		}
		if (!(*i)->getCategory().empty())
		{
			putTabs((*i).depth()+1);
			file << "<category>" << (*i)->getCategory() << 
					"</category>" << endl;
		}
		if ((*i)->getText() != "")
		{
			string str = (*i)->getText().getStr();

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
			file << "<year>" << date.year() << "</year>" << endl;
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
	while ((index = str.find(orig,index)) != string::npos) \
	{ \
		str.erase(index,1); \
		str.insert(index, alt); \
		index += 4; \
	} \
	} while (0)

void Writer::amp(string& str)
{
	string::size_type index;
	replace('&', "&amp;");
	replace('<', "&lt;");
	replace('>', "&gt;");
}
