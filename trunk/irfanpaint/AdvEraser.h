#pragma once
#include "maskedtool.h"
#include "DibSection.h"

class AdvEraser :
	public MaskedTool
{
private:
	DibSection * backupDibSection;
	//Performs the erasing
	void erase(DibSection * ds);
public:
	//Check the status of the class
	inline void CheckClass()
	{
		if(backupDibSection==NULL)
			throw std::logic_error(ERROR_STD_PROLOG "The class has already been disposed.");
	};
	//Default constructor
	inline AdvEraser(DibSection * ds=NULL)
	{
		backupDibSection=NULL;
		if(ds!=NULL)
			StoreNewDS(ds);
		return;
	};
	//Default destructor
	inline ~AdvEraser(void)
	{
		try
		{
			DisposeDS();
		}
		catch(...)
		{
		}
		return;	
	};
	//Stores a backup DibSection
	void StoreNewDS(DibSection * ds);
	//Disposes the backup DibSection
	void DisposeDS();
	
	//Erases the specified line restoring the original DIB
	inline void EraseLine(
		DibSection * ds, //DibSection on which operate
		POINT beginPt, //Begin point of the line
		POINT endPt, //End point of the line
		HPEN pen, //Pen
		int penWidth=-1 //Pen width (optional)
		)
	{
		CheckClass();
		if(penWidth>0)
			drawLine(beginPt,endPt,pen,penWidth);
		else
			drawLine(beginPt,endPt,pen);
		erase(ds);
		return;
	};
};