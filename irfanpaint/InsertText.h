#pragma once
//Text rendering mode
enum TextRenderingMode
{
	None = 0,						//None
	PathBit = 2,					//Bit that is on if the text is rendered as path
	EmptyPath = PathBit | 0 ,		//The text is rendered as an empty path
	FilledPath = PathBit | 1,		//The text is rendered as a stroke and filled path
	TextBit = 4,					//Bit that is on if the text is rendered as text
	Text = TextBit | 0,				//The text is rendered as non-antialiased text
	AntialiasedText = TextBit | 1	//The text is rendered as antialiased text
};
//Structure that keeps the params used by the insert text dialog
struct InsertTextDlgParams
{
	POINT insertPoint;					//Text insert point
	TextRenderingMode renderingMode;	//Rendering mode
	std::string rtfText;				//Last text inserted (note: this field is updated only if the user presses IDOK)
};
//Shows the insert text dialog; returns false if the user cancelled the dialog
bool ShowInsertTextDialog(HWND hParent, POINT insertPoint);
//Insert text dialog procedure
INT_PTR CALLBACK InsertTextDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//Load the insert text dialog settings
void LoadITDlgSettings(HWND hwnd, INISection & IniSect);
//Save the insert text dialog settings
void SaveITDlgSettings(HWND hwnd, INISection & IniSect);
//Sets the format of the selection from the state of the controls
void UpdateSelFormat(HWND hwnd,DWORD mask=(CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT|CFM_FACE|CFM_SIZE));
//Sets the align of the text from the state of the controls
void UpdateAlign(HWND hwnd);
//Update the state of the controls from the current selection
void UpdateCtrlsState(HWND hwnd);
//Callback of EnumFontFamiliesEx
int CALLBACK InsertText_EnumFontFamExProc(ENUMLOGFONTEX *lpelfe,NEWTEXTMETRICEX *lpntme,DWORD FontType,LPARAM lParam);
//Returns a mask for UpdateSelFormat from the pressed button ID
DWORD MaskFromBtnID(int BtnID);
//"Notifier button" subclassed button window procedure
LRESULT CALLBACK NotifierBtnProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//Structure that keeps the params used by NotifierBtnProc
struct NotifierBtnParams
{
	WNDPROC originalProc;	//Original wndproc of the button
	bool btnLastPushState;	//true if the last time the button was pushed
};
//Subclasses a button making it a "notifier button"
void NotifierBtn_Subclass(HWND btnHandle);
//Desubclasses a "notifier button"
void NotifierBtn_Desubclass(HWND btnHandle);
//Renders the text of the given RichEdit on the given DC on the specified insertion point
void RenderRichEditText(HWND hRichEdit, HDC hDC, POINT insertPoint, TextRenderingMode renderingMode);
//Converts a TextRenderingMode to the corresponding control ID
WORD TRM2CtrlID(TextRenderingMode trm);
//Converts a control ID to the corresponding TextRenderingMode
TextRenderingMode CtrlID2TRM(WORD ctrlID);
//Struct used internally by RenderRichEditText; represents a piece of line with uniform formatting
struct linePiece
{
	std::_tcstring chars;
	HFONT font;
	COLORREF color;
};
//Callback used to read the data from the RichEdit
DWORD CALLBACK InsertText_InEditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
//Message handlers
void InsertText_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL InsertText_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
BOOL InsertText_OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr);