#pragma once
//This class provides a quick way to carry a value that may come from a variety of sources:
//1. a value
//2. a reference
//3. a pointer
//4. a function that returns a value
//5. a function that returns a reference
//6. a function that returns a pointer
template <typename retType> class PointerEx
{
	friend class PointerEx<retType>;
public:
	enum UsedField
	{
		ufValueVal=0x00,
		ufValuePtr=0x01,
		ufFuncVal=ufValueVal|0x10,
		ufFuncPtr=ufValuePtr|0x10,
		ufFuncRef=0x02|0x10,
	};
	typedef retType (* retrFuncVal)(void);
	typedef retType * (* retrFuncPtr)(void);
	typedef retType & (* retrFuncRef)(void);
private:
	retType valueVal;
	retType * valuePtr;
	retrFuncVal funcVal;
	retrFuncPtr funcPtr;
	retrFuncRef funcRef;
	UsedField usedField;
	bool constant; //can I return a non-const pointer/reference to the value?
	inline void checkNULL(void * ptr)
	{
		if(ptr==NULL)
			throw std::invalid_argument(ERROR_STD_PROLOG "You cannot pass NULL to this function.");
	};
	//Internal version without constant check that serves both GetPointer and GetConstPointer
	inline retType * getPointer()
	{
		switch(usedField)
		{
		case ufValueVal:
			return &valueVal;
		case ufValuePtr:
			return valuePtr;
		case ufFuncVal:
			throw std::logic_error(ERROR_STD_PROLOG "You cannot obtain a pointer to a value returned by a function.");
		case ufFuncPtr:
			return (funcPtr)();
		case ufFuncRef:
			return &(funcRef)();
		default:
			throw std::out_of_range(ERROR_STD_PROLOG "The value of usedField is out of range.");
		}
	};
public:
	//Constructors
	inline PointerEx(retType ValueVal, bool Constant) : valueVal(ValueVal)
	{
		usedField = ufValueVal;
		constant = Constant;
	};
	inline PointerEx(retType * ValuePtr, bool Constant) : valuePtr(ValuePtr)
	{
		usedField = ufValuePtr;
		checkNULL(valuePtr);
		constant = Constant;
	};
	inline PointerEx(retrFuncVal FuncVal) : funcVal(FuncVal)
	{
		usedField = ufFuncVal;
		checkNULL(funcVal);
		constant = true; //I cannot return a pointer or a reference to a value returned by a function
	};
	inline PointerEx(retrFuncPtr FuncPtr, bool Constant): funcPtr(FuncPtr)
	{
		usedField = ufFuncPtr;
		checkNULL(funcPtr);
		constant = Constant;
	};
	inline PointerEx(retrFuncRef FuncRef, bool Constant) : funcRef(FuncRef)
	{
		usedField = ufFuncRef;
		checkNULL(funcRef);
		constant = Constant;
	};
	//Getxxx
	retType GetValue()
	{
		switch(usedField)
		{
		case ufValueVal:
			return valueVal;
		case ufValuePtr:
			return *valuePtr;
		case ufFuncVal:
			return (funcVal)();
		case ufFuncPtr:
			return *(funcPtr)();
		case ufFuncRef:
			return (funcRef)();
		default:
			throw std::out_of_range(ERROR_STD_PROLOG "The value of usedField is out of range.");
		}
	};
	inline retType * GetPointer()
	{
		if(constant)
			throw std::logic_error(ERROR_STD_PROLOG "The value carried by this PointerEx instance is constant, you cannot obtain a non-const pointer or reference to it.");
		return getPointer();
	};
	inline retType * const GetConstPointer()
	{
		return getPointer();
	};
	inline retType & GetReference()
	{
		return * GetPointer();
	};
	inline const retType & GetConstReference()
	{
		return * GetConstPointer();
	};
	//Overloaded operators
	inline operator retType ()
	{
		return GetValue();
	};
	inline operator retType * ()
	{
		return GetPointer();
	};
	inline operator retType const * ()
	{
		return GetConstPointer();
	};
	//Misc
	inline UsedField GetUsedField()
	{
		return usedField;
	};
	inline bool IsConstant()
	{
		return constant;
	};
};
