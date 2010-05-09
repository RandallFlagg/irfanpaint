#include "StdAfx.h"
#include ".\languagefile.h"
#include "Globals.h"
#include "Utils.h"

LanguageFile::LanguageFile(TCHAR * IVIniFile)
{
	//Get the language file name
	TCHAR langFileName[MAX_PATH]=_T("");
	useLanguageFile=true;
	GetPrivateProfileString(INISECTION,_T("langFileOverride"),_T(""),langFileName,ARRSIZE(langFileName),IVIniFile);
	if(!*langFileName)
	{
		_tcscpy(langFileName,LANGPREFIX);
		useLanguageFile = 
			//Append the language dll name to the prefix
			GetPrivateProfileString(_T("Language"),_T("DLL"),_T("ENGLISH"),langFileName+_tcslen(langFileName),ARRSIZE(langFileName)-_tcslen(langFileName),IVIniFile)
			&&
			//Check if the language file has an extension (the "ENGLISH" default falue has no extension and indicates to use the included language).
			*PathFindExtension(langFileName)
			&&
			//Remove the extension
			(PathRemoveExtension(langFileName),true) //this operation cannot fail
			&&
			//Append the new extension
			PathAddExtension(langFileName,LANGEXTENSION);
	}
	useLanguageFile=
		//If it's already false do not proceed
		useLanguageFile
		&&
		//Get the path of i_view32.exe
		GetModuleFileName(NULL,langFile,ARRSIZE(langFile))
		&&
		//Remove i_view32.exe from the string
		PathRemoveFileSpec(langFile)
		&&
		//Append the language folder name
		PathAppend(langFile,LANGFOLDER)
		&&
		//Append the language file name
		PathAppend(langFile,langFileName)
		&&
		//Check if the file exists
		PathFileExists(langFile);
	if(!useLanguageFile)
		*langFile=0;
	//Eventually defines readBuffer
	DEF_LOCAL_READ_BUFFER;
	//Load some additional infos
	//Language name
	//3rd resource: file name without prefix and extension
	PathRemoveExtension(langFileName);
	_tcscpy(readBuffer,langFileName+_tcslen(LANGPREFIX));
	//2nd resource: Language\Lang entry in i_view32.ini
	GetPrivateProfileString(_T("Language"),_T("Lang"),readBuffer,readBuffer,ARRSIZE(readBuffer),IVIniFile);
	//1st resource: FileInfo\LanguageName
	GetPrivateProfileString(LANGFILEINFOSECTION,_T("LanguageName"),readBuffer,readBuffer,ARRSIZE(readBuffer),langFile);
	languageName=readBuffer;
	//Translator name
	GetPrivateProfileString(LANGFILEINFOSECTION,_T("TranslatorName"),_T("Unknown"),readBuffer,ARRSIZE(readBuffer),langFile);
	translatorName=readBuffer;
	//Target version
	GetPrivateProfileString(LANGFILEINFOSECTION,_T("TargetVersion"),_T("Unknown"),readBuffer,ARRSIZE(readBuffer),langFile);
	targetVersion=readBuffer;
}
//Get a generic string by ID
const std::_tcstring & LanguageFile::GetString(UINT StringID)
{
	//Check if we have cached the value
	LangMapType::iterator it;
	if((it=langMap.find(std::pair<UINT,UINT>(0,StringID)))!=langMap.end())
        return it->second;
	//Eventually defines readBuffer
	DEF_LOCAL_READ_BUFFER;
	if(useLanguageFile)
	{
		//If there's a language file read from it
		TCHAR stringIDBuf[11]=_T("");
		_sntprintf(stringIDBuf,ARRSIZE(stringIDBuf),_T("%u"),StringID);
		if(GetPrivateProfileString(LANGDEFSECTION,stringIDBuf,_T(""),readBuffer,ARRSIZE(readBuffer),langFile)!=0)
			UnescapeString(readBuffer);
	}
	if(!useLanguageFile || *readBuffer==0)
	{
		_tcsncpy(readBuffer,CANNOTLOADSTRING,ARRSIZE(readBuffer));
		//If there isn't a language file or there was not the requested string in it read from the resources
		LoadString(hCurInstance,StringID,readBuffer,ARRSIZE(readBuffer));
	}
	return langMap[std::pair<UINT,UINT>(0,StringID)]=readBuffer;
}
//Get a dialog string by string and dialog ID
const std::_tcstring & LanguageFile::GetDialogString(UINT DialogID, UINT StringID)
{
	DEF_LOCAL_READ_BUFFER;
	if(!useLanguageFile)
		throw std::logic_error(ERROR_STD_PROLOG "GetDialogString cannot be called if there's no language file or it isn't activated.");
	//Check if we have cached the value
	LangMapType::iterator it;
	if((it=langMap.find(std::pair<UINT,UINT>(DialogID,StringID)))!=langMap.end())
        return it->second;
	TCHAR stringIDBuf[11]=_T(""),dialogIDBuf[11]=_T("");
	_sntprintf(stringIDBuf,ARRSIZE(stringIDBuf),_T("%u"),StringID);
	_sntprintf(dialogIDBuf,ARRSIZE(dialogIDBuf),_T("%u"),DialogID);
	GetPrivateProfileString(dialogIDBuf,stringIDBuf,_T(""),readBuffer,ARRSIZE(readBuffer),langFile);
	UnescapeString(readBuffer);
	return langMap[std::pair<UINT,UINT>(DialogID,StringID)]=readBuffer;
}
//Unescapes a string
void LanguageFile::UnescapeString(TCHAR * str)
{
	TCHAR *readPtr, *writePtr;
	TCHAR escapeKeys[]=_T("rn\\");		//Caratteri validi dopo lo slash
	TCHAR escapeValues[]=_T("\r\n\\");	//Caratteri con cui vengono sostituiti
	for(readPtr=writePtr=str;*readPtr;readPtr++,writePtr++)
	{
		*writePtr=*readPtr;
		if(*readPtr==_T('\\') && readPtr[1]) //Got a slash; if this isn't the last character unescape the sequence
		{
			//Check if it is a valid escape sequence
			TCHAR * escapeChar=_tcschr(escapeKeys,readPtr[1]);
			if(escapeChar!=NULL)
			{
				readPtr++;
				*writePtr=escapeValues[escapeChar-escapeKeys];
			}
		}
	}
	*writePtr=0;
}
//Procedure called by EnumChildWindows (called by LanguageFile::InitializeDialogCaptions)
BOOL CALLBACK IDC_EnumChildProc(HWND hwnd, LPARAM lParam)
{
	IDC_ECP_Params *iep=(IDC_ECP_Params *)lParam;
	const TCHAR * caption=iep->langFile->GetDialogString(iep->dialogID,(UINT)GetWindowLong(hwnd,GWL_ID)).c_str();
	if(*caption)
		SetWindowText(hwnd,caption);
	return TRUE;
}

//Initialize the captions of a dialog
void LanguageFile::InitializeDialogCaptions(HWND hDialog, UINT DialogID)
{
	if(!useLanguageFile)
		return; //there's no need to initialize if we haven't a language file
	//Set the caption of the dialog
	const TCHAR * caption=GetDialogString(DialogID,0).c_str();
	if(*caption)
		SetWindowText(hDialog,caption);
	//Init also the children windows
	IDC_ECP_Params iep={0};
	iep.dialogID=DialogID;
	iep.langFile=this;
	EnumChildWindows(hDialog,IDC_EnumChildProc,(LPARAM)&iep);
}