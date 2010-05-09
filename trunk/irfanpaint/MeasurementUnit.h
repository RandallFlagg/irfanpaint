#pragma once
#include "LanguageFile.h"
#include "Globals.h"
#include "resource.h"
class MUInch;
class MUPixel;
class MUDegree;
class DibSection;
class DefaultUnit;
//Measurement unit
class MeasurementUnit
{
	//Types declarations
private:
public:
	enum DefaultUnit_Enum
	{
		pixel=1,
		inch=2,
		degree=3,
		minvalue=pixel,
		maxvalue=degree
	};
	//Static members
private:
	static MUInch sInch;
	static MUPixel sPixel;
	static MUDegree sDegree;
public:
	//Get one of the base unit classes.
	//You can safely get a pointer from the returned reference and use it until the static members are destroied.
	static DefaultUnit & GetDefaultUnit(DefaultUnit_Enum Unit)
	{
		return GetDefaultUnit((int)Unit);
	};
	//Idem (note: this version should be avoided)
	static DefaultUnit & GetDefaultUnit(int Unit);
	//Instance members
private:
public:
	//Gets the distance between two points in the current measurement unit; points given in pixel
	virtual double GetDistance (DibSection & DibSect, POINT pt1, POINT pt2) const=0;
	//Idem, but in string form with the associated symbol
	virtual std::_tcstring GetDistanceString (DibSection & DibSect, POINT pt1, POINT pt2, unsigned int precision=4) const;
	//Gets the unit name
	virtual std::_tcstring GetName() const=0;
	//Gets the unit symbol
	virtual std::_tcstring GetSymbol() const=0;
	//Returns a definition of the measurement unit
	virtual std::_tcstring GetDefinition() const=0;
	//Returns true if there's a space between the number and the measure unit
	virtual bool SpaceBeforeSymbol() const{return true;};
	//Comparer with strings
	bool operator==(const std::_tcstring & op2){return GetName()==op2;};
	//Another comparer with strings (needed by MeasurementUnitsContainer::FindMeasurementUnit)
	friend bool operator==(MeasurementUnit * op1, const std::_tcstring & op2);
	//Virtual destructor
	virtual ~MeasurementUnit(){};
};
//Default unit
class DefaultUnit : public MeasurementUnit
{
public:
	//Returns the value corresponding to this default unit in MeasurementUnit::DefaultUnit_Enum
	virtual MeasurementUnit::DefaultUnit_Enum GetDefaultUnitID() const=0;
	std::_tcstring GetDefinition() const
	{
		return (SpaceBeforeSymbol()?_T("1 "):_T("1")) + GetSymbol() + _T(" (") + langFile->GetString(IDS_MUCLASS_DEFAULTUNIT) + _T(")");
	};
};
//Pixel measurement unit
class MUPixel : public DefaultUnit
{
public:
	double GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const;
	std::_tcstring GetName() const{return langFile->GetString(IDS_MUCLASS_PIXEL);};
	std::_tcstring GetSymbol() const{return _T("px");};
	MeasurementUnit::DefaultUnit_Enum GetDefaultUnitID() const {return MeasurementUnit::pixel;};
};
//Inch measurement unit
class MUInch : public DefaultUnit
{
public:
	double GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const;
	std::_tcstring GetName() const{return langFile->GetString(IDS_MUCLASS_INCH);};
	std::_tcstring GetSymbol() const{return _T("in");};
	MeasurementUnit::DefaultUnit_Enum GetDefaultUnitID() const {return MeasurementUnit::inch;};
};
//Degree measurement unit
class MUDegree : public DefaultUnit
{
public:
	double GetDistance(DibSection & DibSect, POINT pt1, POINT pt2) const;
	std::_tcstring GetName() const{return langFile->GetString(IDS_MUCLASS_DEGREE);};
	std::_tcstring GetSymbol() const{return _T("°");};
	bool SpaceBeforeSymbol() const{return false;};
	MeasurementUnit::DefaultUnit_Enum GetDefaultUnitID() const {return MeasurementUnit::degree;};
};
//Derived measurement unit
class DerivedUnit : public MeasurementUnit
{
	//Base unit
	MeasurementUnit * baseUnit;
	//Numer of base units that make this unit
	double coefficient;
	//Name of the unit
	std::_tcstring unitName;
	//Symbol of the unit
	std::_tcstring unitSymbol;
	//Init the members
	void init(double Coefficient, MeasurementUnit * BaseUnit, std::_tcstring & UnitName, std::_tcstring & UnitSymbol)
	{
		if(Coefficient==0)
			throw std::invalid_argument(ERROR_STD_PROLOG "Coefficient mustn't be 0.");
		if(BaseUnit==NULL)
			throw std::invalid_argument(ERROR_STD_PROLOG "BaseUnit must not be NULL.");
		if(UnitName.length()==0)
			throw std::invalid_argument(ERROR_STD_PROLOG "UnitName must be a non-null string.");
		if(UnitSymbol.length()==0)
			throw std::invalid_argument(ERROR_STD_PROLOG "UnitName must be a non-null string.");
		coefficient = Coefficient;
		baseUnit = BaseUnit;
		unitName = UnitName;
		unitSymbol = UnitSymbol;
	}
public:
	//Constructors
	DerivedUnit(double Coefficient, MeasurementUnit * BaseUnit, std::_tcstring UnitName, std::_tcstring UnitSymbol)
	{
		init(Coefficient, BaseUnit, UnitName, UnitSymbol);
	};
	DerivedUnit(double Coefficient, DefaultUnit_Enum BaseUnit, std::_tcstring UnitName, std::_tcstring UnitSymbol)
	{
		init(Coefficient, &MeasurementUnit::GetDefaultUnit(BaseUnit), UnitName, UnitSymbol);
	};
	//Others
	//Remove the given unit from the dependencies of this class
	void RemoveFromDependencies(DerivedUnit * Unit);
	//Makes the definition of the unit relative to a default unit
	void Absolutize();
	//Returns true if the given object is one of this object's dependencies
	bool IsDependency(MeasurementUnit * Unit);
	//Gets the absolute definition of the unit
	std::pair<double, DefaultUnit *> GetAbsoluteDefinition() const;
	//Getters
	double GetCoefficient() const{return coefficient;};
	MeasurementUnit * const GetBaseUnit() const{return baseUnit;};
	std::_tcstring GetName() const{return unitName;};
	std::_tcstring GetSymbol() const{return unitSymbol;};
	double GetDistance (DibSection & DibSect, POINT pt1, POINT pt2) const;
	std::_tcstring GetDefinition() const;
};

//MeasurementUnits container
class MeasurementUnitsContainer
{
public:
	//Map typedef
	typedef std::vector<MeasurementUnit *> MUCcont;
private:
	//Struct used in the deserialization
	struct serUnit
	{
		std::_tcstring Name;
		std::_tcstring Symbol;
		double Coefficient;
		long BaseUnit;
		long DefaultUnit; //If this field is set the others are not considered
		bool InConversion; //Used by convertSU to detect circular references
		bool Invalid; //Used by convertSU to mark circular references
	};
	//Helper of DeSerializeFromINI; converts a SU and all its dependencies in DerivedUnits
	MeasurementUnit * convertSU(serUnit & SU, std::vector<serUnit> & SUs, std::vector<MeasurementUnit *> & PIV);
	//Helper of DeSerializeFromINI; removes the given SU, all the SUs that refer to it and fix the other references
	void removeSU(unsigned long suID, std::vector<serUnit> & SUs);
	//Map that contains the measurement units
	MUCcont measureUnits;
	//Cleans the container and adds the default units
	void init();
public:
	//Constructor
	MeasurementUnitsContainer();
	//Destructor
	~MeasurementUnitsContainer();
	//Creates and adds to the map a measurement unit
	DerivedUnit * CreateAddMeasurementUnit(double Coefficient, MeasurementUnit * BaseUnit, std::_tcstring UnitName, std::_tcstring UnitSymbol)
	{
		DerivedUnit * temp = new DerivedUnit(Coefficient, BaseUnit, UnitName, UnitSymbol);
		measureUnits.push_back(temp);
		return temp;
	};
	//Idem
	DerivedUnit * CreateAddMeasurementUnit(double Coefficient, MeasurementUnit::DefaultUnit_Enum BaseUnit, std::_tcstring UnitName, std::_tcstring UnitSymbol)
	{
		DerivedUnit * temp = new DerivedUnit(Coefficient, BaseUnit, UnitName, UnitSymbol);
		measureUnits.push_back(temp);
		return temp;
	};
	//Idem
	DerivedUnit * CreateAddMeasurementUnit(double Coefficient, std::_tcstring BaseUnit, std::_tcstring UnitName, std::_tcstring UnitSymbol)
	{
		MUCcont::const_iterator it=FindMeasurementUnit(BaseUnit);
		if(it==measureUnits.end())
			throw std::invalid_argument(ERROR_STD_PROLOG "The specified base unit is not managed by this container.");
		return CreateAddMeasurementUnit(Coefficient,*it,UnitName,UnitSymbol);
	};
	//Adds to the given measurement unit to the map
	//N.B.: the object must be allocated using the new keyword; the caller must not delete it.
	void AddMeasurementUnit(MeasurementUnit * Unit)
	{
		measureUnits.push_back(Unit);
	};
	//Removes (and deletes) the given measurement unit from the map
	void RemoveMeasurementUnit(std::_tcstring UnitName);
	//Idem
	void RemoveMeasurementUnit(MeasurementUnit * Unit);
	//Idem
	void RemoveMeasurementUnit(MUCcont::iterator elem);
	//Finds a measurement unit
	MUCcont::iterator FindMeasurementUnit(std::_tcstring UnitName)
	{
		return find(measureUnits.begin(),measureUnits.end(),UnitName);
	};
	MUCcont::const_iterator FindMeasurementUnit(std::_tcstring UnitName) const
	{
		return find(measureUnits.begin(),measureUnits.end(),UnitName);
	};
	MUCcont::iterator FindMeasurementUnit(const MeasurementUnit * Unit)
	{
		return find(measureUnits.begin(),measureUnits.end(),Unit);
	};
	MUCcont::const_iterator FindMeasurementUnit(const MeasurementUnit * Unit) const
	{
		return find(measureUnits.begin(),measureUnits.end(),Unit);
	};
	//Saves the content in a INI file
	void SerializeToINI(INISection & IniSect);
	//Loads the content from a INI file
	void DeSerializeFromINI(INISection & IniSect);
	//Removes the entries relative to the serialization from the INI file
	void RemoveDataFromINI(INISection & IniSect);
	//Returns a suitable string to display to the user resulting from the interrogation of all the measurement units
	std::_tcstring GetDistanceString(DibSection & DibSect, POINT pt1, POINT pt2, unsigned int precision=4) const;
	//Getters
	//Returns a reference to the internal container
	//N.B.: you should never add, remove or modify the map; use the MeasurementUnitsContainer methods instead
	MUCcont & GetMeasurementUnitsContainer()
	{
		return measureUnits;
	};
};