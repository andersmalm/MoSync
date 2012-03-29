/* Copyright (C) 2010 MoSync AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#include <mawvsprintf.h>
#include <maapi.h>
#include <maassert.h>

#ifdef USE_NEWLIB
#include <wchar.h>
#else
#include <conprint.h>
#endif

#define STRLOG(str) maWriteLog(str, sizeof(str)-1)

int MAMain(void) GCCATTRIB(noreturn);
int MAMain(void) {
	const wchar* str = L"蝮 㦩햭Ｑӭܨপ૩ిฐະშኡᎣᑷᾨⅦ⿏タ World!";
	wchar buf[256];
#ifdef USE_NEWLIB
	int res = swprintf(buf, 256, L"%S\n", str);
#else
	int res = wsprintf(buf, L"%S\n", str);
#endif
	wprintf(L"%i\n", res);
#ifdef USE_NEWLIB
	fputws(buf, stdout);
#endif
	wlprintfln(L"%S", buf);
	STRLOG("__test\n");
	wprintf(L"%S", buf);	//this one fails.
	printf("notha line...\n");
	maSetColor(0xFFFFFF);
	maDrawTextW(0, 10, buf);
	maUpdateScreen();
	FREEZE;
}
