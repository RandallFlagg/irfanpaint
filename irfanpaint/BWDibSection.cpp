#include "StdAfx.h"
#include ".\bwdibsection.h"
//#define BWDIBSECTION_DIAGNOSTIC
#ifdef BWDIBSECTION_DIAGNOSTIC
#include "Globals.h"
#endif
BWDibSection::BWDibSection(long Width, long Height)
{
	//Argument check
	if(Width<=0)
		throw std::out_of_range(ERROR_STD_PROLOG "Width must be a positive integer.");
	if(Height<=0)
		throw std::out_of_range(ERROR_STD_PROLOG "Height must be a positive integer.");
	//Prepare the header
	BWBITMAPINFO bi={0};
	bi.bh.biSize=sizeof(bi.bh);
	bi.bh.biBitCount=1;
	bi.bh.biWidth=Width;
	bi.bh.biHeight=Height;
	bi.bh.biPlanes=1;
	bi.bh.biCompression=BI_RGB;
	//Setup the colors
	//Black
	bi.colors[0].rgbRed=bi.colors[0].rgbGreen=bi.colors[0].rgbBlue=0;
	//White
	bi.colors[1].rgbRed=bi.colors[1].rgbGreen=bi.colors[1].rgbBlue=255;
	//Init
	init(bi);
}
BWDibSection::BWDibSection(const BWBITMAPINFO & DibInfo)
{
	//Init
	init(DibInfo);
}
void BWDibSection::init(const BWBITMAPINFO & DibInfo)
{
	//A little check
	if(DibInfo.bh.biBitCount!=1)
		throw std::invalid_argument(ERROR_STD_PROLOG "DibInfo.bh.biBitCount must be 1; BWDibSection can handle only B/W bitmaps.");
	//Store the dib header
	dibInfo=DibInfo;
	//Store the size of a row
	bytesPerLine=getBytesPerLine(dibInfo.bh.biWidth,dibInfo.bh.biBitCount);
	//Create the DC
	if((compDC=CreateCompatibleDC(NULL))==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the compatible DC; CreateCompatibleDC returned NULL.");
	//Create the dibsection
	if((dsHandle=CreateDIBSection(NULL,(BITMAPINFO *)&dibInfo,DIB_RGB_COLORS,(LPVOID *)&dibBits,NULL,0))==NULL)
	{
#ifdef BWDIBSECTION_DIAGNOSTIC
		MessageBox(hMainWindow,_T("Now it's time to make the minidump."),_T("Debug message"),MB_ICONINFORMATION);
		Sleep(30000);
#endif
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the dibsection; CreateDIBSection returned NULL.\nDibInfo:" + BWBI2String(dibInfo));
	}
	SelectBitmap(compDC,dsHandle);
}

BWDibSection::~BWDibSection(void)
{
	//Delete the DC
	DeleteDC(compDC);
	//Delete the dibsection
	DeleteBitmap(dsHandle);
}

//Returns a string representation of a BWBITMAPINFO structure
std::string BWDibSection::BWBI2String(const BWBITMAPINFO & DibInfo)
{
#pragma push_macro("OUTFIELD")
#pragma push_macro("OUTFIELDPREF")
#define OUTFIELD(field)					<<std::endl<<"\t"<<#field ": "<<DibInfo.field
#define OUTFIELDPREF(field,prefix)	<<std::endl<<"\t"<<#field ": "<<prefix DibInfo.field
	std::ostringstream os;
	os
		OUTFIELD(bh.biSize)
		OUTFIELD(bh.biWidth)
		OUTFIELD(bh.biHeight)
		OUTFIELD(bh.biPlanes)
		OUTFIELD(bh.biBitCount)
		OUTFIELD(bh.biCompression)
		OUTFIELD(bh.biSizeImage)
		OUTFIELD(bh.biXPelsPerMeter)
		OUTFIELD(bh.biYPelsPerMeter)
		OUTFIELD(bh.biClrUsed)
		OUTFIELD(bh.biClrImportant)
		OUTFIELDPREF(colors[0],"0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<*(DWORD*)&)
		OUTFIELDPREF(colors[1],"0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<*(DWORD*)&);
    return os.str();
#undef OUTFIELD
#undef OUTFIELDPREF
#pragma pop_macro("OUTFIELD")
#pragma pop_macro("OUTFIELDPREF")
}