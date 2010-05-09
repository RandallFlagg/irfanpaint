#include "StdAfx.h"			//Standard headers
#include ".\dibsection.h"	//DibSection.cpp header

//Performs the initialization part common to packed and unpacked DIB
void DibSection::commonPreInit(LPBITMAPINFO Headers)
{
	zeroFields();
	//Calculate the size of the headers
	headersSize = sizeof(BITMAPINFOHEADER) + numColorEntries(Headers->bmiHeader.biBitCount)*sizeof(RGBQUAD);
	//Calculate the size of each line
	bytesPerLine=GetBytesPerLine(Headers->bmiHeader.biWidth, Headers->bmiHeader.biBitCount);
	//Calculate the size of the image
	bmpDataSize = Headers->bmiHeader.biSizeImage;
	if(bmpDataSize==0)
	{
		bmpDataSize = bytesPerLine * Headers->bmiHeader.biHeight;
	}
	//Eventually update the biClrUsed field
	if(Headers->bmiHeader.biClrUsed==0)
	{
		Headers->bmiHeader.biClrUsed=numColorEntries(Headers->bmiHeader.biBitCount);
	}
}

//Performs the post-initialization part common to packed and unpacked DIB
void DibSection::commonPostInit(LPBYTE BmpData)
{
	//Eventually copy the bitmap data in the new DIB
	if(BmpData!=NULL)
		memcpy(bmpData,BmpData,bmpDataSize);
	/*else
		memset(bmpData,0,bmpDataSize);*/
	//Create the compatible DC
	compDC=CreateCompatibleDC(NULL);
	if(compDC==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the compatible DC; CreateCompatibleDC returned NULL.");
	//Select the DibSection in the compatible DC
	SelectObject(compDC,dsHandle);
}

//Inits the DibSection keeping internally a packed DIB
void DibSection::initPacked(LPBITMAPINFO Headers)
{
	packed=true;
	//Create the memory mapped file
	memoryMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)(headersSize + bmpDataSize), NULL);
	if(memoryMappedFile==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the memory mapped file; CreateFileMapping returned NULL.");
	//Map the file in memory
	headers=(LPBITMAPINFO)MapViewOfFile(memoryMappedFile,FILE_MAP_ALL_ACCESS,0,0,0);
	if(headers==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot map the memory mapped file; MapViewOfFile returned NULL.");
	//Copy the headers in the new memory-mapped file
	memcpy(headers,Headers,headersSize);
	//Create the DibSection
	dsHandle=CreateDIBSection(NULL,headers,DIB_RGB_COLORS,(VOID **)&bmpData,memoryMappedFile,(DWORD)headersSize);
	if(dsHandle==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the DibSection; CreateDIBSection returned NULL.");
}

//Inits the DibSection letting CreateDIBSection manage the memory as it wants
void DibSection::initUnpacked(LPBITMAPINFO Headers)
{
	packed=false;
	//Create the DibSection
	dsHandle=CreateDIBSection(NULL,Headers,DIB_RGB_COLORS,(VOID **)&bmpData,NULL,0);
	if(dsHandle==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the DibSection; CreateDIBSection returned NULL.");
	headers=(LPBITMAPINFO)new BYTE[headersSize];
	memcpy((void *)headers,Headers,headersSize);
}

void DibSection::init(LPBITMAPINFO Headers, LPBYTE BmpData, bool Packed, bool FromPacked)
{
	try
	{
		//Common pre-initialization
		commonPreInit(Headers);
		//If Headers is from a packed DIB if we need it now we can compute the address of BmpData
		if(FromPacked && BmpData==NULL)
			BmpData=(LPBYTE)Headers+headersSize;
		//Actually create the DibSection
		if(Packed)
			initPacked(Headers);
		else
			initUnpacked(Headers);
		//Finish the last operations
		commonPostInit(BmpData);
	}
	catch(...)
	{
		//If an exception occours release any unreleased object before dying
		dispose();
		throw;
	}
}

//Constructor from an already existent packed DIB
DibSection::DibSection(LPBYTE SourceDIB, bool Packed)
{
	init((LPBITMAPINFO)SourceDIB,NULL,Packed,true);
}

//Constructor from scratch
DibSection::DibSection(LPBITMAPINFO Headers, LPBYTE BmpData, bool Packed)
{
	init(Headers,BmpData,Packed,false);
}

//Constructor from another DibSection (copy constructor)
DibSection::DibSection(DibSection & Other)
{
	init(Other.GetHeaders(),Other.GetBmpData(),Other.IsPacked(),false);
}

//Constructor from another DibSection (w/ packed/unpacked choice)
DibSection::DibSection(DibSection & Other, bool Packed)
{
	init(Other.GetHeaders(),Other.GetBmpData(),Packed,false);
}

//Constructor from scratch with only often used fields
DibSection::DibSection(DWORD Width, DWORD Height, WORD BitsPerPixel, RGBQUAD * Palette, bool Packed)
{
	//Calculate the structure size
	int colorNumber=numColorEntries(BitsPerPixel);
	size_t structSize=sizeof(BITMAPINFOHEADER)+colorNumber;
	//Create the headers
	boost::scoped_ptr<BITMAPINFO> bi((BITMAPINFO *)new unsigned char[structSize]);
	//Zero the fields
	memset(bi.get(),0,structSize);
	//Init the headers
	bi->bmiHeader.biSize=sizeof(bi->bmiHeader);
	bi->bmiHeader.biBitCount=BitsPerPixel;
	bi->bmiHeader.biWidth=Width;
	bi->bmiHeader.biHeight=Height;
	bi->bmiHeader.biPlanes=1;
	bi->bmiHeader.biCompression=BI_RGB;
	//Copy the colors
	if(colorNumber>0)
	{
		for(int i=0;i<colorNumber;i++)
			bi->bmiColors[i]=Palette[i];
	}
	//Init the class
	init(bi.get(),NULL,Packed,false);
}

/*
//Constructor from a packed DIB
DibSection::DibSection(LPBYTE SourceDIB)
{
	//Zero the fields
	zeroFields();
	try
	{
		//Step 1: calculate image sizes
		//Consider the first header
		LPBITMAPINFO bi=(LPBITMAPINFO)SourceDIB;

		//Step 2: create the DibSection
		//Create the memory mapped file
		memoryMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)(headersSize + imageSize), NULL);
		if(memoryMappedFile==NULL)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the memory mapped file; CreateFileMapping returned NULL.");
		//Map the file in memory
		headers=(LPBITMAPINFO)MapViewOfFile(memoryMappedFile,FILE_MAP_ALL_ACCESS,0,0,0);
		if(headers==NULL)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot map the memory mapped file; MapViewOfFile returned NULL.");
		//Copy the source DIB in the new memory-mapped file
		memcpy(headers,SourceDIB,imageSize+headersSize);
		//Create the DibSection
		dsHandle=CreateDIBSection(NULL,headers,DIB_RGB_COLORS,(void **)&bmpData,memoryMappedFile,(DWORD)headersSize);
		if(dsHandle==NULL)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the DibSection; CreateDIBSection returned NULL.");
		//Select the DibSection in the compatible DC
		SelectObject(compDC,dsHandle);
	}
	catch(...)
	{
		dispose();
		throw;
	}
}
*/
//Disposes the class
void DibSection::dispose()
{
	if(compDC!=NULL)
		DeleteDC(compDC);
	if(dsHandle!=NULL)
		DeleteObject(dsHandle);
	if(headers!=NULL)
	{
		if(packed)
			UnmapViewOfFile(headers);
		else
			delete[] headers;
	}
	if(memoryMappedFile!=NULL)
		CloseHandle(memoryMappedFile);
	zeroFields();
}
//Zeroes all the fields of the class
void DibSection::zeroFields()
{
	dsHandle=NULL;
	compDC=NULL;
	bmpData=NULL;
	headers=NULL;
	memoryMappedFile=NULL;
	bytesPerLine=bmpDataSize=headersSize=0;
}
/*
//Initializes the class
void DibSection::init(LPVOID hDIB)
{
	HDC tDC=GetDC(NULL);
	//Create the memory-mapped file
	m_memFMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwBitmapInfoSize + dwImageSize, NULL);
	if(m_memFMapping==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the memory-mapped file; CreateFileMapping returned NULL.");
	//Map the file in memory
	headers=(LPBITMAPINFO)MapViewOfFile(m_memFMapping,FILE_MAP_ALL_ACCESS,0,0,0);
	if(headers==NULL)
	{
		dispose(false);
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot map the memory-mapped file in memory; MapViewOfFile returned NULL.");
	}
	//Copy the DIB in the new memory-mapped file
	memcpy(headers,hDIB,dwImageSize+dwBitmapInfoSize);
	//Create the DibSection
	m_hBmpDIB = CreateDIBSection(tDC, lpBI, DIB_RGB_COLORS, (LPVOID *)&bmpData, m_memFMapping, dwBitmapInfoSize);
	ReleaseDC(NULL, tDC);
	if(m_hBmpDIB==FALSE)
	{
		dispose(false);
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the DIBSection; CreateDIBSection returned NULL.");
	}
	//Free the old DIB
	GlobalUnlock(*m_pImgDIB);
	GlobalFree(*m_pImgDIB);
	//Set the new DIB in pImgDIB
	*m_pImgDIB=headers;
	//Create the compatible DC
	compDC=CreateCompatibleDC(NULL);
	if(!compDC)
		throw std::runtime_error(ERROR_STD_PROLOG "Cannot create the compatible DC; CreateCompatibleDC returned NULL.");
	//Select the DibSection in the DC
	m_exBmp=SelectObject(compDC,m_hBmpDIB);
	if(m_InitCBFunc!=NULL)
		m_InitCBFunc(this);
}*/

//Returns the palette index of the specified pixel (no checks version)
BYTE DibSection::BlindGetPixelIndex(const long x,const long y)
{
	if (headers->bmiHeader.biBitCount==8)
		return bmpData[getRowOffset(y) + x];
	else
	{
		BYTE pos;
		BYTE iDst=bmpData[getRowOffset(y) + (x*headers->bmiHeader.biBitCount >> 3)];
		if(headers->bmiHeader.biBitCount==4)
		{
			pos = (BYTE)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (BYTE)(iDst >> pos);
		}
		else if(headers->bmiHeader.biBitCount==1)
		{
			pos = (BYTE)(7-x%8);
			iDst &= (0x01<<pos);
			return (BYTE)(iDst >> pos);
		}
	}
	return 0;
}
//Returns the color of the specified pixel (no checks version)
RGBQUAD DibSection::BlindGetPixelColorRQ(const long x, const long y)
{
	RGBQUAD rgb;
	if (headers->bmiHeader.biClrUsed)
		rgb = GetPalette()[BlindGetPixelIndex(x,y)];
	else
	{
		BYTE* iDst  = bmpData + getRowOffset(y) + x*3;
		rgb.rgbBlue = *iDst++;
		rgb.rgbGreen= *iDst++;
		rgb.rgbRed  = *iDst;
	}
	return rgb;
}
//Sets the color of the specified pixel (no checks version)
void DibSection::BlindSetPixelColorRQ(long x,long y,RGBQUAD c)
{
	if (headers->bmiHeader.biClrUsed)
	{
		BlindSetPixelIndex(x,y,BlindGetPaletteIndex(c));
	}
	else
	{
		BYTE* iDst  = bmpData + getRowOffset(y) + x*3;
		*iDst++ = c.rgbBlue;
		*iDst++ = c.rgbGreen;
		*iDst   = c.rgbRed;
	}
}
//Sets the palette index of the specified pixel (no checks version)
void DibSection::BlindSetPixelIndex(const long x, const long y, BYTE index)
{
	if(headers->bmiHeader.biBitCount==8)
	{
		bmpData[getRowOffset(y) + x]=index;
		return;
	}
	else
	{
		BYTE pos;
		BYTE* iDst=&bmpData[getRowOffset(y) + (x*headers->bmiHeader.biBitCount >> 3)];
		if(headers->bmiHeader.biBitCount==4)
		{
			pos = (BYTE)(4*(1-x%2));
			*iDst &= ~(0x0F<<pos);
			*iDst |= ((index & 0x0F)<<pos);
			return;
		}
		else if(headers->bmiHeader.biBitCount==1)
		{
			pos = (BYTE)(7-x%8);
			*iDst &= ~(0x01<<pos);
			*iDst |= ((index & 0x01)<<pos);
			return;
		}
	}
}
//Get the clipping region of the DC of the dibsection and the bounds of it; if the bounds are enough to determine where to draw region is set to NULL
void DibSection::GetClippingRegionStuff(HRGN & region, RECT & bounds)
{
	SIZE rgnSize=GetDibSize();
	bounds.left=0;
	bounds.top=0;
	bounds.right=rgnSize.cx-1;
	bounds.bottom=rgnSize.cy-1;
	region=CreateRectRgn(0,0,1,1); //dummy region
	if(GetClipRgn(GetCompDC(),region)==1)
	{
		//There's a clipping region
		//Determine the type of the region and its box
		switch(GetRgnBox(region,&bounds))
		{
		case 0:
		case NULLREGION:
		case SIMPLEREGION:
			//bounds are enough
			DeleteRgn(region);
			region=NULL;
			break;
		case COMPLEXREGION:
            //We need to PtInRegion every time
			break;
		default:
			//Impossible
			throw std::runtime_error(ERROR_STD_PROLOG "GetRgnBox returned an unexpected value.");
			break;
		}
	}
	else
	{
		DeleteRgn(region);
		region=NULL;
	}
	//Determine the smallest possible rectangle to fill
	bounds.left=max(0,bounds.left);
	bounds.top=max(0,bounds.top);
	bounds.right=min(rgnSize.cx-1,bounds.right);
	bounds.bottom=min(rgnSize.cy-1,bounds.bottom);
}