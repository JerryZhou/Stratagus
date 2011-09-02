/*
    attachconsole.cpp - WINAPI AttachConsole
    Copyright (C) 2009-2011  Pali Rohár <pali.rohar@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#if ( defined(WIN32) || defined(_MSC_VER) ) && ( defined(NO_STDIO_REDIRECT) || ! defined(REDIRECT_OUTPUT) )

#define WINVER 0x0501
#include <windows.h>
#include <winternl.h>
#include <wincon.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
#include <iostream>
#endif

static int fixmode = 0;

/// Check if HANDLE is attached to console
static int WINAPI_CheckIfConsoleHandle(HANDLE handle) {

	wchar_t filename[MAX_PATH];
	unsigned long int length;

	// Try to get filename of HANDLE
	NtQueryObject(handle, ObjectNameInformation, filename, MAX_PATH, &length);

	// Filename start at position 8
	if ( length > 8 )
		return 0;
	else
		return 1;

}

/// Try to reopen FILE* from WINAPI HANDLE
static void WINAPI_ReopenFileFromHandle(HANDLE handle, FILE * file, const char * mode) {

	int fd;
	FILE * newfile;

	if ( ! handle || handle == INVALID_HANDLE_VALUE )
		return;

	// Get file descriptor from HANDLE
	fd = _open_osfhandle((intptr_t)handle, O_TEXT);

	if ( fd < 0 )
		return;

	// Get C structure FILE* from file descriptior
	newfile = _fdopen(fd, mode);

	if ( ! newfile )
		return;

	// Close current file
	fclose(file);

	// Set new file from HANDLE
	*file = *newfile;

	setvbuf(file, NULL, _IONBF, 0);

	// If stdout/stderr write 2 empty lines to cmd console
	if ( ! fixmode && strcmp(mode, "w") == 0 ) {

		printf("\n\n");
		fixmode = 1;

	}

}

/// Try to set std HANDLE from FILE*
static void WINAPI_SetStdHandleFromFile(int type, FILE * file) {

	int fd;
	HANDLE handle;

	fd = fileno(file);

	if ( fd < 0 )
		return;

	handle = (HANDLE)_get_osfhandle(fd);

	if ( ! handle || handle == INVALID_HANDLE_VALUE )
		return;

	SetStdHandle(type, handle);

}

/// Try attach console of parent process for std input/output in Windows NT, 2000, XP or new
static void WINAPI_AttachConsole(void) {

	OSVERSIONINFO osvi;
	int hasVersion;
	int version;
	int attached;
	int reopen_stdin;
	int reopen_stdout;
	int reopen_stderr;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	hasVersion = GetVersionEx(&osvi);

	if ( ! hasVersion )
		return;

	version = 0;
	version |= osvi.dwMinorVersion;
	version |= osvi.dwMajorVersion << 8;

	if ( version < 0x0500 )
		return;

	// Ignore if HANDLE is not attached console
	reopen_stdin = WINAPI_CheckIfConsoleHandle(GetStdHandle(STD_INPUT_HANDLE));
	reopen_stdout = WINAPI_CheckIfConsoleHandle(GetStdHandle(STD_OUTPUT_HANDLE));
	reopen_stderr = WINAPI_CheckIfConsoleHandle(GetStdHandle(STD_ERROR_HANDLE));

	attached = AttachConsole(ATTACH_PARENT_PROCESS);

	if ( ! attached )
		return;

	if ( reopen_stdin )
		WINAPI_ReopenFileFromHandle(GetStdHandle(STD_INPUT_HANDLE), stdin, "r");
	else
		WINAPI_SetStdHandleFromFile(STD_INPUT_HANDLE, stdin);

	if ( reopen_stdout )
		WINAPI_ReopenFileFromHandle(GetStdHandle(STD_OUTPUT_HANDLE), stdout, "w");
	else
		WINAPI_SetStdHandleFromFile(STD_OUTPUT_HANDLE, stdout);

	if ( reopen_stderr )
		WINAPI_ReopenFileFromHandle(GetStdHandle(STD_ERROR_HANDLE), stderr, "w");
	else
		WINAPI_SetStdHandleFromFile(STD_ERROR_HANDLE, stderr);

#ifdef __cplusplus
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();
	std::ios::sync_with_stdio();
#endif

}

/// This section set that WINAPI_AttachConsole() will be called at application startup before main()
#ifdef _MSC_VER
#pragma section(".CRT$XCU", long, read)
__declspec(allocate(".CRT$XCU")) void (*initialize)(void) = WINAPI_AttachConsole;
#else
__attribute__((constructor)) static void initialize(void) { WINAPI_AttachConsole(); }
#endif

#endif
