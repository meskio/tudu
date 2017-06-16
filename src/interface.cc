
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
	wstring action;

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
			if (KEY_SF == key) move_down(); // shift+down
			if (KEY_SR == key) move_up(); // shift+up
			config.clearKeys();
		}
		else if (0xc == key)  //  Ctrl-L
		{
			resizeTerm();
		}
		else if (config.getAction(key, action))
		{
			screen.infoClear();
			if (L"quit" == action)
			{
				if (writer.save())
					break;
				else
					screen.infoMsg("File can not be saved");
			}
			if (L"quitNoSave" == action)
				if (screen.confirmQuit()) break;
			if (L"out" == action) left();
			if (L"in" == action) right();
			if (L"down" == action) down();
			if (L"up" == action) up();
			if (L"move_down" == action) move_down();
			if (L"move_up" == action) move_up();
			if (L"delete" == action) del();
			if (L"delDeadline" == action) delDeadline();
			if (L"delPriority" == action) delPriority();
			if (L"delSched" == action) delSched();
			if (L"paste" == action) paste();
			if (L"pasteUp" == action) pasteUp();
			if (L"pasteChild" == action) pasteChild();
			if (L"done" == action) done();
			if (L"addTodo" == action) addLine();
			if (L"addTodoUp" == action) addLineUp();
			if (L"editTitle" == action) modifyLine();
			if (L"editDeadline" == action) editDeadline();
			if (L"setPriority" == action) setPriority();
			if (L"setCategory" == action) setCategory();
			if (L"editText" == action) editText();
			if (L"editSched" == action) editSched();
			if (L"schedUp" == action) schedUp();
			if (L"schedDown" == action) schedDown();
			if (L"downText" == action) downText();
			if (L"upText" == action) upText();
			if (L"collapse" == action) collapse();
			if (L"hideDone" == action) hide_done();
			if (L"search" == action) search();
			if (L"searchNext" == action) search_next();
			if (L"searchPrev" == action) search_prev();
			if (L"cmd" == action) command_line();
			if (L"sortByTitle" == action) sortByTitle();
			if (L"sortByDone" == action) sortByDone();
			if (L"sortByDeadline" == action) sortByDeadline();
			if (L"sortByPriority" == action) sortByPriority();
			if (L"sortByCategory" == action) sortByCategory();
			if (L"sortByUser" == action) sortByUser();
			if (L"sortRevTitle" == action) sortRevTitle();
			if (L"sortRevDone" == action) sortRevDone();
			if (L"sortRevDeadline" == action) sortRevDeadline();
			if (L"sortRevPriority" == action) sortRevPriority();
			if (L"sortRevCategory" == action) sortRevCategory();
			if (L"sortRevUser" == action) sortRevUser();
			if (L"save" == action) save();
			if (L"help" == action) help();
		}
		else if (key==' ') done();
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

	/* if there is no task to display create a dummy one */
	if (isHide(cursor) && !prev() && !next())
	{
		while (cursor.out());
		cursor.addChildUp(new ToDo());
		inherit();
		cursor_line = 0;
	}

	/* get the percent done of the whole tree */
	iToDo aux = cursor;
	while (cursor.out());
	while (--cursor);
	screen.infoPercent(cursor.percentUp());
	cursor = aux;

	/* redraw screen */
	fitCursor();
	int line = cursor_line;
	while ((cursor_line > 0) && prev());
	if (cursor_line < 0)
	{
		screen.drawTitle(cursor_line, cursor.depth(), 
		                 cursor->getTitle(), -cursor_line);
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
	bool needRedraw = false;
	int treeLines = screen.treeLines();
	int taskLines = screen.taskLines(cursor.depth(), *cursor);
	iToDo task = cursor;

	/* keep the tree on the screen if it can fit there */
	int line = cursor_line;
	while (prev());
	int firstLine = cursor_line;
	iToDo firstTask = cursor;
	while (cursor_line < treeLines-5)
		if (!next()) break;
	if ((cursor_line < treeLines-5) || (firstLine > 0))
	{
		cursor_line = 0;
		cursor = firstTask;
		needRedraw = true;
		while (cursor != task) next();
	}
	else
	{
		cursor_line = line;
		cursor = task;
	}

	/* check if the task is out of the screen scroll */
	if (taskLines > treeLines-8)
	{
		if (cursor_line + taskLines >= treeLines)
		{
			cursor_line = treeLines - taskLines;
			needRedraw = true;
		}
		else if (cursor_line < 0)
		{
			cursor_line = 0;
			needRedraw = true;
		}
	}
	else if (cursor_line + taskLines >= treeLines - 4)
	{
		int line = cursor_line;
		cursor_line = treeLines - taskLines - 4;
		if (cursor_line < 0) cursor_line = 0;
		if (cursor_line != line) needRedraw = true;
	}
	else if (cursor_line < 4)
	{
		int line = cursor_line;
		while (cursor_line > line - 4)
			if (!prev()) break;
		cursor_line = 0;
		while (task != cursor) next();
		if (cursor_line != line) needRedraw = true;
	}

	return needRedraw;
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
	if (categories.empty() && hidden_categories.count(L""))
		hideCat = true;
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
		if (cat != categories.end()) cursor->addCategory(*cat);
		else cursor->addCategory(NONE_CATEGORY);
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
		if (editLine(title) && (title != L""))
		{
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
	iToDo aux = cursor;
	++aux;
	if (aux.end())
		return;

	pToDo t = &(*cursor);
	cursor.del();
	eraseCursor();
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

	screen.infoClear();
	if (save == Editor::NOT_SAVED)
	{
		cursor->getTitle() = oldTitle;
		drawTodo();
	}
	else
	{
		cursor->getTitle() = str;
		if (cursor->sched().valid())
			screen.drawSched(sched, &(*cursor));
	}

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
	screen.infoMsg("Editing categories. Separate multiple categories with ','. Press ENTER to save or ESC to abort edit");

	Editor::return_t save;
	wstring category = cursor->getCategoriesStr();
	save = screen.setCategory(cursor_line, category, category.length());
	while (save == Editor::RESIZE)
	{
		resizeTerm();
		save = screen.setCategory(cursor_line, category);
	}

	if (save == Editor::SAVED)
		cursor->setCategoriesStr(category);

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
	if (!editLine(title) || (title == L""))
		del();
}

void Interface::addLineUp()
{
	cursor.addChildUp(new ToDo());
	inherit();
	drawTodo();
	wstring title;
	if (!editLine(title) || (title == L""))
		del();
}

void Interface::modifyLine()
{
	wstring title;
	editLine(title);
}

void Interface::editText()
{
	char* editor = config.getEditor();
	if (strlen(editor) != 0)
	{
		char s[86];
		char* argv[32];
		int argc;
		char* res;
		Text& text = cursor->getText();

		/* create a temporal file */
		char template_name[]="/tmp/cmguiTuduXXXXXX";
		int fout=mkstemp(template_name);
		close(fout);

		wofstream ofs(template_name);
		ofs.imbue(locale(""));
		ofs << text;
		ofs.close();
		sprintf(s, editor, template_name);

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
		
		wifstream ifs(template_name);
		ifs.imbue(locale(""));
		ifs >> text;
		ifs.close();
		unlink(template_name);

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
			if ((cursor->getCategories().count(NONE_CATEGORY)) && cursor->getTitle().empty())
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
	sortOrder = L't' + sortOrder;
	drawTodo();
}

void Interface::sortByDone()
{
	sortOrder = L'd' + sortOrder;
	drawTodo();
}

void Interface::sortByDeadline()
{
	sortOrder = L'l' + sortOrder;
	drawTodo();
}

void Interface::sortByPriority()
{
	sortOrder = L'p' + sortOrder;
	drawTodo();
}

void Interface::sortByCategory()
{
	sortOrder = L'c' + sortOrder;
	drawTodo();
}

void Interface::sortByUser()
{
	sortOrder = L'u' + sortOrder;
	drawTodo();
}

void Interface::sortRevTitle()
{
	sortOrder = L'T' + sortOrder;
	drawTodo();
}

void Interface::sortRevDone()
{
	sortOrder = L'D' + sortOrder;
	drawTodo();
}

void Interface::sortRevDeadline()
{
	sortOrder = L'L' + sortOrder;
	drawTodo();
}

void Interface::sortRevPriority()
{
	sortOrder = L'P' + sortOrder;
	drawTodo();
}

void Interface::sortRevCategory()
{
	sortOrder = L'C' + sortOrder;
	drawTodo();
}

void Interface::sortRevUser()
{
	sortOrder = L'U' + sortOrder;
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


	wstring str[LINES_HELP];
	int i = 0;
	str[i++] = L"  TASK CREATION\n";
	str[i++] = L"  " + list[L"addTodo"] + L"\tadd a new task below the cursor\n";
	str[i++] = L"  " + list[L"addTodoUp"] + L"\tadd a new task above the cursor\n";
	str[i++] = L"  " + list[L"editTitle"] + L"\tedit task's title\n";
	str[i++] = L"  " + list[L"editText"] + L"\tadd/edit task's long description text\n";
	str[i++] = L"  " + list[L"setPriority"] + L"\tadd/edit task's priority\n";
	str[i++] = L"  " + list[L"setCategory"] + L"\tadd/edit task's category\n";
	str[i++] = L"  " + list[L"editDeadline"] + L"\tadd/edit task's deadline\n";
	str[i++] = L"  " + list[L"editSched"] + L"\tadd/edit taks's scheduled date\n";
	str[i++] = L"\n";
	str[i++] = L"  NAVIGATION\n";
	str[i++] = L"  " + list[L"in"] +L"\tgo one level deeper\n";
	str[i++] = L"  " + list[L"out"] +L"\tgo to the outer lever\n";
	str[i++] = L"  " + list[L"down"] +L"\tgo down\n";
	str[i++] = L"  " + list[L"up"] +L"\tgo up\n";
	str[i++] = L"  " + list[L"collapse"] +L"\tcollapse/expand a task\n";
	str[i++] = L"  " + list[L"hideDone"] +L"\thide/unhide tasks marked as done\n";
	str[i++] = L"  " + list[L"move_down"] +L"\tmove a task downwards\n";
	str[i++] = L"  " + list[L"move_up"] +L"\tmove a task upwards\n";
	str[i++] = L"  " + list[L"schedUp"] +L"\tmove up the task on the scheduler\n";
	str[i++] = L"  " + list[L"schedDown"] + L"\tmove down the task on the scheduler\n";
	str[i++] = L"  " + list[L"downText"] +L"\tscroll text down\n";
	str[i++] = L"  " + list[L"upText"] +L"\tscroll text up\n";
	str[i++] = L"  " + list[L"search"] +L"\tsearch on titles\n";
	str[i++] = L"  " + list[L"searchNext"] +L"\tgo to next search result\n";
	str[i++] = L"  " + list[L"save"] +L"\tsave changes\n";
	str[i++] = L"  " + list[L"quit"] +L"\tquit\n";
	str[i++] = L"  " + list[L"quitNoSave"] +L"\tquit without saving\n";
	str[i++] = L"\n";
	str[i++] = L"  EDITING\n";
	str[i++] = L"  " + list[L"done"] +L"\tmark/unmark a task as done\n";
	str[i++] = L"  " + list[L"delete"] +L"\tdelete a task\n";
	str[i++] = L"  " + list[L"delPriority"] +L"\tdelete task's priority\n";
	str[i++] = L"  " + list[L"delDeadline"] +L"\tdelete task's deadline\n";
	str[i++] = L"  " + list[L"delSched"] +L"\tdelete task's scheduled date\n";
	str[i++] = L"  " + list[L"paste"] +L"\tpaste last deletion below the cursor\n";
	str[i++] = L"  " + list[L"pasteUp"] +L"\tpaste last deletion above the cursor\n";
	str[i++] = L"  " + list[L"pasteChild"] +L"\tpaste last deletion as a subtask\n";
	str[i++] = L"\n";
	str[i++] = L"  COMMANDS\n";
	str[i++] = L"  :help\tshow manual page\n";
	str[i++] = L"\n";
	str[i++] = L"  Please refer to the manual page for more commands, sorting\n";
	str[i]   = L"  by different criteria and category management.\n";
	screen.helpPopUp(str, i);
}
