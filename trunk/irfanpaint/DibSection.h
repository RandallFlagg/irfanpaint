#pragma once
//The DibSection class incapsulates a DibSection obtained from a DIB created in a
//memory-mapped file and a compatible DC in which the DibSection is selected.
class DibSection
{
private:
	//Handle to the DibSection
	HBITMAP dsHandle;
	//Compatible DC in which is selected dsHandle
	HDC compDC;
	//Pointer to the bits of the DIB
	LPBYTE bmpData;
	//Pointer to the headers (followed by the DIB bits if the DibSection is packed)
	LPBITMAPINFO headers;
	//Size of a line in bytes
	size_t bytesPerLine;
	//Size of the data part of the image in bytes
	size_t bmpDataSize;
	//Size of the headers
	size_t headersSize;
	//Handle to the memory mapped file
	HANDLE memoryMappedFile;
	//Indicates if the DIB is packed
	bool packed;
	//Returns the number of color entries (in the palette) of a DIB
	static inline int numColorEntries(WORD nBitsPerPixel)
	{
	    int nColors = 0;
		switch (nBitsPerPixel)
		{
			case 1:  nColors = 2;   break;
			case 4:  nColors = 16;  break;
			case 8:  nColors = 256; break;
			case 16:
			case 24:
			case 32: nColors = 0;   break; // 16,24 or 32 bpp have no color table

			default:
				throw std::invalid_argument(ERROR_STD_PROLOG "Unsupported nBitsPerPixel value. The supported bit depth values are 1, 4, 8, 16, 24, 32.");
		}
		return nColors;
	};
	//Returns the offset from the beginning of the DIB data section for the given bitmap row (private version, without checks)
	inline size_t getRowOffset(int y)
	{
		//The rows usually are stored in reverse order
		return (headers->bmiHeader.biHeight-1-y)*bytesPerLine;
	};
	//Disposes the class
	void dispose();
	//Zeroes all the fields of the class
	void zeroFields();
	//Inits the DibSection
	void init(LPBITMAPINFO Headers, LPBYTE BmpData, bool Packed, bool FromPacked);
	//Performs the pre-initialization part common to packed and unpacked DIB
	void commonPreInit(LPBITMAPINFO Headers);
	//Inits the DibSection keeping internally a packed DIB
	void initPacked(LPBITMAPINFO Headers);
	//Inits the DibSection letting CreateDIBSection manage the memory as it wants
	void initUnpacked(LPBITMAPINFO Headers);
	//Performs the post-initialization part common to packed and unpacked DIB
	void commonPostInit(LPBYTE BmpData);
public:
	//Constructor from an already existent packed DIB
	DibSection(LPBYTE SourceDIB, bool Packed);
	//Constructor from scratch
	DibSection(LPBITMAPINFO Headers, LPBYTE BmpData, bool Packed);
	//Constructor from scratch with only often used fields
	DibSection(DWORD Width, DWORD Height, WORD BitsPerPixel, RGBQUAD * Palette, bool Packed);
	//Constructor from another DibSection (copy constructor)
	DibSection(DibSection & Other);
	//Constructor from another DibSection (w/ packed/unpacked choice)
	DibSection(DibSection & Other, bool Packed);
	//Destructor
	~DibSection(void)
	{
		dispose();
	};
	//Returns the bytes that a line of the DIB takes
	static inline int GetBytesPerLine(int nWidth, WORD nBitsPerPixel)
	{
		return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
	};
	//Returns true if the DIB is packed
	inline bool IsPacked()
	{
		return packed;
	};
	//Returns the handle of the DibSection
	inline HBITMAP GetDSHandle()
	{
		return dsHandle;
	};
	//Returns the pointer to the internal packed DIB
	inline LPBYTE GetPackedDIB()
	{
		if(!packed)
			throw std::runtime_error(ERROR_STD_PROLOG "This DibSection instance isn't packed.");
		return (LPBYTE)headers;
	};
	//Returns the pointer to the headers
	inline LPBITMAPINFO GetHeaders()
	{
		return headers;
	};
	//Returns the pointer to the DIB bits
	inline LPBYTE GetBmpData()
	{
		return bmpData;
	};
	//Returns compDC if it is up to date, otherwise creates a new DibSection and selects it in _compDC
	inline HDC GetCompDC()
	{
		return compDC;
	};
	//Returns the number of palette entries of the DIB
	inline int GetPaletteEntries()
	{
		return numColorEntries(headers->bmiHeader.biBitCount);
	};
	//Returns a pointer to the palette if the DIB has one, otherwise returns NULL
	inline RGBQUAD * GetPalette()
	{
		return IsIndexed()?headers->bmiColors:NULL;
	};
	//Returns true if the image is indexed
	inline bool IsIndexed()
	{
		return headers->bmiHeader.biClrUsed!=0;
	};
	//Returns the image size (in bytes)
	inline size_t GetBmpDataSize()
	{
		return bmpDataSize;
	};
	//Returns the headers size (in bytes)
	inline size_t GetHeadersSize()
	{
		return headersSize;
	};
	//Returns the image dimensions
	inline SIZE GetDibSize()
	{
		SIZE ret={headers->bmiHeader.biWidth,headers->bmiHeader.biHeight};
		return ret;
	};
	//Returns the width of the DIB
	inline long GetDibWidth()
	{
		return headers->bmiHeader.biWidth;
	};
	//Returns the height of the DIB
	inline long GetDibHeight()
	{
		return headers->bmiHeader.biHeight;
	};
	//The "Blindxxx" functions omit argument check; use with caution
	//Returns the palette index of the specified pixel
	inline BYTE GetPixelIndex(const long x, const long y)
	{
		ArgsInRange(x,y);
		if(!IsIndexed())
			throw std::logic_error(ERROR_STD_PROLOG "The DIB is not indexed.");
		return BlindGetPixelIndex(x,y);
	};
	BYTE BlindGetPixelIndex(const long x,const long y);
	//Returns the color of the specified pixel
	//As COLORREF (slower)
	COLORREF GetPixelColorCR(const long x, const long y)
	{
		return RQ2CR(GetPixelColorRQ(x,y));
	};
	COLORREF BlindGetPixelColorCR(const long x, const long y)
	{
		return RQ2CR(BlindGetPixelColorRQ(x,y));
	};
	//As RGBQUAD (faster)
	RGBQUAD GetPixelColorRQ(const long x, const long y)
	{
		ArgsInRange(x,y);
		return BlindGetPixelColorRQ(x,y);
	}
	RGBQUAD BlindGetPixelColorRQ(const long x, const long y);
	//Sets the color of the specified pixel
	//As RGBQUAD (faster)
	void BlindSetPixelColorRQ(const long x, const long y, RGBQUAD color);
	void SetPixelColorRQ(const long x, const long y, RGBQUAD color)
	{
		ArgsInRange(x,y);
		BlindSetPixelColorRQ(x,y,color);
	};
	//As COLORREF (may be slower)
	void SetPixelColorCR(const long x, const long y, COLORREF color)
	{
		//If color is a "false" COLORREF created with DIBINDEX just call SetPixelIndex
		if(HIWORD(color)==0x10FF)
		{
			SetPixelIndex(x,y,(BYTE)LOWORD(color));
		}
		else
			SetPixelColorRQ(x,y,CR2RQ(color));
	};
	void BlindSetPixelColorCR(const long x, const long y, COLORREF color)
	{
		//If color is a "false" COLORREF created with DIBINDEX just call SetPixelIndex
		if(HIWORD(color)==0x10FF)
		{
			BlindSetPixelIndex(x,y,(BYTE)LOWORD(color));
		}
		else
			BlindSetPixelColorRQ(x,y,CR2RQ(color));
	};
	//Set the palette index of the specified pixel (in indexed DIBs)
	void BlindSetPixelIndex(const long x, const long y, BYTE index);
	void SetPixelIndex(const long x, const long y, BYTE index)
	{
		ArgsInRange(x,y);
		if(!IsIndexed())
			throw std::logic_error(ERROR_STD_PROLOG "The DIB is not indexed.");
		BlindSetPixelIndex(x,y,index);
	};
	//Get the palette index corresponding to the given color
	//Color as RGBQUAD (faster)
	BYTE BlindGetPaletteIndex(RGBQUAD color)
	{
		const int paletteEntries=numColorEntries(headers->bmiHeader.biBitCount);
		for(BYTE index=0; index<paletteEntries; index++)
		{
			if(
				headers->bmiColors[index].rgbRed==color.rgbRed &&
				headers->bmiColors[index].rgbGreen==color.rgbGreen &&
				headers->bmiColors[index].rgbBlue==color.rgbBlue
				)
                return index;
		};
		throw std::invalid_argument(ERROR_STD_PROLOG "The given color is not available in the palette.");
	}
	BYTE GetPaletteIndex(RGBQUAD color)
	{
		if(!IsIndexed())
			throw std::logic_error(ERROR_STD_PROLOG "The DIB is not indexed");
        return BlindGetPaletteIndex(color);		
	};
	//Color as COLORREF (slower)
	inline BYTE GetPaletteIndex(COLORREF color)
	{
		return GetPaletteIndex(CR2RQ(color));
	};
	inline BYTE BlindGetPaletteIndex(COLORREF color)
	{
		return BlindGetPaletteIndex(CR2RQ(color));
	};
	//Checks if the arguments are in range
	inline void ArgsInRange(const long x, const long y)
	{
		if(!BlindArgsInRange(x,y))
			throw std::out_of_range(ERROR_STD_PROLOG "x or y not in valid range.");
	};
	//Checks if the arguments are in range; this version does not throw any exception and does not check the class state
	inline bool BlindArgsInRange(const long x, const long y) throw()
	{
		return !((x<0)||(y<0)||(x>=headers->bmiHeader.biWidth)||(y>=headers->bmiHeader.biHeight));
	};
	//Converts a COLORREF to a RGBQUAD
	inline static RGBQUAD CR2RQ(COLORREF color)
	{
		RGBQUAD rgb;
		rgb.rgbRed=GetRValue(color);
		rgb.rgbGreen=GetGValue(color);
		rgb.rgbBlue=GetBValue(color);
		return rgb;
	};
	//Converts a RGBQUAD to a COLORREF
	inline static COLORREF RQ2CR(RGBQUAD color)
	{
		return RGB(color.rgbRed,color.rgbGreen,color.rgbBlue);
	};
	//Get the clipping region of the DC of the dibsection and the bounds of it; if the bounds are enough to determine where to draw region is set to NULL
	void GetClippingRegionStuff(HRGN & region, RECT & bounds);

};