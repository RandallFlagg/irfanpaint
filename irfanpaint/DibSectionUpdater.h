#pragma once
class DibSection;
//The DibSectionUpdater class keeps managedDibSection up-to-date with the DIB changes of IrfanView
class DibSectionUpdater
{
public:
	//Prototype of the callback function "DibSection initialized"
	typedef void DSInitCB(DibSection * JustCreated);
private:
	//Memory location monitored by the class
	HGLOBAL * monitoredLocation;
	//Managed DibSection
	DibSection * managedDibSection;
	//Callback function "DibSection initialized"
	DSInitCB * initCB;
	//Destroys the currently managed DibSection, eventually flushing the changes to the current IV DIB
	void dispose(bool FlushChanges);
	//Inits a new DibSection
	void init();
public:
	//Constructor
	DibSectionUpdater(HGLOBAL * MonitoredLocation, DSInitCB * InitCB);
	//Checks the state of the synchronization, eventually fixing it
	bool CheckState(bool TryToRepair=true);
	//Destructor
	~DibSectionUpdater(void);
	//Returns a pointer to the current DibSection
	inline DibSection * GetDibSection()
	{
		CheckState();
		return managedDibSection;
	};
};