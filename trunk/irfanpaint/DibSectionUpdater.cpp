#include "StdAfx.h"
#include ".\dibsectionupdater.h"
#include "DibSection.h"

//Constructor
DibSectionUpdater::DibSectionUpdater(HGLOBAL * MonitoredLocation, DSInitCB * InitCB)
{
	monitoredLocation=MonitoredLocation;
	initCB=InitCB;
	managedDibSection=NULL;
	init();
}

//Checks the state of the synchronization, eventually fixing it
bool DibSectionUpdater::CheckState(bool TryToRepair)
{
	if(managedDibSection == NULL || *monitoredLocation!=managedDibSection->GetPackedDIB())
	{
		dispose(false);
		if(TryToRepair && *monitoredLocation!=NULL)
			init();
		else
			return false;
	}
	return true;
}

//Inits a new DibSection
void DibSectionUpdater::init()
{
	if(*monitoredLocation==NULL || managedDibSection!=NULL)
		return;
	LPBYTE sourceDIB=(LPBYTE)GlobalLock(*monitoredLocation);
	managedDibSection=new DibSection(sourceDIB,true);
	GlobalUnlock(*monitoredLocation);
	GlobalFree(*monitoredLocation);
	*monitoredLocation=managedDibSection->GetPackedDIB();
	(*initCB)(managedDibSection);
}

//Destroys the currently managed DibSection, eventually flushing the changes to the current IV DIB
void DibSectionUpdater::dispose(bool FlushChanges)
{
	if(managedDibSection==NULL)
		return;
	if(FlushChanges)
	{
		size_t fullImageSize=managedDibSection->GetBmpDataSize()+managedDibSection->GetHeadersSize();
		//Allocate the memory for the image;
		//even when GlobalAlloc returns NULL *monitoredLocation is set to NULL to limit the damage (IV will just show no image)
		if((*monitoredLocation=GlobalAlloc(GMEM_FIXED,fullImageSize))==NULL)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot allocate memory for the image; GlobalAlloc returned NULL.");
		//Copy the content of the DibSection in the just allocated memory
		memcpy(*(LPVOID *)monitoredLocation,managedDibSection->GetPackedDIB(),fullImageSize);
	}
	//Destroy the DibSection
	delete managedDibSection;
	managedDibSection=NULL;
}

//Destructor
DibSectionUpdater::~DibSectionUpdater(void)
{
	dispose(CheckState(false));
}
