//Global definitions
#pragma once
//Forward declarations
class ColorUtils;
class DibSection;
class AdvEraser;
class PrecisionCursor;
class WindowMagnetizer;
class UIToolsContainer;
class LanguageFile;
class INISection;
class DibSectionUpdater;
namespace Objects
{
	class ObjectsContainer;
};
extern HINSTANCE hCurInstance;				//Current dll instance
extern ColorUtils fgColor;					//Current FG color
extern ColorUtils bgColor;					//Current BG color
extern HWND hIVWindow, hToolBoxWindow;		//Windows handles to the IVW and to the ToolBox
extern HWND hMainWindow;					//Window handle to the IV main Window
extern HDC IVWDC;							//DC of hIVWindow
extern UIToolsContainer * UITC;				//UI Tools container
extern DibSectionUpdater * dsUpdater;		//DibSection updater object
extern WindowMagnetizer windowMagnetizer;	//WindowMagnetizer object
extern PrecisionCursor normPC;				//Normal precision cursor
extern PrecisionCursor clsrPC;				//Clone tool source cursor
extern LanguageFile * langFile;				//Language file
extern INISection * iniSect;				//INI section
#ifdef INCLUDE_OBJECTS
extern Objects::ObjectsContainer objects;	//Objects container
#endif
extern bool fillFlag;						//Fill flag
extern BYTE threshold;						//Tolerance threshold
extern bool ignoreINIWriteErrors;			//Flag that indicates if IP should ignore INI-file writing errors
//Misc #define
#define CAPTIONBUFFERSLENGTH	(1024)		//Size of buffers used to store captions
#define MESSAGEBUFFERSLENGTH	(4096)		//Size of buffers used to store messages
#define TEXTBOXBUFFERSLENGTH	(1024)		//Size of buffers used to retrieve textboxes text