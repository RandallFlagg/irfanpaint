// IrfanPaint.cpp
//

#include "stdafx.h"								//Standard inclusions
#include "resource.h"							//Resources #define
#include "ArrowSettings.h"						//ArrowSettings dialog
#include "IrfanPaint.h"							//IrfanPaint.cpp header
#include "ColorUtils.h"							//ColorUtils inline class
#include "DibSection.h"							//DibSection class
#include "CloneTool.h"							//CloneTool class
#include "PrecisionCursor.h"					//PrecisionCursor class
#include "Arrow.h"								//Arrow APIs
#include "Utils.h"								//Support functions & macros
#include "Globals.h"							//Global declarations
#include "ToolbarButtons.h"						//Toolbar buttons #define
#include "Tools.h"								//Tools routines
#include "PenBrushSettings.h"					//Pen and brush settings dialog
#include "WindowMagnetizer.h"					//Window magnetizer class
#include "InsertText.h"							//Insert text dialog
#include "Compatibility.h"						//Compatibility fixes
#include "LanguageFile.h"						//Language file
#include "INISection.h"							//INI section
#include "DibSectionUpdater.h"					//DibSection Updater
#include "Objects.h"							//Objects
//Globals
HINSTANCE hCurInstance=NULL;					//Current dll instance
ColorUtils fgColor(0x000000);					//Current FG color
ColorUtils bgColor(0xFFFFFF);					//Current BG color
HWND hIVWindow=NULL, hToolBoxWindow=NULL;		//Windows handles to the IVW and to the ToolBox
HWND hMainWindow=NULL;							//Windows handle to the IV main Window
WNDPROC realIVWndProc=NULL;						//Real IVW wndproc
WNDPROC realMainIVWndProc=NULL;					//Real Main IVW wndproc
HDC IVWDC=NULL;									//DC of hIVWindow
//DibSection * DibSect=NULL;						//DibSection object
DibSectionUpdater * dsUpdater;					//DibSection Updater
PrecisionCursor normPC;							//Normal precision cursor
PrecisionCursor clsrPC;							//Clone tool source cursor
WindowMagnetizer windowMagnetizer;				//Window magnetizer
LanguageFile * langFile;						//Language file
UIToolsContainer * UITC=NULL;					//UI Tools container
#ifdef USE_OBJECTS
Objects::ObjectsContainer objects;				//Objects
#endif
HIMAGELIST tbImgList=NULL;						//Imagelist of the toolbar
INISection * iniSect=NULL;						//INI section
bool fillFlag;									//Fill flag
BYTE threshold;									//Tolerance threshold
#pragma warning (push)
#pragma warning (disable:4100)
//DLL entrypoint
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hCurInstance = (HINSTANCE) hModule; //Set the current instance
		SetupFixes(); //Setup the fixes
		//TODO: make this work fine!
		//std::locale::global(std::locale("")); //Setup the locale
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if(IsWindow(hToolBoxWindow)) //If we haven't already done it...
			CleanUp(); //... let's clean up.
		DisableFixes();
		break;
	}
    return TRUE;
}
#pragma warning (pop)
//Performs the cleanup
void CleanUp()
{
	static bool IAmRunning; //Is cleanup already running?
	if(IAmRunning)
		return; //If so, exit!
	IAmRunning=true; //Yes, I am cleaning!
	try
	{
		if(IsWindow(hToolBoxWindow))
			DestroyWindow(hToolBoxWindow); //Destroy the toolbox (if it is still alive)
		if(realIVWndProc!=NULL) //If it is subclassed...
		{
			SetWindowLongPtr(hIVWindow,GWLP_WNDPROC,(__int3264)(LONG_PTR)realIVWndProc); //...remove the subclassing from the IVW
			realIVWndProc=NULL; //Memorandum
		}
		if(realMainIVWndProc!=NULL) //If it is subclassed...
		{
			//MessageBox(0,"De-subclass realMainIVWndProc","CleanUp",MB_ICONINFORMATION);
			SetWindowLongPtr(hMainWindow,GWLP_WNDPROC,(__int3264)(LONG_PTR)realMainIVWndProc); //...remove the subclassing from the mainIVW
			realMainIVWndProc=NULL; //Memorandum
		}
		//Delete the various global objects used throughout IrfanPaint
		delete dsUpdater;
		dsUpdater=NULL;
		delete UITC;
		UITC = NULL;
		delete langFile;
		langFile=NULL;
		delete iniSect;
		iniSect=NULL;
		//Reset the refresh state of the precision cursors
		normPC.ResetRefresh();
		clsrPC.ResetRefresh();
	}
	catch(exception &ex)
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__));
		return;
	}
	if(IVWDC!=NULL)
	{
		ReleaseDC(hIVWindow,IVWDC); //Release the DC to IVW
		IVWDC=NULL;
	}
	//Unload the RichEdit module
	FreeRichEdit();
	//Tell to IV that the plugin has closed
	SendMessage(hMainWindow,WM_PAINT_PLUGIN_CLOSED,0,0);
	//Update the IV Window
	InvalidateRect(hMainWindow,NULL,TRUE);
	//Enable the IV Window (it may have been disabled by a dialog)
	EnableWindow(hMainWindow,TRUE);
	IAmRunning=false; //I have finished!
	return;
}
#pragma warning (push)
#pragma warning (disable:4100)
//Closes the plugin
int __cdecl ClosePlugin(int iDummy,char *szDummy)
{
	CleanUp(); //actually call CleanUp();
	return 0;
}
//Returns "About..." information to IV
int __cdecl GetPlugInInfo(char *versionString, char *fileFormats)
{
	//Size of the provided buffers
	//fileFormats - versionString = 260, so I assume it's the size of both buffers
	//which seem to be allocated on the stack and reused for every plugin
	const size_t buffersLength=260;
	if(fileFormats!=NULL)
	{
		std::_tcstring s_fileFormats;
		try
		{
			//Get a customized about message
			s_fileFormats=GetAboutMessage(true,true,true,false,true,true);
			//Copy it in the buffer converting it to ANSI
			_tcstombs(fileFormats,s_fileFormats.c_str(),buffersLength);
		}
		catch(...)
		{
			//If something fails copy a minimal version string
			strncpy(fileFormats,"IrfanPaint by Matteo Italia",buffersLength);
		}
		//Some more safety
		fileFormats[buffersLength-1]='\0';
	}
	if(versionString!=NULL)
	{
		std::_tcstring s_versionString;
		try
		{
			//Get the version information
			s_versionString=GetAboutMessage(false,false,false,true,true,false);
			//Copy it in the buffer converting it to ANSI
			_tcstombs(versionString,s_versionString.c_str(),buffersLength);
		}
		catch(...)
		{
			//If something fails we don't have anything to show
			strncpy(versionString,"n.a.",buffersLength);
		}
		//Some more safety
		versionString[buffersLength-1]='\0';
	}
	return 0;
}
#pragma warning (pop)
//Function invoked by IrfanView; performs the init
BOOL __cdecl ShowIrfanPaintTB(HANDLE * f_pImgDIB, HWND f_hMainWindow, HWND f_hIVWindow, char *f_iniFile, char *errMSG)
{
	try
	{
		//Check the parameters
		if(*f_pImgDIB==NULL)
		{
			strcpy(errMSG,"f_pImgDIB points to NULL.");
			return FALSE;
		}
		//Close if we are already running 
		if(IsWindow(hToolBoxWindow))
		{
			CleanUp();
			return TRUE;
		}
		//Without this all the dialogs may have a random parent
		hMainWindow=f_hMainWindow;
		TCHAR iniFile[MAX_PATH];
//#define NO_OS_CHECK
#ifndef NO_OS_CHECK
		//Preventive OS check
		if(GetVersion()>= 0x80000000)
		{
			//This plug-in doesn't work on Windows 9x/ME
			MessageBoxA(hMainWindow,"This plug-in doesn't work on Windows 95, 98 and ME.","Unsupported operating system",MB_ICONERROR);
			return FALSE;
		}
#endif
		_mbstotcs(iniFile,f_iniFile,ARRSIZE(iniFile));
		iniFile[ARRSIZE(iniFile)-1]=_T('\0');
		iniSect=new INISection(iniFile,INISECTION);
		iniSect->Read();
		//Init the language file
		langFile = new LanguageFile(iniFile);
		//Check the version
		if(langFile->LanguageFilePresent())
		{
			std::_tcstring::size_type pos = langFile->GetTargetVersion().find_first_of(_T("*"));
			std::_tcstring curVersion = GetAboutMessage(false,false,false,true,false,false);
			bool rightVersion;
			if(pos==std::_tcstring::npos)
				rightVersion=(curVersion==langFile->GetTargetVersion());
			else
				rightVersion=(curVersion.compare(0,pos,langFile->GetTargetVersion(),0,pos))==0;
			if(!rightVersion)
			{
				//Avoid displaying wrong strings
				langFile->ActivateLanguageFile(false);
				//Check if the user wants to use wrong language files
				if(ErrMsgBox(hMainWindow,IDS_ERR_INVALIDLANGFILEVERSION,NULL,MB_ICONQUESTION | MB_YESNO)==IDYES)
					langFile->ActivateLanguageFile(true);
			}
		}
		//Common controls initialization (even if unnecessary)
		INITCOMMONCONTROLSEX ic;
		ic.dwSize=sizeof(ic);
		ic.dwICC=ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS;
		InitCommonControlsEx(&ic);
		//Init the copies of the parameters
		hIVWindow = f_hIVWindow;
		//Create a new dibsection from the IV DIB
		dsUpdater = new DibSectionUpdater(f_pImgDIB, DibSectCreate);
		//Get a DC to the IVWindow
		IVWDC=GetDC(hIVWindow);
		//Init the precision cursors
		normPC.SetDC(IVWDC);
		clsrPC.SetDC(IVWDC);
		clsrPC.SetCross(true);
		//Create the UITools container
		PointerEx<HDC> p_ivwdc(&IVWDC,true);
		PointerEx<HDC> p_compdc(&GetDSCompDC);
		UITC = new UIToolsContainer(p_ivwdc,p_compdc);
		//Create the toolbox
		hToolBoxWindow=CreateDialog(hCurInstance,MAKEINTRESOURCE(IDD_TOOLBOX),hMainWindow,TBDialogProc);
		if(!hToolBoxWindow)
		{
			//Reports the error and exit
			strncpy(errMSG,"Cannot create the dialog.",255);
			CleanUp();
			return FALSE;
		}
		//Perform the subclass
		realMainIVWndProc=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hMainWindow,GWLP_WNDPROC,(__int3264)(LONG_PTR)MainIVWndProc);
		realIVWndProc=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hIVWindow,GWLP_WNDPROC,(__int3264)(LONG_PTR)IVWndProc);
		//Invalidate the area of the main window (otherwise weird things happen)
		InvalidateRect(hMainWindow,NULL,TRUE);
		//Notify IV that we've created the plugin window
		SendMessage(hMainWindow,WM_PLUGIN_HWND,PLUGIN_ID_PAINT,(LPARAM)hToolBoxWindow);
	}
	catch(exception &ex)
	{
		strncpy(errMSG,ex.what(),255);
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__));
		CleanUp();
		return FALSE;
	}
	//Ok, we've done
	return TRUE;
}

//ToolBox dialog procedure
INT_PTR CALLBACK TBDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	try
	{
		//Message switch
		switch(uMsg)
		{
		case WM_INITDIALOG: //Dialog initialization
			hToolBoxWindow=hwndDlg;
			//Populates the toolbar
			AddToolbarButtons(GetDlgItem(hwndDlg,IDC_TB_TOOLBAR));
			//Set the range of the width updown control
			UpDown_SetRange(GetDlgItem(hwndDlg,IDC_TB_UD_WIDTH),MAXPENWIDTH,MINPENWIDTH);
			//Set the range of the tolerance updown control
			UpDown_SetRange(GetDlgItem(hwndDlg,IDC_TB_UD_TOLERANCE),255,0);
			//Init the window magnetizer
			windowMagnetizer.SetMainWindow(hMainWindow);
			windowMagnetizer.SetToolbar(hToolBoxWindow);
			windowMagnetizer.EnableMagnetization(true);
			//Load the settings
			LoadSettings(*iniSect);
			//Load the captions of the child windows (and of itself)
			langFile->InitializeDialogCaptions(hwndDlg, IDD_TOOLBOX);
			//The colors loaded from the settings may be not compatible with the loaded image
			AdaptColors(dsUpdater->GetDibSection());
			//Applies some settings
			SyncPenWidthUD();
			SyncToleranceUD();
			SetFillCH(fillFlag);
			UITC->SyncCurTool();
			//Give the focus to a static control (this way no other control is highlighted)
			SetFocus(GetDlgItem(hwndDlg,IDC_TB_COLORPAL));
			//Show the toolbox
			ShowWindow(hToolBoxWindow, SW_SHOW);
			return FALSE;
		case WM_ENABLE: //Enable state changed
			if(wParam) //Window enabled
			{
				//Set the focus on a "neutral" element; this not to give the focus to the toolbar control, that would display inappropriate tooltips.
				SetFocus(GetDlgItem(hwndDlg,IDC_TB_COLORPAL));
			}
			return TRUE;
		case WM_DRAWITEM: //Windows wants us to draw the content of a control
			{
				LPDRAWITEMSTRUCT lpDIS=(LPDRAWITEMSTRUCT)lParam;
				switch(wParam) //Control switch
				{
				case IDC_TB_COLORPAL: //Color pane
					//Even if we use only cached brush (i.e. we don't need to delete any brush) is always good to restore the originary objects in the DC;
					//we do so using BEGIN_SELOBJ and END_SELOBJ at the beginning and in the end.
					BEGIN_SELOBJ(lpDIS->hDC,GetSysColorBrush(COLOR_3DFACE),brush);
					BEGIN_SELOBJ(lpDIS->hDC,GetStockPen(BLACK_PEN),pen);
					//Paint the background with the dialog color
					PatBlt(lpDIS->hDC,EXPANDRECT_C(lpDIS->rcItem),PATCOPY);
					RECT fgRct, bgRct, swapRct;
					//Calculate the rectangles
					CalcColorPaneRects(&(lpDIS->rcItem),&fgRct,&bgRct,&swapRct,NULL);
					//Setup the arrow settings
					ArrowSettings tas;
					tas.arrowBase=5;
					tas.arrowLength=2;
					tas.drawFirstArrow=false;
					tas.drawSecondArrow=true;
					tas.openArrowHead=false;
					//Apply the ACUPs
					fgColor.ApplyACUPs(lpDIS->hDC);
					//Draw the bg rectangle
					SelectBrush(lpDIS->hDC,bgColor.GetBrush());
					Rectangle(lpDIS->hDC,EXPANDRECT_C(bgRct));
					//Draw the fg rectangle
					SelectBrush(lpDIS->hDC,fgColor.GetBrush());
					Rectangle(lpDIS->hDC,EXPANDRECT_C(fgRct));
					//Draw the swap arrows
					SelectBrush(lpDIS->hDC,GetStockBrush(BLACK_BRUSH));
					MoveToEx(lpDIS->hDC,swapRct.right-3,swapRct.top+3,NULL);
					ArrowTo(lpDIS->hDC,swapRct.left,swapRct.top+3,&tas);
					MoveToEx(lpDIS->hDC,swapRct.right-3,swapRct.top+3,NULL);
					ArrowTo(lpDIS->hDC,swapRct.right-3,swapRct.bottom,&tas);
					//Restore the originary DC objects
					END_SELOBJ(lpDIS->hDC,pen);
					END_SELOBJ(lpDIS->hDC,brush);
					//If the picker is the current tool ask it to redraw its little color pane; this is done simulating a WM_MOUSEMOVE
					if(UITC->GetCurrentToolID()==IDC_TB_PICKER)
						SimMM(hIVWindow,false);
					return TRUE;
				} //End control switch
				return FALSE;
			}
			//End WM_DRAWITEM
		case WM_NOTIFY: //Notifications from common controls
			{
				LPNMHDR hdr=(LPNMHDR)lParam;
				switch(hdr->code) //Notification codes switch
				{
				case TTN_GETDISPINFO: //The tooltip control of the toolbar requests what it should display
					{
						LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO) lParam;
						//The text to display is in the static buffer
						lpnmtdi->hinst=NULL;
						lpnmtdi->lpszText=NULL;
						if(!(lpnmtdi->uFlags&TTF_IDISHWND)) //The code that follows works only if idFrom is the button ID
						{
							UINT stringID=0;
							//Are the extended tooltips enabled?
							bool exToolTips=IsToolCheckedTB(IDC_TB_HELP);
							//If it's a non-tool button proceed in the normal way
							if((lpnmtdi->hdr.idFrom>=FIRSTNONTOOLBUTTON)&&(lpnmtdi->hdr.idFrom<=LASTNONTOOLBUTTON))
								stringID=(UINT)lpnmtdi->hdr.idFrom+(exToolTips?SHIFT_IDS_EX:0);
							else //Otherwise ask to the UIControls container if there's a control with that ID and ask directly to it what tooltip we have to display
							{
								UIBaseTool * tool=UITC->GetTool<UIBaseTool>((unsigned int)lpnmtdi->hdr.idFrom,false);
								if(tool!=NULL)
								{
									stringID=exToolTips?tool->GetTooltipExID():tool->GetTooltipID();
								}
							}
							//If we got a string ID load it
							if(stringID!=0)
								lpnmtdi->lpszText=const_cast<LPTSTR>(langFile->GetString(stringID).c_str());
						}
						return TRUE;
					}
				case NM_RCLICK: //The user right-clicked a button on the toolbar
					{
						LPNMMOUSE lpnmmouse = (LPNMMOUSE) lParam;
						//The notification comes from the toolbar
						if(hdr->hwndFrom==GetDlgItem(hwndDlg,IDC_TB_TOOLBAR))
						{
							UIBaseTool * tool = UITC->GetTool<UIBaseTool>((unsigned int)lpnmmouse->dwItemSpec,false);
							if(tool!=NULL)
							{
								EnableWindow(hwndDlg,FALSE);
								MessageReturnValue ret=tool->OnShowOptionsRequest(hMainWindow);
								EnableWindow(hwndDlg,TRUE);
								if((bool)ret)
								{
                                    SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,(__int3264)(LRESULT)ret);
									return TRUE;
								}
							}
						}
					}
				}
				return FALSE;
			}
		case WM_SETCURSOR: //Cursor move
			//Erase the precision cursors
			normPC.Hide();
			clsrPC.Hide();
			//Erase the picker color pane
			ErasePickerCPane();
			return FALSE;
			//End WM_SETCURSOR
		case WM_COMMAND: //Selections, clicks, etc.
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				switch(LOWORD(wParam)) //Controls switch
				{
				case IDC_TB_CH_FILL: //Click on the "fill" checkbox: set the internal flag accordingly
					fillFlag=GetFillCH();
					break;
				case IDC_TB_COLORPAL: //Click on the color static
					{
						RECT colPaneRct, fgRct, bgRct, swapRct;
						POINT pt;
						GetClientRect((HWND)lParam,&colPaneRct);
						CalcColorPaneRects(&colPaneRct,&fgRct,&bgRct,&swapRct, NULL); //Calculate the various rects
						GetCursorPos(&pt);
						ScreenToClient((HWND)lParam,&pt);
						//Decide what to do depending on the cursor position
						if(PtInRect(&swapRct,pt)) //Swap colors
						{
							COLORREF tcol;
							tcol=fgColor.GetColor();
							fgColor.SetColor(bgColor.GetColor());
							bgColor.SetColor(tcol);
						}
						else if(PtInRect(&fgRct,pt)) //Ask a new fg color
						{
							EnableWindow(hMainWindow,FALSE);
							fgColor.SetColor(GetColor(hwndDlg,fgColor.GetColor(),dsUpdater->GetDibSection())); //Set the new fg color
							EnableWindow(hMainWindow,TRUE);
						}
						else if(PtInRect(&bgRct,pt)) //Ask a new bg color
						{
							EnableWindow(hMainWindow,FALSE);
							bgColor.SetColor(GetColor(hwndDlg,bgColor.GetColor(),dsUpdater->GetDibSection())); //Set the new bg color
							EnableWindow(hMainWindow,TRUE);
						}
						else //Do nothing
						{
							return FALSE;
						}
						InvalidateRect((HWND)lParam,NULL,FALSE); //Update the color static
						UpdateWindow((HWND)lParam);
						break;
					}
				case IDC_TB_PBSETTINGS:
					EnableWindow(hwndDlg,FALSE);
					ShowPenBrushSettings(hMainWindow);
					EnableWindow(hwndDlg,TRUE);
					break;
				case IDC_TB_ABOUT: //Click on the "About..." button
					{
						EnableWindow(hwndDlg,FALSE);
						INT_PTR ret=DialogBox(hCurInstance,MAKEINTRESOURCE(IDD_ABOUT),hMainWindow,ABDialogProc);
						EnableWindow(hwndDlg,TRUE);
						if(ret==IDABORT)
							CleanUp();
						break;
					}
				case IDC_TB_HELP: //Click on the "Exhaustive tooltips" button: sets correct the delay of the tooltips
					{
						int duration=IsToolCheckedTB(IDC_TB_HELP)?30000:-1;
						SendMessage((HWND)Toolbar_GetToolTips(GetDlgItem(hwndDlg,IDC_TB_TOOLBAR)),TTM_SETDELAYTIME,TTDT_AUTOPOP,MAKELONG(duration,0));
					}
				default:
					//All the others can be the tool buttons; even if they are not the UIToolsContainer::SetCurrentToolID is very fast and reject correctly wrong input
					{
						//Click on the tools buttons: set the CurrentTool and enable/disable the extctrls
						UITC->SetCurrentToolID(LOWORD(wParam),(LOWORD(wParam)==IDC_TB_PICKER) && ((GetModifierKeysState()&MK_SHIFT)==0),false);
					break;
					}
				}
				break;
			case EN_CHANGE:
				switch(LOWORD(wParam)) //Controls switch
				{
				case IDC_TB_TX_WIDTH: //Witdh textbox
					//The text in the width textbox has changed: let's validate it!
					{
						static UINT widthLastValue=MINPENWIDTH;
						if(ValidateUIntFieldChange(hwndDlg,LOWORD(wParam),widthLastValue,MINPENWIDTH,MAXPENWIDTH))
							SetPenWidth((int)widthLastValue,false);
					}
					break; //case IDC_TB_TX_WIDTH:
				case IDC_TB_TX_TOLERANCE:
					//The text in the tolerance textbox has changed: let's validate it!
					{
						static UINT toleranceLastValue=0;
						if(ValidateUIntFieldChange(hwndDlg,LOWORD(wParam),toleranceLastValue,0,255))
							threshold=(BYTE)toleranceLastValue;
					}
					break; //case IDC_TB_TX_TOLERANCE:
				}
			}
			return FALSE;
		case WM_CLOSE: //The user requested the window to close
			DestroyWindow(hwndDlg); //Commits suicide :P
			return TRUE;
		case WM_DESTROY: //The window is being destroyed
			try
			{
				//Save the settings
				SaveSettings(*iniSect);
			}
			catch(exception)
			{
				if(!ignoreINIWriteErrors)
				{
					DWORD lastError=GetLastError();
					ErrMsgBox(hwndDlg,IDS_ERR_SAVESETTINGSFAIL,&lastError);
				}
			}
			//Destroy the ImageList
			ImageList_Destroy(tbImgList);
			return TRUE;
		case WM_NCDESTROY: //Idem, but the child windows have been destroyed
			//The toolbox window no longer exists
			hToolBoxWindow=NULL;
			//Perform the real cleanup
			CleanUp();
			return TRUE;
		case WM_SIZING: //Pre-window resize and move
		case WM_MOVING:
			//Notify the change to the window magnetizer
			windowMagnetizer.OnToolbarBoundsChanging(lParam);
			return TRUE;
		case WM_SIZE: //Window resize and move
		case WM_MOVE:
			//Notify the change to the window magnetizer
			windowMagnetizer.OnToolbarBoundsChanged();
			return TRUE;
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		normPC.ResetRefresh();
		clsrPC.ResetRefresh();
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		CleanUp();
	}
	return FALSE;
}
#pragma warning (push)
#pragma warning (disable:4100)
//"About IrfanPaint" dialog procedure
INT_PTR CALLBACK ABDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	try
	{
		switch(uMsg)
		{
		case WM_INITDIALOG:
			{
				//Load the captions of the child windows (and of itself)
				langFile->InitializeDialogCaptions(hwndDlg, IDD_ABOUT);
				//Set the version information
				std::_tcstring str;
				try
				{
					str=GetAboutMessage(true,true,false,true,true,true);
				}
				catch(...)
				{
					str=_T("Cannot retrieve version data.");
				}
				if(langFile->LanguageFileActivated())
				{
					std::_tcstring translationAbout=langFile->GetString(IDS_AB_ABOUTTRANSLATION);
					ReplaceString(translationAbout,_T("%language%"),langFile->GetLanguageName());
					ReplaceString(translationAbout,_T("%translator%"),langFile->GetTranslatorName());
					str+=_T("\r\n") + translationAbout;
				}
				SetDlgItemText(hwndDlg,IDC_AB_TEXT,str.c_str());
				//Set the credits
				SetDlgItemText(hwndDlg,IDC_AB_THANKS,langFile->GetString(IDS_THANKS).c_str());
				return TRUE;
			}
		case WM_COMMAND:
			if(HIWORD(wParam)==BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
				case IDOK:
					EndDialog(hwndDlg,0);
					break;
				case IDC_AB_SITE:
					ShellExecute(hwndDlg,_T("open"),_T("http://www.mitalia.net/irfanpaint/"),NULL,_T("."),SW_SHOW);
					break;
				}
			}
			return TRUE;
		case WM_CLOSE:
			EndDialog(hwndDlg,0);
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		EndDialog(hwndDlg,IDABORT);
	}
	return FALSE;
}
#pragma warning (pop)

//IrfanView main window window procedure
LRESULT CALLBACK MainIVWndProc(HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	try
	{
		//Message switch
		switch(uMsg)
		{
			case WM_MOUSEWHEEL: //Mouse wheel rotated
				switch(GetModifierKeysState()&(MK_SHIFT|MK_CONTROL))
				{
				case MK_SHIFT: //Mouse wheel rotated & SHIFT down => change pen width
					{
						static int delta=0;
						delta+=GET_WHEEL_DELTA_WPARAM(wParam);
						if(abs(delta)>=WHEEL_DELTA)
						{
							int newPenWidth=fgColor.GetPenWidth()+delta/WHEEL_DELTA;
							if((newPenWidth<=MAXPENWIDTH)&&(newPenWidth>=MINPENWIDTH))
								SetPenWidth(newPenWidth);
							delta=delta%WHEEL_DELTA;
						}
						return FALSE;
					} //End mouse wheel rotated & CTRL down
				case (MK_SHIFT | MK_CONTROL): //Mouse wheel rotated & CTRL + SHIFT down => change selected tool
					{
						static int delta=0;
						delta+=GET_WHEEL_DELTA_WPARAM(wParam);
						if(abs(delta)>=WHEEL_DELTA)
						{
							int increment=delta/WHEEL_DELTA,ctool=UITC->GetCurrentToolID()-FIRSTTOOLBUTTON;
							ctool=(ctool+increment)%(LASTTOOLBUTTON-FIRSTTOOLBUTTON+1);
							while(ctool<0)
								ctool+=(LASTTOOLBUTTON-FIRSTTOOLBUTTON+1);
							FORWARD_WM_COMMAND(hToolBoxWindow,ctool+FIRSTTOOLBUTTON,GetDlgItem(hToolBoxWindow,IDC_TB_TOOLBAR),BN_CLICKED,SendMessage);
							delta=delta%WHEEL_DELTA;
						}
						return FALSE;
					} //End mouse wheel rotated & CTRL+SHIFT down
				}
			break;
			//End mouse wheel rotated
			case WM_KEYDOWN: //Key down
			case WM_KEYUP: //Key up/key down
				switch(wParam)
				{
				case VK_CONTROL: //Modifier keys up/down
				case VK_MENU:
				case VK_SHIFT:
					SimMM(hIVWindow,false); //Simulate a WM_MOUSEMOVE (and a WM_SETCURSOR)
					break;
					//End modifier keys up/down
				case VK_SPACE: //Keypresses that must be suppressed
				case VK_BACK:
					return FALSE;
					//End keypresses that must be suppressed
				}
				break; //End key up/key down
			case WM_SETCURSOR: //Cursor moved
				if((HWND)wParam==hIVWindow)
					break;
				//Note: no break
			case WM_ACTIVATE:
				if(wParam!=WA_INACTIVE)
					break;
				//Note: no brak if wParam==WA_INACTIVE => the following code block is executed when the window is deactivated
			case WM_MOUSEMOVE: //Mouse moved
			case WM_NCMOUSEMOVE:
				//Erase the precision cursors
				normPC.Hide();
				clsrPC.Hide();
				//Erase the picker color pane
				ErasePickerCPane();
				break;
				//End mouse or cursor moved
			case WM_MOVE: //Window move & resize
			case WM_SIZE:
				//Notify the change to the window magnetizer
				windowMagnetizer.OnMainWindowBoundsChanged();
				//End window move & resize
				break;
		} //End message switch
	}
	catch(exception &ex) //Handle exceptions
	{
		normPC.ResetRefresh();
		clsrPC.ResetRefresh();
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		CleanUp();
	}
	return CallWindowProc(realMainIVWndProc,hWnd,uMsg,wParam,lParam); //All other messages are passed to the real MainIVW wndproc
} //End MainIVWndProc

//"IrfanViewer" window window procedure
LRESULT CALLBACK IVWndProc(HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	//Return value of UITC->HandleMessage
	MessageReturnValue mrv;
	MsgNumber++; //Increment the message counter (used by the helper functions to avoid stressing too much IV, see i.e. GetShift())
	//If the DibSection is not ready use the normal WndProc
	if(dsUpdater==NULL || !dsUpdater->CheckState())
		return CallWindowProc(realIVWndProc,hWnd,uMsg,wParam,lParam);
	try
	{
		mrv=UITC->HandleMessage(hWnd,uMsg,wParam,lParam);
		if(mrv)
			return mrv;
		//Message switch
		switch(uMsg)
		{
		case WM_PAINT: //Paint requested
			{
				LRESULT lr;
				//Remove the precision cursors
				normPC.BeginRefresh();
				clsrPC.BeginRefresh();
//#define IV_PAINT
#ifdef IV_PAINT
				//Paint the window (send the message to the real wnd proc)
				lr=CallWindowProc(realIVWndProc,NULL,uMsg,wParam,lParam);
#else
				//Does the painting by itself (much much faster, I don't know why)
				PAINTSTRUCT ps;
				if(dsUpdater!=NULL && dsUpdater->CheckState() && GetUpdateRect(hWnd,NULL,FALSE) && BeginPaint(hWnd, &ps))
				{
					RECT dibRect, imgRect, clientRect, dummyRect;
					bool noBorder;
					//Obtain the image rectangle
					GetImageRectPtr(&imgRect);		
					//Obtain the client rectangle
					GetClientRect(hWnd,&clientRect);
					noBorder=EqualRect(&imgRect,&clientRect)!=FALSE;
					//Image drawing
					//Convert the image rectangle in DIB coords
					dibRect=imgRect;
					IVWnd2DIBPoint((POINT *)&dibRect);
					IVWnd2DIBPoint(((POINT *)&dibRect)+1);
					//Eventually expand it to get also the neighbor pixels
					if(dibRect.bottom<dsUpdater->GetDibSection()->GetDibHeight())
						dibRect.bottom++;
					if(dibRect.right<dsUpdater->GetDibSection()->GetDibWidth())
						dibRect.right++;
					//Now convert it back in IVW coords (to round them to the nearest whole pixel)
					imgRect=dibRect;
					DIB2IVWndPoint((POINT *)&imgRect);
					DIB2IVWndPoint(((POINT *)&imgRect)+1);
					if(noBorder)
					{
						imgRect.right=max(imgRect.right,clientRect.right);
						imgRect.bottom=max(imgRect.bottom,clientRect.bottom);
					}
					//Change the stretch mode
					SetStretchBltMode(ps.hdc,/*HALFTONE/*/COLORONCOLOR/**/); //TODO: implement a way to let the user choose HALFTONE/COLORONCOLOR
					//I don't know if this is actually useful, but MSDN says that I must do it, so...
					SetBrushOrgEx(ps.hdc,0,0,NULL);
					//Paint the image
					StretchBlt(
						ps.hdc,
						EXPANDRECT_CS(imgRect),
						dsUpdater->GetDibSection()->GetCompDC(),
						EXPANDRECT_CS(dibRect),
						SRCCOPY);
#ifdef USE_OBJECTS
					//TODO: finish
					//Objects drawing
					{
						//Save the DC
						int savedDC=SaveDC(ps.hdc);
						//Set up the mappings
						SetMapMode(ps.hdc,MM_ANISOTROPIC);
						SetViewportOrgEx(ps.hdc,imgRect.left,imgRect.top,NULL);
						SetViewportExtEx(ps.hdc,imgRect.right-imgRect.left,imgRect.bottom-imgRect.top,NULL);
						SetWindowOrgEx(ps.hdc,dibRect.left,dibRect.top,NULL);
						SetWindowExtEx(ps.hdc,RECTWIDTH(dibRect),RECTHEIGHT(dibRect),NULL);
						objects.DrawAll(ps.hdc,NULL);
						//Restore the DC
						RestoreDC(ps.hdc,savedDC);
					}
#endif
					//BG drawing
					if(!noBorder && IntersectRect(&dummyRect,&clientRect,&ps.rcPaint))
					{
						HBRUSH bgBrush=NULL; //Background brush
						HRGN clientRgn, imageRgn, bgRgn;
						//TODO: remove this INI read
						bgBrush=CreateSolidBrush(GetPrivateProfileInt(_T("Viewing"),_T("BackColor"),0,iniSect->GetINIFile().c_str())&0x00ffffff);
						clientRgn=CreateRectRgnIndirect(&clientRect);
						imageRgn=CreateRectRgnIndirect(&imgRect);
						bgRgn=CreateRectRgn(0,0,1,1);
						CombineRgn(bgRgn,clientRgn,imageRgn,RGN_DIFF);
						DeleteRgn(clientRgn);
						DeleteRgn(imageRgn);
						FillRgn(ps.hdc,bgRgn,bgBrush);
						DeleteRgn(bgRgn);
						DeleteBrush(bgBrush);
					}
					//Selection drawing
					RECT selRect=GetSelectedRect();
					DIB2IVWndPoint((POINT *)&selRect);
					DIB2IVWndPoint(((POINT *)&selRect)+1);
					if(!IsRectEmpty(&selRect))
					{
						//Include also the bottom and right coords
						selRect.bottom++;
						selRect.right++;
						int saveDC=SaveDC(ps.hdc);
						SelectBrush(ps.hdc,GetStockBrush(NULL_BRUSH));
						SelectPen(ps.hdc,GetStockPen(BLACK_PEN));
						SetROP2(ps.hdc,R2_NOT);
						Rectangle(ps.hdc,EXPANDRECT_C(selRect));
						RestoreDC(ps.hdc,saveDC);
					}
					EndPaint(hWnd, &ps);
				}
				lr=0;
#endif
				//Redraw the precision cursors
				normPC.EndRefresh();
				clsrPC.EndRefresh();
//#define TEST_DRAW_GRID
#ifdef TEST_DRAW_GRID
				//Draw a grid
				if(GetZoom()>100)
				{
					RECT clientRect;
					POINT pt;
					int last;
					last=0;
					GetClientRect(hIVWindow,&clientRect);
					for(int x=0;x<clientRect.right;x++)
					{
						pt.x=x;
						pt.y=0;
						IVWnd2DIBPoint(&pt);
						if(pt.x!=last)
						{
							MoveToEx(IVWDC,x,0,NULL);
							LineTo(IVWDC,x,clientRect.bottom);
						}
						last=pt.x;
					}
					last=0;
					for(int y=0;y<clientRect.bottom;y++)
					{
						pt.y=y;
						pt.x=0;
						IVWnd2DIBPoint(&pt);
						if(pt.y!=last)
						{
							MoveToEx(IVWDC,0,y,NULL);
							LineTo(IVWDC,clientRect.right,y);
						}
						last=pt.y;
					}
				}
#endif
//#define TEST_DRAW_BORDER
#ifdef TEST_DRAW_BORDER
				//Draw a line around the image
				BEGIN_SELOBJ(IVWDC,CreatePen(PS_SOLID,1,0x000000FF),pen);
				BEGIN_SELOBJ(IVWDC,GetStockBrush(HOLLOW_BRUSH),brush);
				RECT imgRect=GetImageRect();
				Rectangle(IVWDC,EXPANDRECT_C(imgRect));
				END_SELOBJ(IVWDC,brush);
				DeletePen(END_SELOBJ(IVWDC,pen));
#endif
				return lr;			
			} //End paint requested
		} //End message switch
	}
	catch(exception &ex) //Handle exceptions
	{
		normPC.ResetRefresh();
		clsrPC.ResetRefresh();
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		CleanUp();
	}
	return CallWindowProc(realIVWndProc,hWnd,uMsg,wParam,lParam); //All other messages are passed to the real IVW wndproc
} //End IVWndProc
//Populates the toolbar
void AddToolbarButtons(HWND hToolbar)
{
	const SIZE buttonSize={26,26};
	//Create the ImageList
	tbImgList=ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,8,8);
	//Init the toolbar
	Toolbar_ButtonStructSize(hToolbar);
	Toolbar_SetImageList(hToolbar,0,tbImgList);
	Toolbar_SetButtonSize(hToolbar,buttonSize.cx,buttonSize.cy);
	//Add the UITools buttons
	UITC->AddButtonsToToolbar(hToolbar);
	//Add the other buttons
	//Buttons array
	TBBUTTON btnArr[LASTNONTOOLBUTTON-FIRSTNONTOOLBUTTON+1];
	memset(btnArr,0,sizeof(btnArr));
	//Temp icon
	HICON tIcon;
	//Icon index
	int iconIndex;
	//Add the icons and fill the buttons array
	for(int buttonID=FIRSTNONTOOLBUTTON;buttonID<=LASTNONTOOLBUTTON;buttonID++)
	{
		iconIndex=ImageList_AddIcon(tbImgList,tIcon=(HICON)LoadImage(hCurInstance,MAKEINTRESOURCE(buttonID),IMAGE_ICON,0,0,0));
		DestroyIcon(tIcon);
		btnArr[buttonID-FIRSTNONTOOLBUTTON].idCommand=buttonID;
		btnArr[buttonID-FIRSTNONTOOLBUTTON].iBitmap=iconIndex;
		btnArr[buttonID-FIRSTNONTOOLBUTTON].fsStyle=buttonID==IDC_TB_HELP?BTNS_CHECK:BTNS_BUTTON;
		btnArr[buttonID-FIRSTNONTOOLBUTTON].fsState=TBSTATE_ENABLED;
	}
	//Add the buttons to the toolbar
	Toolbar_AddButtons(hToolbar,LASTNONTOOLBUTTON-FIRSTNONTOOLBUTTON+1,btnArr);
	//Resizing code
	int neededHeight=((int)Toolbar_GetRows(hToolbar)*buttonSize.cy);
	RECT wndRct;
	//Resize the toolbar window
	GetWindowRect(hToolBoxWindow,&wndRct);
	wndRct.bottom+=neededHeight;
	SetWindowPos(hToolBoxWindow,0,0,0,RECTWIDTH(wndRct),RECTHEIGHT(wndRct),SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	//Resize the toolbar control
	GetClientRect(hToolbar,&wndRct);
	wndRct.bottom+=neededHeight;
	SetWindowPos(hToolbar,0,0,0,RECTWIDTH(wndRct),RECTHEIGHT(wndRct),SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	//Move the child controls
	EnumChildWindows(hToolBoxWindow,&ATB_EnumChildProc,neededHeight);
}
//Procedure called by EnumChildWindows (called by AddToolbarButtons)
BOOL CALLBACK ATB_EnumChildProc(HWND hwnd, LPARAM lParam)
{
	if(hwnd!=GetDlgItem(hToolBoxWindow,IDC_TB_TOOLBAR))
	{
		RECT wndRct;
		GetWindowRect(hwnd,&wndRct);
		MapWindowPoints(NULL,hToolBoxWindow,(LPPOINT)&wndRct,2);
		OffsetRect(&wndRct,0,(int)lParam);
		MoveWindow(hwnd,EXPANDRECT_CS(wndRct),TRUE);
	}
	return TRUE;
}
//Performs the init tasks when a new DibSection is created
void DibSectCreate(DibSection * f_this)
{
	AdaptColors(f_this);
	if(UITC!=NULL)
		UITC->OnDibSectCreate(f_this);
}
//Sets the fg, bg and hatch bg color to a color compatible with the loaded image
void AdaptColors(DibSection * ds)
{
	fgColor.SetColor(GetNearestColor(ds->GetCompDC(),fgColor.GetColor()));
	bgColor.SetColor(GetNearestColor(ds->GetCompDC(),bgColor.GetColor()));
	COLORREF hatchBg=fgColor.GetRefACUPs().bgColor;
	hatchBg=GetNearestColor(ds->GetCompDC(),hatchBg);
	bgColor.GetRefACUPs().bgColor=hatchBg;
	fgColor.GetRefACUPs().bgColor=hatchBg;
	InvalidateRect(GetDlgItem(hToolBoxWindow,IDC_TB_COLORPAL),NULL,FALSE);
}
//Calculates the coords of the four color pane rectangles
void CalcColorPaneRects(const RECT * ColPaneRct, RECT * FgRct, RECT * BgRct, RECT * SwapRct, RECT * PickerRct)
{
	SIZE rcSize={ColPaneRct->right-ColPaneRct->left,ColPaneRct->bottom-ColPaneRct->top};
	if(FgRct!=NULL)
	{
		FgRct->left=ColPaneRct->right-(int)(rcSize.cx*COLORPALRCTSHSIZEMULT);
		FgRct->top=ColPaneRct->bottom-(int)(rcSize.cy*COLORPALRCTSVSIZEMULT);
		FgRct->right=ColPaneRct->right;
		FgRct->bottom=ColPaneRct->bottom;
	}
	if(BgRct!=NULL)
	{
		BgRct->left=ColPaneRct->left;
		BgRct->top=ColPaneRct->top;
		BgRct->right=ColPaneRct->left+(int)(rcSize.cx*COLORPALRCTSHSIZEMULT);
		BgRct->bottom=ColPaneRct->top+(int)(rcSize.cy*COLORPALRCTSVSIZEMULT);
	}
	if(SwapRct!=NULL)
	{
		SwapRct->left=ColPaneRct->left+rcSize.cx-COLORPALSWAPRHEIGHT;
		SwapRct->top=ColPaneRct->top;
		SwapRct->right=ColPaneRct->right;
		SwapRct->bottom=ColPaneRct->top+COLORPALSWAPRWIDTH;
	}
	if(PickerRct!=NULL)
	{
		PickerRct->left=ColPaneRct->left;
		PickerRct->top=ColPaneRct->bottom-COLORPALPICKRHEIGHT;
		PickerRct->right=ColPaneRct->left+COLORPALPICKRWIDTH;
		PickerRct->bottom=ColPaneRct->bottom;
	}
}
/*
The ancient Test function... :-)
void CALLBACK Test(
  HWND hwnd,        // handle to owner window
  HINSTANCE hinst,  // instance handle for the DLL
  LPTSTR lpCmdLine, // string the DLL will parse
  int nCmdShow      // show state
)
{
	DialogBox(hCurInstance,MAKEINTRESOURCE(IDD_TOOLBOX),NULL,DialogProc);
	return;
}*/
