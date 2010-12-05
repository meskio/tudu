
/**************************************************************************
 * Copyright (C) 2007-2010 Ruben Pollan Bella <meskio@sindominio.net>     *
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

#include "interface.h"

#define isCollapse() ((cursor->getCollapse()) && (!cursor->actCollapse()))

Interface::Interface(Screen &s, iToDo &t, Sched& sch, Config &c, Writer &w, Cmd &com) 
		: screen(s), cursor(t), sched(sch), config(c), writer(w), copied(NULL), cmd(com)
{
	cmd.get_interface(this);
	search_pattern = L"";

	sortOrder = config.getSortOrder();
	cursor.sort(sortOrder);
	while (prev());
	cursor_line = 0;
}

Interface::~Interface()
{
	delete copied;
}

void Interface::main()
{
	wint_t key;
	string action;

	while (cursor.out());
	while (--cursor);
	drawTodo();

	while (true) 
	{
		if (get_wch(&key) == KEY_CODE_YES)
		{
			screen.infoClear();
			if (KEY_RESIZE == key) resizeTerm();
			if (KEY_LEFT == key) left();
			if (KEY_RIGHT == key) right();
			if (KEY_DOWN  == key) down();
			if (KEY_UP == key) up();
			if (KEY_PPAGE == key) prevPage();
			if (KEY_NPAGE == key) nextPage();
			if (KEY_HOME == key) home();
			if (KEY_END == key) end();
		}
		else if (config.getAction(key, action))
		{
			screen.infoClear();
			if ("quit" == action)
			{
				if (writer.save())
					break;
				else
					screen.infoMsg("File can not be saved");
			}
			if ("quitNoSave" == action)
				if (screen.confirmQuit()) break;
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
	drawTodo();
}

void Interface::drawTodo()
{
	screen.treeClear();
	cursor.sort(sortOrder);
	if (isHide(cursor) && !prev() && !next())
	{
		while (cursor.out());
		cursor.addChildUp(new ToDo());
		inherit();
		cursor_line = 0;
	}
	iToDo aux = cursor;

	while (cursor.out());
	while (--cursor);
	screen.infoPercent(cursor.percentUp());

	/* redraw screen */
	cursor = aux;
	fitCursor();
	int line = cursor_line;
	while ((cursor_line > 0) && prev());
	if (cursor_line < 0)
	{
		screen.drawTitle(cursor_line, cursor.depth(), cursor->getTitle(), -cursor_line);
		next();
	}
	while (cursor_line < screen.treeLines())
	{
		bool isCursor = (cursor == aux);
		screen.drawTask(cursor_line, cursor.depth(), *cursor, isCursor);
		if (isCursor) line = cursor_line;

		if (!next()) break;
	}

	cursor = aux;
	cursor_line = line;
	screen.drawText(cursor->getText());
	screen.drawSched(sched, &(*cursor));
}

bool Interface::next()
{
	iToDo oldCursor = cursor;
	cursor_line += screen.taskLines(cursor.depth(), *cursor);
	do {
		if (!cursor.end() && !isCollapse())
			cursor.in();
		else if (!++cursor)
		{
			if (!cursor.out())
			{
				cursor = oldCursor;
				cursor_line -= screen.taskLines(cursor.depth(), *cursor);
				return false;
			}
			++cursor;
		}
	} while (isHide(cursor));

	return true;
}

bool Interface::prev()
{
	iToDo oldCursor = cursor;
	do {
		if (!--cursor)
		{
			if (!cursor.out())
			{
				cursor = oldCursor;
				return false;
			}
		}
		else
		{
			while (cursor->haveChild() && !isCollapse())
			{
				cursor.in();
				while (++cursor);
				--cursor;
			}
		}
	} while (isHide(cursor));
	cursor_line -= screen.taskLines(cursor.depth(), *cursor);

	return true;
}

void Interface::eraseCursor()
{
	screen.drawTask(cursor_line, cursor.depth(), *cursor, false);
}

bool Interface::fitCursor()
{
	int treeLines = screen.treeLines();
	int taskLines = screen.taskLines(cursor.depth(), *cursor);

	if (taskLines > treeLines-8)
	{
		if (cursor_line + taskLines >= treeLines)
		{
			cursor_line = treeLines - taskLines;
			return true;
		}
		else if (cursor_line < 0)
		{
			cursor_line = 0;
			return true;
		}
	}
	else if (cursor_line + taskLines >= treeLines - 4)
	{
		int line = cursor_line;
		cursor_line = treeLines - taskLines - 4;
		if (cursor_line < 0) cursor_line = 0;
		if (cursor_line != line) return true;
	}
	else if (cursor_line < 4)
	{
		iToDo aux = cursor;
		int line = cursor_line;
		while (cursor_line > line - 4)
			if (!prev()) break;
		cursor_line = 0;
		while (aux != cursor) next();
		if (cursor_line != line) return true;
	}
	else
	{
		iToDo aux = cursor;
		int line = cursor_line;
		while (cursor_line != 0)
			if (!prev()) break;
		if (cursor_line == 0)
		{
			cursor = aux;
			cursor_line = line;
		}
		else
		{
			cursor_line = 0;
			while (aux != cursor) next();
			return true;
		}
	}

	return false;
}

void Interface::drawCursor()
{
	if (fitCursor())
	{
		drawTodo();
	}
	else
	{
		screen.drawTask(cursor_line, cursor.depth(), *cursor, 
				true);
		screen.drawText(cursor->getText());
		screen.drawSched(sched, &(*cursor));
	}
}

bool Interface::isHide(iToDo& todo)
{
	if (todo.end()) return true;

	/* if is done */
	bool hideDone = (config.getHideDone() && todo->done());

	/* if is in hidden category */
	bool hideCat = false;
	set<wstring>& categories = todo->getCategories();
	for (set<wstring>::iterator it = categories.begin();
	     it != categories.end(); it++)
	{
		if (!hidden_categories.count(*it))
		{
			hideCat = false;
			break;
		}
		hideCat = true;
	}
		
	return hideDone || hideCat;
}

void Interface::inherit()
{
	/* inherit category */
	if (cursor.depth())
	{
		iToDo father = cursor;
		father.out();
		cursor->setCategories(father->getCategories());
	}
	else if (hidden_categories.count(L""))
	{
		set<wstring>::iterator cat;
		for (cat = categories.begin(); 
			(cat != categories.end()) && hidden_categories.count(*cat);
			cat++);
		if (cat != categories.end()) cursor->setCategory(*cat);
		else cursor->setCategory(NONE_CATEGORY);
	}
}

void Interface::left()
{
	iToDo aux = cursor;

	while (aux.out() && isHide(aux));
	if (!isHide(aux))
	{
		eraseCursor();
		while (aux != cursor) prev();
		cursor->actCollapse() = false;
		if (cursor->getCollapse())
			drawTodo();
		else
			drawCursor();
	}
	else if (isHide(cursor))
	{
		prev();
		drawTodo();
	}
}

void Interface::right()
{
	bool need_drawTodo = cursor->getCollapse();

	eraseCursor();
	cursor->actCollapse() = true;
	cursor_line += screen.taskLines(cursor.depth(), *cursor);
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

			/* Use the config collapse */
			cursor.out();
			cursor->getCollapse() = config.getCollapse();
			cursor.in();
		}
		else
		{
			del();
		}
		need_drawTodo = false;
	}

	if (need_drawTodo)
		drawTodo();
	else
		drawCursor();
}

void Interface::up()
{
	iToDo aux = cursor;

	if (aux.begin() && config.getLoopMove())
		while (++aux);

	while (--aux && isHide(aux));
	if (!isHide(aux))
	{
		eraseCursor();
		while (aux != cursor)
			if (!prev()) break;
		while (aux != cursor) next();
		drawCursor();
	}
	else if (isHide(cursor))
	{
		prev();
		drawTodo();
	}
}

void Interface::down()
{
	iToDo aux = cursor;

	while (++aux)
	{
		if (aux.end() && config.getLoopMove())
			while (--aux);
		if (!isHide(aux) || (aux == cursor)) break;
	}

	if (!isHide(aux))
	{
		eraseCursor();
		while (aux != cursor)
			if (!next()) break;
		while (aux != cursor) prev();
		drawCursor();
	}
	else if (isHide(cursor))
	{
		prev();
		drawTodo();
	}
}

void Interface::prevPage()
{
	int treeLines = screen.treeLines();
	while (cursor_line >= 0)
		if (!prev()) break;
	iToDo aux = cursor;
	cursor_line = treeLines - 1;
	while (cursor_line >= 0)
		if (!prev()) break;
	cursor = aux;
	cursor_line = treeLines - cursor_line;
	drawTodo();
}

void Interface::nextPage()
{
	int treeLines = screen.treeLines();
	eraseCursor();
	while (cursor_line < treeLines)
		if (!next()) {
			drawCursor();
			return;
		}

	iToDo aux = cursor;
	cursor_line = 0;
	while (cursor_line < treeLines)
		if (!next()) break;
	cursor = aux;
	cursor_line = treeLines - cursor_line;
	drawTodo();
}

void Interface::home()
{
	eraseCursor();
	while (prev());
	drawCursor();
}

void Interface::end()
{
	eraseCursor();
	while (next());
	drawCursor();
}

void Interface::move_up()
{
	pToDo t = &(*cursor);
	cursor.del();
	eraseCursor();
	--cursor;
	cursor_line -= screen.taskLines(cursor.depth(), *cursor);
	cursor.addChildUp(t);
	drawTodo();
}

void Interface::move_down()
{
	pToDo t = &(*cursor);
	cursor.del();
	eraseCursor();
	iToDo aux = cursor;
	aux.addChild(t);
	while (aux != cursor) next();
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
	up();
	drawTodo();
}

void Interface::delDeadline()
{
	cursor->deadline().year() = 1900;
	if ((string::npos != sortOrder.find('l')) ||
	    (string::npos != sortOrder.find('L')))
	{
		drawTodo();
	}
	else
	{
		screen.deadlineClear(cursor_line);
	}
}

void Interface::delPriority()
{
	cursor->priority() = 0;
	if ((string::npos != sortOrder.find('p')) ||
	    (string::npos != sortOrder.find('P')))
	{
		drawTodo();
	}
	else
	{
		screen.priorityClear(cursor_line);
	}
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
		iToDo aux = cursor;
		aux.addChild(copied);
		while (aux != cursor) next();
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
		cursor_line += screen.taskLines(cursor.depth(), *cursor);
		cursor.in();
		cursor.addChildUp(copied);
		sched.add_recursive(copied);
		copied = NULL;
		drawTodo();
	}
}

bool Interface::editLine(wstring& str)
{
	Editor::return_t save;
	wstring oldTitle = cursor->getTitle();

	str = oldTitle;
	screen.infoMsg("Editing todo. Press ENTER to save or ESC to abort edit");
	save = screen.editTitle(cursor_line, cursor.depth(), 
			cursor->haveChild(), str, str.length());
	while ((save == Editor::REDRAW) || (save == Editor::RESIZE))
	{
		cursor->getTitle() = str;
		if (save == Editor::RESIZE)
			resizeTerm();
		else
			drawTodo();
		save = screen.editTitle(cursor_line, cursor.depth(), 
				cursor->haveChild(), str);
	}
	cursor->getTitle() = oldTitle;

	screen.infoClear();
	if (save == Editor::NOT_SAVED) drawTodo();
	return (save == Editor::SAVED);
}

void Interface::editDeadline()
{
	screen.infoMsg("Editing deadline. Press ENTER to save or ESC to abort edit");

	Editor::return_t save;
	Date date = cursor->deadline();
	save = screen.editDeadline(cursor_line, date, cursor->done(), 0);
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.editDeadline(cursor_line, date, cursor->done());
	}

	if ((save == Editor::SAVED) && date.correct())
		cursor->deadline() = date;

	screen.infoClear();
	drawTodo();
}

void Interface::setPriority()
{
	screen.infoMsg("Editing priority. Press ENTER to save or ESC to abort edit");

	Editor::return_t save;
	int priority = cursor->priority();
	save = screen.setPriority(cursor_line, priority);
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.setPriority(cursor_line, priority);
	}

	if (save == Editor::SAVED)
		cursor->priority() = priority;

	screen.infoClear();
	drawTodo();
}

void Interface::setCategory()
{
	screen.infoMsg("Editing category. Press ENTER to save or ESC to abort edit");

	Editor::return_t save;
	wstring category = cursor->getCategory();
	save = screen.setCategory(cursor_line, category, category.length());
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.setCategory(cursor_line, category);
	}

	if (save == Editor::SAVED)
		cursor->setCategory(category);

	screen.infoClear();
	drawTodo();
}

void Interface::addLine()
{
	iToDo aux = cursor;
	aux.addChild(new ToDo());
	while (aux != cursor) next();
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
		Text& text = cursor->getText();

		/* create a temporal file */
		tmpnam(path);
		int fout;
		while ((fout = open(path, O_CREAT|O_WRONLY|O_EXCL, S_IRUSR|S_IWUSR)) == -1)
				tmpnam(path);
		close(fout);

		wofstream ofs(path);
		ofs.imbue(locale(""));
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
		
		wifstream ifs(path);
		ifs.imbue(locale(""));
		ifs >> text;
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

	Editor::return_t save;
	Date date = cursor->sched();
	save = screen.editSched(date, 0);
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.editSched(date);
	}

	if ((save == Editor::SAVED) && date.correct())
		cursor->sched() = date;

	if (cursor->sched().valid())
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
		iToDo aux = cursor;
		while (aux.out()) aux->actCollapse() = false;
		aux = hit;
		while (hit.out()) hit->actCollapse() = true;

		while (next())
			if (cursor == aux) return true;
		while (cursor != aux) prev();
	}
	return res;
}

void Interface::command_line()
{
	Editor::return_t save;
	wstring command(L"");
	save = screen.cmd(command, 0);
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.cmd(command);
	}

	if (save == Editor::SAVED)
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
	Editor::return_t save;
	wstring pattern(L"");
	save = screen.searchText(pattern, 0);
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.searchText(pattern);
	}

	if (save == Editor::SAVED)
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
	sortOrder = 't' + sortOrder;
	drawTodo();
}

void Interface::sortByDone()
{
	sortOrder = 'd' + sortOrder;
	drawTodo();
}

void Interface::sortByDeadline()
{
	sortOrder = 'l' + sortOrder;
	drawTodo();
}

void Interface::sortByPriority()
{
	sortOrder = 'p' + sortOrder;
	drawTodo();
}

void Interface::sortByCategory()
{
	sortOrder = 'c' + sortOrder;
	drawTodo();
}

void Interface::sortByUser()
{
	sortOrder = 'u' + sortOrder;
	drawTodo();
}

void Interface::sortRevTitle()
{
	sortOrder = 'T' + sortOrder;
	drawTodo();
}

void Interface::sortRevDone()
{
	sortOrder = 'D' + sortOrder;
	drawTodo();
}

void Interface::sortRevDeadline()
{
	sortOrder = 'L' + sortOrder;
	drawTodo();
}

void Interface::sortRevPriority()
{
	sortOrder = 'P' + sortOrder;
	drawTodo();
}

void Interface::sortRevCategory()
{
	sortOrder = 'C' + sortOrder;
	drawTodo();
}

void Interface::sortRevUser()
{
	sortOrder = 'U' + sortOrder;
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

#define LINES_HELP 44
void Interface::help()
{
	action_list list;
	config.getActionList(list);


	string str[LINES_HELP];
	int i = 0;
	str[i++] = "  TASK CREATION\n";
	str[i++] = "  " + list["addTodo"] + "\tadd a new task below the cursor\n";
	str[i++] = "  " + list["addTodoUp"] + "\tadd a new task above the cursor\n";
	str[i++] = "  " + list["editTitle"] + "\tedit task's title\n";
	str[i++] = "  " + list["editText"] + "\tadd/edit task's long description text\n";
	str[i++] = "  " + list["setPriority"] + "\tadd/edit task's priority\n";
	str[i++] = "  " + list["setCategory"] + "\tadd/edit task's category\n";
	str[i++] = "  " + list["editDeadline"] + "\tadd/edit task's deadline\n";
	str[i++] = "  " + list["editSched"] + "\tadd/edit taks's scheduled date\n";
	str[i++] = "\n";
	str[i++] = "  NAVIGATION\n";
	str[i++] = "  " + list["in"] + "\tgo one level deeper\n";
	str[i++] = "  " + list["out"] + "\tgo to the outer lever\n";
	str[i++] = "  " + list["down"] + "\tgo down\n";
	str[i++] = "  " + list["up"] + "\tgo up\n";
	str[i++] = "  " + list["collapse"] + "\tcollapse/expand a task\n";
	str[i++] = "  " + list["hideDone"] + "\thide/unhide tasks marked as done\n";
	str[i++] = "  " + list["move_down"] + "\tmove a task downwards\n";
	str[i++] = "  " + list["move_up"] + "\tmove a task upwards\n";
	str[i++] = "  " + list["schedUp"] + "\tmove up the task on the scheduler\n";
	str[i++] = "  " + list["schedDown"] +  "\tmove down the task on the scheduler\n";
	str[i++] = "  " + list["downText"] + "\tscroll text down\n";
	str[i++] = "  " + list["upText"] + "\tscroll text up\n";
	str[i++] = "  " + list["search"] + "\tsearch on titles\n";
	str[i++] = "  " + list["searchNext"] + "\tgo to next search result\n";
	str[i++] = "  " + list["save"] + "\tsave changes\n";
	str[i++] = "  " + list["quit"] + "\tquit\n";
	str[i++] = "  " + list["quitNoSave"] + "\tquit without saving\n";
	str[i++] = "\n";
	str[i++] = "  EDITING\n";
	str[i++] = "  " + list["done"] + "\tmark/unmark a task as done\n";
	str[i++] = "  " + list["delete"] + "\tdelete a task\n";
	str[i++] = "  " + list["delPriority"] + "\tdelete task's priority\n";
	str[i++] = "  " + list["delDeadline"] + "\tdelete task's deadline\n";
	str[i++] = "  " + list["delSched"] + "\tdelete task's scheduled date\n";
	str[i++] = "  " + list["paste"] + "\tpaste last deletion below the cursor\n";
	str[i++] = "  " + list["pasteUp"] + "\tpaste last deletion above the cursor\n";
	str[i++] = "  " + list["pasteChild"] + "\tpaste last deletion as a subtask\n";
	str[i++] = "\n";
	str[i++] = "  COMMANDS\n";
	str[i++] = "  :help\tshow manual page\n";
	str[i++] = "\n";
	str[i++] = "  Please refer to the manual page for more commands, sorting\n";
	str[i]   = "  by different criteria and category management.\n";
	screen.helpPopUp(str, i);
}

