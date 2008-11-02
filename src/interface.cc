
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

#include "interface.h"

#define cursor_line()  (cursor->line-tree_begin)
#define isCollapse() ((cursor->getCollapse()) && (!cursor->actCollapse()))

Interface::Interface(Screen &s, iToDo &t, Sched& sch, Config &c, Writer &w) 
		: screen(s), cursor(t), sched(sch), config(c), writer(w), copied(NULL)
{
	tree_begin = 0;
	tree_end = screen.treeLines();

	strcpy(sortOrder, " ");
	strncat(sortOrder, config.getSortOrder(), 16);
	cursor.sort(sortOrder);
	while (--cursor);
}

Interface::~Interface()
{
	delete copied;
}

void Interface::main()
{
	int key;
	string action;

	while (cursor.out());
	while (--cursor);
	drawTodo();

	while (true) 
	{
		key = getch();
		screen.infoClear();
		if (KEY_RESIZE == key) resizeTerm();
		if (KEY_LEFT == key) left();
		if (KEY_RIGHT == key) right();
		if (KEY_DOWN  == key) down();
		if (KEY_UP == key) up();
		if (config.getAction(key, action))
		{
			if ("quit" == action)
			{
				writer.save();
				break;
			}
			if ("quitNoSave" == action) break;
			if ("out" == action) left();
			if ("in" == action) right();
			if ("down" == action) down();
			if ("up" == action) up();
			if ("move_down" == action) move_down();
			if ("move_up" == action) move_up();
			if ("delete" == action) del();
			if ("delDeadline" == action) delDeadline();
			if ("delPriority" == action) delPriority();
			if ("delSched" == action) delSched();
			if ("paste" == action) paste();
			if ("pasteUp" == action) pasteUp();
			if ("done" == action) done();
			if ("addTodo" == action) addLine();
			if ("addTodoUp" == action) addLineUp();
			if ("editTitle" == action) modifyLine();
			if ("editDeadline" == action) editDeadline();
			if ("setPriority" == action) setPriority();
			if ("setCategory" == action) setCategory();
			if ("editText" == action) editText();
			if ("editSched" == action) editSched();
			if ("downText" == action) downText();
			if ("upText" == action) upText();
			if ("collapse" == action) collapse();
			if ("hideDone" == action) hide_done();
			if ("sortByTitle" == action) sortByTitle();
			if ("sortByDone" == action) sortByDone();
			if ("sortByDeadline" == action) sortByDeadline();
			if ("sortByPriority" == action) sortByPriority();
			if ("sortByCategory" == action) sortByCategory();
			if ("sortByUser" == action) sortByUser();
			if ("sortRevTitle" == action) sortRevTitle();
			if ("sortRevDone" == action) sortRevDone();
			if ("sortRevDeadline" == action) sortRevDeadline();
			if ("sortRevPriority" == action) sortRevPriority();
			if ("sortRevCategory" == action) sortRevCategory();
			if ("sortRevUser" == action) sortRevUser();
			if ("save" == action) save();
			if ("help" == action) help();
		}
	}
}

void Interface::resizeTerm()
{
	screen.resizeTerm();
	tree_end = screen.treeLines() + tree_begin;
	if (cursor->line > tree_end - 4)
	{
		tree_end = cursor->line + 4;
		tree_begin = tree_end - screen.treeLines();
	}
	drawTodo();
}

void Interface::drawTodo()
{
	iToDo aux = cursor;
	screen.treeClear();
	cursor.sort(sortOrder);

	/* calculate lines */
	int line_counter = 0;
	while (cursor.out());
	while (--cursor);
	screen.infoPercent(cursor.percentUp());
	_calculateLines(line_counter);

	/* calculate the new tree_begin and tree_end */
	if (aux->line < tree_begin)
	{
		tree_begin = aux->line;
		tree_end = tree_begin + screen.treeLines();
	}
	if (aux->line >= tree_end)
	{
		tree_end = aux->line+1;
		tree_begin = tree_end - screen.treeLines();
	}

	/* redraw screen */
	cursor = aux;
	while (cursor.out());
	while (cursor->line > tree_begin) --cursor;
	_drawTodo();

	cursor = aux;
	drawCursor();
	screen.drawSched(sched, &(*cursor));
}

void Interface::_calculateLines(int& line_counter)
{
	for (; !(cursor.end()); ++cursor)
	{
		if (!isHide(cursor))
		{
			cursor->line = line_counter++;
			if (!isCollapse())
			{
				cursor.in();
				_calculateLines(line_counter);
				cursor.out();
			}
		}
	}
}

void Interface::_drawTodo()
{
	for (; !(cursor.end()); ++cursor)
	{
		if (cursor->line >= tree_end) break;
		if ((cursor->line >= tree_begin) &&
		    (!isHide(cursor)))
			screen.drawTask(cursor_line(), cursor.depth(), 
					*cursor);
		if (!isCollapse())
		{
			cursor.in();
			_drawTodo();
			cursor.out();
		}
	}
}

void Interface::eraseCursor()
{
	screen.drawTask(cursor_line(), cursor.depth(), *cursor, false);
}

void Interface::drawCursor()
{
	if (cursor->line > tree_end - 4)
	{
		tree_end = cursor->line + 4;
		tree_begin = tree_end - screen.treeLines();
		drawTodo();
	}
	else if ((tree_begin != 0) && (cursor->line < tree_begin + 4))
	{
		if (cursor->line < 4)
			tree_begin = 0;
		else
			tree_begin = cursor->line - 4;
		tree_end = tree_begin + screen.treeLines();
		drawTodo();
	}
	else
	{
		screen.drawTask(cursor_line(), cursor.depth(), *cursor, 
				true);
		screen.drawText(cursor->getText());
		screen.drawSched(sched, &(*cursor));
	}
}

bool Interface::isHide(iToDo& todo)
{
	return (config.getHideDone() && todo->done());
}

void Interface::left()
{
	eraseCursor();
	if (cursor.out() && isHide(cursor))
	{
		up();
	}
	else if (isHide(cursor))
		cursor.addChild(new ToDo());
	cursor->actCollapse() = false;
	if (cursor->getCollapse())
	{
		drawTodo();
	}
	else
	{
		drawCursor();
	}
}

void Interface::right()
{
	bool need_drawTodo = cursor->getCollapse();

	eraseCursor();
	cursor->actCollapse() = true;
	cursor.in();
	while (isHide(cursor) && ++cursor);

	if (cursor.end())
	{
		cursor.addChild(new ToDo());
		drawTodo();
		string title;
		if ((editLine(title)) && (title != ""))
		{
			cursor->getTitle() = title;

			/*
			 * Use the config collapse
			 */
			cursor.out();
			cursor->getCollapse() = config.getCollapse();
			cursor.in();
		}
		else
		{
			del();
		}
	}
	if (need_drawTodo)
	{
		drawTodo();
	}
	else
	{
		drawCursor();
	}
}

void Interface::up()
{
	eraseCursor();
	--cursor;

	/* Jump hide tasks */
	while (isHide(cursor))
	{
		if (cursor.begin()) break;
		--cursor;
	}
	while (isHide(cursor))
	{
		++cursor;
		if (cursor.end())
		{
			--cursor;
			break;
		}
	}
	if (isHide(cursor)) left();

	drawCursor();
}

void Interface::down()
{
	eraseCursor();
	++cursor;
	if (cursor.end()) --cursor;

	/* Jump hide tasks */
	while (isHide(cursor))
	{
		++cursor;
		if (cursor.end())
		{
			--cursor;
			break;
		}
	}
	while (isHide(cursor))
	{
		if (cursor.begin()) break;
		--cursor;
	}
	if (isHide(cursor)) left();

	drawCursor();
}

void Interface::move_up()
{
	pToDo t = &(*cursor);
	cursor.del();
	eraseCursor();
	--cursor;
	cursor.addChildUp(t);
	drawTodo();
}

void Interface::move_down()
{
	pToDo t = &(*cursor);
	cursor.del();
	eraseCursor();
	cursor.addChild(t);
	drawTodo();
}

void Interface::done()
{
	if (cursor->done())
		cursor->done() = false;
	else
		cursor->done() = true;
	if (isHide(cursor))
		down();
	drawTodo();
}

void Interface::del()
{
	if (copied) delete copied;
	copied = &(*cursor);
	sched.del_recursive(&(*cursor));
	cursor.del();
	if (cursor.end() & cursor.begin())
		left();
	else
		up();
	drawTodo();
}

void Interface::delDeadline()
{
	cursor->deadline().year() = 1900;
	screen.deadlineClear(cursor_line());
}

void Interface::delPriority()
{
	cursor->priority() = 0;
	screen.priorityClear(cursor_line());
}

void Interface::delSched()
{
	sched.del(&(*cursor));
	cursor->sched().year() = 1900;
}

void Interface::paste()
{
	if (copied)
	{
		cursor.addChild(copied);
		sched.add_recursive(copied);
		copied = NULL;
		drawTodo();
	}
}

void Interface::pasteUp()
{
	if (copied)
	{
		cursor.addChildUp(copied);
		sched.add_recursive(copied);
		copied = NULL;
		drawTodo();
	}
}

#define startTitle (cursor.depth() * 4 + 7)

bool Interface::editLine(string& str)
{
	bool save;

	str = cursor->getTitle();
	screen.infoMsg("Editing todo. Press enter for save it or esc for not save");
	save = screen.editTitle(cursor_line(), cursor.depth(), 
			cursor->haveChild(), str);
	screen.infoClear();
	drawTodo();
	return save;
}

void Interface::editDeadline()
{
	screen.infoMsg("Editing deadline. Press enter for save it or esc for not save");
	screen.editDeadline(cursor_line(), cursor->deadline(), cursor->done());
	screen.infoClear();
	drawTodo();
}

void Interface::setPriority()
{
	screen.infoMsg("Editing priority. Press enter for save it or esc for not save");
	screen.setPriority(cursor_line(), cursor->priority());
	screen.infoClear();
	drawTodo();
}

void Interface::setCategory()
{
	screen.infoMsg("Editing category. Press enter for save it or esc for not save");
	screen.setCategory(cursor_line(), cursor->category());
	screen.infoClear();
	drawTodo();
}

void Interface::addLine()
{

	cursor.addChild(new ToDo());
	drawTodo();
	string title;
	if ((editLine(title)) && (title != ""))
		cursor->getTitle() = title;
	else
		del();
}

void Interface::addLineUp()
{
	cursor.addChildUp(new ToDo());
	drawTodo();
	string title;
	if ((editLine(title)) && (title != ""))
		cursor->getTitle() = title;
	else
		del();
}

void Interface::modifyLine()
{
	string title;
	if (editLine(title))
		cursor->getTitle() = title;
}

void Interface::editText()
{
	char* editor = config.getEditor();
	if (strlen(editor) != 0)
	{
		char path[L_tmpnam];
		char s[86];
		char* argv[32];
		\
		int argc;
		char* res;
		string str;
		Text& text = cursor->getText();

		/* create a temporal file */
		tmpnam(path);
                int fout;                                                                                        
                while ((fout = open(path, O_CREAT|O_WRONLY|O_EXCL, S_IRUSR|S_IWUSR)) == -1)                      
                        tmpnam(path);                                                                            
                close(fout);

		ofstream ofs(path);
		ofs << text;
		ofs.close();
		sprintf(s, editor, path);

		screen.infoMsg(s);
		argc=0;
		argv[argc] = strtok(s, " \t");
		while ((res = strtok(NULL, " \t")) != NULL)
		{
			if (++argc >= 32) break;
			argv[argc] = res;
		}
		argv[argc+1] = NULL;
		
		switch (fork())
		{
			case 0:
				execvp(argv[0],argv);
				break;
			case -1:
				screen.infoMsg("I can't create a new process.");
				break;
			default:
				wait(NULL);
				break;
		}
		
		ifstream ifs(path);
		char c;
		while (ifs.get(c))
			str += c;
		text = str;
		ifs.close();
		unlink(path);

		echo();
		curs_set(1);
		screen.drawText(cursor->getText());
		noecho();
		curs_set(0);
		
		/* force redraw screen, because it don't recover well sometimes */
		resizeTerm();
	}
	else
	{
		screen.infoMsg("Editing text. Press esc for save it");
		screen.editText(cursor->getText());
		screen.infoClear();
	}
}

void Interface::editSched()
{
	sched.del(&(*cursor));
	screen.editSched(cursor->sched());
	sched.add(&(*cursor));
	screen.drawSched(sched, &(*cursor));
}

void Interface::upText()
{
	screen.scrollUpText(cursor->getText());
}

void Interface::downText()
{
	screen.scrollDownText(cursor->getText());
}

void Interface::collapse()
{
	cursor->getCollapse() = !cursor->getCollapse();
	drawTodo();
}

void Interface::hide_done()
{
	config.getHideDone() = !config.getHideDone();
	if (isHide(cursor)) up();
	if (!config.getHideDone() && cursor->getTitle().empty())
		//FIXME: if the user have an title empty will be destroy
		del();
	drawTodo();
}

void Interface::sortByTitle()
{
	sortOrder[0] = 't';
	drawTodo();
}

void Interface::sortByDone()
{
	sortOrder[0] = 'd';
	drawTodo();
}

void Interface::sortByDeadline()
{
	sortOrder[0] = 'l';
	drawTodo();
}

void Interface::sortByPriority()
{
	sortOrder[0] = 'p';
	drawTodo();
}

void Interface::sortByCategory()
{
	sortOrder[0] = 'c';
	drawTodo();
}

void Interface::sortByUser()
{
	sortOrder[0] = 'u';
	drawTodo();
}

void Interface::sortRevTitle()
{
	sortOrder[0] = 'T';
	drawTodo();
}

void Interface::sortRevDone()
{
	sortOrder[0] = 'D';
	drawTodo();
}

void Interface::sortRevDeadline()
{
	sortOrder[0] = 'L';
	drawTodo();
}

void Interface::sortRevPriority()
{
	sortOrder[0] = 'P';
	drawTodo();
}

void Interface::sortRevCategory()
{
	sortOrder[0] = 'C';
	drawTodo();
}

void Interface::sortRevUser()
{
	sortOrder[0] = 'U';
	drawTodo();
}

void Interface::save()
{
	writer.save();
	cursor.sort(sortOrder);
	screen.infoMsg("File saved");
}

#define LINES_HELP 41
void Interface::help()
{
	action_list list;
	config.getActionList(list);


	string str[LINES_HELP];
	int i = 0;
	str[i++] = "  " + list["out"] + "\tmove out\n";
	str[i++] = "  " + list["in"] + "\tmove in\n";
	str[i++] = "  " + list["down"] + "\tmove down\n";
	str[i++] = "  " + list["up"] + "\tmove up\n";
	str[i++] = "  " + list["move_down"] + "\tmove a todo down\n";
	str[i++] = "  " + list["move_up"] + "\tmove a todo up\n";
	str[i++] = "  " + list["done"] + "\tmark or unmark as done\n";
	str[i++] = "  " + list["delete"] + "\tdelete line\n";
	str[i++] = "  " + list["delDeadline"] + "\tdelete deadline\n";
	str[i++] = "  " + list["delPriority"] + "\tdelete priority\n";
	str[i++] = "  " + list["delSched"] + "\tdelete schedule\n";
	str[i++] = "  " + list["paste"] + "\tpaste the last deleted\n";
	str[i++] = "  " + list["pasteUp"] + "\tpaste the last deleted upper than the cursor\n";
	str[i++] = "  " + list["addTodo"] + "\tadd line\n";
	str[i++] = "  " + list["addTodoUp"] + "\tadd line upper than the cursor\n";
	str[i++] = "  " + list["editTitle"] + "\tmodify line\n";
	str[i++] = "  " + list["editText"] + "\tedit text\n";
	str[i++] = "  " + list["editDeadline"] + "\tedit/add deadline\n";
	str[i++] = "  " + list["editSched"] + "\tedit/add schedule\n";
	str[i++] = "  " + list["setPriority"] + "\tadd or modify the priority\n";
	str[i++] = "  " + list["setCategory"] + "\tadd or modify the category\n";
	str[i++] = "  " + list["downText"] + "\tscroll down the text\n";
	str[i++] = "  " + list["upText"] + "\tscroll up the text\n";
	str[i++] = "  " + list["collapse"] + "\tcollapse childs\n";
	str[i++] = "  " + list["hideDone"] + "\thide done tasks\n";
	str[i++] = "  " + list["sortByTitle"] + "\tsort todo by title\n";
	str[i++] = "  " + list["sortByDone"] + "\tsort todo by done\n";
	str[i++] = "  " + list["sortByDeadline"] + "\tsort todo by deadline\n";
	str[i++] = "  " + list["sortByPriority"] + "\tsort todo by priority\n";
	str[i++] = "  " + list["sortByCategory"] + "\tsort todo by category\n";
	str[i++] = "  " + list["sortByUser"] + "\tsort todo by user order\n";
	str[i++] = "  " + list["sortRevTitle"] + "\tsort todo by title in inverse order\n";
	str[i++] = "  " + list["sortRevDone"] + "\tsort todo by done in inverse order\n";
	str[i++] = "  " + list["sortRevDeadline"] + "\tsort todo by deadline in inverse order\n";
	str[i++] = "  " + list["sortRevPriority"] + "\tsort todo by priority in inverse order\n";
	str[i++] = "  " + list["sortRevCategory"] + "\tsort todo by category in inverse order\n";
	str[i++] = "  " + list["sortRevUser"] + "\tsort todo by user in inverse order\n";
	str[i++] = "  " + list["save"] + "\tsave todo\n";
	str[i++] = "  " + list["help"] + "\tshow help\n";
	str[i++] = "  " + list["quit"] + "\tquit\n";
	str[i] = "  " + list["quitNoSave"] + "\tquit without save\n";
	screen.helpPopUp(str, i);
}
