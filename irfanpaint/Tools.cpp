#include "stdafx.h"			//Standard inclusions
#include "Globals.h"		//Global definitions
#include "ToolbarButtons.h"	//Toolbar buttons #define
#include "Utils.h"			//Support functions & macros
#include "Tools.h"			//Tools.cpp header
#include "CloneTool.h"		//Clone tool
#include "IrfanPaint.h"		//IrfanPaint.cpp header
#include "AdvEraser.h"		//Advanced eraser
#include "InsertText.h"		//Insert text dialog
#include "FloodFill.h"
#include "LanguageFile.h"
#include "MeasurementUnit.h"
#include "ArrowSettings.h"
#include "MeasurementUnitsDialogs.h"
#include "INISection.h"
#include "DibSectionUpdater.h"
//Draws or erases the picker color pane
void DrawPickerCPane(POINTS mousePos, bool erase)
{
	static bool erased=true;
	IVWnd2DIBPoint(&mousePos);
	COLORREF ptColor=0;
	try
	{
		ptColor=GetPixel(dsUpdater->GetDibSection()->GetCompDC(),mousePos.x,mousePos.y); //Get the color
	}
	catch(...)
	{
		erase=true;
	}
	if(ptColor==CLR_INVALID)
		erase=true;
	if(erase&&erased==true)
		return;
	erased=erase;
	RECT colPaneRct, pickerRct;
	//Get HWND and DC of the color pane window 
	HWND cpWnd=GetDlgItem(hToolBoxWindow,IDC_TB_COLORPAL);
	HDC cpDC=GetDC(cpWnd);
	//Calculate the picker color pane position and size
	GetClientRect(cpWnd,&colPaneRct);
	CalcColorPaneRects(&colPaneRct,NULL,NULL,NULL,&pickerRct);
	//If we are outside the image, select a gray pen (to erase the previously drawn rectangle), otherwise a stock black pen
	BEGIN_SELOBJ(cpDC,erase?CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DFACE)):GetStockPen(BLACK_PEN),cpDC_pen);
	//If we are outside the image, select a gray brush (to erase the previously drawn rectangle), otherwise a brush of the color of the pixel
	BEGIN_SELOBJ(cpDC,erase?GetSysColorBrush(COLOR_3DFACE):CreateSolidBrush(ptColor),cpDC_brush);
	//Draw the rectangle
	Rectangle(cpDC,EXPANDRECT_C(pickerRct));
	//Free the brush and the pen (in each case the combination stock object/created object is different)
	if(erase)
	{
		END_SELOBJ(cpDC,cpDC_brush);
		DeletePen(END_SELOBJ(cpDC,cpDC_pen));
	}
	else
	{
		DeleteBrush(END_SELOBJ(cpDC,cpDC_brush));
		END_SELOBJ(cpDC,cpDC_pen);
	}
	//Release the DC to the color pane window
	ReleaseDC(cpWnd, cpDC);
}
//Returns the compDC
HDC GetDSCompDC()
{
	return dsUpdater->GetDibSection()->GetCompDC();
}
//Tool classes
#pragma warning (push)
#pragma warning (disable:4100)
//UIBaseTool
//Adds the button to the toolbar and its image to the ImageList; overridable by inheriters
void UIBaseTool::AddToolbarButton(HWND Toolbar)
{
	TBBUTTON tbbtn={0};
	HIMAGELIST imgList = Toolbar_GetImageList(Toolbar);
	if(imgList==NULL)
		throw std::invalid_argument(ERROR_STD_PROLOG "Toolbar must be a toolbar with an image list attached.");
	HICON tIcon;
	int iconIndex;
	tIcon=(HICON)LoadImage(hCurInstance,MAKEINTRESOURCE(iconID),IMAGE_ICON,0,0,0);
	iconIndex=ImageList_AddIcon(imgList,tIcon);
	DestroyIcon(tIcon);
	if(iconIndex==-1)
		throw std::logic_error(ERROR_STD_PROLOG "Cannot add the icon to the image list.");
	tbbtn.idCommand=toolID;
	tbbtn.iBitmap=iconIndex;
	tbbtn.fsStyle=BTNS_CHECKGROUP;
	tbbtn.fsState=TBSTATE_ENABLED;
	if(!Toolbar_AddButtons(Toolbar,1,&tbbtn))
		throw std::logic_error(ERROR_STD_PROLOG "Cannot add the button to the toolbar."); 
}
//Eventually reverts to the previously saved tool
void UIBaseTool::RevertToPreviousTool()
{
	if(parent!=NULL)
		parent->RevertToPreviousTool();
};
MessageReturnValue UIBaseTool::OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	MessageReturnValue ret;
	int modifierKeys=GetModifierKeysState();
	if(codeHitTest==HTCLIENT)
	{
		HCURSOR cursor=GetToolCursor(modifierKeys,codeHitTest);
		if(cursor!=NULL)
		{
			SetCursor(cursor);
			ret=(LRESULT)TRUE;
		}
		else
		{
			ret=false;
		}
	}
	else
		ret=false;
	AfterSetCursor(modifierKeys,codeHitTest);
	return ret;
};
//UIDraggingTool
void UIDraggingTool::prepareVertexes(int secondX, int secondY, UINT keyFlags)
{
	//The vertexes are computed basing on positions
	last_firstVertex=firstPos;
	last_secondVertex.x=secondX;
	last_secondVertex.y=secondY;
	//Eventual other computations
	VertexesHook(last_firstVertex, last_secondVertex, keyFlags);
	//If the user presses CTRL he wants to consider the first position as the center of the shape
	if(keyFlags & MK_CONTROL) //firstPt is the center of the shape, not the upper-left corner; change it to the upper-left corner
	{
		SIZE objSize={abs(last_firstVertex.x-last_secondVertex.x),abs(last_firstVertex.y-last_secondVertex.y)};
		last_firstVertex.x=last_secondVertex.x+objSize.cx*2*((last_firstVertex.x<last_secondVertex.x)?-1:1);
		last_firstVertex.y=last_secondVertex.y+objSize.cy*2*((last_firstVertex.y<last_secondVertex.y)?-1:1);
	}
}
void UIDraggingTool::eraseLastShape()
{
	HDC IVWDC = parent->GetIVWndDC();
	//Save DC state
	int dcs = SaveDC(IVWDC);
	//We need R2_NOT to erase the previously drawn shapes
	SetROP2(IVWDC,R2_NOT);
	//Get a null brush
	SelectBrush(IVWDC,GetStockBrush(NULL_BRUSH));
	//Move to the first vertex
	MoveToEx(IVWDC,last_firstVertex.x,last_firstVertex.y,NULL);
	//Get a black cosmetic pen
	SelectPen(IVWDC,GetStockPen(BLACK_PEN));
	if(last_secondVertex.x!=INT_MIN)
	{
		//We have to erase the last shape
		EraseOutlinedShape(IVWDC,last_firstVertex,last_secondVertex);
		//Flag that the shape has already been erased
		last_secondVertex.x = INT_MIN;
	}
	//Restore the previous DC state
	RestoreDC(IVWDC,dcs);
}
MessageReturnValue UIDraggingTool::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	//Start the dragging
	//Init the fields
	firstPos.x = x;
	firstPos.y = y;
	last_secondVertex.x = INT_MIN;
	cancel=false;
	//Capture the mouse input
	SetCapture(hIVWindow);
	//Notify the event
	DraggingStarted(firstPos, keyFlags);
	return true;
}
MessageReturnValue UIDraggingTool::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	//If we are aborting or the left mouse button is not pressed return
	if(cancel || !(keyFlags & MK_LBUTTON))
		return true;
	HDC IVWDC = parent->GetIVWndDC();
	//Save DC state
	int dcs = SaveDC(IVWDC);
	//Setup the clipping region of IVWDC
	SetupClippingRegion();
	//Erase the last shape
	eraseLastShape();
	//Prepare the vertexes
	prepareVertexes(x,y, keyFlags);
	//Eventual other computations
	VertexesHook(last_firstVertex, last_secondVertex, keyFlags);
	//We need R2_NOT to erase the previously drawn shapes
	SetROP2(IVWDC,R2_NOT);
	//Get a black cosmetic pen
	SelectPen(IVWDC,GetStockPen(BLACK_PEN));
	//Get a null brush
	SelectBrush(IVWDC,GetStockBrush(NULL_BRUSH));
	//Move to the first vertex
	MoveToEx(IVWDC,last_firstVertex.x,last_firstVertex.y,NULL);
	//Outline the temp shape
	OutlineShape(IVWDC,last_firstVertex,last_secondVertex,keyFlags);
	//Restore the previous DC state
	RestoreDC(IVWDC,dcs);
	return true;
}
MessageReturnValue UIDraggingTool::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	//If we are aborting return
	if(cancel)
		return true;
	HDC IVWDC = parent->GetIVWndDC();
	//Save DC state
	int dcs = SaveDC(IVWDC);
	//Setup the clipping region of IVWDC
	SetupClippingRegion();
	//Erase the last shape
	eraseLastShape();
	//Restore the DC state
	RestoreDC(IVWDC,dcs);
	//Prepare the vertexes
	prepareVertexes(x,y,keyFlags);
	//Perform the actual drawing
	PerformAction(last_firstVertex,last_secondVertex,keyFlags);
	//Release the capture
	ReleaseCapture();
	//Set the cancel flag to avoid problems if there is a WM_MOUSEMOVE with MK_LBUTTON without a WM_LBUTTONDOWN (the operations will be ignored)
	cancel=true;
	return true;
}
MessageReturnValue UIDraggingTool::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT KeyFlags)
{
	//Abort the dragging
	cancel=true;
	//Erase the last shape
	eraseLastShape();
	//Release the capture
	ReleaseCapture();
	//Nofity the event
	DraggingCanceled(last_firstVertex,last_secondVertex,KeyFlags);
	return true;
}
//UIGenShapeTool
void UIGenShapeTool::PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags)
{
	HDC dscdc = parent->GetDibSectionDC();
	//Save DC settings
	int dcs = SaveDC(dscdc);
	//Prepare the DC
	fgColor.ApplyACUPs(dscdc);
	SelectPen(dscdc,fgColor.GetPen());
	SelectBrush(dscdc,fillFlag?bgColor.GetBrush():GetStockBrush(NULL_BRUSH));
	//Convert the vertexes to CDC coords
	IVWnd2DIBPoint(&FirstVertex);
	IVWnd2DIBPoint(&LastVertex);
	//Save the Undo image
	SetNewUndo();
	//Draw the shape
	MoveToEx(dscdc,FirstVertex.x, FirstVertex.y, NULL);
	DrawShape(dscdc,FirstVertex,LastVertex);
	//Restore DC settings
	RestoreDC(dscdc,dcs);
	//There are no temp shapes drawn
	last_secondVertex.x=INT_MIN;
	//Refresh
	UpdateResampledImage();
}
//UIShapeTool
void UIShapeTool::VertexesHook(POINT &FirstVertex, POINT &LastVertex, UINT KeyFlags)
{
	//If the user presses Shift costrain the line to be hortogonal or 45° tilted to the borders
	if(KeyFlags & MK_SHIFT)
		MakeEqualMeasures(LastVertex,FirstVertex);
}
//UILinelikeTool
void UILinelikeTool::VertexesHook(POINT &FirstVertex, POINT &LastVertex, UINT KeyFlags)
{
	//If the user presses Shift costrain the line to be hortogonal or 45° tilted to the borders
	if(KeyFlags & MK_SHIFT)
		CostrainHortOr45Line(LastVertex,FirstVertex);
}
//Real tools
//UIPaintlikeTool
void UIPaintlikeTool::drawSegment(POINT firstVertex, POINT secondVertex, ColorUtils * usedPenSettings, bool useSecondaryPenSettings, bool drawOnScreen)
{
	//Put together DrawSegment's parameters
	HDC hDC;
	int dcs;
	if(drawOnScreen)
	{
		hDC=parent->GetIVWndDC();
	}
	else
	{
		hDC=parent->GetDibSectionDC();
		//If we draw on the DibSection we need to convert the coords
		IVWnd2DIBPoint(&firstVertex);
		IVWnd2DIBPoint(&secondVertex);
	}
	//Save the DC
	dcs=SaveDC(hDC);
	//Apply settings
	usedPenSettings->ApplyACUPs(hDC);
	SelectPen(hDC,usedPenSettings->GetPen());
	MoveToEx(hDC,firstVertex.x,firstVertex.y,NULL);
	//Call DrawSegment
	DrawSegment(hDC,firstVertex,secondVertex,useSecondaryPenSettings);
	//Restore the DC
	RestoreDC(hDC,dcs);
}
bool UIPaintlikeTool::handleDrawing(POINT secondVertex, bool useSecondaryPenSettings)
{
	//If secondayPenSettings is NULL and we should use it returns (no drawing performed)
	if(useSecondaryPenSettings && secondaryPenSettings==NULL)
	{
		//No drawing is taking place
		return false;
	}
	//Set the first vertex of the segment to the second vertex of the last drawn segment
	POINT firstVertex=lastVertex;
	//If it is not valid return
	if(firstVertex.x==INT_MIN)
		return false;
	//Notify the beginning of the drawing
	BeforeDrawing(firstVertex,secondVertex,useSecondaryPenSettings);
	//Get the correct ColorUtils
	ColorUtils * usedPenSettings=useSecondaryPenSettings?secondaryPenSettings:primaryPenSettings;
	//Draw the segment on the DibSection
    drawSegment(firstVertex,secondVertex,usedPenSettings,useSecondaryPenSettings,false);
	if(IsRefreshNeeded(firstVertex,secondVertex,useSecondaryPenSettings))
	{
		//Eventually refresh...
		RECT refreshRect;
		int addedTolerance=(int)ceil(usedPenSettings->GetPenWidth()*GetZoom()/100.0+10);
		refreshRect=MakeRect(firstVertex,secondVertex);
		InflateRect(&refreshRect,addedTolerance,addedTolerance);
		InvalidateRect(hIVWindow,&refreshRect,FALSE);
	}
	else
	{
		//... otherwise draw the segment also on the screen
		drawSegment(firstVertex,secondVertex,usedPenSettings,useSecondaryPenSettings,true);
	}
	//Notify the end of the drawing
	AfterDrawing(firstVertex,secondVertex,useSecondaryPenSettings);
	return true;
}
MessageReturnValue UIPaintlikeTool::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if(onDraggingStart(x,y,keyFlags))
		handleDrawing(lastVertex,false);
	return true;
}
MessageReturnValue UIPaintlikeTool::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	//If we haven't secondaryPenSettings return
	if(secondaryPenSettings == NULL)
		return true;
	if(onDraggingStart(x,y,keyFlags))
		handleDrawing(lastVertex,true);
	return true;
}
MessageReturnValue UIPaintlikeTool::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	if(onDraggingEnding(x,y,keyFlags))
	{
		POINT curPos={x,y};
		handleDrawing(curPos,false);
		onDraggingEnded(x,y,keyFlags);
	}
	return true;
}
MessageReturnValue UIPaintlikeTool::OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	//If we haven't secondaryPenSettings return
	if(secondaryPenSettings == NULL)
		return true;
	if(onDraggingEnding(x,y,keyFlags))
	{
		POINT curPos={x,y};
		handleDrawing(curPos,true);
		onDraggingEnded(x,y,keyFlags);
	}
	return true;
}
MessageReturnValue UIPaintlikeTool::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if(keyFlags&MK_LBUTTON||keyFlags&MK_RBUTTON)
	{
		POINT curPos={x,y};
		//Simulate a WM_SETCURSOR to update the precision cursors position
		SimMM(hwnd,false,true,false);
		//Note: if both buttons are pressed the left button wins
		if(handleDrawing(curPos,(keyFlags&MK_LBUTTON)==0))
            lastVertex=curPos;
		else
			lastVertex.x=INT_MIN;
	}
	return true;
}
bool UIPaintlikeTool::onDraggingStart(int x, int y, UINT KeyFlags)
{
	//These operations are executed only if a dragging is not already taking place
	if(lastVertex.x==INT_MIN)
	{
		//Call the public event handler
		if(!OnDraggingStart(x,y,KeyFlags))
			return false;
		//Set the last vertex to the current position, so a point will be drawn
		lastVertex.x=x;
		lastVertex.y=y;
		//Set a new undo
		SetNewUndo();
		//Hide the cursor
		ShowCursor(FALSE);
		//Disable the resample option
		DisableResample();
		//Simulate a WM_SETCURSOR to update the cursors
		SimMM(hIVWindow,false,true,false);
		//Show the cursor
		ShowCursor(TRUE);
		//Capture the mouse input
		SetCapture(hIVWindow);
		return true;
	}
	return false;
}
bool UIPaintlikeTool::onDraggingEnding(int x, int y, UINT KeyFlags)
{
	//The cleanup operations are executed only if a dragging is taking place and it's ending
	//(both the left and the right mouse buttons mustn't be pressed)
	if(lastVertex.x!=INT_MIN && !(KeyFlags & (MK_RBUTTON|MK_LBUTTON)))
	{
		//Call the public event handler
		if(!OnDraggingEnding(x,y,KeyFlags))
			return false;
		return true;
	}
	return false;
}
void UIPaintlikeTool::onDraggingEnded(int x, int y, UINT KeyFlags)
{
	//Call the public event handler
	OnDraggingEnded(x,y,KeyFlags);
	//Record that the dragging is ended
	lastVertex.x=INT_MIN;
	//Re-enable the resample option (if it was enabled before BeginDrawing) and refresh the IVWND
	ReenableResample();
	//Simulate a WM_SETCURSOR to update the cursors
	SimMM(hIVWindow,false,true,false);
	//Release the capture
	ReleaseCapture();
}
//Update the position and the shape of the precision cursor
void UIPaintlikeTool::AfterSetCursor(int modifierKeys, UINT codeHitTest)
{
	if(codeHitTest==HTCLIENT)
	{
		POINT mp;
		ColorUtils * usedCU;
		GetCursorPos(&mp);
		ScreenToClient(hIVWindow,&mp);
		IVWnd2DIBPoint(&mp);
		normPC.SetPosition(mp,false);
		if((modifierKeys&(MK_LBUTTON|MK_RBUTTON))==MK_RBUTTON&&secondaryPenSettings!=NULL)
			usedCU=secondaryPenSettings;
		else	
			usedCU=primaryPenSettings;
		normPC.SetShape(usedCU->GetPenSettings().dwPenStyle&PS_ENDCAP_MASK,true);
		normPC.Show();
	}
	else
		normPC.Hide();
}
//UIEraser
void UIEraser::DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings)
{
	if(!useSecondaryPenSettings)
		LineTo(hDC,secondVertex.x,secondVertex.y);
	else
		AdvEr.EraseLine(dsUpdater->GetDibSection(),firstVertex,secondVertex,(HPEN)GetCurrentObject(hDC,OBJ_PEN));
}
bool UIEraser::IsRefreshNeeded(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
{
	if(secondaryPenSettingsUsed)
		return true;
	else
		return GetZoom()!=100;
}
//UIColorReplacer
void UIColorReplacer::DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings)
{
	ColorUtils * fgColor=useSecondaryPenSettings?secondaryPenSettings:primaryPenSettings;
	ColorUtils * bgColor=useSecondaryPenSettings?primaryPenSettings:secondaryPenSettings;
	ColRep.ReplaceColorOnLine(dsUpdater->GetDibSection(),firstVertex,secondVertex,fgColor->GetPen(),fgColor->GetColor(),bgColor->GetColor(),threshold);
}

//UIClone
void UIClone::DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings)
{
	ClTool.CloneLine(dsUpdater->GetDibSection(),firstVertex,secondVertex,(useSecondaryPenSettings?secondaryPenSettings:primaryPenSettings)->GetPen(),shift);
}
bool UIClone::OnDraggingStart(int x, int y, UINT KeyFlags)
{
	if(shift.x!=INT_MIN)
		return true;
	else if(srcPt.x!=INT_MIN)
	{
		POINT t_curPos={x,y};
		IVWnd2DIBPoint(&t_curPos);
		shift.x=srcPt.x-t_curPos.x;
		shift.y=srcPt.y-t_curPos.y;
		return true;
	}
	else
	{
		ErrMsgBox(hMainWindow,IDS_ERR_NOCLONESRC);
		return false;
	}
}
MessageReturnValue UIClone::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	//Store the source position
	srcPt.x=x;
	srcPt.y=y;
	//Convert it to DIB coords
	IVWnd2DIBPoint(&srcPt);
	//Mark the shift for recomputing
	shift.x=INT_MIN;
	return true;
}
//Update the position and the shape of the precision cursor
void UIClone::AfterSetCursor(int modifierKeys, UINT codeHitTest)
{
	UIPaintlikeTool::AfterSetCursor(modifierKeys,codeHitTest);
	if(shift.x!=INT_MIN && codeHitTest==HTCLIENT)
	{
		POINT npcPos=normPC.GetPosition();
		//IVWnd2DIBPoint(&npcPos);
		POINT clsrPCPos={npcPos.x+shift.x,npcPos.y+shift.y};
		//DIB2IVWndPoint(&clsrPCPos);
		clsrPC.SetPosition(clsrPCPos);
		clsrPC.SetShape(normPC.GetShape());
		clsrPC.Show();
	}
	else
		clsrPC.Hide();
}
//UIFloodFill
//Performs the floodfill
bool UIFloodFill::PerformAction(POINT Position, UINT KeyFlags, bool Right)
{
	//Convert the position
	IVWnd2DIBPoint(&Position);
	//Check if we are in a valid area of the DIB
	try
	{
		dsUpdater->GetDibSection()->ArgsInRange(Position.x,Position.y);
	}
	catch(std::out_of_range)
	{
		return true;
	}
	//Choose the right ColorUtils basing on the mouse button
	ColorUtils & cu=Right?bgColor:fgColor;
	//Set a new undo
	SetNewUndo();
	//Fill
	//Apply the ACUPs
	cu.ApplyACUPs(dsUpdater->GetDibSection()->GetCompDC());
	//Setup the clipping region
	SetupClippingRegion();
	//Prepare the user for the wait
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	//Do the floodfill
	AdvFloodFill(dsUpdater->GetDibSection(),Position,cu.GetBrush(),threshold);
	//Refresh
	UpdateResampledImage();
	return true;
}
//UIInsertText
bool UIInsertText::PerformAction(POINT Position, UINT KeyFlags, bool Right)
{
	if(!Right)
	{
		//Convert the position to DibSection coords
		IVWnd2DIBPoint(&Position);
		//Insert the text
		EnableWindow(hToolBoxWindow,FALSE);
		ShowInsertTextDialog(hMainWindow,Position);
		EnableWindow(hToolBoxWindow,TRUE);
		//Refresh
		UpdateResampledImage();
	}
	return true;
}
//UIPicker
bool UIPicker::PerformAction(POINT Position, UINT KeyFlags, bool Right)
{
	//Choose the right ColorUtils basing on the mouse button
	ColorUtils & cu=Right?bgColor:fgColor;
	//New color
	COLORREF newColor;
	//Get the new color and set it to the ColorUtils
	IVWnd2DIBPoint(&Position);
	newColor=GetPixel(parent->GetDibSectionDC(),Position.x,Position.y);
	if(newColor!=CLR_INVALID)
		cu.SetColor(newColor);
	//Update the color pane
	InvalidateRect(GetDlgItem(hToolBoxWindow,IDC_TB_COLORPAL),NULL,TRUE);
	UpdateWindow(GetDlgItem(hToolBoxWindow,IDC_TB_COLORPAL));
	return true;
}
MessageReturnValue UIPicker::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	//Eventually revert to the previous tool
	RevertToPreviousTool();
	return true;
}
MessageReturnValue UIPicker::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	POINTS pos={(SHORT)x,(SHORT)y};
	DrawPickerCPane(pos);
	return true;
}
//UILine
void UILine::DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
{
	LineTo(hDC, LastVertex.x, LastVertex.y);
}
//UIArrowline
void UIArrowline::DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
{
	ArrowSettings tas=arrowSettings;
	//If we are writing on IVWDC scale correctly the arrow
	if(hDC==parent->GetIVWndDC())
	{
		tas.arrowBase=(int)(tas.arrowBase*GetZoom()/100.0f);
		tas.arrowLength=(int)(tas.arrowLength*GetZoom()/100.0f);
	}
	ArrowTo(hDC, LastVertex.x, LastVertex.y, &tas);
}
MessageReturnValue UIArrowline::OnShowOptionsRequest(HWND hParent)
{
	ShowArrowSettings(hParent,arrowSettings);
	return (LRESULT)TRUE;
}
void UIArrowline::SaveSettings(INISection & IniSect)
{
	//arrowSettings - written in encoded form
	IniSect.PutKey(_T("arrowSettings"),&arrowSettings,sizeof(arrowSettings));
}
void UIArrowline::LoadSettings(INISection & IniSect)
{
	if(IniSect.KeyExists(_T("arrowSettings")))
		IniSect.GetKey(_T("arrowSettings"),&arrowSettings,sizeof(arrowSettings));
	else
	{
		//Set the default settings for the arrows
		arrowSettings.arrowBase=20;
		arrowSettings.arrowLength=20;
		arrowSettings.drawFirstArrow=false;
		arrowSettings.drawSecondArrow=true;
		arrowSettings.openArrowHead=false;
	}
	//These values aren't really invalid, but they mess up the arrowsettings dialog
	if(arrowSettings.arrowBase == 0)
		arrowSettings.arrowBase=1;
	if(arrowSettings.arrowLength == 0)
		arrowSettings.arrowLength=1;
}
//UIMeasure
void UIMeasure::DraggingCanceled(POINT FirstVertex, POINT LastVertex, UINT KeyFlags)
{
	//Set the status bar to normal state
	Status_Simple(parent->GetStatusBar(),FALSE);
	//Call the base implementation
	UILinelikeTool::DraggingCanceled(FirstVertex,LastVertex,KeyFlags);
}
void UIMeasure::DraggingStarted(POINT & FirstPos, UINT KeyFlags)
{
	//Set the status bar to "simple" state
	Status_Simple(parent->GetStatusBar(),TRUE);
	//Call the base implementation
	UILinelikeTool::DraggingStarted(FirstPos,KeyFlags);
}
void UIMeasure::OutlineShape(HDC hDC, POINT FirstVertex, POINT LastVertex, UINT KeyFlags)
{
	//Call the base implementation
	UILinelikeTool::OutlineShape(hDC,FirstVertex,LastVertex,KeyFlags);
	//Convert IVW points in DIB points
	IVWnd2DIBPoint(&FirstVertex);
	IVWnd2DIBPoint(&LastVertex);
	//Get the string
	std::_tcstring outStr=measurementUnitsContainer.GetDistanceString(*dsUpdater->GetDibSection(),FirstVertex,LastVertex);
	ReplaceString(outStr,_T("\r\n"),_T("; "));
	//Set the text in the status bar
	Status_SetText(parent->GetStatusBar(),SB_SIMPLEID,0,outStr.c_str());
}
//Temp shape drawing
void UIMeasure::DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
{
	LineTo(hDC,LastVertex.x,LastVertex.y);
}
//Actual work
void UIMeasure::PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags)
{
	//Set the status bar to normal state
	Status_Simple(parent->GetStatusBar(),FALSE);
	InvalidateRect(hIVWindow,NULL,FALSE);
	//Convert IVW points in DIB points
	IVWnd2DIBPoint(&FirstVertex);
	IVWnd2DIBPoint(&LastVertex);
	//Measure and show the output
	ShowMeasureOutput(hMainWindow,measurementUnitsContainer,FirstVertex,LastVertex,*dsUpdater->GetDibSection());
}
MessageReturnValue UIMeasure::OnShowOptionsRequest(HWND hParent)
{
	ShowMeasurementUnits(hParent,measurementUnitsContainer);
	return (LRESULT)TRUE;
}
void UIMeasure::SaveSettings(INISection & IniSect)
{
	measurementUnitsContainer.SerializeToINI(IniSect);
}
void UIMeasure::LoadSettings(INISection & IniSect)
{
	measurementUnitsContainer.DeSerializeFromINI(IniSect);
}
//UIStraighten
//Temp shape drawing
void UIStraighten::DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
{
	LineTo(hDC,LastVertex.x,LastVertex.y);
}
//Actual work
void UIStraighten::PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags)
{
	//Calculate the angle
	float angle=(float)(-atan(((double)(LastVertex.y-FirstVertex.y))/((double)(LastVertex.x-FirstVertex.x)))*(180/M_PI));
	if(abs(angle>45))
		angle=angle-90;
	else if(angle<-45)
		angle=angle+90;
	LPARAM lp;
	//Convert it to an LPARAM
	lp=ConvertType<LPARAM>(angle);
	try
	{
		//Tell to IV to rotate the image (no need to refresh, IV does it automatically)
		SendMessage(hMainWindow,WM_ROTATE_IMAGE,COLORREF2DWORD(bgColor.GetColor(),dsUpdater->GetDibSection()->GetPalette()),lp);
	}
	catch(exception &ex) //Quite impossible, but may happen
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__));
	}
}
//UIRectangle
void UIRectangle::DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
{
	Rectangle(hDC,FirstVertex.x,FirstVertex.y,LastVertex.x,LastVertex.y);
}
//UICircle
void UICircle::DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
{
	Ellipse(hDC,FirstVertex.x,FirstVertex.y,LastVertex.x,LastVertex.y);
}
#pragma warning (pop)
//UIToolsContainer
void UIToolsContainer::SetCurrentToolID(unsigned int CurrentToolID, bool CanRevert, bool Throw)
{
	UITCmap::const_iterator elem = tools.find(CurrentToolID);
	if(elem==tools.end() || elem->second==NULL)
	{
		if(Throw)
			throw std::invalid_argument(ERROR_STD_PROLOG "The current tool does not exist or isn't valid.");
#ifdef _DEBUG
		else
			OutputDebugString(_T(__FUNCSIG__) _T(": Invalid tool ID."));
#endif
	}
	else
	{
		if(currentTool!=NULL)
			currentTool->OnToolDeactivating(elem->second);
		if(CanRevert)
		{
			canRevert=true;
			previousToolID=currentToolID;
		}
		elem->second->OnToolActivating(currentTool);
		currentTool = elem->second;
		currentToolID = CurrentToolID;
		SyncCurTool();
	}
}
UIToolsContainer::UIToolsContainer(PointerEx<HDC> IVWndDC, PointerEx<HDC> DibSectionDC)
	: 
	ivWndDC(IVWndDC),
	dibSectionDC(DibSectionDC),
	statusBar(FindWindowEx(hMainWindow,NULL,STATUSCLASSNAME,NULL))
{
	//Create all the UITools
	try
	{
		tools[IDC_TB_PAINTBRUSH]=new UIPaintbrush(IDC_TB_PAINTBRUSH,this,&fgColor,&bgColor);
		tools[IDC_TB_ERASER]=new UIEraser(IDC_TB_ERASER,this,&bgColor);
		tools[IDC_TB_CLONE]=new UIClone(IDC_TB_CLONE,this,&fgColor);
		tools[IDC_TB_ARROW]=new UIArrow(IDC_TB_ARROW, this);
		tools[IDC_TB_LINE]=new UILine(IDC_TB_LINE, this);
		tools[IDC_TB_ARROWLINE]=new UIArrowline(IDC_TB_ARROWLINE, this);
		tools[IDC_TB_FLOODFILL]=new UIFloodFill(IDC_TB_FLOODFILL, this);
		tools[IDC_TB_PICKER]=new UIPicker(IDC_TB_PICKER, this);
		tools[IDC_TB_ROTATE]=new UIStraighten(IDC_TB_ROTATE, this);
		tools[IDC_TB_RECTANGLE]=new UIRectangle(IDC_TB_RECTANGLE, this);
		tools[IDC_TB_CIRCLE]=new UICircle(IDC_TB_CIRCLE, this);
		tools[IDC_TB_INSERTTEXT]=new UIInsertText(IDC_TB_INSERTTEXT,this);
		tools[IDC_TB_COLORREPL]=new UIColorReplacer(IDC_TB_COLORREPL,this,&fgColor,&bgColor);
		tools[IDC_TB_MEASURE]=new UIMeasure(IDC_TB_MEASURE,this);
	}
	catch(std::bad_alloc)
	{
		//If we cannot allocate throw a more comprensible exception
		throw std::bad_alloc(ERROR_STD_PROLOG "Cannot allocate memory for the UITools objects");
	}
	//Misc
	currentTool = NULL;
	canRevert = false;
}
UIToolsContainer::~UIToolsContainer()
{
	UITCmap::const_iterator it,end=tools.end();
	//Deallocate all the objects
	for(it=tools.begin();it!=end;it++)
		delete it->second;
	//Empty the map
	tools.clear();
}
//Eventually forward the passed message to the current UITool class;
//returns true if it mustn't be passed also to the real IVWnd
MessageReturnValue UIToolsContainer::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_MOUSEMOVE:
		return BHANDLE_WM_MOUSEMOVE(hwnd,wParam,lParam,currentTool->OnMouseMove);
	case WM_LBUTTONDOWN:
		return BHANDLE_WM_LBUTTONDOWN(hwnd,wParam,lParam,currentTool->OnLButtonDown);
	case WM_LBUTTONUP:
		return BHANDLE_WM_LBUTTONUP(hwnd,wParam,lParam,currentTool->OnLButtonUp);
	case WM_LBUTTONDBLCLK:
		return BHANDLE_WM_LBUTTONDBLCLK(hwnd,wParam,lParam,currentTool->OnLButtonDown);
	case WM_RBUTTONDOWN:
		return BHANDLE_WM_RBUTTONDOWN(hwnd,wParam,lParam,currentTool->OnRButtonDown);
	case WM_RBUTTONUP:
		return BHANDLE_WM_RBUTTONUP(hwnd,wParam,lParam,currentTool->OnRButtonUp);
	case WM_RBUTTONDBLCLK:
		return BHANDLE_WM_RBUTTONDBLCLK(hwnd,wParam,lParam,currentTool->OnRButtonDown);
	case WM_MBUTTONDOWN:
		return BHANDLE_WM_MBUTTONDOWN(hwnd,wParam,lParam,currentTool->OnMButtonDown);
	case WM_MBUTTONUP:
		return BHANDLE_WM_MBUTTONUP(hwnd,wParam,lParam,currentTool->OnMButtonUp);
	case WM_MBUTTONDBLCLK:
		return BHANDLE_WM_MBUTTONDBLCLK(hwnd,wParam,lParam,currentTool->OnMButtonDown);
#ifdef UIBT_HANDLEMOUSEWHEEL
	case WM_MOUSEWHEEL:
		return BHANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,currentTool->OnMouseWheel);
#endif
	case WM_SETCURSOR:
		return BHANDLE_WM_SETCURSOR(hwnd,wParam,lParam,currentTool->OnSetCursor);
	default:
		return false;
	}
}
//Adds the UI tools buttons and icons to the given toolbar, that must have an attached image list
void UIToolsContainer::AddButtonsToToolbar(HWND Toolbar)
{
	UIToolsContainer::UITCmap::iterator it,end=tools.end();
	for(it=tools.begin();it!=end;it++)
		it->second->AddToolbarButton(Toolbar);
}
//Eventually reverts to the previously saved tool
void UIToolsContainer::RevertToPreviousTool()
{
	if(canRevert)
	{
		canRevert=false;
		SetCurrentToolID(previousToolID);
	}
}
//Called when a new DIB is loaded
void UIToolsContainer::OnDibSectCreate(DibSection * ds)
{
	//Notify to children
	UIToolsContainer::UITCmap::iterator it,end=tools.end();
	for(it=tools.begin();it!=end;it++)
		it->second->OnDibSectCreate(ds);
}
//Saves the settings of the container and of the tools
void UIToolsContainer::SaveSettings(INISection & IniSect)
{
	IniSect.BeginWrite();
	//Tell to the children
	UIToolsContainer::UITCmap::iterator it,end=tools.end();
	for(it=tools.begin();it!=end;it++)
		it->second->SaveSettings(IniSect);
	//Save the settings
	IniSect.PutKey(_T("currentTool"),currentToolID);
	IniSect.EndWrite();
}
//Loads the settings of the container and of the tools
void UIToolsContainer::LoadSettings(INISection & IniSect)
{
	//Load the settings
	IniSect.BeginRead();
	//CurrentTool
	try
	{
		SetCurrentToolID(IniSect.GetKey(_T("currentTool"),FIRSTTOOLBUTTON));
	}
	catch(...)
	{
		SetCurrentToolID(IDC_TB_ARROW);
	}
	//Tell to the children
	UIToolsContainer::UITCmap::iterator it,end=tools.end();
	for(it=tools.begin();it!=end;it++)
		it->second->LoadSettings(IniSect);
	IniSect.EndRead();
}