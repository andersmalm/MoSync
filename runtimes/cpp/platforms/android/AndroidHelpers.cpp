/* Copyright (C) 2010 Mobile Sorcery AB

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

#include "config_platform.h"

#include <stdio.h>

#include <helpers/log.h>
#include <helpers/helpers.h>

#if 1//def LOGGING_ENABLED

void InitLog(const char* filenameOverride) {
}

void LogV(const char* fmt, va_list args) {
	char buf[2048];
	vsprintf(buf, fmt, args);
	__android_log_write(ANDROID_LOG_INFO, "JNI Recompiler", buf);
}

#endif
