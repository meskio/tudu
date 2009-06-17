
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

#include "interface.h"

#define cursor_line()  (cursor->line-tree_begin)
#define isCollapse() ((cursor->getCollapse()) && (!cursor->actCollapse()))

Interface::Interface(Screen &s, iToDo &t, Sched& sch, Config &c, Writer &w, Cmd &com) 
		: screen(s), cursor(t), sched(sch), config(c), writer(w), copied(NULL), cmd(com)
{
	cmd.get_interface(this);

	search_pattern = L"";
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
			if ("pasteChild" == action) pasteChild();
			if ("done" == action) done();
			if ("addTodo" == action) addLine();
			if ("addTodoUp" == action) addLineUp();
			if ("editTitle" == action) modifyLine();
			if ("editDeadline" == action) editDeadline();
			if ("setPriority" == action) setPriority();
			if ("setCategory" == action) setCategory();
			if ("editText" == action) editText();
			if ("editSched" == action) editSched();
			if ("schedUp" == action) schedUp();
			if ("schedDown" == action) schedDown();
			if ("downText" == action) downText();
			if ("upText" == action) upText();
			if ("collapse" == action) collapse();
			if ("hideDone" == action) hide_done();
			if ("search" == action) search();
			if ("searchNext" == action) search_next();
			if ("searchPrev" == action) search_prev();
			if ("cmd" == action) command_line();
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
	if (screen.treeLines() < 8)
	{
		if (cursor->line >= tree_end)
		{
			tree_end = cursor->line;
			tree_begin = tree_end - screen.treeLines();
			drawTodo();
		}
		else if (cursor->line < tree_begin)
		{
			tree_begin = cursor->line;
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
	else if (cursor->line > tree_end - 4)
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
	bool hide;

	/* if is done */
	hide = (config.getHideDone() && todo->done());
	/* if is in hidden category */
	hide = hide || (hidden_categories.count(todo->getCategory()));

	return hide;
}

void Interface::inherit()
{
	/* inherit category */
	if (cursor.depth())
	{
		iToDo father = cursor;
		father.out();
		cursor->setCategory(father->getCategory());
	}
	else if (hidden_categories.count(L""))
	{
		set<wstring>::iterator cat;
		for (cat = categories.begin(); (cat != categories.end()) && hidden_categories.count(*cat); cat++);
		if (cat != categories.end()) cursor->setCategory(*cat);
		else cursor->setCategory(NONE_CATEGORY);
	}
}

void Interface::left()
{
	eraseCursor();
	if (cursor.out() && isHide(cursor))
	{
		up();
	}
	else if (isHide(cursor))
	{
		cursor.addChild(new ToDo());
		inherit();
	}
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
		inherit();
		drawTodo();
		wstring title;
		if ((editLine(title)) && (title != L""))
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
	screen.drawSched(sched, &(*cursor));
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

void Interface::pasteChild()
{
	if (copied)
	{
		cursor->actCollapse() = true;
		cursor.in();
		cursor.addChildUp(copied);
		sched.add_recursive(copied);
		copied = NULL;
		drawTodo();
	}
}

#define startTitle (cursor.depth() * 4 + 7)

bool Interface::editLine(wstring& str)
{
	bool save;

	str = cursor->getTitle();
	screen.infoMsg("Editing todo. Press ENTER to save or ESC to abort edit");
	save = screen.editTitle(cursor_line(), cursor.depth(), 
			cursor->haveChild(), str);
	screen.infoClear();
	return save;
}

void Interface::editDeadline()
{
	screen.infoMsg("Editing deadline. Press ENTER to save or ESC to abort edit");
	screen.editDeadline(cursor_line(), cursor->deadline(), cursor->done());
	screen.infoClear();
	drawTodo();
}

void Interface::setPriority()
{
	screen.infoMsg("Editing priority. Press ENTER to save or ESC to abort edit");
	screen.setPriority(cursor_line(), cursor->priority());
	screen.infoClear();
	drawTodo();
}

void Interface::setCategory()
{
	screen.infoMsg("Editing category. Press ENTER to save or ESC to abort edit");
	screen.setCategory(cursor_line(), *cursor);
	screen.infoClear();
	drawTodo();
}

void Interface::addLine()
{
	cursor.addChild(new ToDo());
	inherit();
	drawTodo();
	wstring title;
	if ((editLine(title)) && (title != L""))
		cursor->getTitle() = title;
	else
		del();
}

void Interface::addLineUp()
{
	cursor.addChildUp(new ToDo());
	inherit();
	drawTodo();
	wstring title;
	if ((editLine(title)) && (title != L""))
		cursor->getTitle() = title;
	else
		del();
}

void Interface::modifyLine()
{
	wstring title;
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

		argc=0;
		argv[argc] = strtok(s, " \t");
		while ((res = strtok(NULL, " \t")) != NULL)
		{
			if (++argc >= 32) break;
			argv[argc] = res;
		}
		argv[argc+1] = NULL;
		
		endwin();
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
		doupdate();
		
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
	}
	else
	{
		screen.infoMsg("Editing text. Press ESC to save");
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

void Interface::schedUp()
{
	sched.up(&(*cursor));
	screen.drawSched(sched, &(*cursor));
}

void Interface::schedDown()
{
	sched.down(&(*cursor));
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
		del();
	drawTodo();
}

bool Interface::_search()
{
	bool res = true;
	iToDo hit = cursor;

	if (hit.search(search_pattern))
	{
		iToDo first_hit = hit;
		/* if is hide search the next one */
		while (isHide(hit))
		{
			hit.search(search_pattern);
			/* if there is only hidden tasks exit */
			if (first_hit == hit)
			{
				res = false;
				break;
			}
		}
	}
	else
		res = false;

	if (res)
	{
		while (cursor.out()) cursor->actCollapse() = false;
		cursor = hit;
		while (hit.out()) hit->actCollapse() = true;
	}
	return res;
}

void Interface::command_line()
{
	wstring command(L"");

	if (screen.cmd(command))
	{
		if (cmd.cmd(command))
		{
			if (isHide(cursor)) up();
			/* destroy empty with NONE category */
			if ((cursor->getCategory() == NONE_CATEGORY) && cursor->getTitle().empty())
					del();
			drawTodo();
		}
	}
}

void Interface::search()
{
	wstring pattern(L"");

	if (screen.searchText(pattern))
	{
		search_pattern = pattern;
		if (_search())
			drawTodo();
		else
			screen.infoMsg("Not found");
	}
}

void Interface::search_next()
{
	if (search_pattern != L"")
		if (_search())
			drawTodo();
		else
			screen.infoMsg("Not found");
	else
		screen.infoMsg("No search pattern");
}

void Interface::search_prev()
{
	if (search_pattern != L"")
		if (_search())
			drawTodo();
		else
			screen.infoMsg("Not found");
	else
		screen.infoMsg("No search pattern");
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
	if (writer.save())
	{
		cursor.sort(sortOrder);
		screen.infoMsg("File saved");
	}
	else
	{
		screen.infoMsg("File can not be saved");
	}
}

#define LINES_HELP 34
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
	str[i++] = "  " + list["paste"] + "\tpaste the last deletion\n";
	str[i++] = "  " + list["pasteUp"] + "\tpaste the last deletion above the cursor\n";
	str[i++] = "  " + list["pasteChild"] + "\tpaste the last deletion as child of the task\n";
	str[i++] = "  " + list["addTodo"] + "\tadd line\n";
	str[i++] = "  " + list["addTodoUp"] + "\tadd line upper than the cursor\n";
	str[i++] = "  " + list["editTitle"] + "\tmodify line\n";
	str[i++] = "  " + list["editText"] + "\tedit text\n";
	str[i++] = "  " + list["editDeadline"] + "\tedit/add deadline\n";
	str[i++] = "  " + list["editSched"] + "\tedit/add schedule\n";
	str[i++] = "  " + list["setPriority"] + "\tadd or modify priority\n";
	str[i++] = "  " + list["setCategory"] + "\tadd or modify category\n";
	str[i++] = "  " + list["downText"] + "\tscroll text down\n";
	str[i++] = "  " + list["upText"] + "\tscroll text up\n";
	str[i++] = "  " + list["schedUp"] + "\tmove up the task on the scheduler\n";
	str[i++] = "  " + list["schedDown"] +  "\tmove down the task on the scheduler\n";
	str[i++] = "  " + list["collapse"] + "\tcollapse children\n";
	str[i++] = "  " + list["hideDone"] + "\thide tasks that are done\n";
	str[i++] = "  " + list["search"] + "\tsearch on titles\n";
	str[i++] = "  " + list["searchNext"] + "\tgo to next search result\n";
	str[i++] = "  " + list["cmd"] + "\topen command input\n";
	str[i++] = "  " + list["save"] + "\tsave todo\n";
	str[i++] = "  " + list["quit"] + "\tquit\n";
	str[i] = "  " + list["quitNoSave"] + "\tquit without save\n";
	screen.helpPopUp(str, i);
}

