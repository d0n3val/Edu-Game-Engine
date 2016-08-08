#include "Globals.h"

#define WIN32_MEAN_AND_LEAN
#include <windows.h>   // we only really need this for OutDebugString :(
#include <stdio.h>
#include "Application.h"

void _log(const char file[], int line, const char* format, ...)
{
	static char tmp_string[4096];
	static char tmp_string2[4096];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, 4096, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, 4096, "\n%s(%d) : %s", file, line, tmp_string);
	OutputDebugString(tmp_string2);
	if (App) {
		sprintf_s(tmp_string2, 4096, "\n%s", tmp_string);
		App->Log(tmp_string2);
	}
}