#pragma once
#include "Arrow.h"
#define MINARROWWL 1 //Min arrow width/length - it must NEVER be less than 1
#define MAXARROWWL 1023 //Max arrow width/length - must be set accordingly to MAXAWLTBTEXTLENGTH
//"Arrow settings" dialog procedure
INT_PTR CALLBACK ASDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Structure that holds the parameters of the dialog
struct ArrowSettingsDlgParam
{
	ArrowSettings * as;
};
//Shows the "Arrow settings" dialog
void ShowArrowSettings(HWND hParent, ArrowSettings & as);
//Returns an ArrowSettings structure based on the settings actually selected in the dialog
ArrowSettings ASFromCtrls(HWND hArrowSettings);