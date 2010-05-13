#include "stdafx.h"				//Standard inclusions
#include "Utils.h"				//Support functions & macros
#include "ToolbarButtons.h"		//Toolbar buttons #define
#include "resource.h"			//Resources #define
#include "Globals.h"			//Global definitions
#include "SelPaletteColor.h"	//Select palette color dialog
#include "DibSection.h"			//DibSection class
#include "WindowMagnetizer.h"	//WindowMagnetizer class
#include "Tools.h"				//UITools classes
#include "IrfanPaint.h"			//IrfanPaint.cpp declares
#include "LanguageFile.h"
#include "INISection.h"
//Message number counter. Used by the helper functions that send messages to IV to decide either to send the message either to use a cached value
unsigned int MsgNumber=0;
//RichEdit dll handle (used by LoadRichEdit)
HMODULE hRichEditModule=NULL;
//Flag that indicates if IP should ignore INI-file writing errors
bool ignoreINIWriteErrors=false;
//Loads the RichEdit module
bool LoadRichEdit()
{
	if(hRichEditModule!=NULL)
		return true;
	else
		return (hRichEditModule=LoadLibrary(_T("Riched20.dll")))!=NULL;
}
//Unloads the RichEdit module
void FreeRichEdit()
{
	if(hRichEditModule!=NULL)
	{
		FreeLibrary(hRichEditModule);
		hRichEditModule=NULL;
	}
}
//Returns a DWORD representing an RGB color from a COLORREF
DWORD COLORREF2DWORD(COLORREF cr, RGBQUAD * palette)
{
	RGBQUAD * rq=palette;
	if(((cr&0x10FF0000)==0x10FF0000)||((cr&0x01000000)==0x01000000)) //If cr is a "false" COLORREF extract the index
	{
		if(palette==NULL) //There's something wrong, return what was passed
			return (DWORD)cr;
		else //Extract the color of the index
		{
			rq=palette+LOWORD(cr);
			return RGB(rq->rgbRed,rq->rgbGreen,rq->rgbBlue);
		}
	}
	else //Return a "cleaned" COLORREF
		return cr & 0x00FFFFFF;

}
//Makes equal the measures of a figure (e.g. the coords describing a rectangle are changed so that they describe a square); only editablePt is changed
void MakeEqualMeasures(POINT & editablePt, POINT fixedPt)
{
	SIZE objSize={abs(fixedPt.x-editablePt.x),abs(fixedPt.y-editablePt.y)};
	//The "right size" is the smallest
	if(objSize.cx<objSize.cy) //The right size is the width
		editablePt.y=fixedPt.y+(SHORT)objSize.cx*((editablePt.y<fixedPt.y)?-1:1); //Calculate the last point's X by consequence
	else //The right size is the heigth
		editablePt.x=fixedPt.x+(SHORT)objSize.cy*((editablePt.x<fixedPt.x)?-1:1); //Calculate the last point's Y by consequence
}
//Costrains a line to be tilted of 45° or hortogonal; only editablePt is changed
void CostrainHortOr45Line(POINT & editablePt, POINT fixedPt)
{
	POINT &p0=fixedPt,&p=editablePt; //Used just to be conform to my little draft
	/*             x
			+-------------------------->
			|
			|			  point that makes the line tilted of 45°
			|                 /\
			|	point          .p2
			|	that <= p1.___/_\.p => cursor position
		  y |	makes     I  /   i
			|	the line  I /    i
			|	vertical  I/     i
			|			  +======.
			|			 p0     p3 => point that makes the line horizontal
			|			 vv
			V		  (first point)
	*/
	POINT p1={p0.x,p.y},p3={p.x,p0.y};
	//The equation I solved to find p2 works only in the first and third quadrant, so eventually mirror the y axis to reduce every case to this one
	bool mirrorY=((p.x>p0.x)&&(p.y<p0.y))||((p.x<p0.x)&&(p.y>p0.y));
	POINT _p={p.x-p0.x,(p.y-p0.y)*(mirrorY?-1:1)}; //_p: p referred to a carthesian plane with origin p0
	POINT _p2={(_p.x+_p.y)/2,(_p.x+_p.y)/2}; //_p2: p2 referred to a carthesian plane with origin p0
	POINT p2={_p2.x+p0.x,_p2.y*(mirrorY?-1:1)+p0.y};
	double p1p=PointsDistance(p1,p), p2p=PointsDistance(p2,p), p3p=PointsDistance(p3,p); //Calculate the distances
	editablePt=(p1p<p2p)?(p1p<p3p?p1:p3):(p2p<p3p?p2:p3); //Chooses the point that is less far away from the point pointed by the cursor
}
//Returns a color chosen by the user through a dialog
COLORREF GetColor(HWND hOwner, COLORREF defColor, DibSection * dibSect)
{
	if(dibSect!=NULL && dibSect->GetPaletteEntries()!=0)
		return SelPaletteColor(hOwner,dibSect->GetPalette(),dibSect->GetPaletteEntries(),defColor);
	else
	{
		CHOOSECOLOR chc;
		static COLORREF acrCustClr[16]; //There we store the user's custom colors
		memset(&chc,0,sizeof(chc));
		chc.lStructSize=sizeof(chc);
		chc.hwndOwner=hOwner;
		chc.Flags=CC_ANYCOLOR|CC_RGBINIT;
		chc.rgbResult=defColor;
		chc.lpCustColors=acrCustClr;
		ChooseColor(&chc);
		return chc.rgbResult;
	}
}
//Returns a string with some information about the dll
std::_tcstring GetAboutMessage(bool insertName, bool insertCompany, bool insertComments, bool insertVersion, bool insertBeta, bool insertConnectors)
{
#pragma push_macro("EXC")
#define EXC std::runtime_error(ERROR_STD_PROLOG "Cannot obtain the version information.")
	TCHAR filename[MAX_PATH];
	TCHAR * appName=NULL;
	TCHAR * companyName=NULL;
	TCHAR * comments=NULL;
	bool inserted=false;
	std::_tcostringstream os;
	UINT uLen;
	if(!GetModuleFileName(hCurInstance,filename,MAX_PATH))
		if(GetLastError()!=0) throw EXC;
	VS_FIXEDFILEINFO * pvsf;
	DWORD dwHandle;
	DWORD cchver = GetFileVersionInfoSize(filename,&dwHandle);
	if (cchver == 0) 
		if(GetLastError()!=0) throw EXC;
	BYTE * pver = new BYTE[cchver];
	try
	{
		//Get the file version information
		if (!GetFileVersionInfo(filename,dwHandle,cchver,pver)) 
			if(GetLastError()!=0) throw EXC;
		//Extract the values
		//Version numbers
		if(!VerQueryValue(pver,_T("\\"),(LPVOID *)&pvsf,&uLen) || uLen!=sizeof(*pvsf)) 
			if(GetLastError()!=0) throw EXC;
		//Application name
		if(!VerQueryValue(pver, _T("\\StringFileInfo\\040904b0\\InternalName"), (LPVOID *)&appName, &uLen))
			if(GetLastError()!=0) throw EXC;
		//Comments
		if(!VerQueryValue(pver, _T("\\StringFileInfo\\040904b0\\Comments"), (LPVOID *)&comments, &uLen))
			if(GetLastError()!=0) throw EXC;
		//Company
		if(!VerQueryValue(pver, _T("\\StringFileInfo\\040904b0\\CompanyName"), (LPVOID *)&companyName, &uLen))
			if(GetLastError()!=0) throw EXC;
		//Build the string
		if(insertName)
		{
			os<<appName;
			inserted=true;
		}
		if(insertCompany)
		{
			if(inserted)
				os<<_T(" ");
			if(insertConnectors)
				os<<_T("by ");
			os<<companyName;
		}
		if(insertComments)
		{
			if(inserted)
				os<<_T(" ");
			if(insertConnectors)
				os<<_T("- ");
			os<<comments;
		}
		if(insertVersion)
		{
			if(inserted)
				os<<std::endl;
			if(insertConnectors)
				os<<_T("v. ");
			if((pvsf->dwFileFlags&VS_FF_PRERELEASE) && insertBeta) //If it's a beta add a ß
				os<<_T("ß");
			os<<(int)HIWORD(pvsf->dwFileVersionMS)<<_T('.')<<(int)LOWORD(pvsf->dwFileVersionMS)<<_T('.')<<(int)HIWORD(pvsf->dwFileVersionLS)<<_T('.')<<(int)LOWORD(pvsf->dwFileVersionLS);
		}
		delete[] pver;
		return os.str();
	}
	catch(...)
	{
		delete[] pver;
		throw;
	}

#undef EXC
#pragma pop_macro("EXC")
}
//Saves the settings of IrfanPaint
void SaveSettings(INISection & IniSect)
{
#pragma push_macro("EXC")
#define EXC std::runtime_error(ERROR_STD_PROLOG "Cannot save the settings; WritePrivateProfileString returned 0.")
	LOGBRUSH tlb;
	nEXTLOGPEN tlp;
	AdditionalCUProps ACUPs;
	TCHAR buffer[1024];
	IniSect.BeginWrite();
	//Settings of the tools container and of the tools
	UITC->SaveSettings(IniSect);
	//ExtendedHelp - written in decimal (but actually write a 0/1)
	IniSect.PutKey(_T("extendedHelp"),IsToolCheckedTB(IDC_TB_HELP));
	//BG brush - written in encoded form
	tlb=bgColor.GetBrushSettings();
	IniSect.PutKey(_T("bgBrush"),&tlb,sizeof(tlb));
	//FG brush - written in encoded form
	tlb=fgColor.GetBrushSettings();
	IniSect.PutKey(_T("fgBrush"),&tlb,sizeof(tlb));
	//BG pen - written in encoded form
	tlp=bgColor.GetPenSettings();
	IniSect.PutKey(_T("bgPen"),&tlp,sizeof(tlp));
	//BG pen user style - written as a string
	if(tlp.dwStyleCount!=0)
		IniSect.PutKey(_T("bgPenUserStyle"),StyleArray2DotSpaceStr(tlp.lpStyle,tlp.dwStyleCount));
	//FG pen - written in encoded form
	tlp=fgColor.GetPenSettings();
	IniSect.PutKey(_T("fgPen"),&tlp,sizeof(tlp));
	//FG pen user style - written as a string
	if(tlp.dwStyleCount!=0)
	{
		IniSect.PutKey(_T("fgPenUserStyle"),StyleArray2DotSpaceStr(tlp.lpStyle,tlp.dwStyleCount));
	}
	//ACUPs - written in encoded form
	ACUPs=fgColor.GetACUPs();
	IniSect.PutKey(_T("ACUPs"),&ACUPs,sizeof(ACUPs));
	//Fill flag - written in decimal (but actually write a 0/1)
	IniSect.PutKey(_T("fillFlag"),fillFlag);
	//Threshold - written in decimal
	IniSect.PutKey(_T("threshold"),threshold);
	//Position and magnetization settings
	RECT tbRect;
	GetWindowRect(hToolBoxWindow,&tbRect);
	IniSect.PutKey(_T("TBXPos"),tbRect.left);
	IniSect.PutKey(_T("TBYPos"),tbRect.top);
	IniSect.PutKey(_T("TBMagnOffset"),windowMagnetizer.GetMagnetizationOffset());
	IniSect.PutKey(_T("TBMagnLimit"),windowMagnetizer.GetMagnetizationLimit());
	switch(windowMagnetizer.GetMagnetizationState())
	{
	case WindowMagnetizer::OnLeftSide:
		_tcscpy(buffer,_T("OnLeftSide"));
		break;
	case WindowMagnetizer::OnRightSide:
		_tcscpy(buffer,_T("OnRightSide"));
		break;
	default:
		_tcscpy(buffer,_T("NotMagnetized"));
	}
	IniSect.PutKey(_T("TBMagnState"),buffer);
	IniSect.EndWrite();
#undef EXC
#pragma pop_macro("EXC")
}
//Loads the settings of IrfanPaint
void LoadSettings(INISection & IniSect)
{
	LOGBRUSH tlb={0};
	nEXTLOGPEN tlp={0};
	AdditionalCUProps ACUPs;
	IniSect.BeginRead();
	//Settings of the tools container and of the tools
	UITC->LoadSettings(IniSect);
	//ExtendedHelp
	if(IniSect.GetKey(_T("extendedHelp"),1))
	{
		CheckToolTB(IDC_TB_HELP);
		FORWARD_WM_COMMAND(hToolBoxWindow,IDC_TB_HELP,GetDlgItem(hToolBoxWindow,IDC_TB_TOOLBAR),0,SendMessage);
	}
	//BG brush
	if(IniSect.KeyExists(_T("bgBrush")))
	{
		try
		{
			IniSect.GetKey(_T("bgBrush"),&tlb,sizeof(tlb));
			bgColor.SetBrushSettings(tlb);
		}
		catch(...)
		{
			bgColor.SetColor(0xFFFFFF, true);
		}
	}
    //FG brush
	if(IniSect.KeyExists(_T("fgBrush")))
	{
		try
		{
			IniSect.GetKey(_T("fgBrush"),&tlb,sizeof(tlb));
			fgColor.SetBrushSettings(tlb);
		}
		catch(...)
		{
			fgColor.SetColor(0x000000, true);
		}
	}
	//BG pen
	if(IniSect.KeyExists(_T("bgPen")))
	{
		IniSect.GetKey(_T("bgPen"),&tlp,sizeof(tlp));
		if(tlp.dwPenStyle&PS_USERSTYLE)
		{
			std::_tcstring bpus=IniSect.GetKey(_T("bgPenUserStyle"),_T("."));
			tlp.dwStyleCount=DotSpaceStr2StyleArray(bpus.c_str(),NULL,true);
			if(tlp.dwStyleCount!=0)
			{
				tlp.lpStyle=new DWORD[tlp.dwStyleCount];
				DotSpaceStr2StyleArray(bpus.c_str(),tlp.lpStyle,true);
			}
			else
			{
				tlp.dwPenStyle&=!PS_USERSTYLE;
				tlp.dwPenStyle|=PS_SOLID;
				tlp.lpStyle=NULL;
			}
		}
		else
		{
			tlp.lpStyle=NULL;
			tlp.dwStyleCount=0;
		}
		try
		{
			bgColor.SetPenSettings(tlp);
		}
		catch(...)
		{
			bgColor.SetPenWidth(1,true);
		}
		if(tlp.lpStyle!=NULL)
			delete [] tlp.lpStyle;
	}
	//FG pen
	if(IniSect.KeyExists(_T("fgPen")))
	{
		IniSect.GetKey(_T("fgPen"),&tlp,sizeof(tlp));
		if(tlp.dwPenStyle&PS_USERSTYLE)
		{
			std::_tcstring fpus=IniSect.GetKey(_T("fgPenUserStyle"),_T("."));
			tlp.dwStyleCount=DotSpaceStr2StyleArray(fpus.c_str(),NULL,true);
			if(tlp.dwStyleCount!=0)
			{
				tlp.lpStyle=new DWORD[tlp.dwStyleCount];
				DotSpaceStr2StyleArray(fpus.c_str(),tlp.lpStyle,true);
			}
			else
			{
				tlp.dwPenStyle&=!PS_USERSTYLE;
				tlp.dwPenStyle|=PS_SOLID;
				tlp.lpStyle=NULL;
			}
		}
		else
		{
			tlp.lpStyle=NULL;
			tlp.dwStyleCount=0;
		}
		try
		{
			fgColor.SetPenSettings(tlp);
		}
		catch(...)
		{
			fgColor.SetPenWidth(1,true);
		}
		if(tlp.lpStyle!=NULL)
			delete [] tlp.lpStyle;
	}
	//ACUPs
	if(IniSect.KeyExists(_T("ACUPs")))
	{
		IniSect.GetKey(_T("ACUPs"),&ACUPs,sizeof(ACUPs));
		ACUPs.bgCU=NULL;
		fgColor.SetACUPs(ACUPs);
		bgColor.SetACUPs(ACUPs);
	}
	//Fill flag
	fillFlag=IniSect.GetKey(_T("fillFlag"),false);
	//Threshold
	threshold=(BYTE)(IniSect.GetKey(_T("threshold"),30)%256);
	//Position and magnetization settings
	if(IniSect.KeyExists(_T("TBMagnState")))
	{
		std::_tcstring tbms=IniSect.GetKey(_T("TBMagnState"),_T("NotMagnetized"));
		if(tbms==_T("OnLeftSide"))
			windowMagnetizer.SetMagnetizationState(WindowMagnetizer::OnLeftSide);
		else if(tbms==_T("OnRightSide"))
			windowMagnetizer.SetMagnetizationState(WindowMagnetizer::OnRightSide);
		else
			windowMagnetizer.SetMagnetizationState(WindowMagnetizer::NotMagnetized);
	}
	else if(!IniSect.KeyExists(_T("TBXPos")))
	{
		//If neither the magnetization offset nor the TB position are specified use the default settings
		windowMagnetizer.SetMagnetizationState(WindowMagnetizer::OnRightSide);
		windowMagnetizer.SetMagnetizationOffset(70);
	}
	windowMagnetizer.SetMagnetizationLimit(abs((int)IniSect.GetKey(_T("TBMagnLimit"),windowMagnetizer.GetMagnetizationLimit())));
	if(windowMagnetizer.GetMagnetizationState()==WindowMagnetizer::NotMagnetized)
	{
		RECT tbRect;
		GetWindowRect(hToolBoxWindow,&tbRect);
		tbRect.left=IniSect.GetKey(_T("TBXPos"),tbRect.left);
		tbRect.top=IniSect.GetKey(_T("TBYPos"),tbRect.top);
		SetWindowPos(hToolBoxWindow, 0, tbRect.left, tbRect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		MoveInWorkingArea(hToolBoxWindow);
	}
	else
	{
		windowMagnetizer.SetMagnetizationOffset(IniSect.GetKey(_T("TBMagnOffset"),windowMagnetizer.GetMagnetizationOffset()));
		windowMagnetizer.UpdateToolbarPos();
		windowMagnetizer.CheckToolbarPos();
	}
	//Ignore INI write errors flag (read only)
	ignoreINIWriteErrors=IniSect.GetKey(_T("IgnoreINIWriteErrors"),false);
	IniSect.EndRead();
}
//Returns the currently selected pen width in the updown
int GetPenWidthUD()
{
	return (int)UpDown_GetPos(GetDlgItem(hToolBoxWindow,IDC_TB_UD_WIDTH));
}
//Syncs the pen width updown with the pen width of the fg ColorUtils
void SyncPenWidthUD()
{
	SetDlgItemInt(hToolBoxWindow,IDC_TB_TX_WIDTH,fgColor.GetPenWidth(),FALSE);
}
//Syncs the tolerance updown with the internal tolerance variable
void SyncToleranceUD()
{
	SetDlgItemInt(hToolBoxWindow,IDC_TB_TX_TOLERANCE,(UINT)threshold,FALSE);
}

//Returns the state of the fill checkbox
bool GetFillCH()
{
	return Button_GetCheck(GetDlgItem(hToolBoxWindow,IDC_TB_CH_FILL))==BST_CHECKED;
}
//Sets the state of the fill checkbox
void SetFillCH(bool fillFlag)
{
	Button_SetCheck(GetDlgItem(hToolBoxWindow,IDC_TB_CH_FILL),fillFlag?BST_CHECKED:BST_UNCHECKED);
}
//Returns the currently selected button in the toolbar
int GetCurrentToolTB()
{
	TBBUTTONINFO tbbi={0};
	tbbi.cbSize=sizeof(tbbi);
	tbbi.dwMask=TBIF_STATE;
	for(int btnID=FIRSTBUTTON;btnID<=LASTBUTTON;btnID++)
	{
		Toolbar_GetButtonInfo(GetDlgItem(hToolBoxWindow,IDC_TB_TOOLBAR),btnID,&tbbi);
		if(tbbi.fsState&TBSTATE_CHECKED)
			return btnID;
	}
	return 0;
}
//Returns the check state of a button in the toolbar
bool IsToolCheckedTB(int tool)
{
	TBBUTTONINFO tbbi={0};
	tbbi.cbSize=sizeof(tbbi);
	tbbi.dwMask=TBIF_STATE;
	Toolbar_GetButtonInfo(GetDlgItem(hToolBoxWindow,IDC_TB_TOOLBAR),tool,&tbbi);
	return (tbbi.fsState&TBSTATE_CHECKED)!=0;
}
//Checks the given button in the toolbar
void CheckToolTB(unsigned int tool)
{
	Toolbar_CheckButton(GetDlgItem(hToolBoxWindow,IDC_TB_TOOLBAR),tool,TRUE);
}
//Returns the first checked button of a series of buttons
int GetCheckedButton(HWND hParent, int firstButton, int lastButton)
{
	for(;firstButton<=lastButton;firstButton++)
		if(IsDlgButtonChecked(hParent,firstButton)==BST_CHECKED)
			return firstButton;
	return 0;
}
//Rounds a number to the nearest integer
long round(double number)
{
	return (long)(number+0.5);
}
//Sets up the clipping region of IVWDC and dsUpdater->GetDibSection()->m_compDC
void SetupClippingRegion()
{
	RECT IVClipRct={0}, DIBClipRct={0};
	HRGN IVClipRgn=NULL, DIBClipRgn=NULL;
	DIBClipRct=GetSelectedRect();
	if(dsUpdater!=NULL && dsUpdater->CheckState())
	{
		if(IsRectEmpty(&DIBClipRct))
		{
			//There's no selection
			IVClipRct=GetImageRect();
			//Leave DIBCliptRct empty => remove the clipping region on the DibSection
		}
		else
		{
			IVClipRct=DIBClipRct;
			DIB2IVWndPoint((POINT *)&IVClipRct);
			DIB2IVWndPoint(((POINT *)&IVClipRct)+1);
		}
		//Create the regions and select them in the DCs
		if(!IsRectEmpty(&IVClipRct))
		{
			IVClipRgn=CreateRectRgnIndirect(&IVClipRct);
			SelectClipRgn(IVWDC,IVClipRgn);
			DeleteObject(IVClipRgn);
		}
		else
			SelectClipRgn(IVWDC,NULL);
		if(!IsRectEmpty(&DIBClipRct))
		{
            DIBClipRgn=CreateRectRgnIndirect(&DIBClipRct);
			SelectClipRgn(dsUpdater->GetDibSection()->GetCompDC(),DIBClipRgn);
			DeleteObject(DIBClipRgn);
		}
		else
			SelectClipRgn(dsUpdater->GetDibSection()->GetCompDC(),NULL);
	}
	else
		SelectClipRgn(IVWDC,NULL);
}
//Returns the rectangle of IVWnd painted with the image
RECT GetImageRect()
{
	RECT imgRect={0};
	GetImageRectPtr(&imgRect);
	return imgRect;
}
//Same as GetImageRect, but returns the info via a reference parameter
void GetImageRectPtr(RECT * imgRect)
{
	if(dsUpdater!=NULL && dsUpdater->CheckState())
	{
		POINT shift=GetShift();
		float zoom=GetZoom();
		GetClientRect(hIVWindow,imgRect);
		imgRect->right=min(imgRect->right,(long)(dsUpdater->GetDibSection()->GetDibWidth()*zoom/100.0));
		imgRect->bottom=min(imgRect->bottom,(long)(dsUpdater->GetDibSection()->GetDibHeight()*zoom/100.0));
		if(shift.x<0)
		{
			imgRect->left-=shift.x;
			imgRect->right-=shift.x;
		}
		if(shift.y<0)
		{
			imgRect->top-=shift.y;
			imgRect->bottom-=shift.y;
		}
	}
}
//Returns the currently selected RECT
RECT GetSelectedRect()
{
	static RECT selRect={0,0};
	static unsigned int lastMsgNumber=0;
	if(lastMsgNumber==0|| lastMsgNumber!=MsgNumber) //We need to obtain a fresh value
	{
		SendMessage(hMainWindow,WM_GET_SELECTION,(WPARAM)&selRect,0);
		lastMsgNumber=MsgNumber;
	}
	//#define it if the WM_GET_SELECTION provides *window* coords instead of DIB coords
#ifdef OLD_WM_GET_SELECTION
	IVWnd2DIBPoint((POINT *)&selRect);
	IVWnd2DIBPoint(((POINT *)&selRect)+1);
#endif
	return selRect;
}
//Returns the current image shift from the upper-left corner of the IVW
POINT GetShift()
{
	static POINT shift;
	static bool centerImage;
	LRESULT lr;
	static unsigned int lastMsgNumber=0;
	if(dsUpdater==NULL || !dsUpdater->CheckState())
	{
		shift.x=0;
		shift.y=0;
	}
	else if(lastMsgNumber==0 || lastMsgNumber!=MsgNumber)
	{
		shift.x=INT_MIN;
		shift.y=INT_MIN;
		lr=SendMessage(hMainWindow,WM_GET_SCROLL_VALUES,(WPARAM)&shift,0);
		if(shift.x==INT_MIN && shift.y==INT_MIN)
		{
			shift.x=LOWORD(lr);
			shift.y=HIWORD(lr);
		}
		//Setup the centerImage setting
		centerImage = GetPrivateProfileInt(_T("Viewing"),_T("Centered"),1,iniSect->GetINIFile().c_str())!=0;
		if(centerImage) //Handle also this case with negative shift values
		{
			LPBITMAPINFO bi=(LPBITMAPINFO)dsUpdater->GetDibSection()->GetPackedDIB();
			RECT cwndRct;
			float zoom=GetZoom();
			GetClientRect(hIVWindow, &cwndRct);
			if(shift.x==0 && bi->bmiHeader.biWidth*zoom/100.0<cwndRct.right)
			{
				shift.x=round(-(cwndRct.right-(bi->bmiHeader.biWidth*zoom/100.0))/2.0);
			}
			if(shift.y==0 && bi->bmiHeader.biHeight*zoom/100.0<cwndRct.bottom)
			{
				shift.y=round(-(cwndRct.bottom-(bi->bmiHeader.biHeight*zoom/100.0))/2.0);
			}
		}
		lastMsgNumber=MsgNumber;
	}
	return shift;
}
//Returns the current image zoom (in %)
float GetZoom()
{
	static float zoom;
	static unsigned int lastMsgNumber=0;
	unsigned int tzoom;
	if(lastMsgNumber==0 || lastMsgNumber!=MsgNumber) //We need to obtain a fresh value
	{
		tzoom=(unsigned int)SendMessage(hMainWindow,WM_GET_ZOOM_VALUE,(WPARAM)&zoom,0);
		if(!BETWEEN(tzoom,zoom-1,zoom+1))
			zoom=(float)tzoom;
		lastMsgNumber=MsgNumber;
	}
	return zoom;
}
//Returns the state of the modifier keys
int GetModifierKeysState()
{
	//Get the modifier keys...
	int modifierKeys=0;
	modifierKeys|=(GetKeyState(VK_CONTROL)&0x8000)?MK_CONTROL:0;
	modifierKeys|=(GetKeyState(VK_MENU)& 0x8000)?MK_ALT:0;
	modifierKeys|=(GetKeyState(VK_SHIFT)& 0x8000)?MK_SHIFT:0;
	modifierKeys|=(GetKeyState(VK_LBUTTON)& 0x8000)?MK_LBUTTON:0;
	modifierKeys|=(GetKeyState(VK_MBUTTON)& 0x8000)?MK_MBUTTON:0;
	modifierKeys|=(GetKeyState(VK_RBUTTON)& 0x8000)?MK_RBUTTON:0;
	return modifierKeys;
}
//Displays an error message box (message specified by ID)
int ErrMsgBox(HWND hwndOwner, int messageID, DWORD * lastError, int flags)
{
	return ErrMsgBox(hwndOwner,langFile->GetString(messageID).c_str(),lastError, flags);
}
//Displays an error message box (message specified explicitly)
int ErrMsgBox(HWND hwndOwner, const TCHAR * message, DWORD * lastError, int flags)
{
	std::_tcostringstream os;
	std::_tcstring ts;
	//Eventually add the lastError
	if(lastError!=NULL)
	{
		os<<message<<std::endl<<"GetLastError: 0x"<<std::hex<<std::setw(8)<<std::setfill(_T('0'))<<*lastError<<" ("<<GetErrorDescription(*lastError).c_str()<<").";
		ts=os.str();
		message=ts.c_str();
	}
	return MessageBox(hwndOwner,message,langFile->GetString(IDS_ERR_CAPTION).c_str(),flags);
}
//Displays an error message box (message based on an exception)
int ErrMsgBox(HWND hwndOwner, exception & exception, const TCHAR * currentProcedureName, int flags)
{
	DWORD lastError=GetLastError(); //the following calls may overwrite it
	std::_tcostringstream os;
	os<<_T("Exception ")<<typeid(exception).name()<<_T(" caught in ")<<currentProcedureName<<_T(".")<<std::endl<<
		exception.what()<<std::endl<<
		_T("GetLastError: 0x")<<std::hex<<std::setw(8)<<std::setfill(_T('0'))<<lastError<<_T(" (")<<GetErrorDescription(lastError)<<_T(").")<<std::endl<<
		_T("IrfanPaint version: ")<<GetAboutMessage(false,false,false,true,true,false)<<_T(".")<<std::endl<<
		_T("Please send a report of this to irfanpaint@mitalia.net, providing as much information as you can (report exactly what this message says and what you were doing when this happened).");
	return ErrMsgBox(hwndOwner,os.str().c_str(),NULL,flags);
}
//Returns a description for the given Windows error code
std::_tcstring GetErrorDescription(DWORD errorCode, bool * success)
{
	TCHAR * description;
	std::_tcstring ret;
	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	(LPTSTR)&description, 0, NULL))
	{
		if(success!=NULL)
			*success=true;
		ret=description;
		LocalFree(description);
		std::_tcstring::iterator it;
		it = ret.end();
		if(*--it==_T('\n'))
			ret.erase(it);
		if(*--it==_T('\r'))
			ret.erase(it);
		if(*--it=='.')
			ret.erase(it);
	}
	else
	{
		if(success!=NULL)
			*success=false;
		ret=_T("no description available");
	}
	return ret;
}
//Simulates a mouse move
void SimMM(HWND hWnd, bool post, bool sendSetCursor, bool sendMouseMove)
{
	//Retrieve the cursor position
	POINT curPtPos;
	GetCursorPos(&curPtPos);
	if(sendSetCursor)
	{
		//Simulate a cursor move, so that the cursor and other things are eventually updated
		MsgSend(hWnd,WM_SETCURSOR,(WPARAM)hWnd,MAKELPARAM(SendMessage(hWnd,WM_NCHITTEST,0,MAKELPARAM(curPtPos.x,curPtPos.y)),WM_MOUSEMOVE),post);
		//                                                ^^^^^^Hit test (maybe the cursor is not in the client area)^^^^^^^
	}
	if(sendMouseMove)
	{
		//Translate cursor position to client coords
		ScreenToClient(hWnd,&curPtPos);
		//Synthetize a WM_MOUSEMOVE
		MsgSend(hWnd,WM_MOUSEMOVE,GetModifierKeysState(),MAKELPARAM(curPtPos.x,curPtPos.y),post);
	}
}
//Sends or posts a message
inline void MsgSend(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, bool post)
{
	if(post)
		PostMessage(hWnd,Msg,wParam,lParam);
	else
		SendMessage(hWnd,Msg,wParam,lParam);
}
//Converts a string made of dots and spaces to a style array; returns the number of elements of the array (you can pass NULL as array to obtain just the number of elements needed)
DWORD DotSpaceStr2StyleArray(const TCHAR * string, DWORD * array, bool corrected)
{
	const TCHAR * curChar;
	unsigned int count=0;
	unsigned int curPos=0;
	if(*string!=_T('.'))
		return 0;
	for(curChar=string;*curChar;curChar++)
	{
		if((*curChar!=_T(' '))&&(*curChar!=_T('.')))
			return 0;
		if((curChar==string)||(*(curChar-1)!=(*curChar)))
			count++;
	}
	if(count==0)
		return 0;
	if(array!=NULL)
	{
		memset(array,0,sizeof(DWORD)*count);
		for(curChar=string;*curChar;curChar++)
		{
			if((curChar!=string)&&(*(curChar-1)!=(*curChar)))
				curPos++;
			array[curPos]++;
		}
		if((count%2)&&corrected)
			array[count]=0;
	}
	return ((count%2)&&corrected)?(count+1):count;
}
//Converts a style array to a string made of dots and spaces
std::_tcstring StyleArray2DotSpaceStr(DWORD * array, int elemCount)
{
	if(elemCount==0)
		return 0;
	int count=0;
	std::_tcstring ret;
	for(int curPos=0;curPos<elemCount;curPos++)
	{
		count+=array[curPos];
		ret.resize(ret.size()+array[curPos],(curPos%2)?_T(' '):_T('.'));
	}
	if(count%2)
        ret.append(ret);
	return ret;
}

//Validates a change done by the user to a textbox that must contain unsigned integers; the new value, if aquired successfully, is in lastValue
bool ValidateUIntFieldChange(HWND parent, WORD ctrlID, UINT & lastValue, UINT minValue, UINT maxValue)
{
	UINT curVal;
	BOOL translated;
	curVal=GetDlgItemInt(parent,ctrlID,&translated,FALSE);
	if(translated && BETWEEN(curVal,minValue,maxValue))
	{
		lastValue=curVal;
		return true;
	}
	else
	{
		//Rollback
		SetDlgItemInt(parent,ctrlID,lastValue,FALSE);
		return false;
	}
}

//Replaces all the instances of search with replace in string; returns the number of substitutions done
unsigned int ReplaceString(std::_tcstring & string,const std::_tcstring & search,const std::_tcstring & replace)
{
	unsigned int ret=0;
	for(std::_tcstring::size_type pos=string.find(search);pos!=string.npos;ret++,pos=string.find(search,pos))
	{
		string.replace(pos,search.length(),replace);
		pos+=replace.length();
	}
	return ret;
}

//Makes sure that a window is entirely in the working area and eventually moves it
void MoveInWorkingArea(HWND hwnd)
{
	RECT wndRect,workingAreaRect;
	//Retrieve the bounds of the working area
	if(!SystemParametersInfo(SPI_GETWORKAREA,0,&workingAreaRect,0))
		return;
	//Retrieve the bounds of the window
	if(!GetWindowRect(hwnd,&wndRect))
		return;
	//Make sure that the window is in the working area and eventually correct the upper-left corner of the window (the only used in the SetWindowPos)
	if(wndRect.left<workingAreaRect.left)
		wndRect.left=workingAreaRect.left;
	else if(wndRect.right>workingAreaRect.right)
		wndRect.left=workingAreaRect.right-RECTWIDTH(wndRect);
	if(wndRect.top<workingAreaRect.top)
		wndRect.top=workingAreaRect.top;
	else if(wndRect.bottom>workingAreaRect.bottom)
		wndRect.top=workingAreaRect.bottom-RECTHEIGHT(wndRect);
	SetWindowPos(hwnd, 0, wndRect.left, wndRect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

}
/*
//Returns the size of a DIB pixel on the viewer window
SIZED GetDIBPixelSize()
{
	RECT imageRect;
	SIZED ret;
	int zoom=GetZoom();
	GetImageRectPtr(&imageRect);
	ret.cx=(imageRect.right-imageRect.left)/(double)round((imageRect.right-imageRect.left)/(zoom/100.0));
	ret.cy=(imageRect.bottom-imageRect.top)/(double)round((imageRect.bottom-imageRect.top)/(zoom/100.0));
	return ret;
}*/
