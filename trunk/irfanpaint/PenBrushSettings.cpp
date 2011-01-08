#include "stdafx.h"
#include "ColorUtils.h"
#include "Globals.h"
#include "PenBrushSettings.h"
#include "Utils.h"
#include "resource.h"
#include "IrfanPaint.h"
#include "LanguageFile.h"
#include "Tools.h"
#include "DibSectionUpdater.h"
//Shows the "Pen and brush settings" dialog
void ShowPenBrushSettings(HWND hParent, PBTabs tabToShow)
{
	PenBrushSettingsDlgParam pbsdp(fgColor,bgColor,tabToShow);
	//Show the dialog box
	if(DialogBoxParam(hCurInstance,MAKEINTRESOURCE(IDD_PENBRUSHSETTINGS),hParent,PBDialogProc,(LPARAM)&pbsdp)==IDABORT)
		CleanUp();
	//Refresh the color pane
	InvalidateRect(GetDlgItem(hToolBoxWindow,IDC_TB_COLORPAL),NULL,FALSE);
	//Update the updown
	SetPenWidth(fgColor.GetPenWidth());
	return;
}
//"Pen and brush settings" dialog procedure
INT_PTR CALLBACK PBDialogProc(HWND hwndDlg,
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
		//Everything is done with the message cracker macros
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hwndDlg,wParam,lParam,PBSettings_OnInitDialog);
		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwndDlg,wParam,lParam,PBSettings_OnCommand);
			return FALSE;
		case WM_NOTIFY:
			return HANDLE_WM_NOTIFY(hwndDlg,wParam,lParam,PBSettings_OnNotify);
		case WM_CLOSE:
			EndDialog(hwndDlg,IDCANCEL);
			return TRUE;
		case WM_DRAWITEM:
			return HANDLE_WM_DRAWITEM(hwndDlg,wParam,lParam,PBSettings_OnDrawItem);
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		EndDialog(hwndDlg,IDABORT);
	}
	return FALSE;
}

//WM_COMMAND
void PBSettings_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	(void) hwndCtl; //To avoid the warning
	//Get the PBSDP
	PenBrushSettingsDlgParam * pbsdp=(PenBrushSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	if(pbsdp==NULL)
		return;
	switch(id)
	{
	case IDOK: //OK button
		if(codeNotify==BN_CLICKED)
		{
			fgColor=pbsdp->workingFG;
			bgColor=pbsdp->workingBG;
		}
		//Note: no break
	case IDCANCEL: //Cancel button
		if(codeNotify==BN_CLICKED)
			EndDialog(hwnd,id);
		break; //case IDCANCEL: (and IDOK:)
	case IDC_PB_B_COLOR: //Click on the color pane: ask the user a new color and update the preview and the color pane
		if(codeNotify==BN_CLICKED)
		{
			pbsdp->GetCurrentCU().SetColor(GetColor(hwnd,pbsdp->GetCurrentCU().GetColor(),dsUpdater->GetDibSection()));
			InvalidateRect(GetDlgItem(hwnd,IDC_PB_B_COLOR),NULL,FALSE);
			UpdatePreviewPane(hwnd);
		}
		break; //case IDC_PB_B_COLOR:
	case IDC_PB_H_COLORP: //Click on the hatch color pane: ask the user a new color and update the preview and the hatch color pane
		if(codeNotify==BN_CLICKED)
		{
			pbsdp->workingBG.GetRefACUPs().bgColor=pbsdp->workingFG.GetRefACUPs().bgColor=GetColor(hwnd,pbsdp->workingBG.GetRefACUPs().bgColor,dsUpdater->GetDibSection());
			InvalidateRect(GetDlgItem(hwnd,IDC_PB_H_COLORP),NULL,FALSE);
			UpdatePreviewPane(hwnd);
		}
		break; //case IDC_PB_H_COLORP:
	case IDC_PB_H_COLOR: //Click on the transparent/opaque secondary hatch color radio buttons
	case IDC_PB_H_TRANSPARENT:
		if(codeNotify==BN_CLICKED)
		{
			//Update the ColorUtils
			CommonControls2ColorUtils(hwnd);
			//Update the preview pane
			UpdatePreviewPane(hwnd);
		}
		break; //case IDC_PB_H_COLOR: / IDC_PB_H_TRANSPARENT:
	case IDC_PB_PREVIEW: //Click on the preview pane
		if(codeNotify==BN_CLICKED)
		{
			pbsdp->swappedColors=!pbsdp->swappedColors;
			//Update the preview pane
			UpdatePreviewPane(hwnd);
		}
		break; //case IDC_PB_PREVIEW:
	case IDC_PB_TX_WIDTH:
		if(codeNotify==EN_CHANGE)
		{
			static UINT lastVal=pbsdp->workingFG.GetPenWidth();
			if(ValidateUIntFieldChange(hwnd,(WORD)id,lastVal,MINPENWIDTH,MAXPENWIDTH))
			{
				//Update the ColorUtils
				CommonControls2ColorUtils(hwnd);
				//Update the preview pane
				UpdatePreviewPane(hwnd);
			}
		}
	default: //Others
		//Brush radiobuttons: update the ColorUtils and the preview pane
		if(
			BETWEEN(id,IDC_PB_B_SOLID,IDC_PB_B_HDI) &&
			codeNotify == BN_CLICKED
			)
		{
			pbsdp->GetCurrentCU().SetBrushSettings(BrushFromCtrls(hwnd,pbsdp->GetCurrentCU().GetColor()));
			UpdatePreviewPane(hwnd);
			break;
		}
		//Pen radiobuttons: update the ColorUtils and the preview pane
		else if(
			BETWEEN(id,IDC_PB_P_SOLID,IDC_PB_P_CUSTOM) ||
			BETWEEN(id,IDC_PB_P_END_ROUND,IDC_PB_P_END_FLAT) ||
			BETWEEN(id,IDC_PB_P_LJ_ROUND,IDC_PB_P_LJ_MITER) ||
			id==IDC_PB_P_TX_CUSTOM && codeNotify == EN_CHANGE
			)
		{
			nEXTLOGPEN lp=PenFromCtrls(hwnd,pbsdp->GetCurrentCU().GetPenWidth());
			if(id == IDC_PB_P_CUSTOM)
			{
				EnableWindow(GetDlgItem(hwnd,IDC_PB_P_TX_CUSTOM),TRUE);
				ShowWindow(GetDlgItem(hwnd,IDC_PB_P_IS_CUSTOM),SW_SHOWNA);
			}
			else if(BETWEEN(id,IDC_PB_P_SOLID,IDC_PB_P_DASHDOTDOT))
			{
				EnableWindow(GetDlgItem(hwnd,IDC_PB_P_TX_CUSTOM),FALSE);
				ShowWindow(GetDlgItem(hwnd,IDC_PB_P_IS_CUSTOM),SW_HIDE);
			}
			pbsdp->GetCurrentCU().SetPenSettings(lp);
			if(((lp.dwPenStyle&PS_STYLE_MASK)==PS_USERSTYLE)&&lp.lpStyle!=NULL)
				delete[] lp.lpStyle;
			UpdatePreviewPane(hwnd);
			break;
		}
	}
}
//WM_INITDIALOG
BOOL PBSettings_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	(void) hwndFocus; //To avoid the warning
	//Associate the parameters to the window
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(__int3264)(LONG_PTR)lParam);
	//Get the PBSDP
	PenBrushSettingsDlgParam * pbsdp=(PenBrushSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	//Create the tabs
	TCITEM ti;
	//Foreground tab
	ti.pszText=const_cast<LPTSTR>(langFile->GetString(IDS_PB_TABS_FG).c_str());
	ti.mask=TCIF_TEXT;
	if((pbsdp->fgTabIndex=TabCtrl_InsertItem(GetDlgItem(hwnd,IDC_PB_TABS),0,&ti))==-1)
		throw std::runtime_error(ERROR_STD_PROLOG "TabCtrl_InsertItem returned -1 creating the \"Foreground\" tab.");
	//Background tab
	ti.pszText=const_cast<LPTSTR>(langFile->GetString(IDS_PB_TABS_BG).c_str());
	if((pbsdp->bgTabIndex=TabCtrl_InsertItem(GetDlgItem(hwnd,IDC_PB_TABS),1,&ti))==-1)
		throw std::runtime_error(ERROR_STD_PROLOG "TabCtrl_InsertItem returned -1 creating the \"Background\" tab.");
	//Select the right tab
	TabCtrl_SetCurSel(GetDlgItem(hwnd,IDC_PB_TABS),pbsdp->GetActiveTabIndex());
	//Send a fake notification
	NMHDR nmh;
	nmh.code=TCN_SELCHANGE;
	nmh.hwndFrom=GetDlgItem(hwnd,IDC_PB_TABS);
	nmh.idFrom=IDC_PB_TABS;
	FORWARD_WM_NOTIFY(hwnd,IDC_PB_TABS,&nmh,SendMessage);
	//Setup the updown control
	//Set the range of the width updown control
	SendDlgItemMessage(hwnd,IDC_PB_UD_WIDTH,UDM_SETRANGE,0,(LPARAM) MAKELONG((short) MAXPENWIDTH, (short) MINPENWIDTH));
	//Init some other controls
	ColorUtils2CommonControls(hwnd);
	//Subclass the textbox
	SetWindowLongPtr(GetDlgItem(hwnd,IDC_PB_P_TX_CUSTOM),GWLP_USERDATA,(__int3264)(LONG_PTR)SetWindowLongPtr(GetDlgItem(hwnd,IDC_PB_P_TX_CUSTOM),GWLP_WNDPROC,(__int3264)(LONG_PTR)SubclTxtProc));
	//Load the captions of the child windows (and of itself)
	langFile->InitializeDialogCaptions(hwnd, IDD_PENBRUSHSETTINGS);
	return TRUE;
}
//WM_NOTIFY
BOOL PBSettings_OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr)
{
	(void) idFrom; //To avoid the warning
	//Get the PBSDP
	PenBrushSettingsDlgParam * pbsdp=(PenBrushSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(pnmhdr->code)
	{
	case TCN_SELCHANGE: //Tab changed
		{
			//Set the new active tab index
			pbsdp->SetActiveTabIndex(TabCtrl_GetCurSel(pnmhdr->hwndFrom));
			//Update the controls
			CtrlsFromBrush(hwnd,pbsdp->GetCurrentCU().GetBrushSettings());
			CtrlsFromPen(hwnd,pbsdp->GetCurrentCU().GetPenSettings());
			InvalidateRect(GetDlgItem(hwnd,IDC_PB_B_COLOR),NULL,FALSE);
		} //End tab changed
		return TRUE;
	}
	return FALSE;
}
//WM_DRAWITEM
void PBSettings_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	//Get the PBSDP
	PenBrushSettingsDlgParam * pbsdp=(PenBrushSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(lpDrawItem->CtlID) //Controls switch
	{
	case IDC_PB_PREVIEW: //Preview pane
		{
			RECT fillRct=lpDrawItem->rcItem;
			HRGN fillRgn=CreateRectRgnIndirect(&fillRct);
			ArrowSettings tas=UITC->GetTool<UIArrowline>(IDC_TB_ARROWLINE)->GetArrowSettings();
			//Paint the background with a white brush
			FillRgn(lpDrawItem->hDC,fillRgn,(HBRUSH)GetStockObject(WHITE_BRUSH));
			//Select the pen and the brush and set the appropriate settigns
			BEGIN_SELOBJ(lpDrawItem->hDC,pbsdp->GetPreviewPaneFGCU().GetPen(),hDC_pen);
			BEGIN_SELOBJ(lpDrawItem->hDC,pbsdp->GetPreviewPaneBGCU().GetBrush(),hDC_brush);
			pbsdp->GetPreviewPaneFGCU().ApplyACUPs(lpDrawItem->hDC);
			//Draw a filled rectangle in the left part of the rectangle
			Rectangle(lpDrawItem->hDC,fillRct.left+5,fillRct.top+5,(fillRct.right-fillRct.left)/2-5,fillRct.bottom-5);
			//Draw a filled arrow in the top right part of the rectangle
			MoveToEx(lpDrawItem->hDC,(fillRct.right-fillRct.left)/2+5,fillRct.top+5,NULL);
			tas.drawSecondArrow=true;
			ArrowTo(lpDrawItem->hDC,fillRct.right-5,fillRct.bottom-25,&tas);
			//Draw a line in the bottom right part of the rectangle
			MoveToEx(lpDrawItem->hDC,(fillRct.right-fillRct.left)/2+5,fillRct.bottom-15,NULL);
			LineTo(lpDrawItem->hDC,fillRct.right-5,fillRct.bottom-5);
			//Deselect the pen and the brush
			END_SELOBJ(lpDrawItem->hDC,hDC_brush);
			END_SELOBJ(lpDrawItem->hDC,hDC_pen);
			DeleteRgn(fillRgn);
			break;
		} //End IDC_PB_PREVIEW
	case IDC_PB_B_COLOR: //Color panes
	case IDC_PB_H_COLORP:
		{
			HBRUSH hbr=CreateSolidBrush((lpDrawItem->CtlID==IDC_PB_B_COLOR?pbsdp->GetCurrentCU().GetColor():pbsdp->GetCurrentCU().GetACUPs().bgColor));
			FillRect(lpDrawItem->hDC,&lpDrawItem->rcItem,hbr);
			DeleteBrush(hbr);
			break;
		} //End IDC_PB_B_COLOR
	}
}

//Returns a LOGBRUSH from the settings of the "PenBrushSettings" dialog
LOGBRUSH BrushFromCtrls(HWND hPBS, COLORREF color)
{
	LOGBRUSH tlb={0};
	int tint=GetCheckedButton(hPBS,IDC_PB_B_HHO,IDC_PB_B_HDI);
	tlb.lbColor=color;
	if(tint==0)
		tlb.lbStyle=BS_SOLID;
	else
	{
		tlb.lbHatch=tint-IDC_PB_B_HHO;
		tlb.lbStyle=BS_HATCHED;
	}
	return tlb;
}
//Sets the "PenBrushSettings" dialog accordingly to the LOGBRUSH passed
bool CtrlsFromBrush(HWND hPBS, LOGBRUSH lb)
{
	if(lb.lbHatch>HS_DIAGCROSS)
		return false;
	switch(lb.lbStyle)
	{
	case BS_SOLID:
		CheckRadioButton(hPBS,IDC_PB_B_SOLID,IDC_PB_B_HDI,IDC_PB_B_SOLID);
		break;
	case BS_HATCHED:
		if(lb.lbHatch>HS_DIAGCROSS)
			return false;
		else
			CheckRadioButton(hPBS,IDC_PB_B_SOLID,IDC_PB_B_HDI,((int)lb.lbHatch)+IDC_PB_B_HHO);
		break;
	default:
		return false;
	}
	return true;
}
//Returns an nEXTLOGPEN from the settings of the "PenBrushSettings" dialog
nEXTLOGPEN PenFromCtrls(HWND hPBS, int width)
{
	nEXTLOGPEN lp={0};
	TCHAR buffer[1024];
	int tint=GetCheckedButton(hPBS,IDC_PB_P_SOLID,IDC_PB_P_CUSTOM);
	lp.dwPenStyle=PS_GEOMETRIC;
	if(tint==0)
		lp.dwPenStyle|=PS_SOLID;
	else if(tint==IDC_PB_P_CUSTOM)
	{
		GetDlgItemText(hPBS,IDC_PB_P_TX_CUSTOM,buffer,ARRSIZE(buffer));
		lp.dwStyleCount=DotSpaceStr2StyleArray(buffer,NULL,true);
		if(lp.dwStyleCount==0)
			lp.dwPenStyle|=PS_SOLID;
		else
		{
			lp.lpStyle=new DWORD[lp.dwStyleCount];
			DotSpaceStr2StyleArray(buffer,lp.lpStyle,true);
			lp.dwPenStyle|=PS_USERSTYLE;
		}
	}
	else
		lp.dwPenStyle|=tint-IDC_PB_P_SOLID;
	tint=GetCheckedButton(hPBS,IDC_PB_P_END_ROUND,IDC_PB_P_END_FLAT);
	if(tint==0)
		lp.dwPenStyle|=PS_ENDCAP_ROUND;
	else
		lp.dwPenStyle|=(tint-IDC_PB_P_END_ROUND)<<8;
	tint=GetCheckedButton(hPBS,IDC_PB_P_LJ_ROUND,IDC_PB_P_LJ_MITER);
	if(tint==0)
		lp.dwPenStyle|=PS_JOIN_ROUND;
	else
		lp.dwPenStyle|=(tint-IDC_PB_P_LJ_ROUND)<<12;
	lp.dwWidth=width;
	return lp;
}
//Sets the "PenBrushSettings" dialog accordingly to the nEXTLOGPEN passed
bool CtrlsFromPen(HWND hPBS, nEXTLOGPEN lp)
{
	std::_tcstring dotSpaceStr;
	if((lp.dwPenStyle&PS_STYLE_MASK)<=PS_DASHDOTDOT)
	{
		CheckRadioButton(hPBS,IDC_PB_P_SOLID,IDC_PB_P_CUSTOM,(lp.dwPenStyle&PS_STYLE_MASK)+IDC_PB_P_SOLID);
		SetDlgItemText(hPBS,IDC_PB_P_TX_CUSTOM,_T("."));
		EnableWindow(GetDlgItem(hPBS,IDC_PB_P_TX_CUSTOM),FALSE);
		ShowWindow(GetDlgItem(hPBS,IDC_PB_P_IS_CUSTOM),SW_HIDE);
	}
	else if((lp.dwPenStyle&PS_STYLE_MASK)==PS_USERSTYLE)
	{
		CheckRadioButton(hPBS,IDC_PB_P_SOLID,IDC_PB_P_CUSTOM,IDC_PB_P_CUSTOM);
		dotSpaceStr=StyleArray2DotSpaceStr(lp.lpStyle,lp.dwStyleCount);
		SetDlgItemText(hPBS,IDC_PB_P_TX_CUSTOM,dotSpaceStr.c_str());
		EnableWindow(GetDlgItem(hPBS,IDC_PB_P_TX_CUSTOM),TRUE);
		ShowWindow(GetDlgItem(hPBS,IDC_PB_P_IS_CUSTOM),SW_SHOWNA);
	}
	else
		return false;
	if((lp.dwPenStyle&PS_ENDCAP_MASK)<=PS_ENDCAP_FLAT)
		CheckRadioButton(hPBS,IDC_PB_P_END_ROUND,IDC_PB_P_END_FLAT,((lp.dwPenStyle&PS_ENDCAP_MASK)>>8)+IDC_PB_P_END_ROUND);
	else
		return false;
	if((lp.dwPenStyle&PS_JOIN_MASK)<=PS_JOIN_MITER)
		CheckRadioButton(hPBS,IDC_PB_P_LJ_ROUND,IDC_PB_P_LJ_MITER,((lp.dwPenStyle&PS_JOIN_MASK)>>12)+IDC_PB_P_LJ_ROUND);
	else
		return false;
	return true;
}
//Subclassed textbox window procedure
LRESULT CALLBACK SubclTxtProc(HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	int textlength;
	WNDPROC origProc=(WNDPROC)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(uMsg) //Message switch
	{
	case WM_CHAR: //Accept only '.', ' ' and VK_BACK and only under certain conditions
		textlength=GetWindowTextLength(hwnd);
		switch(wParam)
		{
		case '.':
		case ' ':
			{
				TCHAR buffer[1024];
				if(textlength<1023 && (wParam!=' '||textlength!=0) && (GetWindowText(hwnd,buffer,ARRSIZE(buffer)), DotSpaceStr2StyleArray(buffer,NULL,true)<16))
					break;
				else
				{
					MessageBeep(MB_ICONERROR);
					return 0;
				}
				break;
			}
		case VK_BACK:
			if(textlength==1)
			{
				MessageBeep(MB_ICONERROR);
				return 0;
			}
			break;
		default:
			MessageBeep(MB_ICONERROR);
			return 0;
		}
		break;
	case WM_PASTE:
		return 0;
	}
	//Pass all the other message to the original WndProc
	return CallWindowProc(origProc,hwnd,uMsg,wParam,lParam);
}
//Update the controls common to all the tabs (settings shared between FG & BG ColorUtils)
void ColorUtils2CommonControls(HWND hPBS)
{
	//Get the PBSDP
	PenBrushSettingsDlgParam * pbsdp=(PenBrushSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hPBS,GWLP_USERDATA);
	//Eventually make order
	pbsdp->workingFG.GetRefACUPs()=pbsdp->workingBG.GetRefACUPs();
	pbsdp->workingBG.SetPenWidth(pbsdp->workingFG.GetPenWidth());
	//Transparent/opaque BG
	CheckRadioButton(hPBS,IDC_PB_H_TRANSPARENT,IDC_PB_H_COLOR,pbsdp->workingBG.GetRefACUPs().transparentBG?IDC_PB_H_TRANSPARENT:IDC_PB_H_COLOR);
	//Pen width
	SetDlgItemInt(hPBS,IDC_PB_TX_WIDTH,pbsdp->workingFG.GetPenWidth(),FALSE);
}
//Update the ColorUtils with the user-selected common settings
void CommonControls2ColorUtils(HWND hPBS)
{
	//Get the PBSDP
	PenBrushSettingsDlgParam * pbsdp=(PenBrushSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hPBS,GWLP_USERDATA);
	//Transparent/opaque BG
	pbsdp->workingFG.GetRefACUPs().transparentBG=(GetCheckedButton(hPBS,IDC_PB_H_TRANSPARENT,IDC_PB_H_COLOR)==IDC_PB_H_TRANSPARENT);
	//Pen width
	BOOL translated;
	UINT retValue=GetDlgItemInt(hPBS,IDC_PB_TX_WIDTH,&translated,FALSE);
	if(!translated)
		retValue=1;
	pbsdp->workingFG.SetPenWidth((int)retValue);
	//Uniformity
	pbsdp->workingBG.GetRefACUPs()=pbsdp->workingFG.GetRefACUPs();
	pbsdp->workingBG.SetPenWidth(pbsdp->workingFG.GetPenWidth());
}
//Updates the preview pane
void UpdatePreviewPane(HWND hPBS)
{
	InvalidateRect(GetDlgItem(hPBS,IDC_PB_PREVIEW),NULL,FALSE);
};