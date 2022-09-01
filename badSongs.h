//---------------------------------------------------------------------------

#ifndef badSongsH
#define badSongsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>

#include <vector> 		// vectors
#include "Songs.h"		// songs
#include "enums.h"      // enums
//---------------------------------------------------------------------------
class TformBadSongs : public TForm
{
__published:	// IDE-managed Components
	TListView *lv;
	TButton *btnCancel;
	TButton *btnDeleteAll;
	TProgressBar *prbar;
	TLabel *labelFound;
	TButton *btnShowInExplorer;
	TTimer *timerShowDelay;
	TLabel *labelProgress;
	TLabel *label2;
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall lvData(TObject *Sender, TListItem *Item);
	void __fastcall btnDeleteAllClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall btnShowInExplorerClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall timerShowDelayTimer(TObject *Sender);
	void __fastcall lvDrawItem(TCustomListView *Sender, TListItem *Item, TRect &Rect, TOwnerDrawState State);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall lvChange(TObject *Sender, TListItem *Item, TItemChange Change);

private:	// User declarations
public:		// User declarations
	__fastcall TformBadSongs(TComponent* Owner);

	void receiveFiles(std::vector <Songs*> vec);    // get that files and show in LV
    void prepareForm(ReasonToDelete reason);        // sets proper btn, labels accordingly to reason
};
//---------------------------------------------------------------------------
extern PACKAGE TformBadSongs *formBadSongs;
//---------------------------------------------------------------------------
#endif
