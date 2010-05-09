#pragma once
//Folder of the language file
#define LANGFOLDER _T("Languages")
//Prefix of the language file
#define LANGPREFIX _T("IP_")
//Extension of the language file
#define LANGEXTENSION _T(".lng")
//Default section of the language file
#define LANGDEFSECTION _T("General")
//File information section of the language file
#define LANGFILEINFOSECTION _T("FileInfo")
//Size of the buffer used with GetPrivateProfileString and LoadString
#define LANGBUFSIZE 16384
//If _MT is defined we are in a multithreaded application, thus using a global buffer can be dangerous (e.g. if two GetString are executing in different threads).
//On the other side defining every time a big buffer in the function is a waste of resource if the application is single-threaded.
#if(defined(_MT) || defined(FORCE_LANGFILE_LOCAL_BUFFER))
	#define DEF_LOCAL_READ_BUFFER			TCHAR readBuffer[LANGBUFSIZE]={0} //Define the buffer
#else
	#define DEF_LOCAL_READ_BUFFER			(*readBuffer=0)	//Just clean it
#endif
class LanguageFile
{
	//The map type
	typedef std::map<std::pair<UINT,UINT>,std::_tcstring> LangMapType;
	//Should the class use the language file?
	bool useLanguageFile;
	//Language file path
	TCHAR langFile[MAX_PATH];
	//Language file map
	LangMapType langMap;
	//Language name
	std::_tcstring languageName;
	//Translator name
	std::_tcstring translatorName;
	//Target version
	std::_tcstring targetVersion;
#if(!defined(_MT) && !defined(FORCE_LANGFILE_LOCAL_BUFFER))
	//Class read buffer, defined only if we are not in a multithreaded application
	TCHAR readBuffer[LANGBUFSIZE];
#endif
	//Unescapes a string
	void UnescapeString(TCHAR * str);
public:
	//Construct a LanguageFile object
	LanguageFile(TCHAR * IVIniFile);
	//Returns true if there's a language file
	bool LanguageFilePresent(){return *langFile!=0;};
	//Get a generic string by ID
	const std::_tcstring & GetString(UINT StringID);
	//Get a dialog string by string and dialog ID
	const std::_tcstring & GetDialogString(UINT DialogID, UINT StringID);
	//Initialize the captions of a dialog
	void InitializeDialogCaptions(HWND hDialog, UINT DialogID);
	//Enables/disables external language file; returns the previous state
	bool ActivateLanguageFile(bool activate)
	{
		bool ret=useLanguageFile;
		if(activate && ! *langFile)
			throw std::invalid_argument(ERROR_STD_PROLOG "Cannot activate the language file if no language file is present.");
		useLanguageFile=activate;
		return ret;
	};
	//Returns true if the language file is activated
	bool LanguageFileActivated(){return useLanguageFile;};
	//Wrappers
	//Language name
	const std::_tcstring & GetLanguageName() {return languageName;};
	//Translator name
	const std::_tcstring & GetTranslatorName() {return translatorName;};
	//Target version
	const std::_tcstring & GetTargetVersion() {return targetVersion;};
	//Language file path
	const TCHAR * GetLanguageFilePath() {return LanguageFilePresent()?langFile:NULL;};
};
//Structure used by InitializeDialogCaptions and IDC_EnumChildProc
struct IDC_ECP_Params
{
	LanguageFile * langFile;
	UINT dialogID;
};
//Procedure called by EnumChildWindows (called by LanguageFile::InitializeDialogCaptions)
BOOL CALLBACK IDC_EnumChildProc(HWND hwnd, LPARAM lParam);