#include "stdafx.h"
#include "ColorUtils.h"
//Recreates the brush and/or the pen; if create is false only destroy the existing object
void ColorUtils::recreateObjs(bool brush, bool pen, bool create)
{
	if(brush)
	{
		if(m_brush!=NULL)
			DeleteBrush(m_brush);
		if(create)
		{
			m_brush=CreateBrushIndirect(&m_brushSet);
			if(m_brush==NULL)
				throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the brush; CreateBrushIndirect returned NULL.");
		}
	}
	if(pen)
	{
		if(m_pen!=NULL)
			DeletePen(m_pen);
		if(create)
		{
			m_pen=ExtCreatePen(m_penSet.dwPenStyle,m_penSet.dwWidth,&m_brushSet,m_penSet.dwStyleCount,m_penSet.lpStyle);
			if(m_pen==NULL)
				throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the pen; ExtCreatePen returned NULL.");
		}
	}
}
//Initializes the brush and/or the pen settings to the default values
void ColorUtils::initSettings(bool brush, bool pen, bool ACUPs)
{
	if(brush)
	{
		m_brush=NULL;
		m_brushSet.lbColor=0;
		m_brushSet.lbHatch=0;
		m_brushSet.lbStyle=BS_SOLID;
	}
	if(pen)
	{
		m_pen=NULL;
		m_penSet.dwPenStyle=PS_GEOMETRIC|PS_SOLID;
		m_penSet.dwStyleCount=0;
		m_penSet.dwWidth=1;
		m_penSet.lpStyle=NULL;
	}
	if(ACUPs)
	{
		m_acup.bgColor=0;
		m_acup.bgCU=NULL;
		m_acup.miterLimit=10.0;
		m_acup.transparentBG=false;
	}
}