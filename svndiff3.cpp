//////////////////////////////////////////////////////////////////////
// svndiff3.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "svndiff3.h"

///////////////////////////////////////////////////////////////////////
// Subversion passes the diff parameters as follows:
//    Param1:      -E
//    Param2:      -m
//    Param3:      -L
//    Param4:      .mine
//    Param5:      -L
//    Param6:      .r<base rev#>
//    Param7:      -L
//    Param8:      .r<current rev#>
//    Param9:      <path to temp local   (MINE)>
//    Param10:     <path to temp base    (OLDER)>
//    Param11:     <path to temp current (YOURS)>
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Function declarations
//////////////////////////////////////////////////////////////////////
bool ParseParams(ref vector<tstring>& params, LPCTSTR lpCmdLine);
bool tcsin(TCHAR c, LPCTSTR lpSet);
void GetDiff3CmdPath(out LPTSTR pszDiff3Cmd, size_t nBufLen);
bool GetTempFileName(out LPTSTR pszFileName, size_t nBufLen);
void AddPathSlash(ref LPTSTR pszPathName, size_t nBufLen);
bool DisplayFile(LPCTSTR pszSrcFileName);


//////////////////////////////////////////////////////////////////////
// Inline templates
//////////////////////////////////////////////////////////////////////
template <size_t size>
void GetDiff3CmdPath(ref _TCHAR (&szDiff3Cmd)[size])
{
	GetDiff3CmdPath(out szDiff3Cmd, size);
}

template <size_t size>
bool GetTempFileName(ref _TCHAR (&szFileName)[size])
{
	return GetTempFileName(out szFileName, size);
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: _tWinMain
//
//  PURPOSE: Main program entry point.
//
//  COMMENTS:
//
//
int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	vector<tstring> params;
	int retval = 1;

	if(ParseParams(ref params, lpCmdLine))
	{
		if(params.size() >= 11)
		{
			_TCHAR szOutputFile[_MAX_PATH];

			if(GetTempFileName(out szOutputFile))
			{
				LPCTSTR pszBaseRev = params[5].c_str();
				LPCTSTR pszCurRev = params[7].c_str();
				LPCTSTR pszMyFile = params[8].c_str();
				LPCTSTR pszBaseFile = params[9].c_str();
				LPCTSTR pszTheirFile = params[10].c_str();
				_TCHAR szDiff3Cmd[_MAX_PATH];
				_TCHAR szDiffParam[2048];

				_stprintf_s(out szDiffParam,
						_TEXT("%s %s %s --output %s --auto --L1 \"Base - %s (%s)\" --L2 \"Theirs - %s (%s)\" --L3 \"Mine - %s\""),
						pszBaseFile, pszTheirFile, pszMyFile, szOutputFile,
						pszBaseFile, pszBaseRev, pszTheirFile, pszCurRev, pszMyFile);

				GetDiff3CmdPath(out szDiff3Cmd);

				try
				{
					intptr_t spawnret = _tspawnlp(_P_WAIT, szDiff3Cmd, szDiff3Cmd, szDiffParam, NULL);
					if(0 == spawnret)
					{
						BOOL bConnectedToConsole = AttachConsole(ATTACH_PARENT_PROCESS);

						if(!bConnectedToConsole)
						{
							// Could not attach to parent console, so create new one.
							bConnectedToConsole = AllocConsole();
						}

						if(bConnectedToConsole)
						{
							if(DisplayFile(szOutputFile))
							{
								retval = 0;
							}
						}
					}
				}
				catch(...)
				{
					retval = 1;
				}

				_tunlink(szOutputFile);
			}
		}
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: tcsin
//
//  PURPOSE: Determine if the character is in the string.
//
//  COMMENTS:
//
//
bool tcsin(TCHAR c, LPCTSTR lpSet)
{
	LPCTSTR lpScan = lpSet;

	while(NULCHAR != *lpScan)
	{
		if(c == *lpScan)
		{
			return true;
		}

		lpScan++;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: ParseParams
//
//  PURPOSE: Parse the single command-line parameter into string array.
//
//  COMMENTS:
//
//
bool ParseParams(ref vector<tstring>& params, LPCTSTR lpCmdLine)
{
	LPCTSTR lpParamSeps = TEXT(" \t");
	LPCTSTR lpEndDoubleQuote = TEXT("\"");
	LPCTSTR lpEndSingleQuote = TEXT("'");

	while(NULCHAR != *lpCmdLine)
	{
		while(tcsin(*lpCmdLine, lpParamSeps))
		{
			lpCmdLine++;
		}

		if(NULCHAR == *lpCmdLine)
		{
			break;
		}

		LPCTSTR lpEndToken = lpParamSeps;

		if(tcsin(*lpCmdLine, lpEndDoubleQuote))
		{
			lpCmdLine++;
			lpEndToken = lpEndDoubleQuote;
		}
		else if(tcsin(*lpCmdLine, lpEndSingleQuote))
		{
			lpCmdLine++;
			lpEndToken = lpEndSingleQuote;
		}

		LPCTSTR lpEndParam = lpCmdLine;

		while(NULCHAR != *lpEndParam
			&& !tcsin(*lpEndParam, lpEndToken))
		{
			lpEndParam++;
		}

		// size_t nParamLen = (size_t) ((lpEndParam - lpCmdLine) / sizeof(TCHAR));
		size_t nParamLen = (size_t) (lpEndParam - lpCmdLine);
		params.push_back(tstring(lpCmdLine, nParamLen));
		lpCmdLine = lpEndParam;
		if(tcsin(*lpCmdLine, lpEndDoubleQuote)
			|| tcsin(*lpCmdLine, lpEndSingleQuote))
		{
			lpCmdLine++;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: DisplayFile
//
//  PURPOSE: Display file contents on stdout.
//
//  COMMENTS:
//
bool DisplayFile(LPCTSTR pszSrcFileName)
{
	int hSrc = 0;
	errno_t err = 0;
	bool bSuccess = true;

	err = _tsopen_s(out &hSrc, pszSrcFileName, _O_RDONLY | _O_BINARY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
	if(0 == err)
	{
		const size_t BUFLEN = 2048;
		short byBuf[BUFLEN];
		int bytesRead = 0;
		DWORD bytesWritten = 0;
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

		while((bytesRead = _read(hSrc, ref byBuf, BUFLEN)) > 0)
		{
			if(!WriteFile(hStdOut, byBuf, bytesRead, &bytesWritten, NULL))
			{
				bSuccess = false;
				break;
			}
		}

		_close(hSrc);
	}
	else
	{
		bSuccess = false;
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: GetDiff3CmdPath
//
//  PURPOSE: Get the pathname to the diff3 command.
//
//  COMMENTS:
//
void GetDiff3CmdPath(out LPTSTR pszDiff3Cmd, size_t nBufLen)
{
	size_t nRet = 0;

	// Get DIFF3CMD environment variable.
	_tgetenv_s(&nRet, out pszDiff3Cmd, nBufLen, _TEXT("DIFF3CMD"));
	if(0 == nRet)
	{
		// Default to kdiff3.exe and let it be found in the PATH.
		_tcscpy_s(ref pszDiff3Cmd, nBufLen, _TEXT("kdiff3.exe"));
	}
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: GetTempFileName
//
//  PURPOSE: Get a temporary filename.
//
//  COMMENTS:
//
bool GetTempFileName(out LPTSTR pszFileName, size_t nBufLen)
{
	size_t nRet = 0;

	_tgetenv_s(&nRet, out pszFileName, nBufLen, _TEXT("TMP"));
	if(0 == nRet)
	{
		_tgetenv_s(&nRet, out pszFileName, nBufLen, _TEXT("TEMP"));
		if(0 == nRet)
		{
			_tgetcwd(out pszFileName, nBufLen);
		}
	}

	AddPathSlash(ref pszFileName, nBufLen);
	_tcscat_s(ref pszFileName, nBufLen, _TEXT("svndiff-XXXXXXX"));
	errno_t retval = _tmktemp_s(ref pszFileName, nBufLen);
	return (0 == retval);
}

//////////////////////////////////////////////////////////////////////
//  FUNCTION: AddPathSlash
//
//  PURPOSE: Ensure there is a trailing slash on the end of a directory path.
//
//  COMMENTS:
//
void AddPathSlash(ref LPTSTR pszPathName, size_t nBufLen)
{
	size_t nLastChar = _tcslen(pszPathName);

	if(_TEXT('\\') != pszPathName[nLastChar])
	{
		_tcscat_s(pszPathName, nBufLen, _TEXT("\\"));
	}
}

//////////////////////////////////////////////////////////////////////
// End of file
//////////////////////////////////////////////////////////////////////
