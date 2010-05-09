#pragma once
//Palette info structure
struct PaletteInfo
{
	RGBQUAD * palette;
	int paletteEntries;
	int selectedIndex;
};
//Returns the color of the specified palette that the user selected
COLORREF SelPaletteColor(HWND hWndParent, RGBQUAD * palette, int paletteEntries, COLORREF defColor);
//Returns the color index of the specified palette that the user selected
int SelPaletteColorIndex(HWND hWndParent, RGBQUAD * palette, int paletteEntries, int selPalIndex);
//Draws or erases the selection rectangle
void DrawEraseSelectionRect(HDC hdc, int rectIndex, bool draw);
//Updates the selected color information provided in IDC_SP_CINFO
void UpdateColorInfoStatic(HWND hwndDlg, const PaletteInfo * pi);
//Select palette color dialog procedure
INT_PTR CALLBACK SCDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);