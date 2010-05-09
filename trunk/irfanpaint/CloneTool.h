#pragma once
#include "MaskedTool.h"
#include "DibSection.h"

//Structure that identifies a shift
struct SHIFT
{
	long x;
	long y;
};
//Clone tool class
class CloneTool : public MaskedTool
{
private: //Private declarations
	//Performs the cloning
	void clone(DibSection * ds, SHIFT shift);
public: //Public declarations
	//Clones a line (2nd overload)
	inline void CloneLine(
		DibSection * ds, //DibSection on which operate
		POINT beginPt, //Begin point of the line
		POINT endPt, //End point of the line
		HPEN pen, //Pen
		SHIFT shift //Shift
		)
	{
		drawLine(beginPt,endPt,pen);
		clone(ds, shift); //Perform the cloning
		return;
	};
	//Default constructor
	inline CloneTool(void) {return;};
	//Destructor
	inline ~CloneTool(void) {return;};
};