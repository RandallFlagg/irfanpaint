#pragma once
//Forward declaration
class ColorUtils;
//Modified EXTLOGPEN
struct nEXTLOGPEN
{
	DWORD dwPenStyle;		//Pen style
	DWORD dwWidth;			//Pen width
	DWORD dwStyleCount;		//Length of custom style array
	DWORD * lpStyle;		//Custom style array (you can deallocate it after passing the structure to the class)
};
//Additional properties to be applied to the DC
struct AdditionalCUProps
{
	bool transparentBG;		//Should the BkMode of the DC be TRANSPARENT?
	COLORREF bgColor;		//BGColor of the DC
	ColorUtils * bgCU;		//BGColor of the DC (if this is set bgColor is ignored)
	FLOAT miterLimit;		//Miter limit
};
//The ColorUtils class stores a color and provides methods to quickly
//obtain brushes and pens of that color
class ColorUtils
{
	friend class ColorUtils;
private:
	LOGBRUSH m_brushSet;		//Brush settings
	nEXTLOGPEN m_penSet;		//Pen settings
	AdditionalCUProps m_acup;	//Additional ColorUtils properties
    HBRUSH m_brush;				//Handle to the brush
	HPEN m_pen;					//Handle to the last created pen
	//Recreates the brush and/or the pen; if create is false only destroy the existing object
	void recreateObjs(bool brush=true, bool pen=true, bool create=true);
	//Initializes the brush and/or the pen settings to the default values
	void initSettings(bool brush=true, bool pen=true, bool ACUPs=true);
public:
	//Constructors
	ColorUtils()
	{
		initSettings();
		recreateObjs();
	}
	ColorUtils(COLORREF color)
	{
		initSettings();
		m_brushSet.lbColor=color;
		recreateObjs();
	}
	ColorUtils(COLORREF color, int width)
	{
		initSettings();
		m_brushSet.lbColor=color;
		m_penSet.dwWidth=width;
		recreateObjs();
	}
	ColorUtils(COLORREF color, nEXTLOGPEN penSet)
	{
		initSettings(true,false);
		setPenSettings(penSet);
		m_brushSet.lbColor=color;
		recreateObjs();
	}
	ColorUtils(LOGBRUSH brushSet)
	{
		m_brushSet=brushSet;
		initSettings(false,true);
		recreateObjs();
	}
	ColorUtils(LOGBRUSH brushSet, nEXTLOGPEN penSet)
	{
		m_brushSet=brushSet;
		setPenSettings(penSet);
		recreateObjs();
	}
	//Copy constructor
	ColorUtils(const ColorUtils& cu)
	{
		//Since we are constructing an object there is no valid value in the private fields; set this to 0 to avoid
		//wrong deallocation
		this->m_penSet.dwStyleCount=0;
		*this=cu; //Actually call the overloaded = operator
	}
	//Overloaded = operator
	ColorUtils & operator=(const ColorUtils& cu)
	{
		this->m_brushSet=cu.m_brushSet;
		this->setPenSettings(cu.m_penSet);
		this->m_acup=cu.m_acup;
		recreateObjs();
		return *this;
	}
	//Destructor
	~ColorUtils(void)
	{
		recreateObjs(true,true,false);
		if(m_penSet.dwStyleCount!=0)
			delete [] m_penSet.lpStyle;
		return;
	}
	//Sets the pen settings (public version, with objs recreation)
	void SetPenSettings(nEXTLOGPEN penSet)
	{
		setPenSettings(penSet);
		recreateObjs(false,true);
	}
	//Sets the pen settings (private version, w/o objs recreation)
	void setPenSettings(nEXTLOGPEN penSet)
	{
		if(m_penSet.dwStyleCount!=0)
			delete [] m_penSet.lpStyle;
		m_penSet=penSet;
		if(penSet.dwStyleCount!=0)
		{
			m_penSet.lpStyle=new DWORD[penSet.dwStyleCount];
			memcpy((void *)m_penSet.lpStyle,(void *)penSet.lpStyle,sizeof(DWORD)*penSet.dwStyleCount);
		}
		else
			penSet.lpStyle=NULL;
	}
	//Returns the pen settings
	nEXTLOGPEN GetPenSettings()
	{
		return m_penSet;
	}
	//Sets the brush settings
	void SetBrushSettings(LOGBRUSH brushSet)
	{
		m_brushSet=brushSet;
		recreateObjs(true,true); //Recreate also the pen, because it depends from the brush settings
	}
	//Returns the brush settings
	LOGBRUSH GetBrushSettings()
	{
		return m_brushSet;
	}
	//Sets the additional CU properties
	void SetACUPs(AdditionalCUProps acup)
	{
		m_acup = acup;
	}
	//Returns the additional CU properties
	AdditionalCUProps GetACUPs()
	{
		return m_acup;
	}
	//Returns the additional CU properties (by reference)
	AdditionalCUProps & GetRefACUPs()
	{
		return m_acup;
	}
	//Applies the ACUPs to a DC
	bool ApplyACUPs(HDC dc)
	{
		bool ret=true;
		COLORREF bgColor=(m_acup.bgCU==NULL?m_acup.bgColor:m_acup.bgCU->GetColor());
		if(m_acup.transparentBG)
			ret&=(SetBkMode(dc,TRANSPARENT)!=0);
		else
			ret&=(SetBkMode(dc,OPAQUE)!=0);
		ret&=(SetMiterLimit(dc,m_acup.miterLimit,NULL)!=0);
		ret&=(SetBkColor(dc,bgColor)!=CLR_INVALID);
		return ret;
	}
	//Checks the brush and the pen handle
	void CheckClass()
	{
		if(m_brush==NULL)
			throw std::runtime_error(ERROR_STD_PROLOG "The brush handle is not valid (it's NULL).");
		if(m_pen==NULL)
			throw std::runtime_error(ERROR_STD_PROLOG "The pen handle is not valid (it's NULL).");
		return;
	}
	//Sets the color of the class
	void SetColor(COLORREF brushColor, bool reset=false)
	{
		initSettings(reset,false,false);
		m_brushSet.lbColor=brushColor;
		//Recreate the objects
		recreateObjs();
		return;
	}
	//Sets the width of the pen
	void SetPenWidth(int width, bool reset=false)
	{
		if(width<0)
			throw std::invalid_argument(ERROR_STD_PROLOG "width must be a positive nonzero integer.");
		initSettings(false,reset,false);
		m_penSet.dwWidth=width;
		recreateObjs(false,true);
	}
	//Gets the width of the pen
	int GetPenWidth()
	{
		return m_penSet.dwWidth;
	}
	//Returns the current color
	COLORREF GetColor()
	{
		CheckClass();
		return m_brushSet.lbColor;
	}
	//Returns the current brush
	HBRUSH GetBrush()
	{
		CheckClass();
		return m_brush;
	}
	//Returns a pen of the given width
	HPEN GetPen()
	{
		CheckClass();
		return m_pen;
	}
};