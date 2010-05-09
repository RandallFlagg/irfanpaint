#pragma once
//Fixes constants
#define NOFIXES			0		//No fixes
#define NT4_HEAPFREE	1		//NT4.0 HeapFree access violation fix

//Eventually setup the compatibily fixes for older operating systems (NT4)
int SetupFixes();
//Disables the compatibility fixes
void DisableFixes();
//Exception filter
LONG WINAPI CompatibilityUEF(_EXCEPTION_POINTERS* ExceptionInfo);