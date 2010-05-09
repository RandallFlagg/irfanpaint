#pragma once
#include "DibSection.h"
#define COLORPALRCTSHSIZEMULT	0.75	//Fraction of width of the color pane that the color rectangles will be wide
#define COLORPALRCTSVSIZEMULT	0.65	//Fraction of height of the color pane that the color rectangles will be high
#define COLORPALSWAPRHEIGHT		10		//Height of the "swap" rectangle
#define COLORPALSWAPRWIDTH		10		//Width of the "swap" rectangle
#define COLORPALPICKRHEIGHT		10		//Height of the "picker" rectangle
#define COLORPALPICKRWIDTH		10		//Width of the "picker" rectangle
//ToolBox dialog procedure
INT_PTR CALLBACK TBDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//"About IrfanPaint" dialog procedure
INT_PTR CALLBACK ABDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//IrfanView main window window procedure
LRESULT CALLBACK MainIVWndProc(HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//"IrfanViewer" window window procedure
LRESULT CALLBACK IVWndProc(HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Function invoked by IrfanView; perform the init
BOOL __cdecl ShowIrfanPaintTB(HANDLE * pImgDIB, HWND hMainWindow, HWND hIVWindow, char *iniFile, char *errMSG);
//Closes the plugin
int __cdecl ClosePlugin(int iDummy,char *szDummy);
//Returns about information to IV
int __cdecl GetPlugInInfo(char *versionString,char *fileFormats);
//Performs the cleanup
void CleanUp();
//Populates the toolbar
void AddToolbarButtons(HWND hToolbar);
//Procedure called by EnumChildWindows (called by AddToolbarButtons)
BOOL CALLBACK ATB_EnumChildProc(HWND hwnd, LPARAM lParam);
//Performs the init tasks when a new DibSection is created
void DibSectCreate(DibSection * f_this);
//Sets the fg, bg and hatch bg color to a color compatible with the loaded image
void AdaptColors(DibSection * ds);
//Calculates the coords of the four color pane rectangles
void CalcColorPaneRects(const RECT * ColPaneRct, RECT * FgRct, RECT * BgRct, RECT * SwapRct, RECT * PickerRct);