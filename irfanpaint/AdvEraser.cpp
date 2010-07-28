#include "StdAfx.h"
#include ".\AdvEraser.h"
#include "Utils.h"

//Performs the erasing
void AdvEraser::erase(DibSection * ds)
{
	//PLSPEC: NT only
	MaskBlt(ds->GetCompDC(),maskOffset.x,maskOffset.y,usedSize.cx,usedSize.cy,
		backupDibSection->GetCompDC(),maskOffset.x,maskOffset.y,
		mask->GetDSHandle(),0,0,
		MAKEROP4(ROP3_NOP,SRCCOPY));
	return;
}

//Stores a new backup DibSection
void AdvEraser::StoreNewDS(DibSection * ds)
{
	backupDibSection.reset(new DibSection(*ds,false));
}