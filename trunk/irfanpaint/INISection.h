#pragma once
//Used to compare elements in the map
struct CaseInsensitive_Less :
	public std::binary_function<std::_tcstring, std::_tcstring, bool>
{
	bool operator()(const std::_tcstring & str1, const std::_tcstring & str2) const;
};

class INISection
{
public:
	//Typedef for the internal map
	typedef std::map<std::_tcstring,std::_tcstring,CaseInsensitive_Less> ISmap;
private:
	//INI file path
	std::_tcstring iniFile;
	//Managed section
	std::_tcstring section;
	//Data read from the INI section
	ISmap data;
	//Read lock count
	unsigned int readLockCount;
	//Write lock count
	unsigned int writeLockCount;
	//String stream used for conversions
	std::_tcstringstream strStream;
	//Key name/value checker; throws an exception if the key name is invalid
	void checkKeyValueThrow(const std::_tcstring KeyName, const std::_tcstring Value) const
	{
		if(!CheckKeyName(KeyName))
			throw std::invalid_argument(ERROR_STD_PROLOG "Invalid key name.");
		if(!CheckValue(Value))
			throw std::invalid_argument(ERROR_STD_PROLOG "Invalid value.");
	};
	//Initialize the class
	void init(const std::_tcstring & INIFile, const std::_tcstring & Section);
	//Returns a byte from a pair of hexadecimal digits
	static unsigned char getByte(const TCHAR * str);
public:
	//Constructor
	INISection(const std::_tcstring INIFile, const std::_tcstring Section)
	{
		init(INIFile,Section);
	}
	//Copy constructor
	INISection(const INISection & is)
	{
		init(is.GetINIFile(),is.GetSection());
	};
	//operator =
	INISection & operator=(const INISection & right)
	{
		init(right.GetINIFile(),right.GetSection());
	};
	//File IO
	//Reads the data from the file in the map
	void Read();
	//Flushes the data to the file
	void Write() const;
	//Lock
	//Locks the read
	void LockRead()
	{
		if(readLockCount+1<readLockCount) //we are going to have an overflow
		{
			throw std::runtime_error(ERROR_STD_PROLOG "Maximum read lock count reached.");
		}
		else
			readLockCount++;
	};
	//Unlocks the read
	void UnlockRead()
	{
		if(readLockCount>0)
			readLockCount--;
	};
	//Idem, but ignores the lock count (just reset it to 0); use carefully
	void ForceReadUnlock()
	{
		readLockCount=0;
	};
	//Returns true if the read is locked
	bool IsReadLocked() const
	{
		return readLockCount>0;
	};
	//Returns the read lock count
	unsigned int GetReadLockCount() const
	{
		return readLockCount;
	};
	//Begins a read session (reads and lock the read)
	void BeginRead()
	{
		Read();
		LockRead();
	}
	//Ends a read session (unlocks the read)
	void EndRead()
	{
		UnlockRead();
	};
	//Locks the write
	void LockWrite()
	{
		if(writeLockCount+1<writeLockCount) //we are going to have an overflow
		{
			throw std::runtime_error(ERROR_STD_PROLOG "Maximum write lock count reached.");
		}
		else
			writeLockCount++;
	};
	//Unlocks the write
	void UnlockWrite()
	{
		if(writeLockCount>0)
			writeLockCount--;
	};
	//Idem, but ignores the lock count (just reset it to 0); use carefully
	void ForceWriteUnlock()
	{
		writeLockCount=0;
	};
	//Returns true if the write is locked
	bool IsWriteLocked() const
	{
		return writeLockCount>0;
	};
	//Returns the write lock count
	unsigned int GetWriteLockCount() const
	{
		return writeLockCount;
	};
	//Begins a write session (reads and lock the read and write)
	void BeginWrite()
	{
		Read();
		LockRead();
		LockWrite();
	}
	//Ends a write session (writes and unlocks the read and write)
	void EndWrite()
	{
		UnlockWrite();
		Write();
		UnlockRead();
	};
	//Map wrappers
	//Writers
	//Writers policy: exception on error
	//String
	void PutKey(const std::_tcstring & Key, const std::_tcstring & Value);
	//Any type that can be written in a stream with the << operator
	template <class type>
		void PutKey(const std::_tcstring & Key, type Value)
	{
		strStream.clear();
		strStream.str(_T(""));
		strStream<<Value;
		PutKey(Key,strStream.str());
	};
	//Structure
	void PutKey(const std::_tcstring & Key, const void * Struct, size_t StructSize);
	//Readers
	//Readers policy: exception when a conversion is impossible,
	//default value returned when the key does not exist.
	//To check if a key is present use the KeyExists function
	//String
	std::_tcstring GetKey(const std::_tcstring & Key, const std::_tcstring & Default) const;
	std::_tcstring GetKey(const std::_tcstring & Key, const TCHAR * Default) const
	{
		return GetKey(Key, std::_tcstring(Default));
	};
	std::_tcstring GetKey(const std::_tcstring & Key, const TCHAR * Default)
	{
		return ((const INISection *)this)->GetKey(Key, std::_tcstring(Default));
	};
	/*std::_tcstring GetKey(const std::_tcstring & Key, const TCHAR * Default) const
	{
		std::_tcstring def(Default);
		return GetKey(Key, def);
	};*/
	//Any type that can be read from a stream with the >> opeartor
	template <class type>
		type GetKey(const std::_tcstring & Key, const type Default)
	{
		type ret;
		//If the key does not exist return the default value
		if(!KeyExists(Key))
			return Default;
		else
		{
			strStream.clear();
			strStream.str(GetKey(Key,_T("")));
			strStream>>ret;
		}
		return ret;
	};
	//Structure (in this case if the key does not exists the structure is left as it is)
	void GetKey(const std::_tcstring & Key, void * Struct, size_t StructSize) const;
	//Delete
	void DeleteKey(const std::_tcstring & Key, bool Throw = false);
	//Misc
	//Checks the name of a key to avoid bad characters; returns true if the name is good
	bool CheckKeyName(const std::_tcstring & KeyName) const;
	//Checks a value to avoid bad characters; returns true if the value is good
	bool CheckValue(const std::_tcstring & Value) const;
	//Returns true if the given key exists
	bool KeyExists(const std::_tcstring & KeyName) const
	{
		return data.find(KeyName)!=data.end();
	};
	//Getters
	//Returns the internal map; use with caution
	ISmap & GetInternalMap()
	{
		return data;
	};
	//Returns the INI file associated to this class
	std::_tcstring GetINIFile() const
	{
		return iniFile;
	};
	//Returns the section associated to this class
	std::_tcstring GetSection() const
	{
		return section;
	};
};
