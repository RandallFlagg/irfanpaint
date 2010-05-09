#pragma once
//Tabs of the P&B settings dialog
enum PBTabs
{
	Foreground=0,
	Background=1
};
//Class that keeps the params used by the P&B settings dialog
struct PenBrushSettingsDlgParam
{
public:
	PBTabs activeTab;		//Current active tab
	ColorUtils workingFG;	//Working ColorUtils of the FG settings
	ColorUtils workingBG;	//Working ColorUtils of the BG settings
	bool swappedColors;		//True if the user has swapped the FG and BG settings in the preview pane
	int fgTabIndex;			//FG tab index
	int bgTabIndex;			//BG tab index
	//Helpers
	//Returns the ColorUtils of the active tab
	inline ColorUtils & GetCurrentCU()
	{
		return activeTab==Background?workingBG:workingFG;
	};
	//Returns the ColorUtils of the inactive tab
	inline ColorUtils & GetNonCurrentCU()
	{
		return activeTab==Foreground?workingBG:workingFG;
	};
	//Returns the ColorUtils that is used as FG ColorUtils in the preview pane
	inline ColorUtils & GetPreviewPaneFGCU()
	{
		return swappedColors?workingBG:workingFG;
	};
	//Returns the ColorUtils that is used as BG ColorUtils in the preview pane
	inline ColorUtils & GetPreviewPaneBGCU()
	{
		return swappedColors?workingFG:workingBG;
	};
	//Set activeTab from a tab index
	void SetActiveTabIndex(int tabIndex)
	{
		if(tabIndex==fgTabIndex)
			activeTab=Foreground;
		else if(tabIndex==bgTabIndex)
			activeTab=Background;
		else
			throw std::runtime_error(ERROR_STD_PROLOG "Invalid tabIndex.");
	};
	//Get the current tab index
	int GetActiveTabIndex()
	{
		switch(activeTab)
		{
		case Foreground:
			return fgTabIndex;
		case Background:
			return bgTabIndex;
		default:
			throw std::runtime_error(ERROR_STD_PROLOG "Invalid activeTab.");
		}
	};
	//Constructor
	PenBrushSettingsDlgParam(ColorUtils & WorkingFG, ColorUtils & WorkingBG, PBTabs ActiveTab)
		: workingFG(WorkingFG), workingBG(WorkingBG), activeTab(ActiveTab)
	{
		swappedColors=false;
		fgTabIndex=0;
		bgTabIndex=1;
	};
};
//Shows the "Pen and brush settings" dialog
void ShowPenBrushSettings(HWND hParent, PBTabs tabToShow=Foreground);
//"Pen and brush settings" dialog procedure
INT_PTR CALLBACK PBDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Returns a LOGBRUSH from the settings of the "PenBrushSettings" dialog
LOGBRUSH BrushFromCtrls(HWND hPBS, COLORREF color);
//Sets the "PenBrushSettings" dialog accordingly to the LOGBRUSH passed
bool CtrlsFromBrush(HWND hPBS, LOGBRUSH lb);
//Returns an nEXTLOGPEN from the settings of the "PenBrushSettings" dialog
nEXTLOGPEN PenFromCtrls(HWND hPBS, int width);
//Sets the "PenBrushSettings" dialog accordingly to the nEXTLOGPEN passed
bool CtrlsFromPen(HWND hPBS, nEXTLOGPEN lp);
//Update the controls common to all the tabs (settings shared between FG & BG ColorUtils)
void ColorUtils2CommonControls(HWND hPBS);
//Update the ColorUtils with the user-selected common settings
void CommonControls2ColorUtils(HWND hPBS);
//Updates the preview pane
void UpdatePreviewPane(HWND hPBS);
//Subclassed textbox window procedure
LRESULT CALLBACK SubclTxtProc(HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Message handlers
void PBSettings_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL PBSettings_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
BOOL PBSettings_OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr);
void PBSettings_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
