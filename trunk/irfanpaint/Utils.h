//Helper functions and macros
#pragma once
#include "DibSection.h" //DibSection class
#include "Arrow.h"		//Arrow API
#include "Globals.h"	//Global declarations
#include "ColorUtils.h" //ColorUtils class
#include "PrecisionCursor.h" //PrecisionCursor class
#include "DibSectionUpdater.h"
#include <cmath>
//Structures
//Point with double-precision coords
struct POINTD
{
	double x;
	double y;
};
//User messages
#define WM_GET_SCROLL_VALUES	(WM_USER+700)
/*
	WM_GET_SCROLL_VALUES

	This message is sent to hMainWindow to retrieve the scroll values.
		wParam
			NULL or pointer to a POINT structure.
		lParam
			Not used; must be 0.
		Return value
			Low word: x shift value.
			High word: y shift value.
	Remarks:
		The shift values are in screen pixels, not in DIB pixels. See IVWnd2DIBPoint and DIB2IVWndPoint to understand how they're treated.
		If wParam is not NULL it is considered as a pointer to a POINT structure, which is filled with the shift values.
		This removes the 16-bit per coord limit of the original version of this message.
		Note that on previous versions of IV the wParam is not considered, so if you use the wParam to get the shift you 
		should always check if the struct is changed (e.g. by setting its members to INT_MIN before sending the message 
		and checking if they are changed) and fall back to the original return value method if they aren't changed.
	See also:
		GetShift(), IVWnd2DIBPoint(), DIB2IVWndPoint()

*/
#define WM_GET_ZOOM_VALUE		(WM_USER+701)
/*
	WM_GET_ZOOM_VALUE

	This message is sent to hMainWindow to retrieve the zoom factor (in %).
		wParam
			Pointer to a float or NULL; if it is not NULL the zoom factor is stored here.
		lParam
			Not used; must be 0.
		Return value
			The zoom factor in %.
	Remarks:
		The use of wParam for a float pointer is available from after-4.20 IV betas.
	See also:
		GetZoom(), IVWnd2DIBPoint(), DIB2IVWndPoint()
*/
#define WM_SET_NEW_UNDO			(WM_USER+674)
/*
	WM_SET_NEW_UNDO

	This message is sent to hMainWindow to set a new undo DIB.
		WM_GET_ZOOM_VALUE
	This message is sent to IV to retrieve the zoom factor (in %).
		wParam
			Not used; must be 0.
		lParam
			Not used; must be 0.
		Return value
			The zoom factor in %.
	Remarks:
		Actually there's a single undo DIB, so no multiple undo support is available.
	See also:
		UIBaseTool::SetNewUndo()
*/
#define WM_ROTATE_IMAGE			(WM_USER+705)
/*
	WM_ROTATE_IMAGE

	This message is sent to hMainWindow to rotate the image.
		wParam
			Color used for the background.
		lParam
			Degrees of the rotation; actually a float converted to LPARAM with ConvertType.
		Return value
			 Not used.
	Remarks:
		The color passed via wParam is a COLORREF with the high byte set to 0 (as the ones created with the RGB macro). It is
		not defined if other types of COLORREF may work, but they probably don't.
		Use ConvertType to convert the float value of the roation to lParam.
	See also:
		UIStraighten::PerformAction()
*/
#define WM_SET_RESAMPLE			(WM_USER+706)
/*
	WM_SET_RESAMPLE

	This message is sent to hMainWindow to interact with its resample view option.
		wParam
			If 0 the resample option (if enabled) is temporarily disabled.
			If 1 the resample option is restored to the state it was before the last WM_SET_RESAMPLE with wParam 0.
			If 2 the cached resample image is updated (if the resample function is enabled) and the IV window is repainted.
		lParam
			Not used; must be 0.
		Return value
			Not used.
	Remarks:
		For every WM_SET_RESAMPLE with wParam=1 there should be a WM_SET_RESAMPLE with wParam=2 (and viceversa).
		This message is used just for temporary adjustments, not to enable/disable the option and write the settings to the INI file.
	See also:
		UIBaseTool::DisableResample(), UIBaseTool::ReenableResample(), UIBaseTool::UpdateResampledImage()

*/
#define WM_PAINT_PLUGIN_CLOSED	(WM_USER+707)
/*
	WM_PAINT_PLUGIN_CLOSED

	This message is sent to hMainWindow to signal that the plugin has been closed.
		wParam
			Not used; must be 0.
		lParam
			Not used; must be 0.
		Return value
			Not used.
	Remarks:
		This message must be sent because IV do not perform some checks on the DIB memory when IrfanPaint is active, and needs to know
		when it's closed to know if it can do them.
	See also:
		CleanUp()

*/
#if 0
//These messages are no longer supported (from after-4.20 IV betas); use WM_GET_SELECTION instead
#define WM_GET_SELECTION_UL		(WM_USER+702)
#define WM_GET_SELECTION_LR		(WM_USER+703)
/*
	WM_GET_SELECTION_UL
	WM_GET_SELECTION_LR

    These messages are sent to hMainWindow to obtain the upper-left (_UL) and lower-right (_LR) corner of the current selection rectangle.
		wParam
			Not used; must be 0.
		lParam
			Not used; must be 0.
		Return value
			Low word: x coord.
			High word: y coord.
	Remarks:
		The coords are relative to the IrfanViewer window client area.
		These messages are superseded by WM_GET_SELECTION.
	See also:
		WM_GET_SELECTION
*/
#endif
#define WM_GET_SELECTION           WM_USER+708 
/*
	WM_GET_SELECTION

	This message is sent to hMainWindow to obtain the current selection rectangle.
		wParam
			Pointer to a RECT structure to be filled with the selection data.
		lParam
			Not used; must be 0.
		Return value:
			Not used.
	Remarks:
		The coords are relative to the IrfanViewer window client area.
	See also:
		GetSelectedRect()

*/
#define WM_PLUGIN_HWND              WM_USER+710
#define PLUGIN_ID_PAINT             101
/*
	WM_PLUGIN_HWND

	This message is sent to hMainWindow to notify it of the plugin window handle.
		wParam
			Plugin ID. PLUGIN_ID_PAINT for the Paint plugin.
		lParam
			Handle of the plugin window.
		Return value:
			Not used.
	Remarks:
		At the moment this is just used so that IV can forward to IP some key messages (ESC press as WM_COMMAND).
	See also:
	    ShowIrfanPaintTB()

*/
//DC objects selection helpers
#define BEGIN_SELOBJ(dc, obj, id) HGDIOBJ _t ## id = SelectObject(dc, obj)
#define END_SELOBJ(dc, id) SelectObject(dc, _t ## id)
/*
	Example of use of the BEGIN_SELOBJ and END_SELOBJ macros:

	HDC dc;
	//...
	BEGIN_SELOBJ(dc,CreateSolidBrush(0x000000FF),brush); //create the brush, select it in the DC and save the old brush for restoring
	//...
	DeleteBrush(END_SELOBJ(dc,brush)); //restore the old brush in the DC and delete the new one
*/
//Other helpers
#define BETWEEN(n,min,max) ((n)>=(min)&&(n)<=(max))
/*
	Note that the argument n is evaluated two times.

	Example of use of the BETWEEN macro:

	int n;
	//...
	if(BETWEEN(n,15,17))
		cout<<"n is between 15 and 17";
*/
//Gets the width of a rectangle
#define RECTWIDTH(rect) ((rect).right-(rect).left)
//Gets the height of a rectangle
#define RECTHEIGHT(rect) ((rect).bottom-(rect).top)
//Trick to use a RECT structure instead of the single coords
#define EXPANDRECT_C(rect) (rect).left,(rect).top,(rect).right,(rect).bottom
//Trick to use a RECT structure instead of the coords and of the size
#define EXPANDRECT_CS(rect) (rect).left,(rect).top,RECTWIDTH(rect),RECTHEIGHT(rect)
//Get a pointer to the upper left corner of a RECT
#define ULCORNER(rect) ((POINT *)(&(rect)))
//Get a pointer to the lower right corner of a RECT
#define LRCORNER(rect) ((POINT *)(&(rect))+1)
/*
	Example of use of the EXPANDRECT_* helper macros:

	RECT srcRect,destRect;
	HDC srcDC,destDC;
	//...
	Rectangle(destDC,EXPANDRECT_C(destRect));
	BitBlt(destDC,EXPANDRECT_CS(destRect),srcDC,EXPANDRECT_CS(srcRect));
*/
//INI file section used by SaveSettings & LoadSettings to store the settings
#define INISECTION _T("Paint")
//Min pen width - it must NEVER be less than 1
#define MINPENWIDTH 1
//Max pen width
#define MAXPENWIDTH 9999
//Misc
#ifndef ROP3_NOP
#define ROP3_NOP		0x00AA0029	//"NOP" ROP3
#endif
#ifndef ROP3_DPa
#define ROP3_DPa		0x00A000C9	//"DPa" ROP3
#endif
//Message number counter. Used by the helper functions that send messages to IV to decide either to send the message either to use a cached value
extern unsigned int MsgNumber;
//Makes a RECT from two POINT(S)
template <class rectType, class pt1Type, class pt2Type> inline rectType _MakeRect(pt1Type pt1, pt2Type pt2)
{
	rectType tRect;
	tRect.top=pt1.y;
	tRect.left=pt1.x;
	tRect.right=pt2.x;
	tRect.bottom=pt2.y;
	//Normalize the RECT
	NormalizeRect(&tRect);
	return tRect;
};
#define MakeRect _MakeRect<RECT>
//Normalizes a RECT
template<class rectType>inline void NormalizeRect(rectType * rect)
{
	register int tswap;
	if(rect->top>rect->bottom)
	{
		tswap=rect->top;
		rect->top=rect->bottom;
		rect->bottom=tswap;
	}
	if(rect->left>rect->right)
	{
		tswap=rect->left;
		rect->left=rect->right;
		rect->right=tswap;
	}
	return;
};
//Makes equal the measures of a figure (e.g. the coords describing a rectangle are changed so that they describe a square); only editablePt is changed
void MakeEqualMeasures(POINT & editablePt, POINT fixedPt);
//Costrains a line to be tilted of 45° or hortogonal; only editablePt is changed
void CostrainHortOr45Line(POINT & editablePt, POINT fixedPt);
//Returns the distance between two points
template<class pt1Type, class pt2Type> inline double PointsDistance(pt1Type pt1, pt2Type pt2)
{
	double hOff=(pt1.x-pt2.x),vOff=(pt1.y-pt2.y);
	return sqrt(hOff*hOff+vOff*vOff);
};
//Returns a string with some information about the dll
std::_tcstring GetAboutMessage(bool insertName, bool insertCompany, bool insertComments, bool insertVersion, bool insertBeta, bool insertConnectors);
//Returns a color chosen by the user through a dialog
COLORREF GetColor(HWND hOwner, COLORREF defColor, DibSection * dibSect);
//Saves the settings of IrfanPaint
void SaveSettings(INISection & IniSect);
//Loads the settings of IrfanPaint
void LoadSettings(INISection & IniSect);
//Returns the currently selected pen width in the updown
int GetPenWidthUD();
//Syncs the pen width updown with the pen width of the fg ColorUtils
void SyncPenWidthUD();
//Syncs the tolerance updown with the internal tolerance variable
void SyncToleranceUD();
//Returns the state of the fill checkbox
bool GetFillCH();
//Sets the state of the fill checkbox
void SetFillCH(bool fillFlag);
//Returns the currently selected button in the toolbar
int GetCurrentToolTB();
//Returns the check state of a button in the toolbar
bool IsToolCheckedTB(int tool);
//Checks the given button in the toolbar
void CheckToolTB(unsigned int tool);
//Returns the first checked button of a series of buttons
int GetCheckedButton(HWND hParent, int firstButton, int lastButton);
//Rounds a number to the nearest integer
long round(double number);
//Returns the current image shift from the upper-left corner of the IVW
POINT GetShift();
//Returns the rectangle of IVWnd painted with the image
RECT GetImageRect();
//Same as GetImageRect, but returns the info via a reference parameter
void GetImageRectPtr(RECT * imgRect);
//Returns the current image zoom (in %)
float GetZoom();
//Returns the state of the modifier keys
int GetModifierKeysState();
//Converts a POINT(S) referred to the IrfanViewer window to a POINT(S) referred to the bitmap
template <class ptType> void IVWnd2DIBPoint(ptType * IVWndPoint)
{
	if(dsUpdater==NULL || !dsUpdater->CheckState())
		return;
	POINT shift=GetShift();
	float zoom=GetZoom();
#pragma warning (push)
#pragma warning (disable:4244)
	IVWndPoint->x=(long)((IVWndPoint->x+shift.x)/(zoom/100.0));
	IVWndPoint->y=(long)((IVWndPoint->y+shift.y)/(zoom/100.0));
/*
	//This code was used when the paint was done by IV
	if(shift.x<0)
		IVWndPoint->x+=shift.x;
	if(shift.y<0)
		IVWndPoint->y+=shift.y;
	if(zoom<=100)
	{
		IVWndPoint->x=round(IVWndPoint->x/(zoom/100.0));
		IVWndPoint->y=round(IVWndPoint->y/(zoom/100.0));
	}
	else
	{
		SIZED pxSize=GetDIBPixelSize();
		IVWndPoint->x=IVWndPoint->x/pxSize.cx+0.01; //To avoid that a 0.999999... is considered 0 instead of 1
		IVWndPoint->y=IVWndPoint->y/pxSize.cy+0.01;
	}
	if(shift.x>0)
		IVWndPoint->x+=round(shift.x/(zoom/100.0));
	if(shift.y>0)
		IVWndPoint->y+=round(shift.y/(zoom/100.0));
*/
#pragma warning (pop)
	return;
};
//Converts a POINT(S) referred to the bitmap to a POINT(S) referred to the IrfanViewer window
template <class ptType> void DIB2IVWndPoint(ptType * DIBPoint)
{
	if(dsUpdater==NULL || !dsUpdater->CheckState())
		return;
	POINT shift=GetShift();
	float zoom=GetZoom();
#pragma warning (push)
#pragma warning (disable:4244)
	DIBPoint->x*=zoom/100.0;
	DIBPoint->y*=zoom/100.0;
	DIBPoint->x-=shift.x;
	DIBPoint->y-=shift.y;
#pragma warning (pop)
	return;
};
//Sets the current pen width
inline void SetPenWidth(int penWidth, bool updateUpDown=true)
{
	fgColor.SetPenWidth(penWidth);
	bgColor.SetPenWidth(penWidth);
	normPC.SetWidth(penWidth);
	clsrPC.SetWidth(penWidth);
	if(updateUpDown)
		SyncPenWidthUD();
};
#pragma warning(push)
#pragma warning(disable:4127)
//Converts a type in another
template <class outType, class inType> outType ConvertType(inType inVal)
{
	if (sizeof(outType)!=sizeof(inType))
		DebugBreak();
	return *((outType *)&inVal);
};
#pragma warning(pop)
//Sets up the clipping region of IVWDC and dsUpdater->GetDibSection()->m_compDC
void SetupClippingRegion();
//Returns the currently selected RECT
RECT GetSelectedRect();
//Displays an error message box (message specified by ID)
int ErrMsgBox(HWND hwndOwner, int messageID, DWORD * lastError=NULL, int flags=(MB_OK|MB_ICONERROR));
//Displays an error message box (message specified explicitly)
int ErrMsgBox(HWND hwndOwner, const TCHAR * message, DWORD * lastError=NULL, int flags=(MB_OK|MB_ICONERROR));
//Displays an error message box (message based on an exception)
int ErrMsgBox(HWND hwndOwner, exception & exception, const TCHAR * currentProcedureName, int flags=(MB_OK|MB_ICONERROR));
//Simulates a mouse move
void SimMM(HWND hWnd, bool post, bool sendSetCursor=true, bool sendMouseMove=true);
//Sends or posts a message
inline void MsgSend(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, bool post);
//Converts a string made of dots and spaces to a style array; returns the number of elements of the array (you can pass NULL as array to obtain just the number of elements needed)
DWORD DotSpaceStr2StyleArray(const TCHAR * string, DWORD * array, bool corrected);
//Converts a style array to a string made of dots and spaces; returns the number of characters of the string (you can pass NULL to string to obtain just the size of the string needed)
std::_tcstring StyleArray2DotSpaceStr(DWORD * array, int elemCount);
//Returns a DWORD representing an RGB color from a COLORREF
DWORD COLORREF2DWORD(COLORREF cr, RGBQUAD * palette);
//Loads the RichEdit module
bool LoadRichEdit();
//Unloads the RichEdit module
void FreeRichEdit();
//Returns a description for the given Windows error code
std::_tcstring GetErrorDescription(DWORD errorCode=GetLastError(), bool * success=NULL);
//Color comparison routine; use similarColor2 if threshold may be 255 or 0 (in this case it can avoid some calculations)
__forceinline bool similarColor(RGBQUAD color1, RGBQUAD color2, BYTE threshold)
{
	return 
		abs((int)color1.rgbRed-(int)color2.rgbRed)<=threshold && 
		abs((int)color1.rgbGreen-(int)color2.rgbGreen)<=threshold && 
		abs((int)color1.rgbBlue-(int)color2.rgbBlue)<=threshold
		;
}
//Same as similarColor, but avoids some calculations if it can; use this if threshold may be 255 or 0
__forceinline bool similarColor2(RGBQUAD color1, RGBQUAD color2, BYTE threshold)
{
	if(threshold==255)
		return true;
	else if(threshold==0)
	{
		return 
			color1.rgbRed==color2.rgbRed &&
			color1.rgbGreen==color2.rgbGreen &&
			color1.rgbBlue==color2.rgbBlue
			;
	}
	else
		return similarColor(color1,color2,threshold);
}
//Validates a change done by the user to a textbox that must contain unsigned integers
bool ValidateUIntFieldChange(HWND parent, WORD ctrlID, UINT & lastValue, UINT minValue, UINT maxValue);
//Replaces all the instances of search with replace in string; returns the number of substitutions done
unsigned int ReplaceString(std::_tcstring & string,const std::_tcstring & search,const std::_tcstring & replace);
//Makes sure that a window is entirely in the working area and eventually moves it
void MoveInWorkingArea(HWND hwnd);
