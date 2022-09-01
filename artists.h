//---------------------------------------------------------------------------

#ifndef artistsH
#define artistsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------

#include <vector>		// vectors
struct LvItem;          // declaration of LVItem struct


class TformArtists : public TForm
{
__published:	// IDE-managed Components
	TListView *lv;
	TEdit *editQSearch;
	TButton *btnAdd;
	TLabel *Label1;
	TLabel *Label2;
	TComboBox *comboTo;
	TComboBox *comboFrom;
	TTimer *timerShowDelay;
	TProgressBar *prbarIndexation;
	TPanel *panelProgress;
	TLabel *labelProgress;
	void __fastcall editQSearchEnter(TObject *Sender);
	void __fastcall editQSearchExit(TObject *Sender);
	void __fastcall editQSearchKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall editQSearchChange(TObject *Sender);
	void __fastcall btnAddClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall comboFromEnter(TObject *Sender);
	void __fastcall comboToSelect(TObject *Sender);
	void __fastcall comboFromSelect(TObject *Sender);
	void __fastcall lvKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall lvChange(TObject *Sender, TListItem *Item, TItemChange Change);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall lvItemChecked(TObject *Sender, TListItem *Item);
	void __fastcall timerShowDelayTimer(TObject *Sender);
	void __fastcall lvDblClick(TObject *Sender);
	void __fastcall lvKeyPress(TObject *Sender, System::WideChar &Key);

private:	// User declarations
public:		// User declarations
	__fastcall TformArtists(TComponent* Owner);

	void loadArtists(UnicodeString plName);    // loads all artists to lv
	void showItems(const std::vector <LvItem> &LvItems, bool showProgress);  // shows that pushed items in lv
};
//---------------------------------------------------------------------------
extern PACKAGE TformArtists *formArtists;
//---------------------------------------------------------------------------
#endif
