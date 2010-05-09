#include "StdAfx.h"
#include ".\maskedtool.h"
#include "Utils.h"
//Sets up the bitmap
void MaskedTool::setUpBmp(SIZE size)
{
	SIZE maskSize={0};
	if(mask.get()!=NULL)
		maskSize=mask->GetDibSize();
	if(
			mask.get()==NULL
			|| 
			maskSize.cx<size.cx
			|| 
			maskSize.cy<size.cy
			||
			(
				(
					maskSize.cx/size.cx>5 || //avoid wasting space and CPU time (in processing the mask): if the mask is 5 times wider or higher than needed and it's more than 5000 pixel big
					maskSize.cy/size.cy>5
				)
				&&
				(
					(maskSize.cx * maskSize.cy) > 5000
				)
			)
		)
	{
		//If it's not very big make it square (the user movements are +/- of the same length but in every direction) and a bit bigger to avoid continuous bitmap recreation
		if(size.cx*size.cy<50000)
		{
			size.cx=size.cy=(LONG)(max(size.cx,size.cy)*1.2);
		}
		try
		{
			mask.reset(new BWDibSection(size.cx,size.cy));
		}
		catch(...)
		{
			mask.reset();
			throw;
		}
#ifdef _DEBUG
		TCHAR buffer[MESSAGEBUFFERSLENGTH];
		_sntprintf(buffer,ARRSIZE(buffer),_T("New mask bitmap (%ld,%ld)\n"),size.cx,size.cy);
		OutputDebugString(buffer);
#endif
		SetBkMode(mask->GetCompDC(),TRANSPARENT);
	}
	usedSize=size;
	PatBlt(mask->GetCompDC(),0,0,mask->GetDibWidth(),mask->GetDibHeight(),WHITENESS);
}
//Draws a line on the mask (and eventually adjusts the size of the bitmap)
void MaskedTool::drawLine(POINT beginPt, //Begin point of the line
	POINT endPt, //End point of the line
	HPEN pen, //Pen
	int penWidth) //Pen width
{
	SIZE neededSize; //Needed size for the bitmap
	//Set up a bitmap large enough for the current
	neededSize.cx = abs(endPt.x - beginPt.x)+penWidth;
	neededSize.cy = abs(endPt.y - beginPt.y)+penWidth;
	maskOffset.x = min(endPt.x, beginPt.x)-penWidth/2;
	maskOffset.y = min(endPt.y, beginPt.y)-penWidth/2;
	setUpBmp(neededSize); //Set up the bitmap
	//Get the DC of the mask
	HDC dc=mask->GetCompDC();
	//Save its settings
	int savedDC=SaveDC(dc);
	//Draw only in black
	SetROP2(dc,R2_BLACK);
	SelectPen(dc,pen);
	MoveToEx(dc, beginPt.x-maskOffset.x, beginPt.y-maskOffset.y, NULL); //Change the origin...
	LineTo(dc, endPt.x-maskOffset.x, endPt.y-maskOffset.y); //... and draw the line
	//Restore the settings of the DC
	RestoreDC(dc,savedDC);
}
//Draws a line on the mask (and eventually adjusts the size of the bitmap); no-penWidth version, deduct it from pen and call the other function
void MaskedTool::drawLine(POINT beginPt, //Begin point of the line
	POINT endPt, //End point of the line
	HPEN pen) //Pen
{
	struct
	{
		EXTLOGPEN elp;
		DWORD otherStyleEntries[15];
	} exPenSettings; //Extended pen settings (include also all the style entries)
	LOGPEN penSettings; //Pen settings
	int penWidth; //Pen width
	int structSize=GetObject(pen,0,NULL); //Structure size
	if(structSize==0)
		throw std::invalid_argument(ERROR_STD_PROLOG "pen is not a valid GDI handle.");
	else if(structSize==sizeof(penSettings)) //Simple LOGPEN
	{
		if(GetObject(pen,sizeof(penSettings),&penSettings)==0)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot obtain pen width.");
		penWidth=penSettings.lopnWidth.x;
	}
	else if(structSize<=sizeof(exPenSettings)) //EXTLOGPEN eventually with some style entries
	{
		if(GetObject(pen,sizeof(exPenSettings),&exPenSettings)==0)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot obtain pen width.");
		penWidth=exPenSettings.elp.elpWidth;
	} //Other kind of GDI handle (or GDI handle with more pen style entries - that today (2008) cannot exist)
	else
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot obtain pen width.");
	drawLine(beginPt,endPt,pen,penWidth);
	
}