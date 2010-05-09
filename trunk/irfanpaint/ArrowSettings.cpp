#include "stdafx.h"			//Standard inclusions
#include "ArrowSettings.h"	//ArrowSettings.cpp header
#include "Globals.h"		//Global declarations
#include "resource.h"		//Resources #define
#include "Arrow.h"			//Arrow API
#include "Utils.h"			//Support functions
#include "LanguageFile.h"	//Language file
#include "Tools.h"
//"Arrow settings" dialog procedure
INT_PTR CALLBACK ASDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	switch(uMsg) //Message switch
	{
	case WM_INITDIALOG:
		{
			//Associate the parameters to the window
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(__int3264)(LONG_PTR)lParam);
			//Get the ASDP
			ArrowSettingsDlgParam * asdp=(ArrowSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			//Load the captions of the child windows (and of itself)
			langFile->InitializeDialogCaptions(hwndDlg, IDD_ARROWSETTINGS);
			//Set the range of the width updown control
			UpDown_SetRange(GetDlgItem(hwndDlg,IDC_AS_UD_WIDTH),MAXARROWWL,MINARROWWL);
			//Set the current width to the current value
			SetDlgItemInt(hwndDlg,IDC_AS_TX_WIDTH,(UINT)asdp->as->arrowBase,FALSE);
			//Set the range of the length updown control
			UpDown_SetRange(GetDlgItem(hwndDlg,IDC_AS_UD_LENGTH),MAXARROWWL,MINARROWWL);
			//Set the current length to the current value
			SetDlgItemInt(hwndDlg,IDC_AS_TX_LENGTH,(UINT)asdp->as->arrowLength,FALSE);
			//Set the "first arrow" check
			Button_SetCheck(GetDlgItem(hwndDlg,IDC_AS_CH_FIRST),asdp->as->drawFirstArrow?BST_CHECKED:BST_UNCHECKED);
			//Set the "second arrow" check
			Button_SetCheck(GetDlgItem(hwndDlg,IDC_AS_CH_SECOND),asdp->as->drawSecondArrow?BST_CHECKED:BST_UNCHECKED);
			//Set the "open head" check
			Button_SetCheck(GetDlgItem(hwndDlg,IDC_AS_CH_OPENHEAD),asdp->as->openArrowHead?BST_CHECKED:BST_UNCHECKED);
			return TRUE;
		}
		break;
	case WM_COMMAND: //Selections, clicks, etc.
		switch(LOWORD(wParam)) //Controls switch
		{
		case IDC_AS_TX_WIDTH:
		case IDC_AS_TX_LENGTH:
			if(HIWORD(wParam)==EN_CHANGE) //The text in the textboxes has changed: let's validate it!
			{
				static UINT widthLastVal=MINARROWWL;
				static UINT lengthLastVal=MINARROWWL;
				if(ValidateUIntFieldChange(hwndDlg,LOWORD(wParam),((LOWORD(wParam)==IDC_AS_TX_WIDTH)?widthLastVal:lengthLastVal),MINARROWWL,MAXARROWWL))
					InvalidateRect(GetDlgItem(hwndDlg,IDC_AS_PREVIEW),NULL,TRUE); //Repaint the preview frame
			}
			break;
		case IDC_AS_CH_FIRST: //The state of the checkboxes has changed
		case IDC_AS_CH_SECOND:
		case IDC_AS_CH_OPENHEAD:
			InvalidateRect(GetDlgItem(hwndDlg,IDC_AS_PREVIEW),NULL,TRUE); //Repaint the preview frame
			break;
		case IDOK: //The user pressed OK
			{
				//Get the ASDP
				ArrowSettingsDlgParam * asdp=(ArrowSettingsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				//Assign to the given ArrowSettings the new values
				*(asdp->as)=ASFromCtrls(hwndDlg); //save the settings
			}
		case IDCANCEL: //The user pressed Cancel (note: there's no break)
			EndDialog(hwndDlg,NULL);
			break;
		}
		return FALSE;
	case WM_DRAWITEM: //the system asks to draw an owner-draw control
		{
			LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
			switch(dis->CtlID)
			{
			case IDC_AS_PREVIEW:
				{
					HRGN paintRgn=CreateRectRgnIndirect(&dis->rcItem);
					POINT pt1, pt2;
					ArrowSettings tAS=ASFromCtrls(hwndDlg);
					pt1.x=10;
					pt2.x=dis->rcItem.right-10;
					pt1.y=pt2.y=dis->rcItem.top+(dis->rcItem.bottom-dis->rcItem.top)/2;
					FillRgn(dis->hDC,paintRgn,GetSysColorBrush(COLOR_WINDOW)); //fill the background
					DeleteObject(paintRgn);
					DrawArrow(dis->hDC,pt1,pt2, &tAS); //draw the arrow
					return FALSE;
				}
			}
			return FALSE;
		}
	}
	return FALSE;
}
//Shows the "Arrow settings" dialog
void ShowArrowSettings(HWND hParent, ArrowSettings & as)
{
	ArrowSettingsDlgParam asdp={0};
	asdp.as=&as;
	DialogBoxParam(hCurInstance,MAKEINTRESOURCE(IDD_ARROWSETTINGS),hParent,ASDialogProc,(LPARAM)&asdp);
	return;
}
//Returns an ArrowSettings structure based on the settings actually selected in the dialog
ArrowSettings ASFromCtrls(HWND hArrowSettings)
{
	ArrowSettings tAS;
	BOOL bTrans;
	tAS.drawFirstArrow=Button_GetCheck(GetDlgItem(hArrowSettings,IDC_AS_CH_FIRST))!=0;
	tAS.drawSecondArrow=Button_GetCheck(GetDlgItem(hArrowSettings,IDC_AS_CH_SECOND))!=0;
	tAS.openArrowHead=Button_GetCheck(GetDlgItem(hArrowSettings,IDC_AS_CH_OPENHEAD))!=0;
	tAS.arrowBase=GetDlgItemInt(hArrowSettings,IDC_AS_TX_WIDTH,&bTrans,FALSE);
	if(!bTrans)
		tAS.arrowBase=MINARROWWL; //if it cannot convert assign the minimum value
	tAS.arrowLength=GetDlgItemInt(hArrowSettings,IDC_AS_TX_LENGTH,&bTrans,FALSE);
	if(!bTrans)
		tAS.arrowBase=MINARROWWL; //if it cannot convert assign the minimum value
	return tAS;
}