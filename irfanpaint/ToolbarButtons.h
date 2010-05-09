//Toolbar buttons #define
#pragma once
#define SHIFT_IDS_EX					1000
//Define all the toolbar buttons so that they have the same ID of the relative icons
#define IDC_TB_ARROW                    1001	//Done
#define IDC_TB_PAINTBRUSH               1002	//Done
#define IDC_TB_ERASER                   1003	//Done
#define IDC_TB_CLONE					1004	//Done
#define IDC_TB_COLORREPL				1005	//Done
#define IDC_TB_INSERTTEXT				1006	//Done
#define IDC_TB_LINE                     1007	//Done
#define IDC_TB_ARROWLINE                1008	//Done
#define IDC_TB_CIRCLE                   1009	//Done
#define IDC_TB_RECTANGLE                1010	//Done
#define IDC_TB_FLOODFILL                1011	//Done
#define IDC_TB_PICKER                   1012	//Done
#define IDC_TB_ROTATE					1013	//Done
#define IDC_TB_MEASURE					1014	//Working on
#define IDC_TB_PBSETTINGS				1015	//Done
#define IDC_TB_HELP						1016	//Done
#define IDC_TB_ABOUT                    1017	//Done
//Derived #defines
#define IDI_TB_ARROW                    IDC_TB_ARROW
#define IDS_TB_ARROW                    IDC_TB_ARROW
#define IDI_TB_PAINTBRUSH               IDC_TB_PAINTBRUSH
#define IDS_TB_PAINTBRUSH               IDC_TB_PAINTBRUSH
#define IDC_PAINTBRUSH                  IDC_TB_PAINTBRUSH
#define IDI_TB_ERASER                   IDC_TB_ERASER
#define IDS_TB_ERASER                   IDC_TB_ERASER
#define IDC_ERASER                      IDC_TB_ERASER
#define IDI_TB_CLONE                    IDC_TB_CLONE
#define IDS_TB_CLONE                    IDC_TB_CLONE
#define IDI_TB_COLORREPL                IDC_TB_COLORREPL
#define IDS_TB_COLORREPL                IDC_TB_COLORREPL
#define IDI_TB_LINE                     IDC_TB_LINE
#define IDS_TB_LINE                     IDC_TB_LINE
#define IDI_TB_ARROWLINE                IDC_TB_ARROWLINE
#define IDS_TB_ARROWLINE                IDC_TB_ARROWLINE
#define IDI_TB_CIRCLE                   IDC_TB_CIRCLE
#define IDS_TB_CIRCLE                   IDC_TB_CIRCLE
#define IDI_TB_RECTANGLE                IDC_TB_RECTANGLE
#define IDS_TB_RECTANGLE                IDC_TB_RECTANGLE
#define IDC_INSERTTEXT                  IDC_TB_INSERTTEXT
#define IDI_TB_INSERTTEXT               IDC_TB_INSERTTEXT
#define IDS_TB_INSERTTEXT               IDC_TB_INSERTTEXT
#define IDI_TB_FLOODFILL                IDC_TB_FLOODFILL
#define IDS_TB_FLOODFILL                IDC_TB_FLOODFILL
#define IDC_FLOODFILL                   IDC_TB_FLOODFILL
#define IDI_TB_PICKER                   IDC_TB_PICKER
#define IDS_TB_PICKER                   IDC_TB_PICKER
#define IDC_PICKER                      IDC_TB_PICKER
#define IDI_TB_ROTATE                   IDC_TB_ROTATE
#define IDS_TB_ROTATE                   IDC_TB_ROTATE
#define IDI_TB_MEASURE					IDC_TB_MEASURE
#define IDS_TB_MEASURE					IDC_TB_MEASURE
#define IDI_TB_PBSETTINGS               IDC_TB_PBSETTINGS
#define IDS_TB_PBSETTINGS               IDC_TB_PBSETTINGS
#define IDI_TB_HELP                     IDC_TB_HELP
#define IDS_TB_HELP                     IDC_TB_HELP
#define IDI_TB_ABOUT                    IDC_TB_ABOUT
#define IDS_TB_ABOUT                    IDC_TB_ABOUT

#define IDS_TB_ARROW_EX                 (IDS_TB_ARROW + SHIFT_IDS_EX)
#define IDS_TB_PAINTBRUSH_EX            (IDS_TB_PAINTBRUSH + SHIFT_IDS_EX)
#define IDS_TB_ERASER_EX                (IDS_TB_ERASER + SHIFT_IDS_EX)
#define IDS_TB_CLONE_EX                 (IDS_TB_CLONE + SHIFT_IDS_EX)
#define IDS_TB_COLORREPL_EX             (IDS_TB_COLORREPL + SHIFT_IDS_EX)
#define IDS_TB_LINE_EX                  (IDS_TB_LINE + SHIFT_IDS_EX)
#define IDS_TB_ARROWLINE_EX             (IDS_TB_ARROWLINE + SHIFT_IDS_EX)
#define IDS_TB_CIRCLE_EX                (IDS_TB_CIRCLE + SHIFT_IDS_EX)
#define IDS_TB_RECTANGLE_EX             (IDS_TB_RECTANGLE + SHIFT_IDS_EX)
#define IDS_TB_INSERTTEXT_EX            (IDS_TB_INSERTTEXT + SHIFT_IDS_EX)
#define IDS_TB_FLOODFILL_EX             (IDS_TB_FLOODFILL + SHIFT_IDS_EX)
#define IDS_TB_PICKER_EX                (IDS_TB_PICKER + SHIFT_IDS_EX)
#define IDS_TB_ROTATE_EX                (IDS_TB_ROTATE + SHIFT_IDS_EX)
#define IDS_TB_MEASURE_EX				(IDS_TB_MEASURE + SHIFT_IDS_EX)
#define IDS_TB_PBSETTINGS_EX            (IDS_TB_PBSETTINGS + SHIFT_IDS_EX)
#define IDS_TB_HELP_EX                  (IDS_TB_HELP + SHIFT_IDS_EX)
#define IDS_TB_ABOUT_EX                 (IDS_TB_ABOUT + SHIFT_IDS_EX)
//FIRSTx and LASTx macros - often used in for statements
#define FIRSTBUTTON						IDC_TB_ARROW
#define LASTBUTTON						IDC_TB_ABOUT
#define FIRSTTOOLBUTTON					IDC_TB_ARROW //First tool button of the toolbar
#define LASTTOOLBUTTON					IDC_TB_MEASURE //Last tool button of the toolbar
#define FIRSTNONTOOLBUTTON				IDC_TB_PBSETTINGS //First not tool button of the toolbar
#define LASTNONTOOLBUTTON				IDC_TB_ABOUT //Last not tool button of the toolbar
#define FIRSTICON						FIRSTBUTTON //First icon of the toolbar
#define LASTICON						LASTBUTTON //Last icon of the toolbar
#define FIRSTTOOLICON					FIRSTTOOLBUTTON //First tool icon of the toolbar
#define LASTTOOLICON					LASTTOOLBUTTON //Last tool icon of the toolbar
#define FIRSTNONTOOLICON				FIRSTNONTOOLBUTTON //First non tool icon of the toolbar
#define LASTNONTOOLICON					LASTNONTOOLBUTTON //Last non tool icon of the toolbar
