
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

#ifndef DATE_H
#define DATE_H

#include <ctime>

class Date
{
public:
	Date(int day = 1, int month = 1, int year = 1900);

	int& day(int d = 0);
	int& month(int m = 0);
	int& year(int y = 0);
	bool valid(); /* if valid date if year == 1900 is invalid */
	bool correct(); /* if this date exist */
	int daysLeft();
	Date operator-(int days);
	Date operator+(int days);
	bool operator<(Date d);
	bool operator>(Date d);
	bool operator!=(Date d);
	bool operator==(Date d);
protected:
	int _day, _month, _year;
};

#endif
