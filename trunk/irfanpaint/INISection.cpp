#include "StdAfx.h"
#include ".\inisection.h"
#include "Utils.h"
#include <functional>
//Initialize the class
void INISection::init(const std::_tcstring & INIFile, const std::_tcstring & Section)
{
	if(!PathFileExists(INIFile.c_str()))
		throw std::invalid_argument(ERROR_STD_PROLOG "The given INIFile does not exist.");
	iniFile=INIFile;
	section=Section;
	readLockCount=0;
	writeLockCount=0;
	//Set the classic C locale so that numbers/dates can be read without problems regardless of regional settings
	strStream.imbue(std::locale::classic());
	//When the stream cannot extract a valid value it throws an exception
	strStream.exceptions(std::ios::failbit);
}

bool INISection::CheckKeyName(const std::_tcstring & KeyName) const
{
	return 
		KeyName.size()>=1	//Avoid 0-length strings
		&&
		KeyName[0]!=_T('[')	//Keys beginning with a "[" would be considered sections
		&&
		KeyName[0]!=_T(';')	//Keys beginning with a ";" would be considered comments
		&&
		KeyName.find(_T('='))==KeyName.npos; //Keys containing a "=" would be interpreted alone as a key-value couple
}
//Checks a value to avoid bad characters; returns true if the value is good
bool INISection::CheckValue(const std::_tcstring & Value) const
{
	return
		Value.find(_T('\n'))==Value.npos //Avoid CR and LF
		&&
		Value.find(_T('\r'))==Value.npos;
}
void INISection::DeleteKey(const std::_tcstring & Key, bool Throw)
{
	ISmap::iterator it=data.find(Key);
	if(it!=data.end())
		data.erase(it);
	else if(Throw)
		throw std::invalid_argument(ERROR_STD_PROLOG "The key does not exist.");
}

void INISection::Read()
{
	//Check the lock
	if(IsReadLocked())
		return;
	//Potential problem: the buffer is limited
	TCHAR readBuffer[65535];
	size_t strLen=0;
	TCHAR * equalPos;
	//Read the section
	GetPrivateProfileSection(section.c_str(),readBuffer,ARRSIZE(readBuffer),iniFile.c_str());
	//Decode the string
	for(TCHAR * readPtr=readBuffer;*readPtr;readPtr+=strLen+1)
	{
		strLen=_tcslen(readPtr);
		equalPos=_tcschr(readPtr,_T('='));
		if(equalPos==NULL)
			continue;
		*equalPos=0;
		if(*readPtr==0)
			continue;
		data[readPtr]=(TCHAR *)(equalPos+1);
	}
}

void INISection::Write() const
{
	//Check the lock
	if(IsWriteLocked())
		return;
	//Build the string to write
	std::_tcstring output;
	ISmap::const_iterator it, end=data.end();
	for(it=data.begin();it!=end;it++)
		output+=it->first+_T("=")+it->second+_T('\0');
	//Add the final NUL
	output+=_T('\0');
	//Write
	if(!WritePrivateProfileSection(section.c_str(),output.c_str(),iniFile.c_str()))
		throw std::runtime_error(ERROR_STD_PROLOG "WritePrivateProfileSection failed.");
}

//Writers
//String
void INISection::PutKey(const std::_tcstring & Key, const std::_tcstring & Value)
{
	//Check
	checkKeyValueThrow(Key,Value);
	//Assign
	data[Key]=Value;
}
//Structure
void INISection::PutKey(const std::_tcstring & Key, const void * Struct, size_t StructSize)
{
	unsigned char checksum=0;
	const unsigned char * castedStruct=(const unsigned char *)Struct;
	strStream.clear();
	strStream.str(_T(""));
	TCHAR oldFill=strStream.fill(_T('0'));
	strStream<<std::hex;
	for(const unsigned char * readPtr=castedStruct;readPtr<castedStruct+StructSize;readPtr++)
	{
		strStream<<std::setw(2)<<*readPtr;
		checksum=checksum+*readPtr;
	}
	strStream<<std::setw(2)<<checksum;
	PutKey(Key,strStream.str());
	strStream<<std::dec;
	strStream.fill(oldFill);
}
//Readers
//String
std::_tcstring INISection::GetKey(const std::_tcstring & Key, const std::_tcstring & Default) const
{
	if(!KeyExists(Key))
		return Default;
	else
		return data.find(Key)->second;
}
//Structure
void INISection::GetKey(const std::_tcstring & Key, void * Struct, size_t StructSize) const
{
	if(!KeyExists(Key))
		return;
	unsigned char checksum=0, checksum2=0;
	unsigned char * castedStruct=(unsigned char *)Struct;
	//Get the string
	std::_tcstring strValue=GetKey(Key,_T(""));
	if(strValue.size()==0)
		return;
	const TCHAR * readPtr=strValue.c_str();
	//Length check
	if(strValue.size()!=(StructSize+1)*2)
		throw std::runtime_error(ERROR_STD_PROLOG "The stored value is not valid for this structure.");
	//Convert the string
    for(unsigned char * writePtr=castedStruct;
		writePtr<castedStruct+StructSize;
		writePtr++,readPtr+=2)
	{
		*writePtr=getByte(readPtr);
		checksum=checksum+*writePtr;
	}
	checksum2=getByte(readPtr);
	//Compare the checksums
	if(checksum!=checksum2)
		throw std::runtime_error(ERROR_STD_PROLOG "Invalid checksum.");
}

//Returns a byte from a pair of hexadecimal digits
unsigned char INISection::getByte(const TCHAR * str)
{
	unsigned char ret=0;
	TCHAR ch;
	for(int i=0;i<2;i++)
	{
		ch=_totupper(str[i]);
		if(BETWEEN(ch,_T('0'),_T('9')))
			ret|=(ch-_T('0'))<<((1-i)<<2);
		else if(BETWEEN(ch,_T('A'),_T('F')))
			ret|=(ch-_T('A')+10)<<((1-i)<<2);
		else
			throw std::runtime_error(ERROR_STD_PROLOG "Invalid character.");
	}
	return ret;
}
//Used to compare elements in the map
bool CaseInsensitive_Less::operator()(const std::_tcstring & str1, const std::_tcstring & str2) const
{
	return _tcsicmp(str1.c_str(),str2.c_str())<0;
}