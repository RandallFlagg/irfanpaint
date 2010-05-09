#pragma once
//Precision cursors cross size
#define PREC_CURSOR_CROSS_SIZE		10

class PrecisionCursor
{
	POINT m_position;		//Position of the cursor
	unsigned int m_width;	//Width of the cursor
	DWORD m_shape;			//Shape of the cursor; it actually can be PS_ENDCAP_ROUND or PS_ENDCAP_SQUARE
	bool m_cross;			//Specifies if the cursor has a cross inside
	//m_lastXXXs
	RECT m_lastRct;			//Surrounding rectangle of the last drawn cursor
	DWORD m_lastShape;		//Shape of the last drawn cursor
	bool m_lastCross;		//Cross state of the last drawn cursor
	//Others
	HDC m_wndDC;			//The DC to write on
	bool m_shown;			//Is the cursor shown?
	bool m_prevShown;		//Was the cursor shown before BeginRefresh?
	int m_refreshCount;		//Number of times that BeginRefresh has been called without an EndRefresh
	//Draws the cursor on the DC
	void drawCursor(RECT cursorRct, DWORD cursorShape, bool cross);
	//Inits the fields of the class
	void init()
	{
		//Fields init
		m_position.x=0;
		m_position.y=0;
		m_width=1;
		m_shape=PS_ENDCAP_ROUND;
		m_cross=false;
		m_shown=false;
		m_prevShown=false;
	};
public:
	//Default constuctor
	PrecisionCursor()
	{
		m_wndDC=NULL;
		init();
	};
	PrecisionCursor(HDC wndDC)
	{
		m_wndDC=wndDC;
	};
	//Shows the precision cursor
	void Show();
	//Hides the precision cursor
	inline void Hide()
	{
		if(m_shown)
		{
			drawCursor(m_lastRct,m_lastShape,m_lastCross);
			m_shown=false;
		}
	};
	//Pre-refresh operations (actually removes the cursor); insert a call to this at the beginning of the WM_PAINT handler
	inline void BeginRefresh()
	{
		if(m_refreshCount++==0)
		{
			//Save the current show state
			m_prevShown=m_shown;
			//Hide the cursor
			Hide();
		}
	};
	//Post-refresh operations (actually shows the cursor if it was visible before the call to BeginRefresh); insert a call to this at the end of the WM_PAINT handler
	inline void EndRefresh()
	{
		if(m_refreshCount==0)
			return;
		if(--m_refreshCount==0)
		{
			//If the cursor was visible vefore BeginRefresh...
			if(m_prevShown)
				Show(); //... show it again
		}
	};
	//Resets the refresh operations
	inline void ResetRefresh()
	{
		m_refreshCount=1;
		EndRefresh();
	}
	//Returns true if the cursor is visible, false otherwise
	inline bool IsVisible()
	{
		return m_shown;
	};
	//Setters
	//Sets the width of the cursor
	inline void SetWidth(unsigned int width, bool redraw=true)
	{
		if(redraw)
			BeginRefresh();
		m_width=width;
		if(redraw)
			EndRefresh();
	};
	//Sets the position of the cursor in DIB coords (POINT version)
	inline void SetPosition(POINT pt, bool redraw=true)
	{
		//Call the other overload
		SetPosition(pt.x,pt.y,redraw);
	};
	//Sets the position of the cursor in DIB coords (x-y version)
	inline void SetPosition(int x, int y, bool redraw=true)
	{
		if(redraw)
			BeginRefresh();
		m_position.x=x;
		m_position.y=y;
		if(redraw)
			EndRefresh();
	};
	//Sets the cross state
	inline void SetCross(bool cross, bool redraw=true)
	{
		if(redraw)
			BeginRefresh();
		m_cross=cross;
		if(redraw)
			EndRefresh();
	};
	//Sets the shape of the cursor
	inline void SetShape(DWORD shape, bool redraw=true)
	{
		if(redraw)
			BeginRefresh();
		m_shape=shape;
		if(redraw)
			EndRefresh();
	};
	//Sets the DC to work on
	inline void SetDC(HDC wndDC)
	{
		BeginRefresh();
		m_wndDC=wndDC;
		EndRefresh();
	};
	//Getters
	//Returns the width of the cursor
	inline unsigned int GetWidth()
	{
		return m_width;
	};
	//Returns the position of the cursor in DIB coords
	inline POINT GetPosition()
	{
		return m_position;
	};
	//Gets the cross state
	inline bool GetCross()
	{
		return m_cross;
	};
	//Gets the shape of the cursor
	inline DWORD GetShape()
	{
		return m_shape;
	};
	//Gets the DC on which we are working on
	inline HDC GetDC()
	{
		return m_wndDC;
	};
};