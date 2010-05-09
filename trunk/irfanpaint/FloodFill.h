#pragma once

class DibSection;
class BWDibSection;
//Floodfill function - decides the fastest routine to use - always call this function, not the other ones
void AdvFloodFill(DibSection * ds, POINT startPt, HBRUSH fillBrush, BYTE threshold);
//Scanline floodfill algorithm - color version
void FloodFillScanlineStack(DibSection * ds, POINT startPt, COLORREF newColor, BYTE threshold);
//Scanline floodfill algorithm - brush version
void FloodFillScanlineStack(DibSection * ds, POINT startPt, HBRUSH fillBrush, BYTE threshold);
//Scanline floodfill algorithm - internal color version
void InternalFloodFillScanlineStack(DibSection * ds, BWDibSection * outMask, POINT startPt, COLORREF newColor, BYTE threshold);