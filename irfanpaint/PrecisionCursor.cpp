#include "StdAfx.h"
#include ".\precisioncursor.h"
#include "Utils.h"

//Draws the cursor on the DC
void PrecisionCursor::drawCursor(RECT cursorRct, DWORD cursorShape, bool cross)
{
	//Save the DC state
	int DCState=SaveDC(m_wndDC);
	//Set the NOT ROP2
	SetROP2(m_wndDC,R2_NOT);
	//Remove any clipping region
	SelectClipRgn(m_wndDC,NULL);
	//Select a null brush
	SelectBrush(m_wndDC,GetStockBrush(NULL_BRUSH));
	//Draw the cursor
	switch(cursorShape)
	{
	case PS_ENDCAP_ROUND:
		Ellipse(m_wndDC,EXPANDRECT_C(cursorRct));
		break;
	case PS_ENDCAP_SQUARE:
		Rectangle(m_wndDC,EXPANDRECT_C(cursorRct));
		break;
	default:
		break;
	}
	//Eventually draw the cross
	if(cross)
	{
		//Find the medium point between the center and the pixel next to it
		POINT pos=m_position,pos2=m_position;
		pos2.x++;
		pos2.y++;
		DIB2IVWndPoint(&pos);
		DIB2IVWndPoint(&pos2);
		pos.x=(pos.x+pos2.x)/2;
		pos.y=(pos.y+pos2.y)/2;
		//Draw the cross
		MoveToEx(m_wndDC,pos.x-PREC_CURSOR_CROSS_SIZE,pos.y-PREC_CURSOR_CROSS_SIZE,NULL);
		LineTo(m_wndDC,pos.x+PREC_CURSOR_CROSS_SIZE,pos.y+PREC_CURSOR_CROSS_SIZE);
		MoveToEx(m_wndDC,pos.x+PREC_CURSOR_CROSS_SIZE,pos.y-PREC_CURSOR_CROSS_SIZE,NULL);
		LineTo(m_wndDC,pos.x-PREC_CURSOR_CROSS_SIZE,pos.y+PREC_CURSOR_CROSS_SIZE);
	}
	//Restore the DC state
	RestoreDC(m_wndDC,DCState);
}
//Shows the precision cursor
void PrecisionCursor::Show()
{
	if(!m_shown)
	{
		/*			m_lastRct.left=m_position.x-(m_width*m_zoom/200);
			m_lastRct.top=m_position.y-(m_width*m_zoom/200);
			m_lastRct.right=m_lastRct.left+(m_width*m_zoom/100);
			m_lastRct.bottom=m_lastRct.top+(m_width*m_zoom/100);*/
		//Calculate the cursor rectangle
		m_lastRct.left=m_position.x-m_width/2;
		m_lastRct.top=m_position.y-m_width/2;
		m_lastRct.right=m_lastRct.left+m_width;
		m_lastRct.bottom=m_lastRct.top+m_width;
		DIB2IVWndPoint((POINT *)&m_lastRct);
		DIB2IVWndPoint(((POINT *)&m_lastRct)+1);
		//Backup the values
		m_lastShape=m_shape;
		m_lastCross=m_cross;
		//Draw the cursor
		drawCursor(m_lastRct,m_lastShape,m_lastCross);
		m_shown=true;
	}
};