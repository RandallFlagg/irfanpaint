#pragma once
class WindowMagnetizer
{
public:
	//Possible magnetization states
	enum MagnetizationState
	{
		NotMagnetized,		//The windows are not magnetized
		OnLeftSide,			//The toolbar is magnetized on the left side of the main window
		OnRightSide			//The toolbar is magnetized on the right side of the main window
	};
private:
	//Is magnetization enabled?
	bool enabled;
	//How are the windows magnetized?
	MagnetizationState magnState;
	//Offset of the toolbar relative to the top of the main window
	int magnOffset;
	//Distance limit from the main window before the toolbar gets magnetized
	int magnLimit;
	//Main window (the window to which the toolbar is magnetized) handle
	HWND mainWindow;
	//Toolbar (the window that gets magnetized) handle
	HWND toolbar;
	//Returns the new toolbar position accordingly to the current magnetization settings
	POINT getToolbarPos(LPRECT tbRect = NULL);
	//Last "focus rectangle" drawn
	RECT lastFocusRect;
	//Flag that indicates that magnState must not be changed
	bool lockMagnState;
	//Returns the new toolbar magnetization state accordingly to the current magnetization settings
	MagnetizationState getNewMagnState(LPRECT tbRect=NULL);
	//Draws a rectangle on the given DC with a NOT ROP2
	void DrawNotRectangle(HDC hDC, CONST LPRECT rect);
public:
	//Default constructor
	WindowMagnetizer(void)
	{
		magnState = MagnetizationState::NotMagnetized;
		enabled=false;
		mainWindow=NULL;
		toolbar=NULL;
		magnLimit=16;
		magnOffset=0;
		lastFocusRect.left=INT_MIN;
	};
	//Returns true if the class is initialized (when both mainWindow and toolbar are not NULL)
	bool IsInitialized()
	{
		return (mainWindow!=NULL)&&(toolbar!=NULL);
	};
	//Initializes the main window handle
	void SetMainWindow(HWND MainWindow)
	{
		if(IsWindow(MainWindow)||MainWindow==NULL)
			mainWindow=MainWindow;
		else
			throw std::invalid_argument(ERROR_STD_PROLOG "MainWindow must be a valid window handle.");
	};
	//Returns the main window handle
	HWND GetMainWindow()
	{
		return mainWindow;
	};
	//Initializes the toolbar handle
	void SetToolbar(HWND Toolbar)
	{
		if(IsWindow(Toolbar)||Toolbar==NULL)
			toolbar=Toolbar;
		else
			throw std::invalid_argument(ERROR_STD_PROLOG "Toolbar must be a valid window handle.");
	};

	//Returns the toolbar handle
	HWND GetToolbar()
	{
		return toolbar;
	};

	//Enables/disables magnetization
	void EnableMagnetization(bool Enable)
	{
		enabled = Enable;
	};
	//Returns true if the magnetization is enabled
	bool IsMagnetizationEnabled()
	{
		return enabled;
	};
	//Set the magnetization state
	void SetMagnetizationState(MagnetizationState MagnState)
	{
		magnState = MagnState;
	};
	//Returns the magnetization state
	MagnetizationState GetMagnetizationState()
	{
		return magnState;
	};
	//Set the distance from the main window before which the toolbar gets magnetized
	void SetMagnetizationLimit(int MagnLimit)
	{
		if(MagnLimit>=0)
			magnLimit=MagnLimit;
		else
			throw std::invalid_argument(ERROR_STD_PROLOG "MagnLimit must be a positive integer or zero.");
	};
	//Returns the distance from the main window before which the toolbar gets magnetized
	int GetMagnetizationLimit()
	{
		return magnLimit;
	};
	//Sets the vertical offset of the toolbar from the top of the main window
	void SetMagnetizationOffset(int MagnOffset)
	{
		magnOffset=MagnOffset;
	};
	//Returns the vertical offset of the toolbar from the top of the main window
	int GetMagnetizationOffset()
	{
		if(GetMagnetizationState()!=NotMagnetized)
			return magnOffset;
		else
			return 0;
	};
	//Call this function when the toolbar recieves a WM_MOVE or WM_SIZE
	void OnToolbarBoundsChanged();
	//Call this function when the toolbar recieves a WM_MOVING or WM_SIZING
	void OnToolbarBoundsChanging(LPARAM lParam);
	//Call this function when the main window recieves a WM_MOVE or WM_SIZE
	void OnMainWindowBoundsChanged()
	{
		UpdateToolbarPos();
	};
	//Updates the toolbar position accordingly to the current magnetization settings
	void UpdateToolbarPos(LPRECT tbRect=NULL);
	//Checks the toolbar position and eventually magnetizes it
	void CheckToolbarPos(LPRECT tbRect=NULL);
};