#include "stdafx.h"
#include "resource.h"
#include "Globals.h"
#include "InsertText.h"
#include "Utils.h"
#include "PenBrushSettings.h"
#include "Tools.h"
#include "IrfanPaint.h"
#include "LanguageFile.h"
#include "INISection.h"
#include "DibSectionUpdater.h"
#include "Objects.h"
//Shows the insert text dialog; returns false if the user cancelled the dialog
bool ShowInsertTextDialog(HWND hParent, POINT insertPoint)
{
	static InsertTextDlgParams itdp={0};
	INT_PTR ret;
	itdp.insertPoint=insertPoint;
	itdp.renderingMode=AntialiasedText;
	LoadRichEdit();
	ret=DialogBoxParam(hCurInstance,MAKEINTRESOURCE(IDD_INSERTTEXT),hParent,InsertTextDlgProc,(LPARAM)&itdp);
	if(ret==IDABORT)
		CleanUp();
	return ret==IDOK;
}
//Insert text dialog procedure
INT_PTR CALLBACK InsertTextDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	try
	{
		//Message switch
		switch(uMsg)
		{
		//Everything is done with the message cracker macros
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hwndDlg,wParam,lParam,InsertText_OnInitDialog);
		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwndDlg,wParam,lParam,InsertText_OnCommand);
			return FALSE;
		case WM_NOTIFY:
			return HANDLE_WM_NOTIFY(hwndDlg,wParam,lParam,InsertText_OnNotify);
		case WM_CLOSE:
			EndDialog(hwndDlg,IDCANCEL);
			return TRUE;
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		EndDialog(hwndDlg,IDABORT);
	}
	return FALSE;
}

//Message handler for WM_NOTIFY
BOOL InsertText_OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr)
{
	switch(pnmhdr->code)
	{
	case EN_SELCHANGE:
		if(idFrom==IDC_IT_RE_TEXT) //The selection of the richedit control changed
			UpdateCtrlsState(hwnd);
		break;
	}
	return FALSE;
}

//Message handler for WM_COMMAND
void InsertText_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	InsertTextDlgParams * itdp=(InsertTextDlgParams *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(id) //Control ID switch
	{
	//Usual cases for buttons
	case IDOK: //OK/Cancel buttons
		if(codeNotify==BN_CLICKED)
		{
//#define INCLUDE_OBJECTS
#ifdef INCLUDE_OBJECTS
			HDC dc=CreateEnhMetaFile(NULL,NULL,NULL,NULL);
			//Apply the ACUPs
			fgColor.ApplyACUPs(dc);
			//Render the text
			POINT pt={0,0};
			RenderRichEditText(GetDlgItem(hwnd,IDC_IT_RE_TEXT),dc,pt,itdp->renderingMode);
			HENHMETAFILE mf=CloseEnhMetaFile(dc);
			Objects::MetafileObject * mo=new Objects::MetafileObject(itdp->insertPoint,mf);
			Objects::RenderedObject * ro=new Objects::RenderedObject((Objects::Object *)mo);
			objects.GetObjects().push_front((Objects::Object *)ro);
#else
			//Set a new undo
			UIBaseTool::SetNewUndo();
			//Setup the clipping region
			SetupClippingRegion();
			//Apply the ACUPs
			fgColor.ApplyACUPs(dsUpdater->GetDibSection()->GetCompDC());
			//Render the text
			RenderRichEditText(GetDlgItem(hwnd,IDC_IT_RE_TEXT),dsUpdater->GetDibSection()->GetCompDC(),itdp->insertPoint,itdp->renderingMode);
#endif
			//Save the text formatting
			//Select the first character (example for format/paragraph settings)
			CHARRANGE cr;
			cr.cpMin=0;
			cr.cpMax=1;
			SendDlgItemMessage(hwnd,IDC_IT_RE_TEXT,EM_EXSETSEL,0,(LPARAM)&cr);
			//Save the settings
			try
			{
				SaveITDlgSettings(hwnd,*iniSect);
			}
			catch(exception)
			{
				if(!ignoreINIWriteErrors)
				{
					DWORD lastError=GetLastError();
					ErrMsgBox(hwnd,IDS_ERR_SAVESETTINGSFAIL,&lastError);
				}
			}
			//Save the text (in RTF) in itdp->rtfText for future use
			itdp->rtfText.clear();
			EDITSTREAM es={0};
			es.dwCookie=(DWORD_PTR)itdp;
			es.pfnCallback=InsertText_InEditStreamCallback;
			SendDlgItemMessage(hwnd,IDC_IT_RE_TEXT,EM_STREAMOUT,SF_RTFNOOBJS|SFF_PLAINRTF,(LPARAM)&es);
		}
		else
			break;
		//Note: if(codeNotify==BN_CLICKED) there's no break
	case IDCANCEL:
		if(codeNotify==BN_CLICKED)
			EndDialog(hwnd,id);
		break;
	case IDC_IT_BN_PREVIEW: //Preview button
		//TODO: make this better
		if(codeNotify==BN_PUSHED)
		{
			//Save the DC
			int savedDC=SaveDC(IVWDC);
			RECT wndExt=GetImageRect();
			//Refresh the IV window
			InvalidateRect(hIVWindow,NULL,FALSE);
			UpdateWindow(hIVWindow);
			//Setup the clipping region
			SetupClippingRegion();
			//Set up the mappings
			SetMapMode(IVWDC,MM_ANISOTROPIC);
			SetViewportOrgEx(IVWDC,wndExt.left,wndExt.top,NULL);
			SetViewportExtEx(IVWDC,wndExt.right-wndExt.left,wndExt.bottom-wndExt.top,NULL);
			IVWnd2DIBPoint((POINT *)&wndExt);
			IVWnd2DIBPoint(((POINT *)&wndExt)+1);
			SetWindowOrgEx(IVWDC,wndExt.left,wndExt.top,NULL);
			SetWindowExtEx(IVWDC,RECTWIDTH(wndExt),RECTHEIGHT(wndExt),NULL);
			//Apply the ACUPs
			fgColor.ApplyACUPs(IVWDC);
			//Render the text
			RenderRichEditText(GetDlgItem(hwnd,IDC_IT_RE_TEXT),IVWDC,itdp->insertPoint,itdp->renderingMode);
			//Restore the DC
			RestoreDC(IVWDC,savedDC);
		}
		else if(codeNotify==BN_UNPUSHED)
		{
			//Refresh the IV window
			InvalidateRect(hIVWindow,NULL,FALSE);
			UpdateWindow(hIVWindow);
		}
		break; //case IDC_IT_BN_PREVIEW:
	case IDC_IT_CH_BOLD: //Format buttons
	case IDC_IT_CH_ITALIC:
	case IDC_IT_CH_UNDERLINE:
	case IDC_IT_CH_STRIKETHROUGH:
		if(codeNotify==BN_CLICKED)
		{
			//Update the selection
			UpdateSelFormat(hwnd,MaskFromBtnID(id));
			//Set the focus on the richedit
			SetFocus(GetDlgItem(hwnd,IDC_IT_RE_TEXT));
		}
		break; //Format buttons
	case IDC_IT_RB_ALLEFT: //Align buttons
	case IDC_IT_RB_ALCENTER:
	case IDC_IT_RB_ALRIGHT:
		if(codeNotify==BN_CLICKED)
		{
			//Update all the text
			UpdateAlign(hwnd);
			//Set the focus on the richedit
			SetFocus(GetDlgItem(hwnd,IDC_IT_RE_TEXT));
		}
		break;
	case IDC_IT_RB_RA_EMPTYPATH: //"Render as" buttons
	case IDC_IT_RB_RA_FILLEDPATH:
	case IDC_IT_RB_RA_TEXT:
	case IDC_IT_RB_RA_AATEXT:
		if(codeNotify==BN_CLICKED)
		{
			itdp->renderingMode=CtrlID2TRM((WORD)id);
			UpdateCtrlsState(hwnd);
		}
		break; //"Render as" buttons
	case IDC_IT_BN_SELCOLOR: //Select color button
		if(itdp->renderingMode&PathBit) //The text must be rendered as path
		{
			ShowPenBrushSettings(hwnd); //Show the P&B settings
		}
		else //The text must be rendered as text
		{
			//Get the selected text color
			CHARFORMAT cf={0};
			HWND hre=GetDlgItem(hwnd,IDC_IT_RE_TEXT);
			cf.cbSize=sizeof(cf);
			RichEdit_GetCharFormat(hre,SCF_SELECTION,&cf);
			//Ask a new color to the user
			cf.crTextColor=GetColor(hwnd,cf.crTextColor,dsUpdater->GetDibSection());
			//Set the new color
			//Avoid changing other attributes that may be different in the selection
			cf.dwMask=CFM_COLOR;
			//Disable CFE_AUTOCOLOR (that is considered with the mask CFM_COLOR)
			cf.dwEffects=0;
			//Apply the settings
			RichEdit_SetCharFormat(hre,SCF_SELECTION,&cf);
			//Move the focus on the richedit
			SetFocus(hre);
		}
		break; //case IDC_IT_BN_SELCOLOR:
	case IDC_IT_CO_FONTSIZE: //Font size combobox
		switch(codeNotify) //Notify code switch
		{
		case CBN_SELENDOK: //An item in the combobox has been selected
			//Update the text (we need this to happen now)
			ComboBox_SetCurSel(hwndCtl,ComboBox_GetCurSel(hwndCtl));
			//Move the focus on the richedit; this way the focus doesn't stay on the
			//combobox and the edit is applied in the CBN_KILLFOCUS handler (see below)
			SetFocus(GetDlgItem(hwnd,IDC_IT_RE_TEXT));
			break; //case CBN_SELENDOK:
		case CBN_KILLFOCUS: //The combobox has lost the focus
			{
				//Check if the user inserted a valid number
				BOOL translated;
				GetDlgItemInt(hwnd,id,&translated,FALSE);
				if(!translated)
				{
					//No, he didn't; restore the correct settings
					TCHAR buffer[TEXTBOXBUFFERSLENGTH];
					GetDlgItemText(hwnd,id,buffer,TEXTBOXBUFFERSLENGTH);
					if(*buffer)
					{
						std::_tcstring msg=langFile->GetString(IDS_ERR_NOTANUMBER);
						ReplaceString(msg,_T("%what%"),buffer);
						ErrMsgBox(hwnd,msg.c_str());
					}
					UpdateCtrlsState(hwnd);
					SetFocus(hwndCtl);
				}
				else
				{
					//Yes, he did; update the text formatting
					UpdateSelFormat(hwnd,CFM_SIZE);
				}
			}
			break; //case CBN_KILLFOCUS:
		} //switch(codeNotify)
		break; //case IDC_IT_CO_FONTSIZE:
	case IDC_IT_CO_FONTNAME:
		switch(codeNotify) //Notify code switch
		{
		case CBN_SELENDOK: //An item in the combobox has been selected
			//Update the text (we need this to happen now)
			ComboBox_SetCurSel(hwndCtl,ComboBox_GetCurSel(hwndCtl));
			//Move the focus on the richedit; this way the focus doesn't stay on the
			//combobox and the edit is applied in the CBN_KILLFOCUS handler (see below)
			SetFocus(GetDlgItem(hwnd,IDC_IT_RE_TEXT));
			break; //case CBN_SELENDOK:
		case CBN_KILLFOCUS:
			{
				TCHAR buffer[LF_FACESIZE];
				//Get the inserted text
				GetWindowText(hwndCtl,buffer,ARRSIZE(buffer));
				//If it is the beginning of an existing item, select that item
				if(ComboBox_SelectString(hwndCtl,0,buffer)==CB_ERR)
				{
					//Otherwise set it back to the font of the selection
					UpdateCtrlsState(hwnd);
				}
				else
				{
					//Now a valid item is selected; update the font of the selection
					UpdateSelFormat(hwnd,CFM_FACE);
				}
			}
			break; //case CBN_KILLFOCUS;
		} //switch(caseNotify)
		break; //case IDC_IT_CO_FONTNAME:
	} //switch(id)
}

#pragma warning (push)
#pragma warning (disable:4100)
//Message handler for WM_INITDIALOG
BOOL InsertText_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	//Handle of the richedit
	HWND hre=GetDlgItem(hwnd,IDC_IT_RE_TEXT);
	//Handle of the font names combo
	HWND hfn=GetDlgItem(hwnd,IDC_IT_CO_FONTNAME);
	//Handle of the font sizes combo
	HWND hfs=GetDlgItem(hwnd,IDC_IT_CO_FONTSIZE);
	//InsertText dialog parameters
	InsertTextDlgParams * itdp=(InsertTextDlgParams *)lParam;
	//Associate the parameters to the window
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(__int3264)(LONG_PTR)lParam);
	//Subclass the preview button
	NotifierBtn_Subclass(GetDlgItem(hwnd,IDC_IT_BN_PREVIEW));
	//Load the captions of the child windows (and of itself)
	langFile->InitializeDialogCaptions(hwnd, IDD_INSERTTEXT);
	//Set the icons of the buttons
	for(int buttonID=IDC_IT_CH_BOLD;buttonID<=IDC_IT_BN_SELCOLOR;buttonID++)
		Button_SetImage(GetDlgItem(hwnd,buttonID),IMAGE_ICON,(HICON)LoadImage(hCurInstance,MAKEINTRESOURCE(buttonID),IMAGE_ICON,0,0,LR_SHARED));
	//Add fonts to the combobox
	LOGFONT lf={0};
	lf.lfCharSet=DEFAULT_CHARSET;
	lf.lfFaceName[0]=_T('\0'); //actually nonnecessary
	//Avoid flickering
	SetWindowRedraw(hfn,FALSE);
	//Enumerate the fonts
	EnumFontFamiliesEx(dsUpdater->GetDibSection()->GetCompDC(),&lf,(FONTENUMPROC)&InsertText_EnumFontFamExProc,(LPARAM)hfn,0);
	//Reenable redrawing
	SetWindowRedraw(hfn,TRUE);
	//Enable the extended UI
	ComboBox_SetExtendedUI(hfn,TRUE);
	//Add font sizes
	//Font sizes array
	int fontSizes[]={8,9,10,12,14,16,18,20,22,24,26,28,36,48,72};
	TCHAR buffer[4]; //Melius abondare quam deficere
	//Avoid flickering
	SetWindowRedraw(hfs,FALSE);
	for(int counter=0;counter<ARRSIZE(fontSizes);counter++)
		ComboBox_AddString(hfs,_itot(fontSizes[counter],buffer,10));
	//Reenable redrawing
	SetWindowRedraw(hfs,TRUE);
	//Enable the extended UI
	ComboBox_SetExtendedUI(hfs,TRUE);
	//Tell to the richedit which events we want to recieve
	RichEdit_SetEventMask(hre,ENM_SELCHANGE);
	//Setup the dialog settings
    LoadITDlgSettings(hwnd, *iniSect);
	//Eventually load the last text
	if(itdp->rtfText.length()>0)
	{
		SETTEXTEX ste={0};
		CHARRANGE cr={0};
		cr.cpMin=0;
		cr.cpMax=-1;
		ste.codepage=CP_ACP; //Even if it's ignored, since we are setting RTF text
		ste.flags=ST_DEFAULT;
		//Set the text
		RichEdit_SetTextEx(hre,&ste,itdp->rtfText.c_str());
		//Select all the text
		RichEdit_ExSetSel(hre,&cr);
	}
	//Update controls state
	UpdateCtrlsState(hwnd);
	//Move the window away from the insertion point
	//Get the position of the insertion point on the screen
	POINT insertPoint=itdp->insertPoint;
	DIB2IVWndPoint(&insertPoint);
	ClientToScreen(hIVWindow,&insertPoint);
	//Calculate the "forbidden area"
	RECT forbiddenRect;
	forbiddenRect.left=insertPoint.x-200;
	forbiddenRect.right=insertPoint.x+200;
	forbiddenRect.top=insertPoint.y-60;
	forbiddenRect.bottom=insertPoint.y+40;
	//Get our position
	RECT wndRect;
	GetWindowRect(hwnd,&wndRect);
	RECT dummy;
	//Determine if we are in the "forbidden area"
	if(IntersectRect(&dummy,&wndRect,&forbiddenRect))
	{
		//We are in the forbidden area; move away
		//Get the desktop window
		RECT desktopRect;
		GetWindowRect(GetDesktopWindow(),&desktopRect);
		SIZE wndSize={RECTWIDTH(wndRect),RECTHEIGHT(wndRect)};
		if(desktopRect.bottom>=forbiddenRect.bottom+wndSize.cy) //1st try: move down
		{
			wndRect.top=forbiddenRect.bottom;
			wndRect.bottom=forbiddenRect.bottom+wndSize.cy;
		}
		else if(desktopRect.top<=forbiddenRect.top-wndSize.cy) //2nd try: move up
		{
			wndRect.top=forbiddenRect.top-wndSize.cy;
			wndRect.bottom=forbiddenRect.top;
		}
		else if(desktopRect.right>=forbiddenRect.right+wndSize.cx) //3rd try: move right
		{
			wndRect.left=forbiddenRect.right;
			wndRect.right=forbiddenRect.right+wndSize.cx;
		}
		else if(desktopRect.left>=forbiddenRect.left-wndSize.cy) //4th try: move left
		{
			wndRect.left=forbiddenRect.left-wndSize.cy;
			wndRect.right=forbiddenRect.left;
		}
		//Otherwise there's nothing to do
		//Move the window
		MoveWindow(hwnd,wndRect.left,wndRect.top,wndSize.cx,wndSize.cy,FALSE);
	}
	//Give the focus to the richedit
	SetFocus(hre);
	return FALSE;
}
#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable:4100)
//Callback of EnumFontFamiliesEx
int CALLBACK InsertText_EnumFontFamExProc(ENUMLOGFONTEX *lpelfe,NEWTEXTMETRICEX *lpntme,DWORD FontType,LPARAM lParam)
{
	//If it's a TrueType font add it to the combobox
	if((FontType&TRUETYPE_FONTTYPE)&& ComboBox_FindStringExact((HWND)lParam,0,lpelfe->elfLogFont.lfFaceName)==CB_ERR)
	{
		ComboBox_AddString((HWND)lParam,lpelfe->elfLogFont.lfFaceName);
	}
	return TRUE;
}
#pragma warning (pop)
//Update the state of the controls from the current selection
void UpdateCtrlsState(HWND hwnd)
{
	HWND hre=GetDlgItem(hwnd,IDC_IT_RE_TEXT);
	InsertTextDlgParams * itdp=(InsertTextDlgParams *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	CHARFORMAT cf={0};
	PARAFORMAT pf={0};
	int alignBtn=IDC_IT_RB_ALLEFT;
	int renderAsBtn=-1;
	cf.cbSize=sizeof(cf);
	pf.cbSize=sizeof(pf);
	//Bold, italic, etc
	RichEdit_GetCharFormat(hre,SCF_SELECTION,&cf);
	CheckDlgButton(hwnd,IDC_IT_CH_BOLD,(cf.dwEffects&CFE_BOLD)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hwnd,IDC_IT_CH_ITALIC,(cf.dwEffects&CFE_ITALIC)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hwnd,IDC_IT_CH_UNDERLINE,(cf.dwEffects&CFE_UNDERLINE)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hwnd,IDC_IT_CH_STRIKETHROUGH,(cf.dwEffects&CFE_STRIKEOUT)?BST_CHECKED:BST_UNCHECKED);
	//Align
	RichEdit_GetParaFormat(hre,&pf);
	switch(pf.wAlignment)
	{
	case PFA_LEFT:
		alignBtn=IDC_IT_RB_ALLEFT;
		break;
	case PFA_RIGHT:
		alignBtn=IDC_IT_RB_ALRIGHT;
		break;
	case PFA_CENTER:
		alignBtn=IDC_IT_RB_ALCENTER;
		break;
	}
	CheckRadioButton(hwnd,IDC_IT_RB_ALLEFT,IDC_IT_RB_ALRIGHT,alignBtn);
	//Size
	SetDlgItemInt(hwnd,IDC_IT_CO_FONTSIZE,(UINT)cf.yHeight/20,FALSE);
	//Font face
	SetWindowText(GetDlgItem(hwnd,IDC_IT_CO_FONTNAME),cf.szFaceName);
	//Rendering mode
	renderAsBtn=TRM2CtrlID(itdp->renderingMode);
	if(renderAsBtn==0)
		renderAsBtn=IDC_IT_RB_RA_FILLEDPATH;
	CheckRadioButton(hwnd,IDC_IT_RB_RA_EMPTYPATH,IDC_IT_RB_RA_AATEXT,renderAsBtn);
	Button_SetImage(GetDlgItem(hwnd,IDC_IT_BN_SELCOLOR),IMAGE_ICON,(HICON)LoadImage(hCurInstance,MAKEINTRESOURCE((itdp->renderingMode&PathBit)?IDI_TB_PBSETTINGS:IDI_IT_SELCOLOR),IMAGE_ICON,0,0,LR_SHARED));
}

//Sets the format of the selection from the state of the controls
void UpdateSelFormat(HWND hwnd, DWORD mask)
{
	CHARFORMAT cf={0};
	//Bold, italic, etc
	cf.cbSize=sizeof(cf);
	cf.dwMask=mask;
	if(IsDlgButtonChecked(hwnd,IDC_IT_CH_BOLD))
		cf.dwEffects|=CFE_BOLD;
	if(IsDlgButtonChecked(hwnd,IDC_IT_CH_ITALIC))
		cf.dwEffects|=CFE_ITALIC;
	if(IsDlgButtonChecked(hwnd,IDC_IT_CH_UNDERLINE))
		cf.dwEffects|=CFE_UNDERLINE;
	if(IsDlgButtonChecked(hwnd,IDC_IT_CH_STRIKETHROUGH))
		cf.dwEffects|=CFE_STRIKEOUT;
	//Size
	BOOL translated;
	UINT size;
    size=GetDlgItemInt(hwnd,IDC_IT_CO_FONTSIZE,&translated,FALSE);
	if(translated)
	{
		cf.dwMask|=CFM_SIZE;
		cf.yHeight=size*20;
	}
	//Font face
	GetWindowText(GetDlgItem(hwnd,IDC_IT_CO_FONTNAME),cf.szFaceName,ARRSIZE(cf.szFaceName));
	RichEdit_SetCharFormat(GetDlgItem(hwnd,IDC_IT_RE_TEXT),SCF_SELECTION,&cf);
}

//Sets the align of the text from the state of the controls
void UpdateAlign(HWND hwnd)
{
	PARAFORMAT pf={0};
	int id=GetCheckedButton(hwnd,IDC_IT_RB_ALLEFT,IDC_IT_RB_ALRIGHT);
	HWND hre=GetDlgItem(hwnd,IDC_IT_RE_TEXT);
	CHARRANGE lastSel,allSel;
	pf.cbSize=sizeof(pf);
	pf.dwMask=PFM_ALIGNMENT;
	switch(id)
	{
	case IDC_IT_RB_ALLEFT:
		pf.wAlignment=PFA_LEFT;
		break;
	case IDC_IT_RB_ALCENTER:
		pf.wAlignment=PFA_CENTER;
		break;
	case IDC_IT_RB_ALRIGHT:
		pf.wAlignment=PFA_RIGHT;
		break;
	default:
		return;
	}
	RichEdit_ExGetSel(hre,&lastSel);
	allSel.cpMin=0;
	allSel.cpMax=-1;
	RichEdit_ExSetSel(hre,&allSel);
	RichEdit_SetParaFormat(hre,&pf);
	RichEdit_ExSetSel(hre,&lastSel);
}
//Returns a mask for UpdateSelFormat from the pressed button ID
DWORD MaskFromBtnID(int BtnID)
{
	DWORD mask=0;
	switch(BtnID)
	{
	case IDC_IT_CH_BOLD:
		mask=CFM_BOLD;
		break;
	case IDC_IT_CH_ITALIC:
		mask=CFM_ITALIC;
		break;
	case IDC_IT_CH_UNDERLINE:
		mask=CFM_UNDERLINE;
		break;
	case IDC_IT_CH_STRIKETHROUGH:
		mask=CFM_STRIKEOUT;
		break;
	}
	return mask;

}
//Subclassed button window procedure
LRESULT CALLBACK NotifierBtnProc(HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	NotifierBtnParams * btnParams=(NotifierBtnParams *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	WNDPROC originalProc=btnParams->originalProc; //this because in WM_DESTROY btnParams is deallocated
	switch(uMsg)
	{
	case BM_SETSTATE:
		{
			if(btnParams->btnLastPushState != (wParam!=0))
			{
				PostMessage(GetParent(hwnd),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(hwnd),wParam?BN_PUSHED:BN_UNPUSHED),(LPARAM)hwnd);
				btnParams->btnLastPushState=(wParam!=0);
			}
			break;
		}
	case WM_NCDESTROY:
		//Cleanup
		NotifierBtn_Desubclass(hwnd);
		break;
	}
	return CallWindowProc(originalProc,hwnd,uMsg,wParam,lParam);
}
//Subclasses a button making it a "notifier button"
void NotifierBtn_Subclass(HWND btnHandle)
{
	NotifierBtnParams * btnParams=new NotifierBtnParams;
	btnParams->btnLastPushState=(Button_GetState(btnHandle)&BST_PUSHED)!=0;
	btnParams->originalProc=(WNDPROC)(LONG_PTR)GetWindowLongPtr(btnHandle,GWLP_WNDPROC);
	SetWindowLongPtr(btnHandle,GWLP_USERDATA,(__int3264)(LONG_PTR)btnParams);
	SetWindowLongPtr(btnHandle,GWLP_WNDPROC,(__int3264)(LONG_PTR)NotifierBtnProc);
}
//Desubclasses a "notifier button"
void NotifierBtn_Desubclass(HWND btnHandle)
{
	NotifierBtnParams * btnParams=(NotifierBtnParams *)(LONG_PTR)GetWindowLongPtr(btnHandle,GWLP_USERDATA);
	if(btnParams==NULL)
		throw std::logic_error(ERROR_STD_PROLOG "The button is not subclassed.");
	SetWindowLongPtr(btnHandle,GWLP_WNDPROC,(__int3264)(LONG_PTR)btnParams->originalProc);
	delete btnParams;
	SetWindowLongPtr(btnHandle,GWLP_USERDATA,(__int3264)(LONG_PTR)NULL);
}
//Renders the text of the given RichEdit on the given DC on the specified insertion point
void RenderRichEditText(HWND hRichEdit, HDC hDC, POINT insertPoint, TextRenderingMode renderingMode)
{
	typedef std::list<linePiece>		lineContainer;
	DWORD dcYRes=GetDeviceCaps(hDC, LOGPIXELSY);	//Vertical resolution of the DC
	int DCState=SaveDC(hDC);			//Save the DC state
	std::_tcstring accumulator;			//Accumulator
	CHARRANGE prevSelection;			//Selection of the user
	POINT prevScrollPosition;			//Scroll position
	lineContainer linePieces;			//Pieces of the line
	CHARRANGE cr={-1};					//Used to select a character
	CHARFORMAT curCf={0}, lastCf={0};	//Format of the current and previous character
	PARAFORMAT pf={0};					//Format of the paragraph
	HDC cdc=CreateCompatibleDC(hDC);	//DC compatible with hDC used for drawing simulations
	long txLen=GetWindowTextLength(hRichEdit)+1;	//Length of the text including the NUL
	TCHAR * curCharPtr;					//Pointer to the current character
	boost::scoped_array<TCHAR> text(NULL);	//Buffer that contains the text
	bool firstLine=true;				//Flag set if the first line hasn't been rendered yet
	//Disable redraw
	SetWindowRedraw(hRichEdit,FALSE);
	//Get the current scroll position and selection
	RichEdit_GetScrollPos(hRichEdit,&prevScrollPosition);
	RichEdit_ExGetSel(hRichEdit,&prevSelection);
	//Setup the DC
	//Set the transparent BK mode to avoid the background
	int prevBkMode=SetBkMode(hDC,TRANSPARENT);
	SelectPen(hDC,fgColor.GetPen());
	SelectBrush(hDC,bgColor.GetBrush());
	try
	{
		//Allocate the necessary space
		try
		{
			text.reset(new TCHAR[txLen]);
		}
		catch(std::bad_alloc)
		{
			//Write something more comprehensible
			throw std::bad_alloc(ERROR_STD_PROLOG "Cannot allocate memory to retrieve the content of the RichEdit.");
		}
		GetWindowText(hRichEdit,text.get(),txLen);
		//Init the structures
		curCf.cbSize=lastCf.cbSize=sizeof(curCf);
		pf.cbSize=sizeof(pf);
		SetTextAlign(hDC, TA_LEFT | TA_BASELINE | TA_NOUPDATECP);
		for(curCharPtr=text.get();curCharPtr<=(text.get()+txLen);curCharPtr++)
		{
			if(*curCharPtr==_T('\r'))
				continue; //Everything has been already done the previous iteration (with '\n')
			//Select the range
			cr.cpMin++;
			cr.cpMax=cr.cpMin+1;
			RichEdit_ExSetSel(hRichEdit,&cr);
			//Get the format
			RichEdit_GetCharFormat(hRichEdit,SCF_SELECTION,&curCf);
			//Fix the first run
			if(curCharPtr==text.get())
				lastCf=curCf; 
			//If the format is different or the line is finished move the string in the linePieces array with its font
			if(*curCharPtr==_T('\n') || *curCharPtr==0 || memcmp(&curCf,&lastCf,sizeof(curCf))!=0)
			{
				//Prepare the font
				linePiece lp;
				lp.font=CreateFont(-MulDiv(lastCf.yHeight,dcYRes,72*20),
					0,
					0,
					0,
					(lastCf.dwEffects & CFE_BOLD)?FW_BOLD:FW_NORMAL,
					(lastCf.dwEffects & CFE_ITALIC)?TRUE:FALSE,
					(lastCf.dwEffects & CFE_UNDERLINE)?TRUE:FALSE,
					(lastCf.dwEffects & CFE_STRIKEOUT)?TRUE:FALSE,
					lastCf.bCharSet,
					OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS,
					(renderingMode==AntialiasedText)?ANTIALIASED_QUALITY:NONANTIALIASED_QUALITY,
					lastCf.bPitchAndFamily,
					lastCf.szFaceName);
				if(lp.font==NULL)
					throw std::runtime_error(ERROR_STD_PROLOG "Cannot render text; CreateFont returned NULL.");
				lp.chars = accumulator;
				lp.color = lastCf.crTextColor;
				linePieces.push_back(lp);
				accumulator.clear();
				lastCf=curCf;
				//Get the paragraph info
				RichEdit_GetParaFormat(hRichEdit,&pf);
				if(pf.cTabCount==0)
				{
					//The default tab size that (WTF!) isn't stated anywere on MSDN; got it with manual measure
					pf.cTabCount=1;
					pf.rgxTabs[0]=48;
				}
			}
			//If the row is finished, render it
			if(*curCharPtr==_T('\n')||*curCharPtr==0)
			{
				SIZE totalLineSize={0};
				POINT renderStartPoint;
				DWORD partialLineSize;
				//Calculate the total line size
				//Simulate drawing using the compatible DC (GetTabbedTextExtend is not good for calculating the extent of several pieces of lines)
				for(lineContainer::iterator it=linePieces.begin();it!=linePieces.end();it++)
				{
					SelectFont(cdc,it->font);
					partialLineSize=TabbedTextOut(cdc,totalLineSize.cx,0,it->chars.c_str(),(int)it->chars.length(),pf.cTabCount,(INT*)pf.rgxTabs,0);
					totalLineSize.cx+=LOWORD(partialLineSize);
					totalLineSize.cy=max(totalLineSize.cy,HIWORD(partialLineSize));
				}
				//If it's not the first line, move to the next line
				if(!firstLine)
					insertPoint.y+=totalLineSize.cy;
				//Select a stock font to avoid deleting a still selected font
				SelectFont(cdc,GetStockFont(SYSTEM_FONT));
				if(totalLineSize.cx!=0)
				{
					//Calculate the line start point
					renderStartPoint=insertPoint;
					switch(pf.wAlignment)
					{
					case PFA_RIGHT:
						renderStartPoint.x-=totalLineSize.cx;
						break;
					case PFA_CENTER:
						renderStartPoint.x-=totalLineSize.cx/2;
						break;
						//Nothing to do for PFA_LEFT
					}
					//Move to the start point
					MoveToEx(hDC,renderStartPoint.x,renderStartPoint.y,NULL);
					//Horizontal offset
					int xoffset=0;
					//Render the text
					for(lineContainer::iterator it=linePieces.begin();it!=linePieces.end();it++)
					{
						LONG ret;
						//Select the font
						SelectFont(hDC,it->font);
						//Set the color
						SetTextColor(hDC,it->color);
						if(renderingMode&PathBit)
							BeginPath(hDC);
						//Render the text
						ret=TabbedTextOut(hDC,renderStartPoint.x+xoffset,renderStartPoint.y,it->chars.c_str(),(int)it->chars.length(),pf.cTabCount,(INT*)pf.rgxTabs,renderStartPoint.x);
						//If we must render as path close the path and select the fg pen and bg brush
						//The shorter is the path, the faster is the rendering; don't ask me why.
						if(renderingMode&PathBit)
						{
							EndPath(hDC);
							//Path rendering modes
							switch(renderingMode)
							{
							case EmptyPath:
								StrokePath(hDC);
								break;
							case FilledPath:
								//Set back to the original mode to have the right filling
								SetBkMode(hDC,prevBkMode);
								StrokeAndFillPath(hDC);
								//Reset to transparent
								SetBkMode(hDC,TRANSPARENT);
								break;
							}
						}
						//Update the position
						xoffset+=LOWORD(ret);
						//Select a stock font to avoid deleting a still selected font
						SelectFont(hDC,GetStockFont(SYSTEM_FONT));
						//Delete the font
						DeleteFont(it->font);
					}
				}
				//Clear the line pieces that we have just rendered
				linePieces.clear();
				//The first line has been surely rendered
				firstLine = false;
			}
			else
			{
				//Put the character in the accumulator
				accumulator.push_back(*curCharPtr);
			}
		}
	}
	catch(std::exception &ex)
	{
		ErrMsgBox(GetParent(hRichEdit),ex,_T(__FUNCSIG__));
		//Cleanup
		//Delete all the fonts
        for(lineContainer::iterator it=linePieces.begin();it!=linePieces.end();it++)
			DeleteFont(it->font);
		linePieces.clear(); //Actually useless, but this way we avoid to use the invalid font handles
	}
	//No need to do other things for other rendering modes
	//Final cleanup
	RestoreDC(hDC,DCState);
	DeleteDC(cdc);
	//Set the previous scroll position and selection
	RichEdit_ExSetSel(hRichEdit,&prevSelection);
	RichEdit_SetScrollPos(hRichEdit,&prevScrollPosition);
	//Reenable redraw
	SetWindowRedraw(hRichEdit,TRUE);
}
//Load the insert text dialog settings
void LoadITDlgSettings(HWND hwnd, INISection & IniSect)
{
	IniSect.Read();
	InsertTextDlgParams * itdp=(InsertTextDlgParams *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	CHARFORMAT cf={0};
	PARAFORMAT pf={0};
	cf.cbSize=sizeof(cf);
	pf.cbSize=sizeof(pf);
	UINT tInt;
	//Masks
	cf.dwMask=(CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT|CFM_FACE|CFM_COLOR|CFM_SIZE);
	pf.dwMask=PFM_ALIGNMENT;
	//Font name
	IniSect.GetKey(_T("ITFontName"),_T("Arial")).copy(cf.szFaceName,sizeof(cf.szFaceName));;
	//Font size
	cf.yHeight=IniSect.GetKey(_T("ITFontSize"),12)*20;
	//Color (use always the current fg color)
	cf.crTextColor=fgColor.GetColor();
	//Effects masked with the effects that we can handle (avoid having strange effects that we cannot render)
	cf.dwEffects=(IniSect.GetKey(_T("ITFontEffects"),0)&(CFE_BOLD|CFE_ITALIC|CFE_STRIKEOUT|CFE_UNDERLINE));
	//Paragraph alignment
	pf.wAlignment=(WORD)IniSect.GetKey(_T("ITParaAlign"),PFA_LEFT);
	if(pf.wAlignment!=PFA_LEFT && pf.wAlignment!=PFA_CENTER && pf.wAlignment != PFA_RIGHT)
		pf.wAlignment=PFA_LEFT;
	//Rendering mode
	tInt=IniSect.GetKey(_T("ITTextRenderingMode"),(int)AntialiasedText);
	switch(tInt) //I cannot assign an integer to an enum; this way I also discard invalid values
	{
	case EmptyPath:
		itdp->renderingMode=EmptyPath;
		break;
	case FilledPath:
		itdp->renderingMode=FilledPath;
		break;
	case Text:
		itdp->renderingMode=Text;
		break;
	case AntialiasedText:
		itdp->renderingMode=AntialiasedText;
		break;
	default:
		itdp->renderingMode=FilledPath;
		break;
	}
	//Apply
	SendDlgItemMessage(hwnd,IDC_IT_RE_TEXT,EM_SETCHARFORMAT,SCF_ALL,(LPARAM)&cf);
	SendDlgItemMessage(hwnd,IDC_IT_RE_TEXT,EM_SETPARAFORMAT,0,(LPARAM)&pf);
}

//Save the insert text dialog settings
void SaveITDlgSettings(HWND hwnd, INISection & IniSect)
{
	IniSect.BeginWrite();
	InsertTextDlgParams * itdp=(InsertTextDlgParams *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	CHARFORMAT cf={0};
	PARAFORMAT pf={0};
	cf.cbSize=sizeof(cf);
	pf.cbSize=sizeof(pf);
	//Obtain the char & para format
	SendDlgItemMessage(hwnd,IDC_IT_RE_TEXT,EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);
	SendDlgItemMessage(hwnd,IDC_IT_RE_TEXT,EM_GETPARAFORMAT,0,(LPARAM)&pf);
	//Font name
	IniSect.PutKey(_T("ITFontName"),cf.szFaceName);
	//Font size
	IniSect.PutKey(_T("ITFontSize"),cf.yHeight/20);
	//Effects
	IniSect.PutKey(_T("ITFontEffects"),cf.dwEffects&(CFE_BOLD|CFE_ITALIC|CFE_STRIKEOUT|CFE_UNDERLINE));
	//Paragraph alignment
	IniSect.PutKey(_T("ITParaAlign"),pf.wAlignment);
	//Rendering mode
	IniSect.PutKey(_T("ITTextRenderingMode"),itdp->renderingMode);
	IniSect.EndWrite();
}
//Converts a TextRenderingMode to the corresponding control ID
WORD TRM2CtrlID(TextRenderingMode trm)
{
	//Rendering mode
	switch(trm)
	{
	case EmptyPath:
		return IDC_IT_RB_RA_EMPTYPATH;
	case FilledPath:
		return IDC_IT_RB_RA_FILLEDPATH;
	case Text:
		return IDC_IT_RB_RA_TEXT;
	case AntialiasedText:
		return IDC_IT_RB_RA_AATEXT;
	default:
		return 0;
	}
}
//Converts a control ID to the corresponding TextRenderingMode
TextRenderingMode CtrlID2TRM(WORD ctrlID)
{
	
	switch(ctrlID)
	{
	case IDC_IT_RB_RA_EMPTYPATH:
		return EmptyPath;
	case IDC_IT_RB_RA_FILLEDPATH:
		return FilledPath;
	case IDC_IT_RB_RA_TEXT:
		return Text;
	case IDC_IT_RB_RA_AATEXT:
		return AntialiasedText;
	default:
		return None;
	}
}
//Callback used to read the data from the RichEdit
DWORD CALLBACK InsertText_InEditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	InsertTextDlgParams * itdp=(InsertTextDlgParams *)dwCookie;
	itdp->rtfText.append((char *)pbBuff,cb);
	*pcb=cb;
	return 0;
}