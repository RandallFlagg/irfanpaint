#pragma once
#include "BWDibSection.h"

//MaskedTool: base class for all the tools that require a B/W bitmap as a mask for a MaskBlt
class MaskedTool
{
protected:
	SIZE usedSize;					//Size of the image stored in m_bmp
	POINT maskOffset;				//Offset of mask in the image on which we operate
	boost::scoped_ptr<BWDibSection> mask;	//DibSection used as mask
	//Sets up the bitmap
	void setUpBmp(SIZE size);
	//Draws a line on the mask (and eventually adjusts the size of the bitmap); no-penWidth version, deduct it from pen and call the other function
	void drawLine(POINT beginPt, //Begin point of the line
		POINT endPt, //End point of the line
		HPEN pen); //Pen
	//Draws a line on the mask (and eventually adjusts the size of the bitmap)
	void drawLine(
		POINT beginPt, //Begin point of the line
		POINT endPt, //End point of the line
		HPEN pen, //Pen
		int penWidth); //Pen width
};