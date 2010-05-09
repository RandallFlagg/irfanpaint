#include "stdafx.h"
#include "resource.h"
#include "Globals.h"
#include "Utils.h"
#include "SelPaletteColor.h"
#include "LanguageFile.h"
//Sizes
#define SP_ROWNUMBER		16	//Number of rows
#define SP_COLNUMBER		16	//Number of columns
#define SP_LEFTMARGIN		4	//Left margin of the "color table"
#define SP_TOPMARGIN		4	//Top margin of the "color table"
#define SP_BRECTHSIZE		18	//Horizontal size of the bounding rect
#define SP_BRECTVSIZE		18  //Vertical size of the bounding rect
#define SP_IRECTHMARGIN		2	//Width of the horizontal margin between the color rect and the bounding rect
#define SP_IRECTVMARGIN		2	//Heigth of the vertical margin between the color rect and the bounding rect
#define SP_BRECTHDIST		2	//Horizontal distance between two bounding rectangles
#define SP_BRECTVDIST		2	//Vertical distance between two bounding rectangles
//Returns the color of the specified palette that the user selected
COLORREF SelPaletteColor(HWND hWndParent, RGBQUAD * palette, int paletteEntries, COLORREF defColor)
{
	//Get the default (selected) index
	int defIndex;
	if(((defColor&0x10FF0000)==0x10FF0000)||((defColor&0x01000000)==0x01000000)||defColor==CLR_INVALID) //If defColor is a "false" COLORREF extract directly the index...
		defIndex=LOWORD(defColor);
	else //... otherwise read the palette to find a match
	{
		for(defIndex=paletteEntries-1;defIndex;defIndex--)
		{
			if(RGB(palette[defIndex].rgbRed, palette[defIndex].rgbGreen, palette[defIndex].rgbBlue)==defColor)
				break;
		}
	}
	//Get the output of the other function
	int index=SelPaletteColorIndex(hWndParent, palette, paletteEntries, defIndex);
	//Get a pointer to the first palette color
	RGBQUAD * rgq=palette;
	if(index==-1) //The user pressed "Cancel"
	{
		if(defColor&0xFF000000) //An invalid defColor means that, if the user pressed cancel, the caller wants the first color of the palette
			index=0;
		else
			return defColor; //Return defColor
	}
	rgq+=index; //Get the needed palette entry
	return RGB(rgq->rgbRed, rgq->rgbGreen, rgq->rgbBlue); //Convert it to a COLORREF and return
}
//Returns the color index of the specified palette that the user selected
int SelPaletteColorIndex(HWND hWndParent, RGBQUAD * palette, int paletteEntries, int selPalIndex=0)
{
	if(palette==NULL)
		throw std::invalid_argument(ERROR_STD_PROLOG "palette must be a valid non-NULL pointer.");
	if(paletteEntries<=0 || paletteEntries>SP_ROWNUMBER*SP_COLNUMBER)
		throw std::invalid_argument(ERROR_STD_PROLOG "paletteEntries must be greater than zero and less than " TOSTRING(SP_ROWNUMBER) "*" TOSTRING(SP_COLNUMBER) ".");
	PaletteInfo palInfo;
	palInfo.palette=palette;
	palInfo.paletteEntries=paletteEntries;
	palInfo.selectedIndex=selPalIndex;
	if(DialogBoxParam(hCurInstance, MAKEINTRESOURCE(IDD_SELPALCOLOR),hWndParent,SCDialogProc, (LPARAM)&palInfo)==IDOK)
		return palInfo.selectedIndex;
	else
		return -1;
}
//Select palette color dialog procedure
INT_PTR CALLBACK SCDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	switch(uMsg) //Message switch
	{
	case WM_INITDIALOG: //Dialog initialization
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG)lParam); //Associate a pointer to the structure to the window
		//Load the captions of the child windows (and of itself)
		langFile->InitializeDialogCaptions(hwndDlg, IDD_SELPALCOLOR);
		//Update the selected color information static
		UpdateColorInfoStatic(hwndDlg, (PaletteInfo *) lParam);
		return TRUE;
	case WM_PAINT: //Paint
		{
			PAINTSTRUCT ps;
			RECT trect;
			PaletteInfo * palInfo = (PaletteInfo *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA); //Retrieve the pointer
			RGBQUAD * rq; //Pointer to the color
			HDC hdc=BeginPaint(hwndDlg, &ps); //Start to paint
			HBRUSH defBrush=NULL; //Default brush associated to the DC
			HBRUSH tBrush; //Temporary brush
			//Draw all the filled rectangles
			for(int y=0; y<=palInfo->paletteEntries/SP_COLNUMBER; y++)
			{
				for(int x=0; x<SP_COLNUMBER; x++)
				{
					if(x+y*SP_COLNUMBER==palInfo->paletteEntries)
						break;
					rq=palInfo->palette+x+y*SP_COLNUMBER; //Get the color
					//Select the new brush in the DC
					tBrush=(HBRUSH)SelectObject(hdc,CreateSolidBrush(RGB(rq->rgbRed,rq->rgbGreen,rq->rgbBlue)));
					//If it is the first time we have to store somewhere the default brush...
					if(x==0&&y==0)
						defBrush=tBrush;
					else //... otherwise we can safely delete it, since we created it.
						DeleteObject(tBrush);
					//Calculate the dimensions of the rectangle
					trect.left=x*(SP_BRECTHSIZE+SP_BRECTHDIST)+SP_IRECTHMARGIN+SP_LEFTMARGIN;
					trect.right=trect.left+SP_BRECTHSIZE-2*SP_IRECTHMARGIN;
					trect.top=y*(SP_BRECTVSIZE+SP_BRECTVDIST)+SP_IRECTVMARGIN+SP_TOPMARGIN;
					trect.bottom=trect.top+SP_BRECTVSIZE-2*SP_IRECTVMARGIN;
					//Draw it!
					Rectangle(hdc,EXPANDRECT_C(trect));
					//If it is the currently selected color draw also the bounding rectangle
					if(x+y*SP_COLNUMBER==palInfo->selectedIndex)
						DrawEraseSelectionRect(hdc,x+y*SP_COLNUMBER,true);
				}
			}
			//Delete the last brush and select the default one
			DeleteBrush((HBRUSH)SelectObject(hdc,defBrush));
			//End the painting
			EndPaint(hwndDlg, &ps);
			return TRUE;
		}
	case WM_LBUTTONDBLCLK: //Left mouse double click
	case WM_LBUTTONDOWN: //Left mouse button down
		{
			PaletteInfo * palInfo = (PaletteInfo *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA); //See above
			POINTS mousePos=MAKEPOINTS(lParam);
			POINTS colorRectPos;
			HDC hdc=GetDC(hwndDlg);
			//Subtract the margins
			mousePos.x-=SP_LEFTMARGIN;
			mousePos.y-=SP_TOPMARGIN;
			//Calculate the corresponding rectangle
			colorRectPos.x=mousePos.x/(SP_BRECTHSIZE+SP_BRECTHDIST);
			colorRectPos.y=mousePos.y/(SP_BRECTVSIZE+SP_BRECTVDIST);
			//Validity check
			if(colorRectPos.x+colorRectPos.y*SP_COLNUMBER>=palInfo->paletteEntries||colorRectPos.x>=SP_COLNUMBER||colorRectPos.y>=SP_ROWNUMBER||colorRectPos.x<0||colorRectPos.y<0)
				return FALSE;
			//Erase the bounding rectangle of the previously selected color
			DrawEraseSelectionRect(hdc, palInfo->selectedIndex,false);
			//Set the new selected color
			palInfo->selectedIndex=colorRectPos.x+colorRectPos.y*SP_COLNUMBER;
			//Draw the bounding rectangle of the newly selected color
			DrawEraseSelectionRect(hdc, palInfo->selectedIndex,true);
			ReleaseDC(hwndDlg,hdc);
			//Update the selected color information static
			UpdateColorInfoStatic(hwndDlg, palInfo);
			if(uMsg==WM_LBUTTONDBLCLK) //If the user double-clicked a color...
				EndDialog(hwndDlg, IDOK); //... also end the dialog
			return TRUE;
		}
	case WM_COMMAND: //Button clicks etc.
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam)); //End the dialog telling if the user pressed OK or Cancel
			return TRUE;
		}
		break;
	case WM_CLOSE: //Window closed with the [X] button
		EndDialog(hwndDlg,IDCANCEL); //Same as above
		return TRUE;
	}
	return FALSE;
}
//Draws or erases the selection rectangle
void DrawEraseSelectionRect(HDC hdc, int rectIndex, bool draw)
{
	RECT trect;
	//Calculate the position
	int x=rectIndex%SP_COLNUMBER;
	int y=rectIndex/SP_COLNUMBER;
	trect.left=x*(SP_BRECTHSIZE+SP_BRECTHDIST)+SP_LEFTMARGIN;
	trect.right=trect.left+SP_BRECTHSIZE;
	trect.top=y*(SP_BRECTVSIZE+SP_BRECTVDIST)+SP_TOPMARGIN;
	trect.bottom=trect.top+SP_BRECTVSIZE;
	//Select the right pen & brush
	BEGIN_SELOBJ(hdc,draw?GetStockObject(BLACK_PEN):CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNFACE)),hdc_pen);
	BEGIN_SELOBJ(hdc,GetStockObject(NULL_BRUSH),hdc_brush);
	//Draw the rectangle
	Rectangle(hdc,EXPANDRECT_C(trect));
	//Re-select the previously selected pen & brush, disposing the eventually created pen
	END_SELOBJ(hdc,hdc_brush);
	if(draw)
		END_SELOBJ(hdc,hdc_pen);
	else
		DeleteObject(END_SELOBJ(hdc,hdc_pen));
	return;
}
//Updates the selected color information provided in IDC_SP_CINFO
void UpdateColorInfoStatic(HWND hwndDlg, const PaletteInfo * pi)
{
	//The string is composed in this way:
	//Index: xxx; color: RGB(rrr, ggg, bbb), HTML #rrggbb\0
	//51 chars, round to 64 (tomorrow never knows :-P)
	TCHAR buffer[64];
	//Color to examinate
	RGBQUAD * rq=pi->palette+pi->selectedIndex;
	_sntprintf(buffer,ARRSIZE(buffer),_T("Index: %d; color: RGB(%d, %d, %d), HTML #%02x%02x%02x"),pi->selectedIndex,(int)rq->rgbRed,(int)rq->rgbGreen,(int)rq->rgbBlue,(int)rq->rgbRed,(int)rq->rgbGreen,(int)rq->rgbBlue);
	SetDlgItemText(hwndDlg, IDC_SP_CINFO, buffer);
}
