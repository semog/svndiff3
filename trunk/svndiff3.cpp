//////////////////////////////////////////////////////////////////////
// svndiff3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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
//  FUNCTION: _tmain
//
//  PURPOSE: Main program entry point.
//
//  COMMENTS:
//       After performing the merge, this applet needs to print the contents
//       of the merged file to stdout.
//       Return an errorcode of 0 on successful merge, 1 if unresolved conflicts
//       remain in the result.  Any other errorcode will be treated as fatal.
//
int _tmain(int argc, _TCHAR* argv[])
{
	int retval = 1;

	if(argc >= 12)
	{
		_TCHAR szOutputFile[_MAX_PATH];

		if(GetTempFileName(out szOutputFile))
		{
			LPCTSTR pszBaseRev = argv[6];
			LPCTSTR pszCurRev = argv[8];
			LPCTSTR pszMyFile = argv[9];
			LPCTSTR pszBaseFile = argv[10];
			LPCTSTR pszTheirFile = argv[11];
			_TCHAR szDiff3Cmd[_MAX_PATH];
			_TCHAR szDiffParam[2048];

			_stprintf_s(out szDiffParam,
					_TEXT("%s %s %s --output %s --auto --L1 \"Base (%s)\" --L2 \"Theirs (%s)\" --L3 \"Mine\""),
					pszBaseFile, pszTheirFile, pszMyFile, szOutputFile,
					pszBaseRev, pszCurRev);

			GetDiff3CmdPath(out szDiff3Cmd);

			try
			{
				intptr_t spawnret = _tspawnlp(_P_WAIT, szDiff3Cmd, szDiff3Cmd, szDiffParam, NULL);
				if(0 == spawnret)
				{
					if(DisplayFile(szOutputFile))
					{
						retval = 0;
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

	return retval;
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
		int bytesWritten = 0;

		while((bytesRead = _read(hSrc, ref byBuf, BUFLEN)) > 0)
		{
			bytesWritten = fwrite(byBuf, 1, bytesRead, stdout);
			if(bytesWritten != bytesRead)
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
