// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>

#ifdef UNICODE
#define tstring	wstring
typedef wchar_t* LPTSTR;
typedef const wchar_t* LPCTSTR;
#else
#define tstring string
typedef char* LPTSTR;
typedef const char* LPCTSTR;
#endif


// Define some pseudo-keywords to be more explicit
#define ref
#define out

#define NULCHAR	TEXT('\0')
