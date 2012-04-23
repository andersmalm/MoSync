/* Copyright (C) 2009 Mobile Sorcery AB

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

#include <ma.h>
#include <conprint.h>

int MAMain(void) __attribute((noreturn));
int MAMain(void) {
	const int baseTime = maGetMilliSecondCount();
	InitConsole();
	gConsoleLogging = 1;
	printf("Hello World!\n");

	while(1) {
		MAEvent event;
		while(maGetEvent(&event)) {
			switch(event.type) {
				case EVENT_TYPE_CLOSE:
					maExit(0);
					break;
				case EVENT_TYPE_FOCUS_GAINED:
					printf("focus gained: %d", maGetMilliSecondCount() - baseTime);
					break;
				case EVENT_TYPE_FOCUS_LOST:
					printf("focus lost: %d", maGetMilliSecondCount() - baseTime);
					break;
				case EVENT_TYPE_KEY_PRESSED:
					printf("kp %i(%c)\n", event.key, event.key);
					if(event.key == MAK_0)
						maExit(0);
					break;
			}
		}
		maWait(0);
	}
}
