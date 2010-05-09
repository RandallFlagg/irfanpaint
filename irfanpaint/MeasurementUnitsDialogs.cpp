#include "stdafx.h"
#include "MeasurementUnitsDialogs.h"
#include "resource.h"
#include "Globals.h"
#include "IrfanPaint.h"
#include "Utils.h"
#include "LanguageFile.h"
//Measurement units dialog
//Shows the "Measurement Units" dialog
void ShowMeasurementUnits(HWND hParent, MeasurementUnitsContainer & MUC)
{
	MeasurementUnitsDlgParam mudp={0};
	mudp.MUC=&MUC;
	if(DialogBoxParam(hCurInstance,MAKEINTRESOURCE(IDD_MEASUREMENTUNITS),hParent,MUDialogProc,(LPARAM)&mudp)==IDABORT)
		CleanUp();
}

//"Measurement units" dialog procedure
INT_PTR CALLBACK MUDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	try
	{
		//Message switch
		switch(uMsg)
		{
		//Everything is done with the message cracker macros
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hwndDlg,wParam,lParam,MU_OnInitDialog);
		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwndDlg,wParam,lParam,MU_OnCommand);
			return FALSE;
		case WM_NOTIFY:
			return HANDLE_WM_NOTIFY(hwndDlg,wParam,lParam,MU_OnNotify);
		case WM_CLOSE:
			EndDialog(hwndDlg,IDC_MU_BN_CLOSE);
			return TRUE;
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		EndDialog(hwndDlg,IDABORT);
	}
	return FALSE;
}

//WM_COMMAND
void MU_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	(void) hwndCtl; //To avoid the warning
	//Get the MUDP
	MeasurementUnitsDlgParam * mudp=(MeasurementUnitsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	//Declare some vars
	HWND hListView=GetDlgItem(hwnd,IDC_MU_LV_UNITS); //ListView handle
	INT selIndex=ListView_GetNextItem(hListView,-1,LVNI_SELECTED); //Find the selected item;
	MeasurementUnitsContainer::MUCcont & muc=mudp->MUC->GetMeasurementUnitsContainer(); //Reference to avoid long lines of code
	switch(id) //ID switch
	{
	case IDC_MU_BN_CLOSE:
		if(codeNotify==BN_CLICKED)
			EndDialog(hwnd,id);
		break;
	case IDC_MU_BN_MOVEUP:
	case IDC_MU_BN_MOVEDOWN:
		if(codeNotify==BN_CLICKED)
		{
			INT swapIndex; //Selected index and index of the item that will be swapped with it
			if(selIndex==-1)
				break;
			swapIndex=((id==IDC_MU_BN_MOVEUP)?selIndex-1:selIndex+1); //Find the item to swap with the selected one
			if(swapIndex<0 || (unsigned int)swapIndex>=mudp->MUC->GetMeasurementUnitsContainer().size())
				break;
			//Swap the positions in the vector
			MeasurementUnit * tswap = muc[selIndex];
			muc[selIndex]=muc[swapIndex];
			muc[swapIndex]=tswap;
			//Select the right item
			ListView_SetItemState(hListView,swapIndex,LVIS_SELECTED,LVIS_SELECTED);
			//Send a fake notification to update the 
			//Update the items
			//Note: this method may seem strange, but if you use just ListView_Update weird things happen (try to understand)
			ListView_SetItemText(hListView,swapIndex,0,LPSTR_TEXTCALLBACK);
			ListView_SetItemText(hListView,selIndex,0,LPSTR_TEXTCALLBACK);
		}
		break;
	case IDC_MU_BN_REMOVE:
		if(codeNotify==BN_CLICKED)
		{
			if(selIndex==-1)
				break;
			//The user wants to remove the selected item
			//Check if the item is a default one (nonremovable) (note: this should not happen, since the button is automatically disabled for these items, but who knows...)
			if(dynamic_cast<DerivedUnit *>(muc[selIndex])==NULL)
				break;
			//Ask confirm...
			if(MessageBox(hwnd,langFile->GetString(IDS_MU_DELETECONFIRM).c_str(),langFile->GetString(IDS_MU_DELETECONFIRM_CAPTION).c_str(),MB_YESNO | MB_ICONWARNING)==IDYES)
			{
				//... and delete
				//Remove the item from the muc...
				mudp->MUC->RemoveMeasurementUnit(muc.begin()+selIndex);
				//... and from the listview
                ListView_DeleteItem(hListView,selIndex);
			}
		}
		break;
	case IDC_MU_BN_EDIT:
		if(codeNotify==BN_CLICKED)
		{
			if(selIndex==-1)
				break;
			//The user wants to edit the selected item
			//Check if the item is a default one (nonremovable)
			//Although the button is normally disabled when such item is selected, if the user double-clicks over an item the event handler makes this notification blindly
			DerivedUnit * du=dynamic_cast<DerivedUnit *>(muc[selIndex]);
			if(du==NULL)
				break;
			if(ShowEditMeasurementUnit(hwnd,*du,*(mudp->MUC)))
			{
				//Update the item (see before for explanation of this method)
				ListView_SetItemText(hListView,selIndex,0,LPSTR_TEXTCALLBACK);
				ListView_Update(hListView,selIndex);
			}
		}
		break;
	case IDC_MU_BN_ADD:
		if(codeNotify==BN_CLICKED)
		{
			//The user wants to add a new item
			//Create a default DerivedUnit and edit it
			DerivedUnit * du = new DerivedUnit(1,MeasurementUnit::pixel,langFile->GetString(IDS_MU_NAME),langFile->GetString(IDS_MU_SYMBOL));
			if(ShowEditMeasurementUnit(hwnd,*du,*(mudp->MUC)))
			{
				//Add the item
				mudp->MUC->AddMeasurementUnit(du);
				MU_PopulateListView(hListView,*(mudp->MUC));
				ListView_SetItemState(hListView,ListView_GetItemCount(hListView)-1,LVIS_SELECTED,LVIS_SELECTED);
			}
			else
				delete du;
			break;
		}
	case IDC_MU_BN_ABSOLUTIZE:
		if(codeNotify==BN_CLICKED)
		{
			//The user wants to absolutize the current item
			//Check if the item is a default one (that can't be absolutized)
			DerivedUnit * du=dynamic_cast<DerivedUnit *>(muc[selIndex]);
			if(du==NULL)
				break;
			//Absolutize
			du->Absolutize();
			//Update the view
			ListView_Update(hListView,selIndex);
		}
	}
}
//WM_INITDIALOG
BOOL MU_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	(void) hwndFocus; //To avoid the warning
	HWND UnitsListView=GetDlgItem(hwnd,IDC_MU_LV_UNITS);
	//Associate the parameters to the window
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(__int3264)(LONG_PTR)lParam);
	//Get the MUDP
	MeasurementUnitsDlgParam * mudp=(MeasurementUnitsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	//Init the listview
	//Create the columns
	LVCOLUMN lc={0};
	lc.mask=LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM; 
	lc.fmt=LVCFMT_LEFT;
	for(int i=0;i<3;i++)
	{
		lc.iSubItem=i;
		lc.pszText=const_cast<LPTSTR>(langFile->GetString(IDS_MU_NAME+i).c_str());
		if(ListView_InsertColumn(UnitsListView,i,&lc)==-1)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot add the listview columns.");
	}
	//Populate the listview
	MU_PopulateListView(UnitsListView,*mudp->MUC);
	//Load the captions of the child windows (and of itself)
	langFile->InitializeDialogCaptions(hwnd, IDD_MEASUREMENTUNITS);
	return TRUE;
}
//WM_NOTIFY
BOOL MU_OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr)
{
	//Get the MUDP
	MeasurementUnitsDlgParam * mudp=(MeasurementUnitsDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(idFrom) //Control ID switch
	{
	case IDC_MU_LV_UNITS:
		{
			switch(pnmhdr->code) //Code switch
			{
			case LVN_GETDISPINFO: //A listview wants info about an item
				{
					NMLVDISPINFO * pnmldi=(NMLVDISPINFO *)pnmhdr;
					//Check if the item is not over the array bounds
					if((unsigned int)pnmldi->item.iItem<mudp->MUC->GetMeasurementUnitsContainer().size())
					{
						//Get the right measurement unit
						MeasurementUnit * mu = mudp->MUC->GetMeasurementUnitsContainer()[pnmldi->item.iItem];
						//Copy the requested data
						switch(pnmldi->item.iSubItem)
						{
						case 0:
							_tcsncpy(pnmldi->item.pszText,mu->GetName().c_str(),pnmldi->item.cchTextMax);
							break;
						case 1:
							_tcsncpy(pnmldi->item.pszText,mu->GetSymbol().c_str(),pnmldi->item.cchTextMax);
							break;
						case 2:
							_tcsncpy(pnmldi->item.pszText,mu->GetDefinition().c_str(),pnmldi->item.cchTextMax);
							break;
						}
						//Make sure that the string is NUL-terminated (_tcsncpy do not always add the NUL)
						pnmldi->item.pszText[pnmldi->item.cchTextMax-1]=0;
					}
					break;
				}
			case LVN_ITEMCHANGED: //A listview item state changed
				{
					LPNMLISTVIEW lpnmlv=(LPNMLISTVIEW)pnmhdr;
					if((lpnmlv->uNewState&LVIS_SELECTED) && !(lpnmlv->uOldState&LVIS_SELECTED)) //A listview item has been selected
					{
						//Decide which buttons must be enabled/disabled
						bool moveUp=true, moveDown=true, absolutize=true, edit=true, remove=true;
						MeasurementUnitsContainer::MUCcont & muc=mudp->MUC->GetMeasurementUnitsContainer(); //Reference to avoid long lines of code
						if(lpnmlv->iItem==0)
							moveUp=false;
						else if(lpnmlv->iItem==ListView_GetItemCount(pnmhdr->hwndFrom)-1)
							moveDown=false;
						if(dynamic_cast<DerivedUnit *>(muc[lpnmlv->iItem])==NULL)
						{
							edit=false;
							remove=false;
							absolutize=false;
						}
						EnableButtons(hwnd,moveUp,moveDown,absolutize,true,edit,remove);
					}
					else if(!(lpnmlv->uNewState&LVIS_SELECTED) && (lpnmlv->uOldState&LVIS_SELECTED))
						EnableButtons(hwnd,false,false,false,true,false,false);
					break;
				}
			case NM_DBLCLK:
				{
                    //Simulate a click on the Edit button
					FORWARD_WM_COMMAND(hwnd,IDC_MU_BN_EDIT,GetDlgItem(hwnd,IDC_MU_BN_EDIT),BN_CLICKED,SendMessage);
					break;
				}
			}
		}
	}
	return FALSE;
}
//Populates the listview
void MU_PopulateListView(HWND hListView, MeasurementUnitsContainer & MUC)
{
	LVITEM li={0};
	li.mask=LVIF_TEXT | LVIF_PARAM;
	//The listview will ask to the application the text
	li.pszText=LPSTR_TEXTCALLBACK;
	//Add the items
	unsigned int count=ListView_GetItemCount(hListView);
	for(unsigned int i=0;i<MUC.GetMeasurementUnitsContainer().size();i++)
	{
		if(i<count)
		{
			ListView_SetItemText(hListView,i,0,LPSTR_TEXTCALLBACK);
		}
		else if(ListView_InsertItem(hListView, &li) == -1)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot add the listview items.");
	}
	//Autosize the columns
	for(int i=0;i<3;i++)
		ListView_SetColumnWidth(hListView,i,LVSCW_AUTOSIZE);
}
//Enables/disables the various buttons
void EnableButtons(HWND hwnd, bool MoveUp, bool MoveDown, bool Absolutize, bool Add, bool Edit, bool Remove)
{
	EnableWindow(GetDlgItem(hwnd,IDC_MU_BN_MOVEUP),MoveUp);
	EnableWindow(GetDlgItem(hwnd,IDC_MU_BN_MOVEDOWN),MoveDown);
	EnableWindow(GetDlgItem(hwnd,IDC_MU_BN_ABSOLUTIZE),Absolutize);
	EnableWindow(GetDlgItem(hwnd,IDC_MU_BN_ADD),Add);
	EnableWindow(GetDlgItem(hwnd,IDC_MU_BN_EDIT),Edit);
	EnableWindow(GetDlgItem(hwnd,IDC_MU_BN_REMOVE),Remove);
}
//Edit measurement unit dialog
//Shows the "Edit measurement unit" dialog
bool ShowEditMeasurementUnit(HWND hParent, DerivedUnit & Item, MeasurementUnitsContainer & MUC)
{
	EditMeasurementUnitDlgParam emudp={0};
	emudp.MUC=&MUC;
	emudp.Item=&Item;
	switch(DialogBoxParam(hCurInstance,MAKEINTRESOURCE(IDD_EDITMEASUREMENTUNIT),hParent,EMUDialogProc,(LPARAM)&emudp))
	{
	case IDOK:
		return true;
	case IDABORT:
		CleanUp();
		return false;
	default:
		return false;
	}
}
//"Edit measurement unit" dialog procedure
INT_PTR CALLBACK EMUDialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	try
	{
		//Message switch
		switch(uMsg)
		{
		//Everything is done with the message cracker macros
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hwndDlg,wParam,lParam,EMU_OnInitDialog);
		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwndDlg,wParam,lParam,EMU_OnCommand);
			return FALSE;
		case WM_CLOSE:
			EndDialog(hwndDlg,IDCANCEL);
			return TRUE;
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		EndDialog(hwndDlg,IDABORT);
	}
	return FALSE;
}
//Populates the combobox
void EMU_PopulateComboBox(HWND hComboBox, DerivedUnit & Item, MeasurementUnitsContainer & MUC)
{
	MeasurementUnitsContainer::MUCcont & muc=MUC.GetMeasurementUnitsContainer(); //Reference to avoid long lines of code
	MeasurementUnitsContainer::MUCcont::const_iterator it, end=muc.end();
	int index;
	for(it=muc.begin();it!=end;it++)
	{
		DerivedUnit * du = dynamic_cast<DerivedUnit *>(*it);
		//If the item to add is the item we are editing or one of the items that depends from it don't add it
		if(*it!=&Item && (du==NULL || !du->IsDependency(&Item)))
		{
			if(
				(index=ComboBox_AddString(hComboBox,(*it)->GetSymbol().c_str()))<0
				||
				ComboBox_SetItemData(hComboBox,index,*it)==CB_ERR
				)
				throw std::runtime_error(ERROR_STD_PROLOG "Cannot add the combobox strings.");
		}
	}
}
//Event handlers
//WM_COMMAND
void EMU_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	(void) hwndCtl;
	//Get the EMUDP
	EditMeasurementUnitDlgParam * emudp=(EditMeasurementUnitDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(id)
	{
	case IDOK:
		if(codeNotify==BN_CLICKED)
		{
			try
			{
				*emudp->Item=EMU_ItemFromCtrls(hwnd);
			}
			catch(std::_tcstring & str)
			{
				ErrMsgBox(hwnd,str.c_str());
				break;
			}
			EndDialog(hwnd,id);
		}
		break;
	case IDCANCEL:
		if(codeNotify==BN_CLICKED)
			EndDialog(hwnd,id);
		break;
	case IDC_EMU_TX_SYMBOL:
		if(codeNotify==EN_CHANGE) //The symbol textbox has been changed
		{
			std::_tcstring caption=emudp->DefinitionLabelSyntax;
			TCHAR UnitSymbol[TEXTBOXBUFFERSLENGTH];
			GetDlgItemText(hwnd,IDC_EMU_TX_SYMBOL,UnitSymbol,TEXTBOXBUFFERSLENGTH);
			ReplaceString(caption,_T("%unitsymbol%"),UnitSymbol);
			SetDlgItemText(hwnd,IDC_EMU_LB_DEFINITION2,caption.c_str());
			break;
		}
	}
}
//WM_INITDIALOG
BOOL EMU_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	(void) hwndFocus; //To avoid the warning
	//Associate the parameters to the window
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(__int3264)(LONG_PTR)lParam);
	//Get the EMUDP
	EditMeasurementUnitDlgParam * emudp=(EditMeasurementUnitDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	//Populate the combobox
	EMU_PopulateComboBox(GetDlgItem(hwnd,IDC_EMU_CB_DEFINITION_SYMBOL),*emudp->Item,*emudp->MUC);
	//Set the limits in the textboxes
	Edit_LimitText(GetDlgItem(hwnd,IDC_EMU_TX_NAME),TEXTBOXBUFFERSLENGTH-1);
	Edit_LimitText(GetDlgItem(hwnd,IDC_EMU_TX_SYMBOL),TEXTBOXBUFFERSLENGTH-1);
	Edit_LimitText(GetDlgItem(hwnd,IDC_EMU_TX_DEFINITION_COEFFICIENT),TEXTBOXBUFFERSLENGTH-1);
	//Load the captions of the child windows (and of itself)
	langFile->InitializeDialogCaptions(hwnd, IDD_EDITMEASUREMENTUNIT);
	//Store the definition label syntax
	TCHAR buffer[LANGBUFSIZE]=_T("");
	GetDlgItemText(hwnd,IDC_EMU_LB_DEFINITION2,buffer,LANGBUFSIZE);
	emudp->DefinitionLabelSyntax=buffer;
	//Set up the rest of the dialog from the item
	EMU_CtrlsFromItem(hwnd, *emudp->Item);
	return TRUE;
}
//Populates the controls from the item
void EMU_CtrlsFromItem(HWND hwnd, DerivedUnit & Item)
{
	//Set up the textboxes
	SetDlgItemText(hwnd,IDC_EMU_TX_NAME,Item.GetName().c_str());
	SetDlgItemText(hwnd,IDC_EMU_TX_SYMBOL,Item.GetSymbol().c_str());
	//Convert the coefficient in a string
	std::_tcostringstream os;
	os.precision(12);
	os<<Item.GetCoefficient();
	SetDlgItemText(hwnd,IDC_EMU_TX_DEFINITION_COEFFICIENT,os.str().c_str());
	//Choose from the combobox the right value
	HWND hComboBox=GetDlgItem(hwnd,IDC_EMU_CB_DEFINITION_SYMBOL);
	int count=ComboBox_GetCount(hComboBox);
	for(int i=0; i<count; i++)
	{
		if(Item.GetBaseUnit()==(MeasurementUnit *)ComboBox_GetItemData(hComboBox,i))
		{
			ComboBox_SetCurSel(hComboBox,i);
			break;
		}
	}
}

//Makes a DerivedUnit from the controls
DerivedUnit EMU_ItemFromCtrls(HWND hwnd)
{
	HWND hComboBox=GetDlgItem(hwnd,IDC_EMU_CB_DEFINITION_SYMBOL);
	double Coefficient=1;
	int curSel;
	MeasurementUnit * BaseUnit;
	TCHAR UnitName[TEXTBOXBUFFERSLENGTH], UnitSymbol[TEXTBOXBUFFERSLENGTH], StrCoefficient[TEXTBOXBUFFERSLENGTH];
	//Textboxes
	GetDlgItemText(hwnd,IDC_EMU_TX_NAME,UnitName,TEXTBOXBUFFERSLENGTH);
	GetDlgItemText(hwnd,IDC_EMU_TX_SYMBOL,UnitSymbol,TEXTBOXBUFFERSLENGTH);
	GetDlgItemText(hwnd,IDC_EMU_TX_DEFINITION_COEFFICIENT,StrCoefficient,TEXTBOXBUFFERSLENGTH);
	if(!*UnitName || !*UnitSymbol || !*StrCoefficient)
		throw langFile->GetString(IDS_ERR_EMPTY);
	//Coefficient conversion
	std::_tcistringstream is(StrCoefficient);
	is>>Coefficient;
	if(is.fail() || !is.eof())
	{
		std::_tcstring msg=langFile->GetString(IDS_ERR_NOTANUMBER);
		ReplaceString(msg,_T("%what%"),StrCoefficient);
		throw msg;
	}
	//Base unit
	curSel=ComboBox_GetCurSel(hComboBox);
	if(curSel<0)
		throw langFile->GetString(IDS_ERR_EMPTY);  //This message is not very meaningful, but this should never happen, since the entry is always selected
	BaseUnit=(MeasurementUnit *)ComboBox_GetItemData(hComboBox,curSel);
	if(BaseUnit==NULL)
		throw std::runtime_error(ERROR_STD_PROLOG "The combobox item data is NULL.");
	return DerivedUnit(Coefficient,BaseUnit,UnitName,UnitSymbol);
}

//Shows the "Measure output dialog" 
void ShowMeasureOutput(HWND hParent, MeasurementUnitsContainer & MUC, POINT FirstPos, POINT SecondPos, DibSection & DibSect)
{
	MeasureOutputDlgParam modp={0};
	modp.MUC=&MUC;
	modp.FirstPos=FirstPos;
	modp.SecondPos=SecondPos;
	modp.DibSect=&DibSect;
	if(DialogBoxParam(hCurInstance,MAKEINTRESOURCE(IDD_MEASUREOUTPUT),hParent,MODialogProc,(LPARAM)&modp)==IDABORT)
		CleanUp();
}
//"Edit measurement unit" dialog procedure
INT_PTR CALLBACK MODialogProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	try
	{
		//Message switch
		switch(uMsg)
		{
		//Everything is done with the message cracker macros
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hwndDlg,wParam,lParam,MO_OnInitDialog);
		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwndDlg,wParam,lParam,MO_OnCommand);
			return FALSE;
		case WM_CLOSE:
			EndDialog(hwndDlg,IDOK);
			return TRUE;
		}
	}
	catch(exception &ex) //Handle exceptions
	{
		ErrMsgBox(hMainWindow,ex,_T(__FUNCSIG__),MB_ICONERROR);
		EndDialog(hwndDlg,IDABORT);
	}
	return FALSE;
}
//Event handlers
//WM_COMMAND
void MO_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	//Get the MODP
	MeasureOutputDlgParam * modp=(MeasureOutputDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	HWND hListBox=GetDlgItem(hwnd,IDC_MO_LS_RESULTS);
	switch(id)
	{
	case IDOK:
		if(codeNotify==BN_CLICKED)
			EndDialog(hwnd,id);
		break;
	case IDC_MO_BN_NEWUNIT:
		{
			int index;
			if(codeNotify==BN_CLICKED
				&& 
				ListBox_GetSelCount(hListBox)==1
				&&
				ListBox_GetSelItems(hListBox,1,&index)!=LB_ERR
				&&
				ListBox_GetItemData(hListBox,index)==1
				)
			{
				//The user wants to add a new measurement unit from the current measure
				//Create a new DerivedUnit and edit it
				MeasurementUnit * baseUnit = modp->MUC->GetMeasurementUnitsContainer()[index];
				DerivedUnit * du = new DerivedUnit(baseUnit->GetDistance(*modp->DibSect,modp->FirstPos,modp->SecondPos),baseUnit,langFile->GetString(IDS_MU_NAME),langFile->GetString(IDS_MU_SYMBOL));
				if(ShowEditMeasurementUnit(hwnd,*du,*(modp->MUC)))
				{
					//Add the item
					modp->MUC->AddMeasurementUnit(du);
					MO_PopulateListBox(hListBox,*modp);
				}
				else
					delete du;
				break;     
			}
		}
		break;
	case IDC_MO_BN_COPY:
		{
			if(codeNotify==BN_CLICKED)
			{
				//The user wants to copy the results to the clipboard
				//Array to hold the results
				int selCount=ListBox_GetSelCount(hListBox);
				//String to be copied
				std::_tcstring toCopy;
				if(selCount<0)
					break;
				else if(selCount==0)
					toCopy=modp->MUC->GetDistanceString(*modp->DibSect,modp->FirstPos,modp->SecondPos);
				else
				{
					MeasurementUnitsContainer::MUCcont & muc = modp->MUC->GetMeasurementUnitsContainer(); //To have shorter lines
					int * selected = new int[selCount];
					ListBox_GetSelItems(hListBox,selCount,selected);
					for(int i=0;i<selCount;i++)
					{
						toCopy+=muc[i]->GetDistanceString(*modp->DibSect,modp->FirstPos,modp->SecondPos) + (i!=selCount-1?_T("\r\n"):_T(""));
					}
					delete [] selected;
				}
				if(OpenClipboard(hwnd)) //Open the clipboard
				{
					TCHAR * toCopy2=NULL;
					HANDLE hMem=NULL;
					try
					{
						EmptyClipboard();
						hMem=GlobalAlloc(GMEM_MOVEABLE,sizeof(TCHAR)*(toCopy.size()+1));
						if(hMem==NULL)
							throw std::runtime_error(ERROR_STD_PROLOG "Cannot allocate the memory.");
						toCopy2=(TCHAR *)GlobalLock(hMem);
						if(toCopy2==NULL)
							throw std::runtime_error(ERROR_STD_PROLOG "Cannot lock the memory.");
						toCopy.copy(toCopy2,toCopy.size());
						GlobalUnlock(hMem);
						toCopy2=NULL;
						SetClipboardData(
#ifdef _UNICODE
							CF_UNICODETEXT
#else
							CF_TEXT
#endif
							,hMem);
						CloseClipboard();
					}
					catch(...)
					{
						if(toCopy2!=NULL)
							GlobalUnlock(hMem);
						if(hMem!=NULL)
							GlobalFree(hMem);
						CloseClipboard();
						throw;
					}
				}
			}
		}
		break;
	case IDC_MO_LS_RESULTS:
		switch(codeNotify)
		{
		case LBN_SELCHANGE:
			{
				//The selection has changed
				//Enable/disable the new unit button depending on the kind of entry
				int index;
				EnableWindow(
					GetDlgItem(hwnd,IDC_MO_BN_NEWUNIT),
					ListBox_GetSelCount(hwndCtl)==1
					&&
					ListBox_GetSelItems(hwndCtl,1,&index)!=LB_ERR
					&&
					ListBox_GetItemData(hwndCtl,index)==1
					);
			}
			break;
		}
		break;
	}
}
//WM_INITDIALOG
BOOL MO_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	(void) hwndFocus; //To avoid the warning
	//Associate the parameters to the window
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(__int3264)(LONG_PTR)lParam);
	//Get the MODP
	MeasureOutputDlgParam * modp=(MeasureOutputDlgParam *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	//Populate the combobox
	MO_PopulateListBox(GetDlgItem(hwnd,IDC_MO_LS_RESULTS),*modp);
	//Load the captions of the child windows (and of itself)
	langFile->InitializeDialogCaptions(hwnd, IDD_MEASUREOUTPUT);
	return TRUE;
}
//Populates the listbox with the results
void MO_PopulateListBox(HWND hListBox, MeasureOutputDlgParam & modp)
{
	MeasurementUnitsContainer::MUCcont & muc = modp.MUC->GetMeasurementUnitsContainer(); //To have shorter lines
	int index;
	for(unsigned int i=ListBox_GetCount(hListBox);i<muc.size();i++)
	{
		//Add the items
		index=ListBox_AddString(hListBox,muc[i]->GetDistanceString(*modp.DibSect,modp.FirstPos,modp.SecondPos).c_str());
		if(index<0)
			throw std::runtime_error(ERROR_STD_PROLOG "Cannot add the strings to the listbox.");
		//Set the item data to 0 if the distance is NaN or 1 otherwise
		ListBox_SetItemData(hListBox,index,(_isnan(muc[i]->GetDistance(*modp.DibSect,modp.FirstPos,modp.SecondPos)))?0:1);
	}
}