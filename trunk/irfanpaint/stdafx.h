#pragma once
//Enable two useful warnings disabled by default
#pragma warning(default : 4265) //The class has virtual functions but the destructor isn't virtual
#pragma warning(default : 4640) //Construction of static local object is not thread-safe
//Disable two annoying warnings
#pragma warning(disable : 4512) //Cannot generate an assignment operator
#pragma warning(disable : 4511) //Cannot generate a copy constructor

//Exclude rarely used elements from the Windows header
#define VC_EXTRALEAN
//Strict type checking in the Windows headers
#define STRICT
//Minimum target: Windows NT4
#define _WIN32_WINNT	0x400
//Standard headers
//Windows standard header
#include <windows.h>
//RichEdit
#include <richedit.h>
//Message cracker macros
#include <windowsx.h>
//Common controls
#include <commctrl.h>
//Extensions to windowsx.h and commctrl.h
#include "WinExtensions.h"
//Shell lightweight APIs
#include <shlwapi.h>
//File di intestazione C/C++ standard
#include <cstdio>
//New
#include <new>
//Strings
#include <string>
//String streams
#include <sstream>
//IO manipulators
#include <iomanip>
//scoped_ptr
#include <boost/scoped_ptr.hpp>
//scoped_array
#include <boost/scoped_array.hpp>
//Generic text mappings
//tchar.h default mappings
#include <tchar.h>
//Mappings for the STL string and string-related classes
namespace std
{
	//String
	typedef basic_string<TCHAR>				_tcstring;
	//String streams
	typedef basic_stringstream<TCHAR>		_tcstringstream;
	typedef basic_ostringstream<TCHAR>		_tcostringstream;
	typedef basic_istringstream<TCHAR>		_tcistringstream;
	//String buffer
	typedef basic_stringbuf<TCHAR>			_tcstringbuf;
	//File streams
	typedef basic_fstream<TCHAR>			_tcfstream;
	typedef basic_ifstream<TCHAR>			_tcifstream;
	typedef basic_ofstream<TCHAR>			_tcofstream;
};
//Mappings for conversions from and to TCHAR *
#ifdef _UNICODE
	//TCHAR *	->	char *		=>		wchar_t *	->	char *
	#define _tcstombs	wcstombs
	//TCHAR *	->	wchar_t *	=>		wchar_t *	->	wchar_t *
	#define _tcstowcs	wcsncpy
	//char *	->	TCHAR *		=>		char *		->	wchar_t *
	#define _mbstotcs	mbstowcs
	//wchar_t *	->	TCHAR *		=>		wchar_t *	->	wchar_t *
	#define _wcstotcs	wcsncpy
#else
	//TCHAR *	->	char *		=>		char *		->	char *
	#define _tcstombs	strncpy
	//TCHAR *	->	wchar_t *	=>		char *		->	wchar_t *
	#define _tcstowcs	mbstowcs
	//char *	->	TCHAR *		=>		char *		->	char *
	#define _mbstotcs	strncpy
	//wchar_t *	->	TCHAR *		=>		wchar_t *	->	char *
	#define _wcstotcs	wcstombs
#endif
//Support macro to determine the size of a static array
#define ARRSIZE(arr)	(sizeof(arr)/sizeof(*arr))
//cmath defines also some mathematical constants
#define _USE_MATH_DEFINES
//Math functions
#include <cmath>
//Limits of the data types
#include <climits>
//Floating point support routines
#include <cfloat>
//Disable the C4702 warning for the following two headers
#pragma warning(push)
#pragma warning(disable:4702)
//map STL container
#include <map>
//list STL container
#include <list>
//vector STL container
#include <vector>
#pragma warning(pop)
//stack STL container
#include <stack>
//STL algorithms
#include <algorithm>
//RTTI support
#include <typeinfo>
//Other headers
//Exception support macros
#include "Exceptions.h"
