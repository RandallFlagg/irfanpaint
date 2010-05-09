#include "StdAfx.h"
#include ".\windowmagnetizer.h"
#include "Utils.h"
//Returns the new toolbar position accordingly to the current magnetization settings
POINT WindowMagnetizer::getToolbarPos(LPRECT tbRect)
{
	RECT mainWindowRect, toolbarRect; //Bounds of the two windows
	RECT workingAreaRect={0}; //Bounds of the working area
	POINT toolbarNewPos; //New position of the toolbar
	//Retrieve the bounds of the two windows
	GetWindowRect(mainWindow,&mainWindowRect);
	if(tbRect!=NULL)
		toolbarRect=*tbRect;
	else
		GetWindowRect(toolbar,&toolbarRect);
	//Retrieve the bounds of the working area
	SystemParametersInfo(SPI_GETWORKAREA,0,&workingAreaRect,0);
	if(!IsInitialized() || magnState==WindowMagnetizer::NotMagnetized || !enabled)
	{
		toolbarNewPos.x=toolbarRect.left;
		toolbarNewPos.y=toolbarRect.top;
	}
	else
	{
		//Basing on the magnetization style calculate the new position of the toolbar window
		switch(magnState)
		{
		case WindowMagnetizer::OnLeftSide:
			toolbarNewPos.y=mainWindowRect.top + magnOffset;
			toolbarNewPos.x=mainWindowRect.left - (toolbarRect.right-toolbarRect.left);
			break;
		case WindowMagnetizer::OnRightSide:
			toolbarNewPos.y=mainWindowRect.top + magnOffset;
			toolbarNewPos.x=mainWindowRect.right;
			break;
		default:
			//Tomorrow never knows
			toolbarNewPos.x=toolbarRect.left;
			toolbarNewPos.y=toolbarRect.top;
			break;
		}
		if(toolbarNewPos.x<workingAreaRect.left)
			toolbarNewPos.x=workingAreaRect.left;
		else if	((toolbarNewPos.x+(toolbarRect.right-toolbarRect.left))>workingAreaRect.right)
			toolbarNewPos.x=workingAreaRect.right-(toolbarRect.right-toolbarRect.left);
	}
	return toolbarNewPos;
}
//Updates the toolbar position accordingly to the current magnetization settings
void WindowMagnetizer::UpdateToolbarPos(LPRECT tbRect)
{
	if(!IsInitialized() || magnState==WindowMagnetizer::NotMagnetized || !enabled)
		return;
	POINT toolbarNewPos=getToolbarPos(tbRect);
	//Move the toolbar
	if(tbRect!=NULL)
		OffsetRect(tbRect,toolbarNewPos.x-tbRect->left,toolbarNewPos.y-tbRect->top);
	else
	{
		lockMagnState=true; //To prevent getNewMagnState from changing magnState - this may happen if out-of-working-area correction is used
		SetWindowPos(toolbar, 0, toolbarNewPos.x, toolbarNewPos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		lockMagnState=false;
	}
	return;
}
//Checks the toolbar position and eventually enables magnetization
void WindowMagnetizer::CheckToolbarPos(LPRECT tbRect)
{
	if(!IsInitialized() || !enabled)
		return;
	magnState=getNewMagnState(tbRect);
	UpdateToolbarPos(tbRect);
}
//Returns the new toolbar magnetization state accordingly to the current magnetization settings
WindowMagnetizer::MagnetizationState WindowMagnetizer::getNewMagnState(LPRECT tbRect)
{
	if(lockMagnState)
		return magnState;
	RECT mainWindowRect, toolbarRect; //Bounds of the two windows
	//Retrieve the bounds of the two windows
	GetWindowRect(mainWindow,&mainWindowRect);
	if(tbRect!=NULL)
		toolbarRect=*tbRect;
	else
		GetWindowRect(toolbar,&toolbarRect);
	magnOffset=toolbarRect.top-mainWindowRect.top;
	if(toolbarRect.top>mainWindowRect.bottom || toolbarRect.bottom<mainWindowRect.top)
		return WindowMagnetizer::NotMagnetized;
	else if(BETWEEN(mainWindowRect.left-toolbarRect.right,0,magnLimit))
		return WindowMagnetizer::OnLeftSide;
	else if(BETWEEN(toolbarRect.left-mainWindowRect.right,0,magnLimit))
		return WindowMagnetizer::OnRightSide;
	else
		return WindowMagnetizer::NotMagnetized;
}
//Call this function when the toolbar recieves a WM_MOVING or WM_SIZING
void WindowMagnetizer::OnToolbarBoundsChanging(LPARAM lParam)
{
	BOOL dfw;
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS,0,&dfw,0);
	if(!dfw)
	{
		RECT focusRect=*((LPRECT)lParam);
		HDC screenDC=GetDC(NULL);
		CheckToolbarPos(&focusRect);
		if(lastFocusRect.left!=INT_MIN)
			DrawNotRectangle(screenDC,&lastFocusRect);
		if(EqualRect((LPRECT)lParam,&focusRect))
			lastFocusRect.left=INT_MIN;
		else if(magnState!=WindowMagnetizer::NotMagnetized)
		{
			DrawNotRectangle(screenDC,&focusRect);
			lastFocusRect=focusRect;
		}
		ReleaseDC(NULL,screenDC);
	}
}
//Call this function when the toolbar recieves a WM_MOVE or WM_SIZE
void WindowMagnetizer::OnToolbarBoundsChanged()
{
	//No need to erase the last focus rect, the toolbar already covers it
	lastFocusRect.left=INT_MIN;
	//Eventually move the toolbar
	CheckToolbarPos();
	BOOL dfw;
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS,0,&dfw,0);
	if(!dfw)
	{
		//Completely repaint the toolbar
		InvalidateRect(toolbar,NULL,TRUE);
		RECT tbRect;
		GetWindowRect(toolbar,&tbRect);
		HRGN refRgn = CreateRectRgnIndirect(&tbRect);
		FORWARD_WM_NCPAINT(toolbar,refRgn,SendMessage);
		DeleteRgn(refRgn);
	}
}
//Draws a rectangle on the given DC with a NOT ROP2
void WindowMagnetizer::DrawNotRectangle(HDC hDC, CONST LPRECT rect)
{
	int oldROP=SetROP2(hDC,R2_NOT);
	BEGIN_SELOBJ(hDC,GetStockPen(BLACK_PEN),pen);
	BEGIN_SELOBJ(hDC,GetStockBrush(NULL_BRUSH),brush);
	Rectangle(hDC,rect->left,rect->top,rect->right,rect->bottom);
	END_SELOBJ(hDC,brush);
	END_SELOBJ(hDC,pen);
	SetROP2(hDC,oldROP);
};
/*//Resets the position of the toolbox
void ResetTBPos()
{
	//Retrieve the size and the position of IVW, TB and the desktop
	RECT IVWRect, TBRect, DeskRect;
	POINT newPos; //New position of the toolbar
	GetWindowRect(hIVWindow, &IVWRect);
	GetWindowRect(hToolBoxWindow, &TBRect);
	SystemParametersInfo(SPI_GETWORKAREA,0,&DeskRect,0);
	newPos.y=IVWRect.top;
	//Put the toolbar on the right of IVW if it doesn't exit from the screen; otherwise put it in the extreme right of the screen
	newPos.x=IVWRect.right+(TBRect.right-TBRect.left)>DeskRect.right?DeskRect.right-(TBRect.right-TBRect.left):IVWRect.right;
	//Change the position of the toolbox
	SetWindowPos(hToolBoxWindow, 0, newPos.x, newPos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}*/