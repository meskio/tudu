
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


#include "defs.h"
extern int errno;
#include "interface.h"
#include "screen.h"
#include "data.h"
#include "sched.h"
#include "parser.h"
#include "config.h"

#ifndef SHARE_DIR
#define SHARE_DIR "/usr/local/share/tudu"
#endif

#ifndef ETC_DIR
#define ETC_DIR "/usr/local/etc"
#endif

#define VERSION "TuDu 0.6.1 (2010)"
#define WELCOME_FILE SHARE_DIR"/welcome.xml"
#define CONFIG_FILE ETC_DIR"/tudurc"

#define usage() \
	cout << "Usage: " << argv[0] << " [options]" << endl; \
	cout << "\t-f file\tload tudu file" << endl; \
	cout << "\t-c file\tload specific config file" << endl << endl; \
	cout << "\t-v\tshow version" << endl; \
	cout << "\t-h\tshow this usage message" << endl << endl; \
	cout << "The default config file is in ~/.tudurc" << endl;

#define version() \
	cout << VERSION << endl; \
	cout << "TuDu Copyright (C) 2007-2010 Ruben Pollan Bella <meskio@sindominio.net>" << endl; \
	cout << "TuDu comes with ABSOLUTELY NO WARRANTY; for details type `tudu -vv'" << endl; \
	cout << "This is free software; you are welcome to redistribute it" << endl; \
	cout << "under certain conditions. Type `tudu -vv' for details." << endl;

#define copyright() \
	cout << VERSION << endl; \
	cout << "Copyright (C) 2007-2010 Ruben Pollan Bella <meskio@sindominio.net>" << endl << endl; \
 	cout << "TuDu is free software; you can redistribute it and/or modify" << endl; \
 	cout << "it under the terms of the GNU General Public License as published by" << endl; \
 	cout << "the Free Software Foundation; version 3 of the License." << endl << endl; \
 	cout << "TuDu is distributed in the hope that it will be useful," << endl; \
 	cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl; \
 	cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl; \
 	cout << "GNU General Public License for more details." << endl << endl; \
 	cout << "You should have received a copy of the GNU General Public License" << endl; \
	cout << "along with this program.  If not, see <http://www.gnu.org/licenses/>." << endl;

/* return true for delete lock file and false for close tudu */
bool lock_ask()
{
	cout << endl << "Lock file found" << endl;
	cout << endl << "either" << endl;
	cout << "    Another program may be editing the same file." << endl;
	cout << "    If this is the case, be careful not to end up with two" << endl;
	cout << "    different instances of the same file when making changes." << endl;
	cout << "    Quit, or continue with caution." << endl;
	cout << endl << "or" << endl;
	cout << "    A tudu session for this file crashed." << endl;
	cout << "    \"Edit anyway\" will destroy the lock file." << endl;
	cout << endl << "[E]dit anyway, [Q]uit:";
	string str;
       	cin >> str;

	if ((str[0] == 'E') || (str[0] == 'e'))
		return true;
	else
		return false;
}

int main(int argc, char **argv, char *env[])
{
	int i;
	char file_rc[128], file_xml[128], file_lock[133];

	for (i = 0; strncmp(env[i],"HOME=",5); ++i);

	Config config;
	if (!config.load(CONFIG_FILE))
	{
		fprintf(stderr, "Err: Global config does not exist. The config should be %s\n", CONFIG_FILE);
		exit(1);
	}
	strncpy(file_rc,env[i]+5,119);
	strcat(file_rc,"/.tudurc");
	config.load(file_rc);

	strncpy(file_xml,env[i]+5,117);
	strcat(file_xml,"/.tudu.xml");

	/*
	 * Parse the comand line arguments
	 */
	for (i = 1; i < argc; ++i)
	{
		if (!strncmp("-f",argv[i],2))
		{
			++i;
			if (i < argc)
				strncpy(file_xml, argv[i], 127);
			else
			{
				usage();
				return 0;
			}
		}
		else if (!strncmp("-c",argv[i],2))
		{
			++i;
			if (i < argc)
			{
				if (argv[i][0] != '/')
				{
					int j;
					for (j = 0; strncmp(env[j],"PWD=",4); ++j);
					strncpy(file_rc,env[j]+4,99);
					strcat(file_rc,"/");
					strncat(file_rc,argv[i],27);
				}
				else
				{
					strncpy(file_rc, argv[i], 127);
				}
				config.load(file_rc);
			}
			else
			{
				usage();
				return 0;
			}
		}
		else if (!strncmp("-vv",argv[i],3))
		{
			copyright();
			return 0;
		}
		else if (!strncmp("-v",argv[i],2))
		{
			version();
			return 0;
		}
		else if (!strncmp("--help",argv[i],6) ||
				!strncmp("-h",argv[i],2))
		{
			usage();
			return 0;
		}
	}

	/*
	 * Check and create the lock file
	 */
	strcpy(file_lock,file_xml);
	for (i = strlen(file_lock); (file_lock[i] != '/') && (i > 0); i--);
	if (file_lock[i] == '/') i++;
	if (file_lock[i] != '.')
	{
		file_lock[i] = '.';
		file_lock[i+1] = '\0';
		strcat(file_lock, file_xml+i);
	}
	strcat(file_lock,"_lock");
	// FIXME: it wont work with NFS
	int lock;
	lock = open(file_lock, O_CREAT|O_EXCL, 00666);
	if (lock == -1)
	{
		if (errno == EEXIST)
		{
			if (!lock_ask()) exit(1);
		}
		/* no rights to write in this folder skip the problem */
		else if (errno != EACCES)
		{
			fprintf(stderr, "Err: I can not create the lock file %s\n", file_lock);
			exit(1);
		}
	}
	else
	{
		close(lock);
	}

	/*
	 * Load data
	 */
	ToDo node;
	iToDo it(node);
	Sched sched;
	Parser p(file_xml);
	if (!p.parse(node,sched))
	{
		Parser welcome(WELCOME_FILE);
		/* welcome file don't exist */
		if (!welcome.parse(node,sched))
		{
			fprintf(stderr, "Err: Welcome file does not exist. It should be %s\n", WELCOME_FILE);
			unlink(file_lock);
			exit(1);
		}
	}

	/*
	 * Load end start interface
	 */
	Writer w(file_xml,node);
	Screen screen(config);
	Cmd cmd;
	Interface in(screen,it,sched,config,w,cmd);
	in.main();

	/*
	 * Delete lock file
	 */
	unlink(file_lock);
	return 0;
}
