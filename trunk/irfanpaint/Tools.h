#pragma once
#include "PointerEx.h"
#include "CloneTool.h"
#include "AdvEraser.h"							//AdvEraser class
#include "ColorReplacer.h"
#include "Resource.h"
#include "ToolbarButtons.h"
#include "Globals.h"
#include "MeasurementUnit.h"
#include "MessageHooksHelpers.h"
#include "DibSectionUpdater.h"
//Draws or erases the picker color pane
void DrawPickerCPane(POINTS mousePos, bool erase=false);
//Erases the picker color pane
inline void ErasePickerCPane()
{
	POINTS dummy={0};
	DrawPickerCPane(dummy,true);
};
//Returns the compDC
HDC GetDSCompDC();
//UI Tools
//Forward declaration
class UIToolsContainer;
//Generic UI Tools
#pragma warning (push)
#pragma warning (disable:4100)
//Base class for all the UI tool classes
class UIBaseTool
{
	WORD toolID;
	WORD iconID;
	WORD tooltipID;
	WORD tooltipExID;
	void init(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent)
	{
		SetToolID(ToolID);
		iconID = IconID;
		tooltipID = TooltipID;
		tooltipExID = TooltipExID;
		parent = Parent;
		defaultRetVal = true;
	};
protected:
	//Container
	UIToolsContainer * parent;
	//Default return value for messages
	MessageReturnValue defaultRetVal;
	//Constructors
	UIBaseTool(WORD ToolID, UIToolsContainer * Parent)
	{
		init(ToolID,ToolID,ToolID,ToolID+1000, Parent);
	};
	UIBaseTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent)
	{
		init(ToolID,IconID,TooltipID,TooltipExID, Parent);
	};
	//Misc support functions
	//Eventually reverts to the previously saved tool
	void RevertToPreviousTool();
	//Virtual functions
	//Function called to get the cursor to display; return NULL if you don't want to call SetCursor
	virtual HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest){return NULL;};
	//Function called to perform additional tasks after setting the cursor
	virtual void AfterSetCursor(int modifierKeys, UINT codeHitTest){};
public:
	//Other support functions
	//Sets a new undo image
	static inline void SetNewUndo()
	{
		SendMessage(hMainWindow, WM_SET_NEW_UNDO, 0, 0);
	};
	//Updates the resampled image (if resample option is enabled) and refresh the IVWND
	static inline void UpdateResampledImage()
	{
		//Update the resampled image (if resample option is enabled) and refresh the IVWND
		SendMessage(hMainWindow,WM_SET_RESAMPLE,2,0);
	};
	//Temporarily disable the resample option
	static inline void DisableResample()
	{
		SendMessage(hMainWindow,WM_SET_RESAMPLE,0,0);
	};
	//Reenable the resample option (disabled with TempDisableResample
	static inline void ReenableResample()
	{
		SendMessage(hMainWindow,WM_SET_RESAMPLE,1,0);
	};
	//Enables or disables the "extended controls" of the toolbox (i.e. the width updown and the fill checkbox)
	static inline void EnableExtCtrls(bool widthSel, bool toleranceSel, bool fillSel)
	{
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_TX_WIDTH), widthSel);
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_UD_WIDTH), widthSel);
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_LB_WIDTH), widthSel);
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_TX_TOLERANCE), toleranceSel);
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_UD_TOLERANCE), toleranceSel);
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_LB_TOLERANCE), toleranceSel);
		EnableWindow(GetDlgItem(hToolBoxWindow,IDC_TB_CH_FILL), fillSel);
		return;
	};
	//Saves the settings of the tool
	virtual void SaveSettings(INISection & IniSect){};
	//Loads the settings of the tool
	virtual void LoadSettings(INISection & IniSect){};
	//Windows messages event handlers - they return true if the message has been completely processed and must not be passed to the real IV wndproc
	//WM_MOUSEMOVE
	virtual MessageReturnValue OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags){return defaultRetVal;};
	//WM_LBUTTONDOWN/WM_LBUTTONDBLCLK
	virtual MessageReturnValue OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags){return defaultRetVal;};
	//WM_LBUTTONUP
	virtual MessageReturnValue OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags){return defaultRetVal;};
	//WM_RBUTTONDOWN/WM_RBUTTONDBLCLK
	virtual MessageReturnValue OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags){return defaultRetVal;};
	//WM_RBUTTONUP
	virtual MessageReturnValue OnRButtonUp(HWND hwnd, int x, int y, UINT flags){return defaultRetVal;};
	//WM_MBUTTONDOWN/WM_MBUTTONDBLCLK
	virtual MessageReturnValue OnMButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags){return defaultRetVal;};
	//WM_MBUTTONUP
	virtual MessageReturnValue OnMButtonUp(HWND hwnd, int x, int y, UINT flags){return defaultRetVal;};
#ifdef UIBT_HANDLEMOUSEWHEEL
	//WM_MOUSEWHEEL
	virtual MessageReturnValue OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys){return defaultRetVal;};
#endif
	//WM_SETCURSOR
	virtual MessageReturnValue OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
	//Other event handlers
	//Raised when the tool is being activated (e.g. with a click on the toolbar)
	virtual void OnToolActivating(UIBaseTool * prevActiveTool) {EnableExtCtrls(false,false,false);};
	//Raised when the tool is being deactivated (when another one is being activated)
	virtual void OnToolDeactivating(UIBaseTool * newActiveTool) {return;};
	//Raised when a new DIB is loaded
	virtual void OnDibSectCreate(DibSection * ds) {return;};
	//Raised when the user right-clicks the toolbar button; for the return value see NM_RCLICK
	virtual MessageReturnValue OnShowOptionsRequest(HWND hParent) {return false;};
	//Getters/setters
	WORD GetToolID()
	{
		return toolID;
	};
	void SetToolID(WORD ToolID)
	{
		if(!ToolID)
			throw std::invalid_argument(ERROR_STD_PROLOG "ToolID must be a nonzero value");
		else
			toolID=ToolID;
	};
	WORD GetIconID()
	{
		return iconID;
	};
	void SetIconID(WORD IconID)
	{
		iconID=IconID;
	};
	WORD GetTooltipID()
	{
		return tooltipID;
	};
	void SetTooltipID(WORD TooltipID)
	{
		tooltipID=TooltipID;
	};
	WORD GetTooltipExID()
	{
		return tooltipExID;
	};
	void SetTooltipExID(WORD TooltipExID)
	{
		tooltipExID=TooltipExID;
	};
	//Adds the button to the toolbar and its image to the ImageList; overridable by inheriters
	virtual void AddToolbarButton(HWND Toolbar);
	//Virtual destructor
	virtual ~UIBaseTool(){return;};
};
//Generic single-click tool
class UISingleClickTool : public UIBaseTool
{
protected:
	//Constructors
	UISingleClickTool(WORD ToolID, UIToolsContainer * Parent):UIBaseTool(ToolID, Parent){};
	UISingleClickTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIBaseTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
	//Pure virtual functions
	//Action performed on clicks; position is the poisiton of the click, right is true if the user clicked with the right button
	//Return true if the message has been completely processed and must not be passed to the real IV wndproc
	virtual bool PerformAction(POINT Position, UINT KeyFlags, bool Right)=0;
public:
#pragma warning (push)
#pragma warning (disable:4100)
	MessageReturnValue OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		POINT pos={x,y};
		return PerformAction(pos,keyFlags,false);
	};
	MessageReturnValue OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		POINT pos={x,y};
		return PerformAction(pos,keyFlags,true);
	};
#pragma warning (pop)
};
//Generic dragging tool (the user need to drag and release to perform an action)
class UIDraggingTool : public UIBaseTool
{
private:
	//Helpers
	//Prepares last_firstVertex and last_secondVertex
	void prepareVertexes(int secondX, int secondY, UINT keyFlags);
	//Erases the last shape
	void eraseLastShape();
protected:
	//First position
	POINT firstPos;
	//Vertexes of the last drawn shape
	POINT last_firstVertex;
	POINT last_secondVertex;
	/*
	Important note: a vertex (.*Vertex) is not a position (.*Pos):
	a vertex is the actual vertex of the polygon or line drawn, while a position
	is just the place where the user put the mouse cursor; in other words, the 
	position is raw data (it still has to be eventually costrained to 45°/...),
	while the vertex is already computed.
	*/
	//Flag that indicates to cancel the operation
	bool cancel;
	//Constructors
	UIDraggingTool(WORD ToolID, UIToolsContainer * Parent):UIBaseTool(ToolID, Parent){};
	UIDraggingTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIBaseTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
	//Pure virtual functions
	//Action performed when the dragging is completed
	virtual void PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags)=0;
	//Function called by the default implementation of OutlineShape and EraseOutlinedShape
	//to draw/erase (with R2_NOT ROP) the temporary shapes. You do not need to implement
	//this if you redefine OutlineShape and EraseOutlinedShape so that they do not call
	//DrawShape.
	virtual void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex)=0;
#pragma warning (push)
#pragma warning (disable:4100)
	//Virtual functions
	//Function called when is needed to outline the shape during the dragging;
	//hDC of both the functions is already set to R2_NOT drawing, black stock cosmetic pen,
	//null brush and position of FirstVertex. The default implementation of these two 
	//functions simply calls DrawShape.
	virtual void OutlineShape(HDC hDC, POINT FirstVertex, POINT LastVertex, UINT KeyFlags)
	{
		DrawShape(hDC, FirstVertex, LastVertex);
	};
	//See OutlineShape
	virtual void EraseOutlinedShape(HDC hDC, POINT FirstVertex, POINT LastVertex)
	{
		DrawShape(hDC, FirstVertex, LastVertex);
	};
	//Redefine this function to change vertexes values before the call to OutlineShape
	virtual void VertexesHook(POINT & FirstVertex, POINT & LastVertex, UINT KeyFlags){};
	//Action performed when the dragging is canceled (shape erasing is already done)
	virtual void DraggingCanceled(POINT FirstVertex, POINT LastVertex, UINT KeyFlags){};
	//Action performed when the dragging begins; here you can edit the first point coords
	virtual void DraggingStarted(POINT & FirstPos, UINT KeyFlags){};
#pragma warning (pop)
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(NULL,IDC_CROSS);
	};
public:
	//Event handlers
	MessageReturnValue OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	MessageReturnValue OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	MessageReturnValue OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	MessageReturnValue OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
};
//Generic shape tool (shapes/lines)
class UIGenShapeTool : public UIDraggingTool
{
	/*
	Note: DrawShape of class derived from UIGenShapeTool will also be used
	to draw the last (definitive) shape on the CDC of the DIB
	*/
	//See UIDraggingTool
	void PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags);
public:
	UIGenShapeTool(WORD ToolID, UIToolsContainer * Parent):UIDraggingTool(ToolID, Parent){};
	UIGenShapeTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIDraggingTool(ToolID,IconID,TooltipID,TooltipExID, Parent){};

};
//Shape tool (actual shapes)
class UIShapeTool : public UIGenShapeTool
{
	//See UIGenShapeTool
	void VertexesHook(POINT & FirstVertex, POINT & LastVertex, UINT KeyFlags);
	void OnToolActivating(UIBaseTool * prevActiveTool)
	{
		EnableExtCtrls(true,false,true);
	};
public:
	UIShapeTool(WORD ToolID, UIToolsContainer * Parent):UIGenShapeTool(ToolID, Parent){};
	UIShapeTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIGenShapeTool(ToolID,IconID,TooltipID,TooltipExID, Parent){};
};
//Linelike tools
class UILinelikeTool : public UIGenShapeTool
{
	//See UIGenShapeTool
	void VertexesHook(POINT & FirstVertex, POINT & LastVertex, UINT KeyFlags);
	void OnToolActivating(UIBaseTool * prevActiveTool)
	{
		EnableExtCtrls(true,false,false);
	};
public:
	UILinelikeTool(WORD ToolID, UIToolsContainer * Parent):UIGenShapeTool(ToolID, Parent){};
	UILinelikeTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIGenShapeTool(ToolID,IconID,TooltipID,TooltipExID, Parent){};
};
//Real tools
//Generic (yet nonvirtual) paintbrush-like tool
class UIPaintlikeTool : public UIBaseTool
{
	//Common initialization routine
	void init(ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings)
	{
		lastVertex.x = INT_MIN;
		SetPrimaryPenSettings(PrimaryPenSettings);
		SetSecondaryPenSettings(SecondaryPenSettings);
	};
	//Helpers
	//Prepares the DC, inits things and calls DrawSegment
	void drawSegment(POINT firstVertex, POINT secondVertex, ColorUtils * usedPenSettings, bool useSecondaryPenSettings, bool drawOnScreen);
	//Calls drawSegment to draw the segment on the DibSection and eventually on the screen; returns false if no drawing is taking place
	bool handleDrawing(POINT secondVertex, bool useSecondaryPenSettings);
	//Event handlers
	//Private version of OnDraggingStart; these operations are always performed
	bool onDraggingStart(int x, int y, UINT KeyFlags);
	//Private version of OnDraggingEnding; these operations are always performed
	bool onDraggingEnding(int x, int y, UINT KeyFlags);
	//Private version of OnDraggingEnded; these operations are always performed
	void onDraggingEnded(int x, int y, UINT KeyFlags);
protected:
	//Colorutils used for primary painting (i.e. left click); can't be NULL
	ColorUtils * primaryPenSettings;
	//Colorutils used for secondary painting (i.e. right click); can be NULL
	//If secondaryPenSettings is NULL no drawing will be performed on right mouse clicks
	ColorUtils * secondaryPenSettings;
	//Redefine the standard OnToolActivating enabling the right ExtCtrls and the precision cursor
	void OnToolActivating(UIBaseTool * prevActiveTool)
	{
		normPC.Show();
		EnableExtCtrls(true,false,false);
	};
	void OnToolDeactivating(UIBaseTool * newActiveTool)
	{
		normPC.Hide();
	};
	//Update the position and the shape of the precision cursor
	void AfterSetCursor(int modifierKeys, UINT codeHitTest);
	//Coords of the last vertex drawn (if lastVertex.x == INT_MIN no drawing is taking place)
	POINT lastVertex;
	//Virtual functions
	//Function called to know if it's needed to refresh the IV window
	virtual bool IsRefreshNeeded(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		return GetZoom()!=100;
	};
	//Function called to draw the segment with the given first and second points on the given DC
	//In hDC is already selected the right pen, are already applied the ACUPs and the position
	//is already set to firstVertex
	virtual void DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings)
	{
		ColorUtils * usedPenSettings=useSecondaryPenSettings?secondaryPenSettings:primaryPenSettings;
		if(firstVertex.x==secondVertex.x&&firstVertex.y==secondVertex.y&&(usedPenSettings->GetPenSettings().dwPenStyle&PS_ENDCAP_MASK)!=PS_ENDCAP_ROUND)
		{
			//Workaround for single-pixel drawing with non-round endcaps
			//Save the DC
			int savedDC = SaveDC(hDC);
			//Create a clipping region of the exact size of the endcap
			float halfRgnSide=usedPenSettings->GetPenSettings().dwWidth/2.0f;
			HRGN nr = CreateRectRgn((int)(secondVertex.x-floor(halfRgnSide)),(int)(secondVertex.y-floor(halfRgnSide)),(int)(secondVertex.x+ceil(halfRgnSide)),(int)(secondVertex.y+ceil(halfRgnSide)));
			//Select it
			SelectClipRgn(hDC,nr);
			//Now we can safely delete it
			DeleteRgn(nr);
			//Draw the line; the +1 is needed, otherwise no line will be drawn; anyway the exceeding pixel is not drawn thanks to the clipping region
			LineTo(hDC,firstVertex.x+1,secondVertex.y);
			//Get back to where you once belonged
			RestoreDC(dsUpdater->GetDibSection()->GetCompDC(),savedDC);
		}
		else
			LineTo(hDC,secondVertex.x,secondVertex.y);	
	};
	//Function called before drawing; firstVertex and secondVertex are in IVWindow coords
	virtual void BeforeDrawing(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		normPC.BeginRefresh();
	};
	//Function called after drawing; firstVertex and secondVertex are in IVWindow coords
	virtual void AfterDrawing(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		normPC.EndRefresh();
	};
	//Function called at the start of the dragging; return false to abort the beginning of drawing operations
	virtual bool OnDraggingStart(int x, int y, UINT KeyFlags){return true;};
	//Function called at the end of the dragging to know if it must be ended; return false to abort the cleanup operations
	virtual bool OnDraggingEnding(int x, int y, UINT KeyFlags){return true;};
	//Function called at the end of the dragging for cleanup purposes
	virtual void OnDraggingEnded(int x, int y, UINT KeyFlags){return ;};
public:
	UIPaintlikeTool(WORD ToolID, UIToolsContainer * Parent, ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings):UIBaseTool(ToolID, Parent)
	{
		init(PrimaryPenSettings, SecondaryPenSettings);
	};
	UIPaintlikeTool(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent, ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings):UIBaseTool(ToolID,IconID,TooltipID,TooltipExID, Parent)
	{
		init(PrimaryPenSettings, SecondaryPenSettings);
	};
	//Getters/setters
	ColorUtils * GetPrimaryPenSettings()
	{
		return primaryPenSettings;
	};
	void SetPrimaryPenSettings(ColorUtils * PrimaryPenSettings)
	{
		if(PrimaryPenSettings==NULL)
			throw std::invalid_argument(ERROR_STD_PROLOG "PrimaryPenSettings cannot be NULL.");
		primaryPenSettings = PrimaryPenSettings;
	};
	ColorUtils * GetSecondaryPenSettings()
	{
		return secondaryPenSettings;
	};
	void SetSecondaryPenSettings(ColorUtils * SecondaryPenSettings)
	{
		secondaryPenSettings = SecondaryPenSettings;
	};
	//Event handlers
	MessageReturnValue OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	MessageReturnValue OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	MessageReturnValue OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	MessageReturnValue OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	MessageReturnValue OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
};
//Paintbrush tool
//Doesn't add any functionality to UIPaintlikeTool, just adds the cursor
class UIPaintbrush : public UIPaintlikeTool
{
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(hCurInstance,MAKEINTRESOURCE(IDC_PAINTBRUSH));
	};	
public:
	UIPaintbrush(WORD ToolID, UIToolsContainer * Parent, ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings):UIPaintlikeTool(ToolID,Parent,PrimaryPenSettings,SecondaryPenSettings){};
	UIPaintbrush(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent, ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings):UIPaintlikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent,PrimaryPenSettings,SecondaryPenSettings){};
};
//Eraser tool
class UIEraser : public UIPaintlikeTool
{
protected:
	//Advanced Eraser tool
	AdvEraser AdvEr;
	//Initialize the class
	inline void init()
	{
		if(dsUpdater->CheckState())
			OnDibSectCreate(dsUpdater->GetDibSection());
	};
	void DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings);
	bool IsRefreshNeeded(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed);
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(hCurInstance,MAKEINTRESOURCE(IDC_ERASER));
	};
public:
	UIEraser(WORD ToolID, UIToolsContainer * Parent, ColorUtils * ErasePenSettings):UIPaintlikeTool(ToolID,Parent,ErasePenSettings,ErasePenSettings)
	{
		init();
	};
	UIEraser(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent, ColorUtils * ErasePenSettings):UIPaintlikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent,ErasePenSettings,ErasePenSettings)
	{
		init();
	};
	//Public events
	void OnDibSectCreate(DibSection * ds)
	{
		//Set a new DIB
		AdvEr.StoreNewDS(ds);
	}
};
//Color replacer tool
class UIColorReplacer : public UIPaintlikeTool
{
protected:
	//Color replacer tool
	ColorReplacer ColRep;
	void DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings);
	bool IsRefreshNeeded(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		return true;
	};
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(NULL,IDC_CROSS);
	};
	void OnToolActivating(UIBaseTool * prevActiveTool) {EnableExtCtrls(true,true,false);};
public:
	UIColorReplacer(WORD ToolID, UIToolsContainer * Parent, ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings):UIPaintlikeTool(ToolID,Parent,PrimaryPenSettings,SecondaryPenSettings){};
	UIColorReplacer(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent, ColorUtils * PrimaryPenSettings, ColorUtils * SecondaryPenSettings):UIPaintlikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent,PrimaryPenSettings,SecondaryPenSettings){};
};
//Clone tool
class UIClone : public UIPaintlikeTool
{
	//Init the class
	void init()
	{
		shift.x=INT_MIN;
		srcPt.x=INT_MIN;
	};
protected:
	//Clone Tool object
	CloneTool ClTool;
	//Shift of the src position to the dest position; if shift.x == INT_MIN no shift has been defined
	SHIFT shift;
	//Source point (used only between the user selection of the source point and his first click); if srcPos.x == INT_MIN no srcPos has been defined
	POINT srcPt;
	void DrawSegment(HDC hDC, POINT firstVertex, POINT secondVertex, bool useSecondaryPenSettings);
	bool IsRefreshNeeded(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		return true;
	};
	bool OnDraggingStart(int x, int y, UINT KeyFlags);
	void BeforeDrawing(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		UIPaintlikeTool::BeforeDrawing(firstVertex,secondVertex,secondaryPenSettingsUsed);
		clsrPC.BeginRefresh();
	};
	void AfterDrawing(POINT firstVertex, POINT secondVertex, bool secondaryPenSettingsUsed)
	{
		UIPaintlikeTool::AfterDrawing(firstVertex,secondVertex,secondaryPenSettingsUsed);
		clsrPC.EndRefresh();
	};
	//Redefine the standard OnToolActivating enabling the right ExtCtrls and the precision cursor
	void OnToolActivating(UIBaseTool * prevActiveTool)
	{
		UIPaintlikeTool::OnToolActivating(prevActiveTool);
		clsrPC.Show();
	};
	void OnToolDeactivating(UIBaseTool * newActiveTool)
	{
		UIPaintlikeTool::OnToolDeactivating(newActiveTool);
		clsrPC.Hide();
	};
	//Update the position and the shape of the precision cursor
	void AfterSetCursor(int modifierKeys, UINT codeHitTest);
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		if(srcPt.x==INT_MIN && shift.x==INT_MIN)
			return LoadCursor(NULL,IDC_NO);
		else if(shift.x==INT_MIN)
			return LoadCursor(NULL,IDC_UPARROW);
		else
			return LoadCursor(NULL,IDC_CROSS);
	};
public:
	//Costructors
	UIClone(WORD ToolID, UIToolsContainer * Parent, ColorUtils * PenSettings):UIPaintlikeTool(ToolID,Parent,PenSettings,NULL)
	{
		init();
	};
	UIClone(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent, ColorUtils * PenSettings):UIPaintlikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent,PenSettings,NULL)
	{
		init();
	};
	//Public events
	MessageReturnValue OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void DibSectCreate(DibSection * ds)
	{
		//Reset the clone tool
		srcPt.x = INT_MIN;
		shift.x = INT_MIN;
	}
};
//Arrow tool
class UIArrow : public UIBaseTool
{
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return NULL;
	};
public:
	UIArrow(WORD ToolID, UIToolsContainer * Parent):UIBaseTool(ToolID,Parent)
	{
		defaultRetVal=false;
	};
	UIArrow(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIBaseTool(ToolID,IconID,TooltipID,TooltipExID,Parent)
	{
		defaultRetVal=false;
	};
};
//FloodFill tool
class UIFloodFill : public UISingleClickTool
{
protected:
	//See UISingleClick
	bool PerformAction(POINT position, UINT keyFlags, bool right);
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(hCurInstance,MAKEINTRESOURCE(IDC_FLOODFILL));
	};
	void OnToolActivating(UIBaseTool * prevActiveTool) {EnableExtCtrls(false,true,false);};
public:
	//Constructors
	UIFloodFill(WORD ToolID, UIToolsContainer * Parent):UISingleClickTool(ToolID,Parent){};
	UIFloodFill(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UISingleClickTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
};
//Insert text tool
class UIInsertText : public UISingleClickTool
{
protected:
	//See UISingleClick
	bool PerformAction(POINT position, UINT keyFlags, bool right);
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(hCurInstance,MAKEINTRESOURCE(IDC_INSERTTEXT));
	};
	void OnToolActivating(UIBaseTool * prevActiveTool)
	{
		EnableExtCtrls(true,false,false);
	};
public:
	//Constructors
	UIInsertText(WORD ToolID, UIToolsContainer * Parent):UISingleClickTool(ToolID,Parent){};
	UIInsertText(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UISingleClickTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
};
//Picker tool
class UIPicker : public UISingleClickTool
{
protected:
	//See UISingleClick
	bool PerformAction(POINT position, UINT keyFlags, bool right);
	void OnToolDeactivating(UIBaseTool * prevActiveTool)
	{
		//Refresh the color pane (to erase the eventually drawn picker color rectangle)
		InvalidateRect(GetDlgItem(hToolBoxWindow,IDC_TB_COLORPAL),NULL,FALSE);
	};
	HCURSOR GetToolCursor(int modifierKeys, UINT codeHitTest)
	{
		return LoadCursor(hCurInstance,MAKEINTRESOURCE(IDC_PICKER));
	};
public:
	//Constructors
	UIPicker(WORD ToolID, UIToolsContainer * Parent):UISingleClickTool(ToolID,Parent){};
	UIPicker(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UISingleClickTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
	//Event handlers
	MessageReturnValue OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	MessageReturnValue OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
};
//Line tool
class UILine : public UILinelikeTool
{
protected:
	void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex);
public:
	UILine(WORD ToolID, UIToolsContainer * Parent):UILinelikeTool(ToolID,Parent){};
	UILine(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UILinelikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
};
//Arrowline tool
class UIArrowline : public UILinelikeTool
{
protected:
	//Arrow settings
	ArrowSettings arrowSettings;
	void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex);
	//The arrowline is a bit anomalous (it is a line-like tool but may be filled)
	void OnToolActivating(UIBaseTool * prevActiveTool) {EnableExtCtrls(true,false,true);};
public:
	UIArrowline(WORD ToolID, UIToolsContainer * Parent):UILinelikeTool(ToolID,Parent){};
	UIArrowline(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UILinelikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
	void SaveSettings(INISection & IniSect);
	void LoadSettings(INISection & IniSect);
	MessageReturnValue OnShowOptionsRequest(HWND hParent);
	//Wrappers
	ArrowSettings & GetArrowSettings(){return arrowSettings;};
};
//Measure tool
class UIMeasure : public UILinelikeTool
{
protected:
	//Measurement units container
	MeasurementUnitsContainer measurementUnitsContainer;
	//See UIDraggingTool
	void PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags);
	void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex);
	void OnToolActivating(UIBaseTool * prevActiveTool) {EnableExtCtrls(false,false,false);};
public:
	UIMeasure(WORD ToolID, UIToolsContainer * Parent):UILinelikeTool(ToolID,Parent){};
	UIMeasure(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UILinelikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
	MessageReturnValue OnShowOptionsRequest(HWND hParent);
	void DraggingStarted(POINT & FirstPos, UINT KeyFlags);
	void DraggingCanceled(POINT FirstVertex, POINT LastVertex, UINT KeyFlags);
	void OutlineShape(HDC hDC, POINT FirstVertex, POINT LastVertex, UINT KeyFlags);
	void SaveSettings(INISection & IniSect);
	void LoadSettings(INISection & IniSect);
	//Wrappers
	MeasurementUnitsContainer & GetMeasurementUnitsContainer(){return measurementUnitsContainer;};
};
//Rectangle tool
class UIRectangle : public UIShapeTool
{
protected:
	void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex);
public:
	UIRectangle(WORD ToolID, UIToolsContainer * Parent):UIShapeTool(ToolID,Parent){};
	UIRectangle(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIShapeTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
};
//Circle tool
class UICircle : public UIShapeTool
{
protected:
	void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex);
public:
	UICircle(WORD ToolID, UIToolsContainer * Parent):UIShapeTool(ToolID,Parent){};
	UICircle(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UIShapeTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
};
//Straighten tool
class UIStraighten : public UILinelikeTool
{
protected:
	//See UIDraggingTool
	void PerformAction(POINT FirstVertex, POINT LastVertex, UINT KeyFlags);
	void DrawShape(HDC hDC, POINT FirstVertex, POINT LastVertex);
	void OnToolActivating(UIBaseTool * prevActiveTool) {EnableExtCtrls(false,false,false);};
public:
	UIStraighten(WORD ToolID, UIToolsContainer * Parent):UILinelikeTool(ToolID,Parent){};
	UIStraighten(WORD ToolID, WORD IconID, WORD TooltipID, WORD TooltipExID, UIToolsContainer * Parent):UILinelikeTool(ToolID,IconID,TooltipID,TooltipExID,Parent){};
};
#pragma warning(pop)
//UI Tools container
class UIToolsContainer
{
public:
	//Map typedefs (the template class names are always soooo long!)
	typedef std::map<unsigned int, UIBaseTool *> UITCmap;
private:
	//Map that contains the tools
	UITCmap tools;
	//Pointer to the current tool
	UIBaseTool * currentTool;
	//Current tool ID
	unsigned int currentToolID;
	//Previously selected tool ID
	unsigned int previousToolID;
	//Does previousToolID contain a valid value?
	bool canRevert;
	//Eventually reverts to the previously saved tool
	void RevertToPreviousTool();
	//The previous function is always called by its friend of UIBaseTool
	friend void UIBaseTool::RevertToPreviousTool();
	//Extended pointer to the IrfanView DC
	PointerEx<HDC> ivWndDC;
	//Extended pointer to the DC of the DibSection
	PointerEx<HDC> dibSectionDC;
	//Extended pointer to the status bar
	HWND statusBar;
public:
	//Constructor
	UIToolsContainer(PointerEx<HDC> IVWndDC, PointerEx<HDC>DibSectionDC);
	//Destructor
	~UIToolsContainer();
	//Gets the current tool ID
	unsigned int GetCurrentToolID()
	{
		return currentToolID;
	};
	//Sets the current tool ID; if CanRevert is true, when the just selected tool will have finished
	//its work it may restore the previously selected tool
	void SetCurrentToolID(unsigned int CurrentToolID, bool CanRevert = false, bool Throw=true);
	//Returns the current tool
	UIBaseTool * GetCurrentTool()
	{
		return currentTool;
	};
	//Returns a tool automatically casting it to the given type
	template<class ToolType> ToolType * GetTool(unsigned int ToolID, bool Throw=true)
	{
		UITCmap::iterator it=tools.find(ToolID);
		if(it!=UITC->GetToolsMap().end())
		{
			ToolType * ret;
			ret=dynamic_cast<ToolType *>(it->second);
			if(ret==NULL && Throw)
			{
				std::ostringstream os;
				os<<ERROR_STD_PROLOG<<"A pointer to the tool with the ID "<<ToolID<<" of type "<<typeid(*it->second).name()<<" cannot be casted to "<<typeid(ret).name()<<".";
				throw bad_cast(os.str().c_str());
			}
			return ret;
		}
		else
		{
			if(Throw)
				throw std::invalid_argument(ERROR_STD_PROLOG "ToolID is not a valid tool ID.");
			else
				return NULL;
		}
	};
	//Returns a copy of the tools map
	UITCmap & GetToolsMap()
	{
		return tools;
	};
	//Returns the PointerEx to IrfanView DC
	inline PointerEx<HDC> GetIVWndDC()
	{
		SetupClippingRegion();
		return ivWndDC;
	};
	//Returns the PointerEx to the DC of the DibSection
	inline PointerEx<HDC> GetDibSectionDC()
	{
		SetupClippingRegion();
		return dibSectionDC;
	};
	//Returns the PointerEx to the HWND of the status bar
	inline HWND GetStatusBar()
	{
		return statusBar;
	};
	//Eventually forward the passed message to the current UITool class;
	//returns true if it mustn't be passed also to the real IVWnd
	MessageReturnValue HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//Adds the UI tools buttons and icons to the given toolbar, that must have an attached image list
	void AddButtonsToToolbar(HWND Toolbar);
	//Saves the settings of the container and of the tools
	void SaveSettings(INISection & IniSect);
	//Loads the settings of the container and of the tools
	void LoadSettings(INISection & IniSect);
	//Synchronizes the selected tool with the selected tool of the toolbar
	void SyncCurTool()
	{
		CheckToolTB(currentToolID);
	};
	//Called when a new DIB is loaded
	void OnDibSectCreate(DibSection * ds);
};