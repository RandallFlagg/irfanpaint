#include "stdafx.h"
#include "MeasurementUnit.h"
#include "DibSection.h"
#include "Utils.h"
#include "LanguageFile.h"
#include "Resource.h"
#include "INISection.h"
//MeasurementUnit static fields
MUInch MeasurementUnit::sInch;
MUPixel MeasurementUnit::sPixel;
MUDegree MeasurementUnit::sDegree;
bool operator==(MeasurementUnit * op1, const std::_tcstring & op2)
{
	return *op1==op2;
}

//Get one of the base unit classes.
//You can safely get a pointer from the returned reference and use it until the static members are destroyed.
DefaultUnit & MeasurementUnit::GetDefaultUnit(int Unit)
{
	switch(Unit)
	{
	case inch:
		return sInch;
	case pixel:
		return sPixel;
	case degree:
		return sDegree;
	default:
		throw std::out_of_range(ERROR_STD_PROLOG "Unrecognized unit.");
	}
}
std::_tcstring MeasurementUnit::GetDistanceString(DibSection & DibSect, POINT pt1, POINT pt2, unsigned int precision) const
{
	std::_tcostringstream os;
	os.precision(precision);
	double dist=GetDistance(DibSect,pt1,pt2);
	if(_isnan(dist))
		os<<langFile->GetString(IDS_MUCLASS_NOTDETERMINABLE);
	else
		os<<dist;
	if(SpaceBeforeSymbol())
		os<<_T(" ");
	os<<GetSymbol();
	if(_isnan(dist) && (pt1.x!=pt2.x || pt1.y!=pt2.y) && (DibSect.GetHeaders()->bmiHeader.biXPelsPerMeter==0 || DibSect.GetHeaders()->bmiHeader.biYPelsPerMeter==0))
		os<<_T(" (")<<langFile->GetString(IDS_MUCLASS_NODPI)<<_T(")");
	return os.str();
}
#pragma warning(push)
#pragma warning(disable:4100)
double MUPixel::GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const
{
	return PointsDistance(pt1,pt2);
}
double MUDegree::GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const
{
	return (double)(atan(((double)(pt2.y-pt1.y))/((double)(pt2.x-pt1.x)))*(180.0/M_PI));
}
#pragma warning(pop)
double MUInch::GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const
{
	//Actually the (X|Y)PelsPerMeter should be the number of pixels in a meter,
	//but when the bitmap is loaded IV automatically translates these values
	//in DPI (pixels per inch)
	double HRes=(double)DibSect.GetHeaders()->bmiHeader.biXPelsPerMeter,VRes=(double)DibSect.GetHeaders()->bmiHeader.biYPelsPerMeter;
	//If we don't have one of the resolution value but they are uninfluent
	//(e.g. the horizontal DPI are missing but we have a vertical line)
	//fake it to 1 to avoid a INF value that screws up all the computation even if it is possible
	if(HRes==0 && pt1.x==pt2.x)
		HRes=1;
	if(VRes==0 && pt1.y==pt2.y)
		VRes=1;
	POINTD dpt1={pt1.x/HRes,pt1.y/VRes}, dpt2={pt2.x/HRes,pt2.y/VRes};
	return PointsDistance(dpt1,dpt2);
}
std::_tcstring DerivedUnit::GetDefinition() const
{
	std::_tcostringstream os;
	os<<coefficient;
	if(baseUnit->SpaceBeforeSymbol())
		os<<_T(" ");
	os<<baseUnit->GetSymbol();
	return os.str();
}
double DerivedUnit::GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const
{
	return baseUnit->GetDistance(DibSect,pt1,pt2)/coefficient;
}

void DerivedUnit::RemoveFromDependencies(DerivedUnit * Unit)
{
	if(baseUnit==Unit)
	{
		baseUnit=Unit->GetBaseUnit();
		coefficient*=Unit->GetCoefficient();
	}
	else if(this!=Unit)
		Unit->RemoveFromDependencies(Unit);
}

void DerivedUnit::Absolutize()
{
	std::pair<double, DefaultUnit *> tmp=GetAbsoluteDefinition();
	baseUnit=tmp.second;
	coefficient=tmp.first;
}

std::pair<double, DefaultUnit *> DerivedUnit::GetAbsoluteDefinition() const
{
	DerivedUnit * tmp=dynamic_cast<DerivedUnit *>(baseUnit);
	if(tmp==NULL)
	{
		DefaultUnit * ret=dynamic_cast<DefaultUnit *>(baseUnit);
		if(ret==NULL)
		{
			std::string message(ERROR_STD_PROLOG "Unexpected class type found: ");
			message+=typeid(baseUnit).name();
			throw std::logic_error(message);
		}
		return std::make_pair(coefficient,ret);
	}
	else
	{
		std::pair<double, DefaultUnit *> ret=tmp->GetAbsoluteDefinition();
		ret.first*=coefficient;
		return ret;
	}
}

bool DerivedUnit::IsDependency(MeasurementUnit * Unit)
{
	if(Unit == baseUnit)
		return true;
	DerivedUnit * tmp=dynamic_cast<DerivedUnit *>(baseUnit);
	if(tmp==NULL)
		return false;
	else
		return tmp->IsDependency(Unit);
}
std::_tcstring MeasurementUnitsContainer::GetDistanceString(DibSection & DibSect, POINT pt1, POINT pt2, unsigned int precision) const
{
	//Builds the string
	std::_tcstring ret;
	MUCcont::const_iterator it, end=measureUnits.end();
	for(it=measureUnits.begin();it!=end;it++)
		ret+=(*it)->GetDistanceString(DibSect,pt1,pt2,precision)+(it!=(end-1)?_T("\r\n"):_T(""));
	return ret;
}

void MeasurementUnitsContainer::RemoveMeasurementUnit(MeasurementUnit * Unit)
{
	RemoveMeasurementUnit(FindMeasurementUnit(Unit));
}

void MeasurementUnitsContainer::RemoveMeasurementUnit(std::_tcstring UnitName)
{
	RemoveMeasurementUnit(FindMeasurementUnit(UnitName));
}

void MeasurementUnitsContainer::RemoveMeasurementUnit(MUCcont::iterator elem)
{
	MUCcont::iterator it, end=measureUnits.end();
	if(elem==measureUnits.end())
		throw std::invalid_argument(ERROR_STD_PROLOG "This measurement unit is not managed by this container.");
	DerivedUnit * toRemove = dynamic_cast<DerivedUnit *>(*elem);
	measureUnits.erase(elem);
	if(toRemove!=NULL)
	{
		for(it=measureUnits.begin();it!=end;it++)
		{
			DerivedUnit * du = dynamic_cast<DerivedUnit *>(*it);
			if(du!=NULL)
				du->RemoveFromDependencies(toRemove);
		}
		delete toRemove;
	}
}

MeasurementUnitsContainer::MeasurementUnitsContainer()
{
	init();
}
void MeasurementUnitsContainer::init()
{
	//Clear the container
	measureUnits.clear();
	//Add the default units
	for(int i=MeasurementUnit::minvalue;i<=MeasurementUnit::maxvalue;i++)
		AddMeasurementUnit(&MeasurementUnit::GetDefaultUnit(i));
	//Add the millimeters (they actually are a derived unit, but a very important one)
	CreateAddMeasurementUnit(1/25.4,MeasurementUnit::inch,langFile->GetString(IDS_MUCLASS_MILLIMETER),_T("mm"));
}

MeasurementUnitsContainer::~MeasurementUnitsContainer()
{
	MUCcont::const_iterator it, end=measureUnits.end();
	//Deallocate all the managed objects
	for(it=measureUnits.begin();it!=end;it++)
	{
		DerivedUnit * toRemove = dynamic_cast<DerivedUnit *>(*it);
		if(toRemove!=NULL)
			delete toRemove;
	}
	//Empty the map
	measureUnits.clear();
}

//Saves the content in a INI file
void MeasurementUnitsContainer::SerializeToINI(INISection & IniSect)
{
	IniSect.BeginWrite();
	//Temp buffer
	TCHAR sectionName[1024]={0};
	//Remove the old settings
	RemoveDataFromINI(IniSect);
	//Save the list
	for(MUCcont::size_type pos=0;pos<measureUnits.size();pos++)
	{

		DerivedUnit * du = dynamic_cast<DerivedUnit *>(measureUnits[pos]);
		DefaultUnit * dfu = dynamic_cast<DefaultUnit *>(measureUnits[pos]);
		if(du!=NULL)
		{
			//Find the base unit in the vector and get its ID
			long baseUnit;
			MUCcont::const_iterator it=find(measureUnits.begin(),measureUnits.end(),du->GetBaseUnit());
			if(it==measureUnits.end())
				continue; //Cannot find it, skip
			else
				baseUnit=(long)(it-measureUnits.begin());
			//Base unit
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_BaseUnit"),pos);
			IniSect.PutKey(sectionName,baseUnit);
			//Name
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_Name"),pos);
			IniSect.PutKey(sectionName,du->GetName());
			//Coefficient
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_Coefficient"),pos);
			IniSect.PutKey(sectionName,du->GetCoefficient());
			//Symbol
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_Symbol"),pos);
			IniSect.PutKey(sectionName,du->GetSymbol());
		}
		else if(dfu!=NULL)
		{
			//Base unit
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_DefaultUnit"),pos);
			IniSect.PutKey(sectionName,dfu->GetDefaultUnitID());
		}
	}
	IniSect.EndWrite();
}
//Loads the content from a INI file
void MeasurementUnitsContainer::DeSerializeFromINI(INISection & IniSect)
{
	IniSect.BeginRead();
	//Clear the current container
	measureUnits.clear();
	//Temp buffer
	TCHAR sectionName[1024]={0};
	std::vector<serUnit> readElements;
	for(unsigned int count=0;;count++)
	{
		serUnit su;
		su.InConversion=false;
		su.Invalid=false;
		//Check if its a placeholder for a default unit
		_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_DefaultUnit"),count);
		su.DefaultUnit=IniSect.GetKey(sectionName,0);
		if(su.DefaultUnit==0) //It's not a placeholder
		{
			//Base unit
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_BaseUnit"),count);
			if(!IniSect.KeyExists(sectionName))
				break;
			su.BaseUnit=IniSect.GetKey(sectionName,0);
			//Name
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_Name"),count);
			su.Name=IniSect.GetKey(sectionName,_T(""));
			if(su.Name.size()==0)
				break;
			//Coefficient
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_Coefficient"),count);
			su.Coefficient=IniSect.GetKey<double>(sectionName,0.0);
			if(su.Coefficient==0)
				break;
			//Symbol
			_sntprintf(sectionName,ARRSIZE(sectionName)-1,_T("MUItem%lu_Symbol"),count);
			su.Symbol=IniSect.GetKey(sectionName,_T(""));
			if(su.Symbol.size()==0)
				break;
		}
		//Add the element
		readElements.push_back(su);
	}
	IniSect.EndRead();
	//If we have no elements just reset the container
	if(readElements.size()==0)
	{
		init();
		return;
	}
	//Now we have loaded all the items; let's check and convert them
	//First check: items that point to nonexistent bases, check for missing default units
	std::vector<serUnit>::iterator it;
	bool defaultUnitsPresent[MeasurementUnit::maxvalue-MeasurementUnit::minvalue+1]={false};
	for(it=readElements.begin();it<readElements.end();it++)
	{
		if(it->DefaultUnit==0)
		{
			//Normal unit
			int bu=it->BaseUnit;
			if(bu<0 || bu>=(int)readElements.size())
				removeSU((unsigned long)(it-readElements.begin()),readElements);
		}
		else
		{
			//Placeholder for default unit
			if(it->DefaultUnit<MeasurementUnit::minvalue || it->DefaultUnit>MeasurementUnit::maxvalue)
				removeSU((unsigned long)(it-readElements.begin()),readElements);
			else
				defaultUnitsPresent[it->DefaultUnit-MeasurementUnit::minvalue]=true;
		}
	}
	//Add the elements to the container
	measureUnits.resize(readElements.size(),NULL); 
	for(unsigned int i=0;i<readElements.size();i++)
	{
		if(measureUnits[i]==NULL)
			measureUnits[i]=convertSU(readElements[i],readElements,measureUnits);
	}
	//Remove the eventually NULL items (convertSU returns NULL for invalid items)
	measureUnits.erase(remove(measureUnits.begin(),measureUnits.end(),(MeasurementUnit*)NULL),measureUnits.end());
	//Add the eventually missing default units
	for(unsigned int i=0;i<ARRSIZE(defaultUnitsPresent);i++)
	{
		if(!defaultUnitsPresent[i])
			measureUnits.push_back(&MeasurementUnit::GetDefaultUnit(MeasurementUnit::minvalue+i));
	}
}
//Helper of DeSerializeFromINI; converts a SDU and all its dependencies in DerivedUnits
MeasurementUnit * MeasurementUnitsContainer::convertSU(serUnit & SU, std::vector<serUnit> & SUs, std::vector<MeasurementUnit *> & PIV)
{
	MeasurementUnit * ret;
	if(SU.InConversion || SU.Invalid)
	{
		//We have a circular reference
		SU.Invalid=true; //this way we don't have to recheck the item all the times
		return NULL;
	}
	SU.InConversion=true;
	if(SU.DefaultUnit!=0)
	{
		//It's a placeholder
		ret = &MeasurementUnit::GetDefaultUnit(SU.DefaultUnit);
	}
	else
	{
		//Retrieve the BaseUnit
		MeasurementUnit * BaseUnit;
		if(PIV[SU.BaseUnit]==NULL)
			PIV[SU.BaseUnit]=convertSU(SUs[SU.BaseUnit],SUs,PIV);
		BaseUnit=PIV[SU.BaseUnit];
		if(BaseUnit==NULL)
		{
			ret=NULL;
			SU.Invalid=true;
		}
		else
			ret = new DerivedUnit(SU.Coefficient,BaseUnit,SU.Name,SU.Symbol);
	}
	SU.InConversion=false;
	return ret;
}
//Helper of DeSerializeFromINI; removes the given SU, all the SUs that refer to it and fix the other references
void MeasurementUnitsContainer::removeSU(unsigned long suID, std::vector<serUnit> & SUs)
{
	SUs.erase(SUs.begin()+suID);
	std::vector<serUnit>::iterator it;
	for(it=SUs.begin();it<SUs.end();it++)
	{
		if(it->DefaultUnit==0)
		{
			if(it->BaseUnit==(long)suID)
				removeSU((unsigned long)(it-SUs.begin()),SUs);
			else if(it->BaseUnit>(long)suID)
				it->BaseUnit--;
		}
	}

}
//Removes the entries relative to the serialization from the INI file
void MeasurementUnitsContainer::RemoveDataFromINI(INISection & IniSect)
{
	IniSect.Read();
	typedef std::list<INISection::ISmap::iterator> itList;
	itList toDelete;
	INISection::ISmap & im=IniSect.GetInternalMap();
	{
		std::_tcstring prefix(_T("MUItem"));
		INISection::ISmap::iterator it, end=im.end();
		for(it=im.begin();it!=end;it++)
		{
			if(it->first.compare(0,prefix.size(),prefix)==0)
				toDelete.push_back(it);
		}
	}
	{
		itList::const_iterator it, end=toDelete.end();
		for(it=toDelete.begin();it!=end;it++)
			im.erase(*it);
	}
	IniSect.Write();

}