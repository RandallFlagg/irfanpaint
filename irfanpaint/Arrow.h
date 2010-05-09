//Arrow APIs
#pragma once
struct ArrowSettings
{
	int arrowLength;		//length of the arrow (a)
	int arrowBase;			//length of the base (c) of the arrow (b)
	bool drawFirstArrow;	//indicates if it must draw or not the first arrow
	bool drawSecondArrow;	//indicates if it must draw or not the second arrow
	bool openArrowHead;		//indicates if the arrowhead must not have a base (c)
	/*
	                  a
	               <--->

	               |\     ^
				c->|  \   |
	---------------|   >  | b
	               |  /   |
				   |/     v
	*/
};
//Draws an arrow from the current point to the given point with the given options
void ArrowTo(HDC hDC, int x, int y, ArrowSettings * as);
//Draws an arrow from m_One to m_Two with the given options
void DrawArrow(HDC hDC, POINT m_One, POINT m_Two, ArrowSettings * as);