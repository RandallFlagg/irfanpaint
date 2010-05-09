#include "StdAfx.h"
#include ".\colorreplacer.h"
#include "Utils.h"

//Performs the color replacement
void ColorReplacer::replaceColor(DibSection * ds, COLORREF fgColor, COLORREF bgColor, BYTE threshold)
{
	SIZE rgnSize=ds->GetDibSize();
	RGBQUAD bgColorRq=DibSection::CR2RQ(bgColor);
	//Palette index of the new color (if the image is indexed)
	BYTE fgColorIndex=0;
	//Is the image indexed?
	bool indexed=ds->IsIndexed();
	//Clipping region
	HRGN clipRgn=NULL;
	//Bounds of the replacing
	RECT bounds;
	ds->GetClippingRegionStuff(clipRgn,bounds);
	//Change rgnSize by consequence
	rgnSize.cx=bounds.right-bounds.left+1;
	rgnSize.cy=bounds.bottom-bounds.top+1;
	//Change the bounds to minimize the visited area
	bounds.left=max(bounds.left,maskOffset.x);
	bounds.right=min(bounds.right,maskOffset.x+usedSize.cx-1);
	bounds.top=max(bounds.top,maskOffset.y);
	bounds.bottom=min(bounds.bottom,maskOffset.y+usedSize.cy-1);
	if(indexed)
	{
		//If the image is indexed get the index of the new color
		fgColorIndex=ds->GetPaletteIndex(fgColor);
	}
	for(int x=bounds.left;x<=bounds.right;x++)
	{
		for(int y=bounds.top;y<=bounds.bottom;y++)
		{
			if(!mask->BlindGetPixel(x-maskOffset.x,y-maskOffset.y))
			{
				if(clipRgn!=NULL && !PtInRegion(clipRgn,x,y))
					continue;
				if(similarColor2(bgColorRq,ds->BlindGetPixelColorRQ(x,y),threshold))
				{
					if(indexed)
						ds->BlindSetPixelIndex(x,y,fgColorIndex);
					else
						ds->BlindSetPixelColorCR(x,y,fgColor);
				}
			}
		}
	}
	return;
}