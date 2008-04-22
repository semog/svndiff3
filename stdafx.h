// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include <vector>
#include <string>

using namespace std;

#ifdef UNICODE
#define tstring	wstring
#else
#define tstring string
#endif


// Define some pseudo-keywords to be more explicit
#define ref
#define out

#define NULCHAR	TEXT('\0')
