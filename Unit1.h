//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Buttons.hpp>                    // TSpeedButtons!
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Menus.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Graphics.hpp>
#include "trayicon.h"
#include <Vcl.AppEvnts.hpp>
#include <Vcl.CategoryButtons.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <Vcl.Imaging.GIFImg.hpp>

#include "enums.h"          // enums
#include "Playlists.h"



//---------------------------------------------------------------------------

struct PlayingSong        // for playingSongInfo only
{
	static UnicodeString name();
	static UnicodeString artist();
	static UnicodeString title();
	static bool songIsPlaying();  // returns false if player stopped
};

class Tmform : public TForm
{
__published:	// IDE-managed Components
	TListView *playlist;
	TPanel *artistPan;
	TPanel *titlePan;
	TSpeedButton *shufflebtn;
	TImage *letterF;
	TSpeedButton *prevbtn;
	TSpeedButton *nextbtn;
	TSpeedButton *playbtn;
	TPanel *commCounter;
	TPanel *counter;
	TPanel *currCounter;
	TListBox *playingSongInfo;
	TPanel *filterbtn;
	TLabel *plLabel;
	TSpeedButton *clearbtn;
	TSpeedButton *findDoublesbtn;
	TTimer *timer;
	TLabel *adur;
	TLabel *cdur;
	TPopupMenu *filterPopUp;
	TMenuItem *all;
	TMenuItem *less5;
	TMenuItem *from5to20;
	TMenuItem *more20;
	TMenuItem *custom;
	TLabel *Label2;
	TSpeedButton *countusbtn;
	TSpeedButton *dbs;
	TImage *mute;
	TProgressBar *volume;
	TTimer *globalHKtimer;
	TImage *settings;
	TProgressBar *prBar;
	TPanel *aaPanel;
	TSpeedButton *stopbtn;
	TMenuItem *more100;
	TMenuItem *N1;
	TPopupMenu *trayPopup;
	TMenuItem *Exit1;
	TMenuItem *traySettings;
	TMenuItem *N3;
	TEdit *editQSearch;
	TLabel *labelQSearch;
	TApplicationEvents *ApplicationEvents1;
	TMenuItem *Show1;
	TPanel *panelAlbum;
	TPanel *panelYear;
	TProgressBar *prbarDataLoaded;
	TTrayIcon *tray;
	TPanel *panelMain;
	TButton *btnDefault;
	TTimer *timerMove;
	TPopupMenu *popRClick;
	TMenuItem *Rename1;
	TMenuItem *Delete1;
	TSpeedButton *btnAdd;
	TSpeedButton *btnRight;
	TSpeedButton *btnLeft;
	TLabel *labelPlDuration;
	TLabel *labelPlCount;
	TPopupMenu *popPlItem;
	TMenuItem *copyToPl;
	TMenuItem *hiddenItem;
	TTimer *timerDblClick;
	TSpeedButton *btnCopyPlSong;
	TSpeedButton *btnAddArtists;
	TTimer *timerSwimArt;
	TTimer *timerSwimTit;
	TTimer *timerSwimAlb;
	TLabel *artist;
	TLabel *title;
	TLabel *album;
	TLabel *year;
	TMenuItem *Refresh1;
	TToggleSwitch *togCheckingMode;
	TLabel *labelAllSongs;
	TPopupMenu *pmenuSortBy;
	TMenuItem *asdf1;
	TMenuItem *adsfdf1;
	TPanel *btnSortPlBy;
	TPanel *btnMaximize;
	TPanel *cboxAutoNext;
	TPanel *btnDelPlayingSong;
	TButton *focusbtn;
	TPopupMenu *pmenuFocusBtn;
	TMenuItem *playingItem;
	TMenuItem *selectedItem;
	TMenuItem *scsItem;
	TTimer *delayTimer;
	TPanel *btnOpenDirPlSong;
	TTimer *timerAutoSave;
	TTimer *timerDblPress;
	TMenuItem *Opensourcefolder1;
	TButton *btnInvisible;
	TTimer *timerLWinState;
	TLabel *labelPlPage;
	void __fastcall playlistData(TObject *Sender, TListItem *Item);
	void __fastcall findDoublesbtnClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall playbtnClick(TObject *Sender);
	void __fastcall stopbtnClick(TObject *Sender);
	void __fastcall prevbtnClick(TObject *Sender);
	void __fastcall nextbtnClick(TObject *Sender);
	void __fastcall clearbtnClick(TObject *Sender);
	void __fastcall filterbtnClick(TObject *Sender);
	void __fastcall filterMenuItemClick(TObject *Sender);
	void __fastcall playlistDrawItem(TCustomListView *Sender, TListItem *Item, TRect &Rect, TOwnerDrawState State);
	void __fastcall shufflebtnClick(TObject *Sender);
	void __fastcall timerTimer(TObject *Sender);
	void __fastcall playlistDblClick(TObject *Sender);
	void __fastcall playlistChange(TObject *Sender, TListItem *Item, TItemChange Change);
	void __fastcall countusbtnClick(TObject *Sender);
	void __fastcall dbsClick(TObject *Sender);
	void __fastcall counterClick(TObject *Sender);
	void __fastcall muteClick(TObject *Sender);
	void __fastcall volumeMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall settingsClick(TObject *Sender);
	void __fastcall ds(TObject *Sender);
	void __fastcall mouseEnter(TObject *Sender);
	void __fastcall mouseLeave(TObject *Sender);
	void __fastcall volumeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
          int Y);
	void __fastcall nextbtnMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall Exit1Click(TObject *Sender);
	void __fastcall traySettingsClick(TObject *Sender);
	void __fastcall editQSearchChange(TObject *Sender);
	void __fastcall playlistKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall playlistKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall editQSearchEnter(TObject *Sender);
	void __fastcall editQSearchKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall editQSearchExit(TObject *Sender);
	void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos,
          bool &Handled);
	void __fastcall ApplicationEvents1Message(tagMSG &Msg, bool &Handled);
	void __fastcall debugbtnClick(TObject *Sender);
	void __fastcall trayClick(TObject *Sender);
	void __fastcall Show1Click(TObject *Sender);
	void __fastcall btnAddClick(TObject *Sender);
	void __fastcall Rename1Click(TObject *Sender);
	void __fastcall Delete1Click(TObject *Sender);
	void __fastcall timerMoveTimer(TObject *Sender);
	void __fastcall btnDefaultEnter(TObject *Sender);
	void __fastcall btnDefaultExit(TObject *Sender);
	void __fastcall btnRightClick(TObject *Sender);
	void __fastcall btnLeftClick(TObject *Sender);
	void __fastcall btnDefaultMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall popPlItemPopup(TObject *Sender);
	void __fastcall hiddenItemClick(TObject *Sender);
	void __fastcall trayDblClick(TObject *Sender);
	void __fastcall timerDblClickTimer(TObject *Sender);
	void __fastcall delayTimerTimer(TObject *Sender);
	void __fastcall btnCopyPlSongClick(TObject *Sender);
	void __fastcall timerSwimArtTimer(TObject *Sender);
	void __fastcall timerSwimTitTimer(TObject *Sender);
	void __fastcall timerSwimAlbTimer(TObject *Sender);
	void __fastcall focusbtnMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall Refresh1Click(TObject *Sender);
	void __fastcall playlistMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall togCheckingModeClick(TObject *Sender);
	void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
          int Y);
	void __fastcall btnSortPlByClick(TObject *Sender);
	void __fastcall asdf1Click(TObject *Sender);
	void __fastcall btnMaximizeClick(TObject *Sender);
	void __fastcall btnMinusClick(TObject *Sender);
	void __fastcall btnPlusClick(TObject *Sender);
	void __fastcall btnMaximizeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall btnMaximizeMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall cboxAutoNextClick(TObject *Sender);
	void __fastcall btnDelPlayingSongClick(TObject *Sender);
	void __fastcall playingItemClick(TObject *Sender);
	void __fastcall btnOpenDirPlSongClick(TObject *Sender);
	void __fastcall timerAutoSaveTimer(TObject *Sender);
	void __fastcall btnInvisibleClick(TObject *Sender);
	void __fastcall copyToPlClick(TObject *Sender);
	void __fastcall btnAddArtistsClick(TObject *Sender);
	void __fastcall playlistKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall timerDblPressTimer(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall Opensourcefolder1Click(TObject *Sender);
	void __fastcall ApplicationEvents1Deactivate(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall timerLWinStateTimer(TObject *Sender);
	void __fastcall plLabelClick(TObject *Sender);







private:	// User declarations
	void __fastcall WMSysCommand(TMessage &Msg);                  // on minimize, close !!! look 2 lines lower

BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_DROPFILES, TWMDropFiles, WMDropFiles)            // Drag & Drop
	MESSAGE_HANDLER(WM_SYSCOMMAND, TMessage, WMSysCommand)              // on minimize, close
END_MESSAGE_MAP(TForm)

public:		// User declarations
	friend struct PlayingSong;
	__fastcall Tmform(TComponent* Owner);            // constructor
	void minimize();                             	 // simulates user Minimize click

	void addPlaylist(UnicodeString Caption);         // ADD playlist
	void renamePlaylist(UnicodeString Caption);      // RENAME playlist
	void changeColorOfArrows(short input);           // change colors of arow btns
	void rearrangePlButtons();                       // change positions of buttons
	void refreshPlItemPopMenu();                     // load playlists to popMenu (for pl)
    void restoreFromTray();                          // restores from tray
	void __fastcall WMDropFiles(TWMDropFiles &Message);   // Drag & Drop
	void disableMainForm(bool strongDisabling);
	void enableBtnPanel(TPanel *panel);              // enable / disable panels (that look like btns)
	void disableBtnPanel(TPanel *panel);
	static bool containsOnly (const TShiftState &state, ModifierKey key, ModifierKey key2 = ModifierKey::None);


													 // returns true if only Ctrl was pressed e.g.
	bool songNameShouldBeHidden(const int &indexInSongs);  // should song has name " ------ "?
};
//---------------------------------------------------------------------------
extern PACKAGE Tmform *mform;
//---------------------------------------------------------------------------
#endif
