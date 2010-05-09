#pragma once

class BWDibSection
{
public:
	//BW DIB header and color table
	struct BWBITMAPINFO
	{
		BITMAPINFOHEADER bh;
		RGBQUAD colors[2];
	};
private:
	//Bitmap header and color table
	BWBITMAPINFO dibInfo;
	//Handle of the dibsection
	HBITMAP dsHandle;
	//Compatible DC in which is selected dsHandle
	HDC compDC;
	//Pointer to the bits of the DIB
	LPBYTE dibBits; //not actually an array of bytes, but the BYTE * makes it simple to calculate byte offset
	//Size of a line in bytes
	size_t bytesPerLine;
	//Init the class
	void init(const BWBITMAPINFO & DibInfo);
	//Returns the bytes that a line of the DIB takes
	static inline int getBytesPerLine(int nWidth, int nBitsPerPixel)
	{
		return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
	};
	//Returns the offset from the beginning of the DIB data section for the given bitmap row (private version, without checks)
	inline size_t getRowOffset(int y) const
	{
		//The rows usually are stored in reverse order
		return (dibInfo.bh.biHeight-1-y)*bytesPerLine;
	};
	//Returns a string representation of a BWBITMAPINFO structure
	static std::string BWBI2String(const BWBITMAPINFO & DibInfo);
public:
	//Constructors
	BWDibSection(long Width, long Height);
	BWDibSection(const BWBITMAPINFO & DibInfo);
	//Destructor
	~BWDibSection(void);
	//Public read-only wrappers
	//Bitmap header and color table
	BWBITMAPINFO GetDibInfo() const{ return dibInfo; };
	//Handle of the dibsection; do not delete it
	HBITMAP GetDSHandle() const{ return dsHandle; };
	//Compatible DC in which it's selected dsHandle; do not delete it
	HDC GetCompDC() const { return compDC; };
	//Bits of the DIB
	BYTE * GetDibBits() const { return dibBits; };
	//Size getters
	//Returns the size of the DIB
	SIZE GetDibSize() const
	{
		SIZE ret;
		ret.cx=dibInfo.bh.biWidth;
		ret.cy=dibInfo.bh.biHeight;
		return ret;
	};
	//Returns the width of the DIB
	long GetDibWidth() const { return dibInfo.bh.biWidth; };
	//Returns the height of the DIB
	long GetDibHeight() const { return dibInfo.bh.biHeight; };
	//Pixel functions
	//blindxxx functions perform no check on the arguments - be careful
	//Gets the state of the given pixel of the mask
	bool BlindGetPixel(const long x, const long y) const
	{
		return (dibBits[getRowOffset(y) + (x >> 3)] & (0x01<<(BYTE)(7-x%8)))!=0;
	};
	bool GetPixel(const long x, const long y) const
	{
		CheckArgs(x,y);
		return BlindGetPixel(x,y);
	};
	//Sets the state of the given pixel of the mask
	void BlindSetPixel(const long x, const long y, bool state)
	{
		BYTE pos=(BYTE)(7-x%8);
		BYTE* iDst=&dibBits[getRowOffset(y) + (x >> 3)];
		*iDst &= ~(0x01<<pos);
		*iDst |= ((state & 0x01)<<pos);
	};
	void SetPixel(const long x, const long y, bool state)
	{
		CheckArgs(x,y);
		BlindSetPixel(x,y,state);
	};
	//Check if the x and y are valid for the current mask
	inline void CheckArgs(const long x, const long y) const
	{
		if(x<0 || x>=dibInfo.bh.biWidth)
			throw std::out_of_range(ERROR_STD_PROLOG "x not in valid range.");
		if(y<0 || y>=dibInfo.bh.biHeight)
			throw std::out_of_range(ERROR_STD_PROLOG "y not in valid range.");
	};
};
