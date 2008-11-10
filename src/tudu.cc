
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
#include "screen.h"
#include "data.h"
#include "sched.h"
#include "parser.h"
#include "config.h"

#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

#ifndef SHARE_DIR
#define SHARE_DIR "/usr/local/share/tudu"
#endif

#ifndef ETC_DIR
#define ETC_DIR "/usr/local/etc"
#endif

#define VERSION "TuDu 0.4.1 (2008)"
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
	cout << "TuDu Copyright (C) 2007-2008 Ruben Pollan Bella <meskio@amedias.org>" << endl; \
	cout << "TuDu comes with ABSOLUTELY NO WARRANTY; for details type `tudu -vv'" << endl; \
	cout << "This is free software, and you are welcome to redistribute it" << endl; \
	cout << "under certain conditions; type `tudu -vv' for details." << endl;

#define copyright() \
	cout << VERSION << endl; \
	cout << "Copyright (C) 2007-2008 Ruben Pollan Bella <meskio@amedias.org>" << endl << endl; \
 	cout << "TuDu is free software; you can redistribute it and/or modify" << endl; \
 	cout << "it under the terms of the GNU General Public License as published by" << endl; \
 	cout << "the Free Software Foundation; either version 3 of the License." << endl << endl; \
 	cout << "TuDu is distributed in the hope that it will be useful," << endl; \
 	cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl; \
 	cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl; \
 	cout << "GNU General Public License for more details." << endl << endl; \
 	cout << "You should have received a copy of the GNU General Public License" << endl; \
	cout << "along with this program.  If not, see <http://www.gnu.org/licenses/>." << endl;

int main(int argc, char **argv, char *env[])
{
	int i;
	char file_rc[128], file_xml[128];

	for (i = 0; strncmp(env[i],"HOME=",5); ++i);

	Config config;
	if (!config.load(CONFIG_FILE))
	{
		fprintf(stderr, "Err: Global config don't exist. The config should be %s\n", CONFIG_FILE);
		exit(1);
	}
	strncpy(file_rc,env[i]+5,119);
	strcat(file_rc,"/.tudurc");
	config.load(file_rc);

	strncpy(file_xml,env[i]+5,117);
	strcat(file_xml,"/.tudu.xml");

	for (i = 1; i < argc; ++i)
	{
		if (!strncmp("-f",argv[i],2))
		{
			++i;
			strncpy(file_xml, argv[i], 127);
		}
		else if (!strncmp("-c",argv[i],2))
		{
			++i;
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

	ToDo node("");
	iToDo it(node);
	Sched sched;
	Parser p(file_xml);
	if (!p.parse(node,sched))
	{
		Parser welcome(WELCOME_FILE);
		/* welcome file don't exist */
		if (!welcome.parse(node,sched))
		{
			fprintf(stderr, "Err: Welcome file don't exist. It should be %s\n", WELCOME_FILE);
			exit(1);
		}
	}

	Writer w(file_xml,node);
	Screen screen(config);
	Interface in(screen,it,sched,config,w);
	in.main();
	return 0;
}
