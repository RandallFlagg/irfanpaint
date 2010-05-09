#pragma once
#include "maskedtool.h"
#include "DibSection.h"
//ColorReplacer class
//Replaces the BG color with the FG color
class ColorReplacer :
	public MaskedTool
{
private:
	//Performs the color replacement
	void replaceColor(DibSection * ds, COLORREF fgColor, COLORREF bgColor, BYTE threshold);
public:
	//Default constructor
	ColorReplacer(void) {return;};
	//Destructor
	~ColorReplacer(void) {return;};
	//Replace the color on the given line
	inline void ReplaceColorOnLine(
		DibSection * ds, //DibSection on which operate
		POINT beginPt, //Begin point of the line
		POINT endPt, //End point of the line
		HPEN pen, //Pen
		COLORREF fgColor, //FG color
		COLORREF bgColor, //BG color
		BYTE threshold //Color tolerance
		)
	{
		drawLine(beginPt,endPt,pen);
		replaceColor(ds,fgColor,bgColor,threshold);
	}
};