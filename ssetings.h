//---------------------------------------------------------------------------

#ifndef ssetingsH
#define ssetingsH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Buttons.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.AppEvnts.hpp>
// ============== my includes ===================== //
#include <vector>
#include "AddingFiles.h"
#include "Playlists.h"
#include "Unit1.h"
#include "llistenings.h"
#include <shlobj.h> // for get desktop path

extern AddingFiles af;
extern Playlists pl;
extern UINT startList;                               // count of listenings on the start program
extern UINT todayListenings;                         // listened songs for beginning of day
//---------------------------------------------------------------------------

struct Shortcut
{
	UINT vkCode;
	int col, row;                  // pos in string grid
	bool shift, ctrl, alt, win;
	bool global;
	UnicodeString functionName;    // terminationSong
	UnicodeString keysCaption;     // "Alt + S"

                                                           // constructor
	Shortcut(UINT VkCode, int Col, int Row, bool Shift, bool Ctrl, bool Alt, bool Win, bool Global
	, UnicodeString FunctionName, UnicodeString KeysCaption);

	int findByColRow(int Col, int Row);   // returns index ind vector (-1 if not found)
	static void loadShortcuts();
	static void saveShortcuts();
	static void registerAllShortcuts();
	static void unregisterAllShortcuts();
	void assignModKeys();    // this function for playlists only. It gets shift  states from setForm
	void registerShortcut(Shortcut shortcut);       // REGISTER GLOBAL SHORTCUT!   (returns false if failed)
	void unregisterShortcut(Shortcut shortcut);     // UNREGISTER GLOBAL SHORTCUT!
};

//---------------------------------------------------------------------------

class TsetForm : public TForm
{
__published:	// IDE-managed Components
	TPageControl *PageControl1;
	TTabSheet *hotKeysTab;
	TTreeView *treeV;
	TButton *savebtn;
	TTabSheet *allSongsTab;
	TListView *listView;
	TSearchBox *search;
	TTabSheet *exportSettingsTab;
	TTabSheet *aboutTab;
	TLabel *Label1;
	TLabel *aboutText;
	TStaticText *itIsImportant;
	TStaticText *StaticText1;
	TButton *exportbtn;
	TButton *importbtn;
	TStaticText *StaticText2;
	TLabel *note;
	TSaveDialog *saveD;
	TLabel *Label2;
	TStaticText *StaticText3;
	TTabSheet *filter;
	TLabel *Label3;
	TPanel *panel;
	TPopupMenu *myOwnPopup;
	TMenuItem *first;
	TMenuItem *second;
	TMenuItem *third;
	TMenuItem *fourth;
	TEdit *maxEdit;
	TEdit *minEdit;
	TImage *thumbL;
	TImage *thumbR;
	TSpeedButton *maxbtn;
	TImage *snake;
	TPanel *rangeBar;
	TSpeedButton *applybtn;
	TSpeedButton *twoHundbtn;
	TTabSheet *other;
	TCheckBox *dontAnPPR;
	TCheckBox *neverAnPPR;
	TLabel *Label4;
	TLabel *Label5;
	TRadioGroup *restoreGroup;
	TRadioGroup *toTrayGroup;
	TPanel *separator;
	TCategoryPanelGroup *collapsingGroup;
	TCategoryPanel *CategoryPanel1;
	TCategoryPanel *CategoryPanel2;
	TPanel *Panel1;
	TCategoryPanel *CategoryPanel3;
	TLabel *v1_0label;
	TLabel *v1_1label1;
	TLabel *v1_2_1label;
	TCategoryPanel *CategoryPanel4;
	TLabel *v1_2_0label;
	TCategoryPanel *CategoryPanel5;
	TLabel *v1_2_2label;
	TImageList *imglEscapeBtn;
	TStringGrid *sg;
	TButton *btnX;
	TButton *btnInvis;
	TButton *btnAddRow;
	TButton *btnDeleteRow;
	TCategoryPanel *CategoryPanel6;
	TLabel *v2_0_0label;
	TTabSheet *Backup;
	TLabel *Label6;
	TButton *btnRestoreBackup;
	TListView *lvBackup;
	TCheckBox *cboxAllPl;
	TCheckBox *minimizeOnEsc;
	TLabel *Label7;
	TComboBox *cboxMod1;
	TComboBox *cboxMod2;
	TLabel *Label8;
	TCategoryPanel *CategoryPanel7;
	TLabel *v2_1_0label;
	TFileOpenDialog *openD;
	TCategoryPanel *CategoryPanel8;
	TLabel *v2_1_1label;
	TCategoryPanel *CategoryPanel9;
	TLabel *v2_1_2label;
	TCategoryPanel *CategoryPanel10;
	TLabel *v2_2_0label;
	TCheckBox *cboxShowEndPlMsg;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall savebtnClick(TObject *Sender);
	void __fastcall listViewData(TObject *Sender, TListItem *Item);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall searchClick(TObject *Sender);
	void __fastcall listViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall searchChange(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall exportbtnClick(TObject *Sender);
	void __fastcall importbtnClick(TObject *Sender);
	void __fastcall treeVChange(TObject *Sender, TTreeNode *Node);
	void __fastcall panelMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
		  int Y);
	void __fastcall MenuItemClick(TObject *Sender);
	void __fastcall thumbRMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall maxbtnClick(TObject *Sender);
	void __fastcall minEditChange(TObject *Sender);
	void __fastcall minEditExit(TObject *Sender);
	void __fastcall applybtnClick(TObject *Sender);
	void __fastcall twoHundbtnClick(TObject *Sender);
	void __fastcall toTrayGroupClick(TObject *Sender);
	void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos,
          bool &Handled);
	void __fastcall CategoryPanel2Expand(TObject *Sender);
	void __fastcall sgSetEditText(TObject *Sender, int ACol, int ARow, const UnicodeString Value);
	void __fastcall sgGetEditText(TObject *Sender, int ACol, int ARow, UnicodeString &Value);
	void __fastcall sgDblClick(TObject *Sender);
	void __fastcall sgKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall btnXMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
          int Y);
	void __fastcall sgSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall btnAddRowClick(TObject *Sender);
	void __fastcall btnDeleteRowClick(TObject *Sender);
	void __fastcall lvBackupChange(TObject *Sender, TListItem *Item, TItemChange Change);
	void __fastcall btnRestoreBackupClick(TObject *Sender);
	void __fastcall cboxAllPlClick(TObject *Sender);
	void __fastcall sgKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall cboxMod1Select(TObject *Sender);
	void __fastcall sgMouseWheelDown(TObject *Sender, TShiftState Shift, TPoint &MousePos, bool &Handled);
	void __fastcall sgMouseWheelUp(TObject *Sender, TShiftState Shift, TPoint &MousePos, bool &Handled);
	void __fastcall cboxMod1Enter(TObject *Sender);
	void __fastcall listViewDrawItem(TCustomListView *Sender, TListItem *Item, TRect &Rect, TOwnerDrawState State);
	void __fastcall cboxShowEndPlMsgClick(TObject *Sender);






private:	// User declarations
    void __fastcall WMSysCommand(TMessage &Msg);                  // on minimize, close !!! look 2 lines lower
public:		// User declarations
	__fastcall TsetForm(TComponent* Owner);

	void deleteData();                  // for allSongs editor
	void moveSnake(bool wMax);          // resizing snake
	void refreshBackupList();           // shows all avaible backups
    std::vector <bool> getShiftNameForPlaylists(bool userPressedAlt, bool userPressedCtrl
												, bool userPressedShift, bool userPressedWin);
};
//---------------------------------------------------------------------------
extern PACKAGE TsetForm *setForm;
//---------------------------------------------------------------------------
#endif
