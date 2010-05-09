#include "stdafx.h"
#include "Arrow.h"
#include "Utils.h"
#include <cmath>
//Draws an arrow from the current point to the given point with the given options
void ArrowTo(HDC hDC, int x, int y, ArrowSettings * as)
{
	POINT m_One, m_Two;
	GetCurrentPositionEx(hDC, &m_One);
	m_Two.x=x;
	m_Two.y=y;
	DrawArrow(hDC, m_One, m_Two, as);
}
//Draws an arrow from m_One to m_Two with the given options
void DrawArrow(HDC hDC, POINT m_One, POINT m_Two, ArrowSettings * as)
{
	POINT vert[3];
	double slopy , cosy , siny;
	double arrowBase = as->arrowBase/2.0; //we need the length of half arrow base
	//draw a line between the 2 endpoints
	MoveToEx(hDC, m_One.x, m_One.y, NULL);
	LineTo(hDC, m_Two.x, m_Two.y );

	if(!(as->drawFirstArrow||as->drawSecondArrow))
		return;
	slopy = atan2((double) ( m_One.y - m_Two.y ),
		(double)( m_One.x - m_Two.x ) );
	cosy = cos( slopy );
	siny = sin( slopy ); //need math.h for these functions

	//here is the tough part - actually drawing the arrows
	//a total of 6 lines drawn to make the arrow shape
	//first arrow
	if(as->drawFirstArrow)
	{
		vert[0].x=m_One.x + int(round( - as->arrowLength * cosy - ( arrowBase * siny ) ));
		vert[0].y=m_One.y + int(round( - as->arrowLength * siny + ( arrowBase * cosy ) ));
		vert[1]=m_One;
		vert[2].x=m_One.x + int(round( - as->arrowLength * cosy + ( arrowBase * siny ) ));
		vert[2].y=m_One.y - int(round( arrowBase * cosy + as->arrowLength * siny ));
		//... o di riffa o di raffa se la riga è dritta anche le basi delle frecce lo saranno
		if(m_One.x==m_Two.x)
			vert[0].y=vert[2].y;
		if(m_One.y==m_Two.y)
			vert[0].x=vert[2].x;
		if(as->openArrowHead)
			Polyline(hDC,vert,3);
		else
			Polygon(hDC,vert,3);
	}
	//second arrow
	if(as->drawSecondArrow)
	{
		vert[0].x=m_Two.x + int( as->arrowLength * cosy - ( arrowBase * siny ) );
		vert[0].y=m_Two.y + int( as->arrowLength * siny + ( arrowBase * cosy ) );
		vert[1]=m_Two;
		vert[2].x=m_Two.x + int( as->arrowLength * cosy + arrowBase * siny );
		vert[2].y=m_Two.y - int( arrowBase * cosy - as->arrowLength * siny );
		//vedi sopra
		if(m_One.x==m_Two.x)
			vert[0].y=vert[2].y;
		if(m_One.y==m_Two.y)
			vert[0].x=vert[2].x;
		if(as->openArrowHead)
			Polyline(hDC,vert,3);
		else
			Polygon(hDC,vert,3);
	}
	return;
}