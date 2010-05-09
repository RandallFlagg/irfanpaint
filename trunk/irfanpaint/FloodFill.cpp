#include "stdafx.h"
#include "FloodFill.h"
#include "DibSection.h"
#include "Utils.h"
#include "BWDibSection.h"
//Floodfill function - decides the fastest routine to use - always call this function, not the other ones
void AdvFloodFill(DibSection * ds, POINT startPt, HBRUSH fillBrush, BYTE threshold)
{
	//Check the arguments
	if(ds==NULL)
		throw std::invalid_argument(ERROR_STD_PROLOG "ds must be a valid pointer to a DibSection.");
	ds->ArgsInRange(startPt.x,startPt.y);
	//Output DC
	HDC outDC=ds->GetCompDC();
	//See if we can speed up the thing looking for special threshold values
	if(threshold==0)
	{
		//No threshold - use the GDI APIs
		BEGIN_SELOBJ(outDC,fillBrush,outDC_brush);
		ExtFloodFill(outDC,startPt.x,startPt.y,GetPixel(outDC,startPt.x,startPt.y),FLOODFILLSURFACE);
		END_SELOBJ(outDC,outDC_brush);
	}
	else if(threshold==255)
	{
		//Maximum threshold => everything is filled => just use a FillRgn
		SIZE dibSize=ds->GetDibSize();
		//Create a region of the size of the DIB
		HRGN rgn=CreateRectRgn(0,0,dibSize.cx,dibSize.cy);
		//Get the clipping region (if any) => if there's no clipping region rgn isn't changed
		GetClipRgn(outDC,rgn);
		//Fill the region
		FillRgn(outDC,rgn,fillBrush);
		//Delete the region
		DeleteRgn(rgn);
	}
	else
	{
		//Threshold - use the scanline floodfill algorithm
		FloodFillScanlineStack(ds,startPt,fillBrush,threshold);
	}
}

//Scanline floodfill algorithm - brush version
void FloodFillScanlineStack(DibSection * ds, POINT startPt, HBRUSH fillBrush, BYTE threshold)
{
	//Get the kind of brush we have to eventually simplify the job
	LOGBRUSH lb;
	if(GetObject(fillBrush,sizeof(lb),&lb)==0)
		throw std::invalid_argument(ERROR_STD_PROLOG "fillBrush is not a valid brush.");
	switch(lb.lbStyle)
	{
	case BS_SOLID:	//Solid brush
		//The brush style is solid - we can use a simple floodfill with its color
		FloodFillScanlineStack(ds,startPt,lb.lbColor,threshold);
		break; //BS_SOLID
	case BS_HATCHED:	//Complex brush style
	case BS_DIBPATTERN:
	case BS_DIBPATTERN8X8:
	case BS_DIBPATTERNPT:
	case BS_PATTERN:
	case BS_PATTERN8X8:
		{
			//The brush style is complex - we have to use some tricks
			//hDC of the DibSection
			HDC dsdc=ds->GetCompDC();
			//DIB size
			SIZE dibSize=ds->GetDibSize();
			//Create a mask of the size of the DIB
			BWDibSection mask(dibSize.cx,dibSize.cy);
			HDC cdc=mask.GetCompDC();
			//"White" the mask
			PatBlt(cdc,0,0,dibSize.cx,dibSize.cy,BLACKNESS);
			//Eventually copy the clipping region of the DIB DC to the mask DC
			HRGN clipRgn=CreateRectRgn(0,0,1,1); //dummy region
			if(GetClipRgn(cdc,clipRgn)==1)
			{
				SelectClipRgn(cdc,clipRgn);
			}
			DeleteRgn(clipRgn);
			//Perform the floodfill using the mask as output and black as the fill color
			InternalFloodFillScanlineStack(ds,&mask,startPt,0x00FFFFFF,threshold);
			//If the background mode is transparent and the brush is hatched modify the mask
			if(GetBkMode(dsdc)==TRANSPARENT && lb.lbStyle==BS_HATCHED)
			{
				//Usa a new white brush with the settings of the original brush and a black BG color and AND it with the actual mask
				LOGBRUSH lb2=lb;
				lb2.lbColor=0x00FFFFFF;
				SetBkColor(cdc,0);
				BEGIN_SELOBJ(cdc,CreateBrushIndirect(&lb2),cdc_brush);
				PatBlt(cdc,0,0,dibSize.cx,dibSize.cy,ROP3_DPa);
				DeleteBrush(END_SELOBJ(cdc,cdc_brush));
			}
			//Save the DC
			int dcState=SaveDC(dsdc);
			//Use a MaskBlt to apply the brush only where the mask is white
			SelectBrush(dsdc,fillBrush);
			//For some reason if there's a clipping region MaskBlt behaves strangely
			SelectClipRgn(dsdc,NULL);
			//PLSPEC: NT only
			MaskBlt(dsdc,0,0,dibSize.cx,dibSize.cy,dsdc,0,0,mask.GetDSHandle(),0,0,MAKEROP4(PATCOPY,ROP3_NOP));
			//Restore the DC
			RestoreDC(dsdc,dcState);
		}
		break; //BS_HATCHED etc.
	case BS_NULL:	//Empty brush
	//case BS_HOLLOW: //Same as BS_NULL
	default:	//Unsupported brush
		//Do nothing
		break;	//BS_NULL etc.
	}
}
//Scanline floodfill algorithm - color version
void FloodFillScanlineStack(DibSection * ds, POINT startPt, COLORREF newColor, BYTE threshold)
{
	//Simply call the internal color version
	InternalFloodFillScanlineStack(ds,NULL,startPt,newColor,threshold);
}

//Scanline floodfill algorithm - internal color version
void InternalFloodFillScanlineStack(DibSection * ds, BWDibSection * outMask, POINT startPt, COLORREF newColor, BYTE threshold)
{
//#define TIME_FLOODFILL
#ifdef TIME_FLOODFILL
	LARGE_INTEGER beginCount,endCount,frequency;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&beginCount);
#endif
	//Check the arguments
	//Coords of the start point
	ds->ArgsInRange(startPt.x,startPt.y);
	//Size of the mask (we are going to use its blindSetPixel function, we must be careful)
	if(outMask!=NULL)
	{
		if(outMask->GetDibHeight()<ds->GetDibHeight() || outMask->GetDibWidth()<ds->GetDibWidth())
			throw std::invalid_argument(ERROR_STD_PROLOG "The mask outMask is smaller than the DibSection ds.");
	}
	//Check if the start point is in the clipping region
	if(!PtVisible(ds->GetCompDC(),startPt.x,startPt.y))
		return;
	//Get the DIB size
	SIZE rgnSize=ds->GetDibSize();
	//Stack used for the filling
	std::stack<POINT> stack;
	//Various points
	POINT pt,tempPt;
	//Temp x value
	int x1;
	//Color of the pixel on which the user clicked
	const RGBQUAD oldColor = ds->BlindGetPixelColorRQ(startPt.x,startPt.y);
	//Palette index of the new color (if the image is indexed)
	BYTE newColorIndex=0;
	//Clipping region
	HRGN clipRgn=NULL;
	//Bounds of the filling
	RECT bounds;
	ds->GetClippingRegionStuff(clipRgn,bounds);
	//Change rgnSize by consequence
	rgnSize.cx=bounds.right-bounds.left+1;
	rgnSize.cy=bounds.bottom-bounds.top+1;
	//Is the image indexed?
	bool indexed=ds->IsIndexed();
	if(indexed)
	{
		//If the image is indexed get the index of the new color
		newColorIndex=ds->GetPaletteIndex(newColor);
	}
	bool spanLeft, spanRight,cfr;
	boost::scoped_array<bool> visited(new bool[rgnSize.cx*rgnSize.cy]);
	memset((void *)visited.get(),0,rgnSize.cx*rgnSize.cy*sizeof(*visited.get()));
	stack.push(startPt);
	while(!stack.empty())
	{
		pt=stack.top();
		stack.pop();
		if(visited[(pt.x-bounds.left)+(pt.y-bounds.top)*rgnSize.cx]++)
			continue;
		for(x1=pt.x;x1>=bounds.left && similarColor(ds->BlindGetPixelColorRQ(x1,pt.y),oldColor,threshold); x1--)
			;
		x1++;
		spanLeft=spanRight=false;
		while(x1 <= bounds.right && similarColor(ds->BlindGetPixelColorRQ(x1,pt.y),oldColor,threshold))
		{
			//If we have a complex region we must check if the point is in the region
			if(clipRgn!=NULL && !PtInRegion(clipRgn,x1,pt.y))
				continue;
			//If outMask==NULL we have to paint on the DIB
			if(outMask==NULL)
			{
				//If the DIB is indexed it's much faster to set directly the index than the color (one palette lookup at the beginning of the function instead of one for each pixel)
				if(indexed)
					ds->BlindSetPixelIndex(x1,pt.y,newColorIndex);
				else
					ds->BlindSetPixelColorCR(x1,pt.y,newColor);
			}
			else //Otherwise on the mask
				outMask->BlindSetPixel(x1,pt.y,true);
			if(pt.y>bounds.top)
			{
				cfr=similarColor(ds->BlindGetPixelColorRQ(x1,pt.y-1),oldColor,threshold);
				if(!spanLeft && cfr)
				{
					tempPt.x=x1;
					tempPt.y=pt.y-1;
					stack.push(tempPt);
					spanLeft=true;
				}
				else if(spanLeft && !cfr)
					spanLeft=false;
			}
			if(pt.y<bounds.bottom)
			{
				cfr=similarColor(ds->BlindGetPixelColorRQ(x1,pt.y+1),oldColor,threshold);
				if(!spanRight && cfr)
				{
					tempPt.x=x1;
					tempPt.y=pt.y+1;
					stack.push(tempPt);
					spanRight=true;
				}
				else if(spanRight && !cfr)
					spanRight=false;
			}
			x1++;
		}
    }
	if(clipRgn!=NULL)
		DeleteRgn(clipRgn);
#ifdef TIME_FLOODFILL
	QueryPerformanceCounter(&endCount);
	double secs=(endCount.QuadPart-beginCount.QuadPart)/(double)frequency.QuadPart;
	TCHAR buffer[CAPTIONBUFFERSLENGTH];
	_sntprintf(buffer,ARRSIZE(buffer),_T("%f"),secs);
	MessageBox(0,buffer,_T("Debug message"),MB_ICONINFORMATION);
#endif
}