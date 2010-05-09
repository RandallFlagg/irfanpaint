#pragma once
#include "MeasurementUnit.h"
//Measurement units dialog
//Structure used to keep info associated with the window
struct MeasurementUnitsDlgParam
{
	MeasurementUnitsContainer * MUC;
};
//Shows the "Measurement units" dialog
void ShowMeasurementUnits(HWND hParent, MeasurementUnitsContainer & MUC);
//"Measurement units" dialog procedure
INT_PTR CALLBACK MUDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Populates the listview
void MU_PopulateListView(HWND hListView, MeasurementUnitsContainer & MUC);
//Enables/disables the various buttons
void EnableButtons(HWND hwnd, bool MoveUp, bool MoveDown, bool Absolutize, bool Add, bool Edit, bool Remove);
//Event handlers
//WM_COMMAND
void MU_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
//WM_INITDIALOG
BOOL MU_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
//WM_NOTIFY
BOOL MU_OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr);

//Edit measurement unit dialog
//Structure used to keep info associated with the window
struct EditMeasurementUnitDlgParam
{
	DerivedUnit * Item;
	MeasurementUnitsContainer * MUC;
	std::_tcstring DefinitionLabelSyntax;
};
//Shows the "Edit measurement unit" dialog; returns false if the user pressed Cancel
bool ShowEditMeasurementUnit(HWND hParent, DerivedUnit & Item, MeasurementUnitsContainer & MUC);
//"Edit measurement unit" dialog procedure
INT_PTR CALLBACK EMUDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Populates the combobox
void EMU_PopulateComboBox(HWND hComboBox, DerivedUnit & Item, MeasurementUnitsContainer & MUC);
//Populates the controls from the item
void EMU_CtrlsFromItem(HWND hwnd, DerivedUnit & Item);
//Makes a DerivedUnit from the controls
DerivedUnit EMU_ItemFromCtrls(HWND hwnd);
//Event handlers
//WM_COMMAND
void EMU_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
//WM_INITDIALOG
BOOL EMU_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

//Measure output dialog
//Structure used to keep info associated with the window
struct MeasureOutputDlgParam
{
	MeasurementUnitsContainer * MUC;
	POINT FirstPos;
	POINT SecondPos;
	DibSection * DibSect;
};
//Shows the "Measure output dialog" 
void ShowMeasureOutput(HWND hParent, MeasurementUnitsContainer & MUC, POINT FirstPos, POINT SecondPos, DibSection & DibSect);
//"Edit measurement unit" dialog procedure
INT_PTR CALLBACK MODialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
//Populates the listbox with the results
void MO_PopulateListBox(HWND hListBox, MeasureOutputDlgParam & modp);
//Event handlers
//WM_COMMAND
void MO_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
//WM_INITDIALOG
BOOL MO_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);