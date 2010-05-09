#pragma once
//Edited cracker macros
/* (any type) Cls_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_LBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_RBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnRButtonUp(HWND hwnd, int x, int y, UINT flags) */
#define BHANDLE_WM_RBUTTONUP(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnMButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_MBUTTONDOWN(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnMButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define BHANDLE_WM_MBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
/* (any type) Cls_OnMButtonUp(HWND hwnd, int x, int y, UINT flags) */
#define BHANDLE_WM_MBUTTONUP(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam))
#ifdef UIBT_HANDLEMOUSEWHEEL
/* (any type) Cls_OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys) */
#define BHANDLE_WM_MOUSEWHEEL(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (int)(short)HIWORD(wParam), (UINT)(short)LOWORD(wParam))
#endif
/* (any type) Cls_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg) */
#define BHANDLE_WM_SETCURSOR(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))
//This class is used to carry a return value for a window message and a value that indicates if the
//message has already been completely processed
class MessageReturnValue
{
public:
	LRESULT returnValue;
	bool processed;
	MessageReturnValue()
	{
		processed=false;
		returnValue=FALSE;
	};
	MessageReturnValue(LRESULT ReturnValue)
	{
		processed=true;
		returnValue=ReturnValue;
	};
	MessageReturnValue(bool Processed)
	{
		returnValue=FALSE;
		processed=Processed;
	};
	inline operator bool()
	{
		return processed;
	};
	inline operator LRESULT()
	{
		return returnValue;
	};
};