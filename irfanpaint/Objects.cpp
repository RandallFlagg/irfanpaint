#include "StdAfx.h"
#ifdef INCLUDE_OBJECTS
#include "Objects.h"
#include "DibSection.h"
#include "BWDibSection.h"


void Objects::RenderedObject::SyncBounds(bool UpdateRendering)
{
	//Update the bounds
	bounds=innerObject->GetBounds();
	//Check if the bounds have changed
	if(!rendering || RECTWIDTH(bounds)!=rendering->GetDibWidth() || RECTHEIGHT(bounds)!=rendering->GetDibHeight())
	{
		//If they have changed recreate the rendering...
		mask.reset(new BWDibSection(RECTWIDTH(bounds),RECTHEIGHT(bounds)));
		rendering.reset(new DibSection(RECTWIDTH(bounds),RECTHEIGHT(bounds),24,NULL,false));
		if(UpdateRendering)
			this->UpdateRendering();
	}
}

void Objects::RenderedObject::UpdateRendering()
{
	//Sync the bounds
	SyncBounds(false);
	//Prepare to begin
	HDC maskDC=mask->GetCompDC(), renderingDC=rendering->GetCompDC();
	int savedMaskDC=SaveDC(maskDC), savedRenderingDC=SaveDC(renderingDC);
	try
	{
		//Set up the mappings
		//SetMapMode(renderingDC,MM_TEXT);
		SetWindowOrgEx(renderingDC,bounds.left,bounds.top,NULL);
		SetViewportOrgEx(renderingDC,0,0,NULL);
		//SetMapMode(maskDC,MM_TEXT);
		SetWindowOrgEx(maskDC,bounds.left,bounds.top,NULL);
		SetViewportOrgEx(maskDC,0,0,NULL);
		//Set misc settings for the mask
		SetROP2(maskDC,R2_BLACK);
		SetBkMode(maskDC,TRANSPARENT);
		//White the mask
		PatBlt(maskDC,0,0,mask->GetDibWidth(),mask->GetDibHeight(),WHITENESS);
		//Render
		innerObject->Draw(maskDC,NULL);
		innerObject->Draw(renderingDC,NULL);
	}
	catch(...)
	{
		//Cleanup
		RestoreDC(maskDC,savedMaskDC);
		RestoreDC(renderingDC,savedRenderingDC);
		throw;
	}
	//Cleanup
	RestoreDC(maskDC,savedMaskDC);
	RestoreDC(renderingDC,savedRenderingDC);
}

void Objects::RenderedObject::Draw(HDC DC, RECT * ToPaint)
{
	SyncBounds();
	//PLSPEC: NT only
	MaskBlt(DC,bounds.left,bounds.top,RECTWIDTH(bounds),RECTHEIGHT(bounds),
		rendering->GetCompDC(),0,0,
		mask->GetDSHandle(),0,0,
		MAKEROP4(SRCCOPY,ROP3_NOP));
}

Objects::RenderedObject::RenderedObject(Objects::Object * InnerObject)
{
	//Init to nonsense values
	bounds.left=bounds.top=0;
	bounds.right=bounds.bottom=-1;
	if(InnerObject==NULL)
		throw std::invalid_argument(ERROR_STD_PROLOG "InnerObject must not be NULL.");
	innerObject.reset(InnerObject);
	SyncBounds();
}

void Objects::ObjectsContainer::DrawAll(HDC DC, RECT * ToPaint)
{
	ObjContainer::iterator it, end = objects.end();
	for(it=objects.begin();it!=end;it++)
	{
		(*it)->Draw(DC,ToPaint);
	}
}

void Objects::MetafileObject::Draw(HDC DC, RECT * ToPaint)
{
	//if(!
		PlayEnhMetaFile(DC,metaFile,&bounds)
	//	)
	//	throw std::runtime_error(ERROR_STD_PROLOG "Cannot play enhanced metafile; PlayEnhMetaFile returned FALSE.")
	;
}

Objects::MetafileObject::MetafileObject(POINT InsertPoint, HENHMETAFILE MetaFile)
{
	ENHMETAHEADER emh;
	if(MetaFile==NULL || GetEnhMetaFileHeader(MetaFile,sizeof(emh),&emh)!=sizeof(emh))
		throw std::invalid_argument(ERROR_STD_PROLOG "Invalid metafile.");
	bounds.left=InsertPoint.x+emh.rclBounds.left;
	bounds.top=InsertPoint.y+emh.rclBounds.top;
	bounds.right=bounds.left+RECTWIDTH(emh.rclBounds);
	bounds.bottom=bounds.top+RECTHEIGHT(emh.rclBounds);
	metaFile=MetaFile;
}

Objects::MetafileObject::MetafileObject(RECT Bounds, HENHMETAFILE MetaFile)
{
	bounds=Bounds;
	if(MetaFile==NULL)
		throw std::invalid_argument(ERROR_STD_PROLOG "Invalid metafile.");
	metaFile=MetaFile;
}
#endif