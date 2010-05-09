#include "stdafx.h"
#include "Compatibility.h"
#include "Utils.h"

int fixes=NOFIXES;
LPTOP_LEVEL_EXCEPTION_FILTER prevFilter=NULL;

//Eventually setup the compatibily fixes for older operating systems (NT4)
int SetupFixes()
{
	OSVERSIONINFO ovf;
	ovf.dwOSVersionInfoSize=sizeof(ovf);
	GetVersionEx(&ovf);
	if(
		ovf.dwPlatformId==VER_PLATFORM_WIN32_NT && ovf.dwMajorVersion==4 && ovf.dwMinorVersion==0 //Platform check
		&&
		!(fixes & NT4_HEAPFREE) //Fix check
		)
	{
		//We are on Windows NT 4.0, the fix for HeapFree hasn't been already applied
		prevFilter=SetUnhandledExceptionFilter(&CompatibilityUEF);
		fixes|=NT4_HEAPFREE;
	}
	return fixes;
}
//Disables the compatibility fixes
void DisableFixes()
{
	if(fixes | NT4_HEAPFREE)
	{
		SetUnhandledExceptionFilter(prevFilter);
		fixes &= ~NT4_HEAPFREE;
	}
}
//
//Exception filter
LONG WINAPI CompatibilityUEF(_EXCEPTION_POINTERS* ExceptionInfo)
{
	//NT4 HeapFree fix
	if((fixes&NT4_HEAPFREE) && ExceptionInfo->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION)
	{
		void * faultAddress=((BYTE *)GetProcAddress(GetModuleHandle(_T("ntdll.dll")),"RtlFreeHeap"))+0x2a;
		if(ExceptionInfo->ExceptionRecord->ExceptionAddress==faultAddress && *(BYTE *)faultAddress==0x8a)
		{
			//Skip the faulty instruction
			ExceptionInfo->ContextRecord->Eip+=3;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

	}
	if(prevFilter!=NULL)
		return prevFilter(ExceptionInfo);
	else
		return EXCEPTION_CONTINUE_SEARCH;
}

/*

LPBYTE fixAddress=(LPBYTE)GetModuleHandle(_T("JPEG2000"));
	if(fixAddress!=NULL)
	{
		DWORD oldProtect,dummy;
		fixAddress+=0x1a74;
		size_t length=0xc6;
		VirtualProtect(fixAddress,(SIZE_T)length,PAGE_EXECUTE_READWRITE,&oldProtect);
		memset(fixAddress,0x90,length);
		VirtualProtect(fixAddress,(SIZE_T)length,oldProtect,&dummy);
	}

	*/