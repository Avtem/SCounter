//	  This is my second attemp to create Music Player of my dream. I realized a bunch
//	  of mistakes in previous code and give a new name! Now it called "Scounter"
//	  what means "Song counter", but in one world. Here I will write code from 0 (but
//	  good functions I'll copy, though)
//
//	02.11.2018
/////////////////////////////////////////////////////////////////////////////////////

#include <vcl.h>
#pragma hdrstop
#include "Unit1.h"
#pragma package(smart_init)
#pragma link "trayicon"
#pragma resource "*.dfm"


Tmform *mform;

	  // ============ MY INCLUDES ============
#include <windows.h>
#include <vector>
#include <shlwapi.h>                    // for ctrl+backspace
#include <shellapi.h>                   // for deleting folder

#include "AddingFiles.h"
#include "enums.h"                      // enums
#include "ssetings.h"
#include "Playlists.h"
#include "Songs.h"
#include "llistenings.h"
#include "PlaylistWindow.h"
#include "libraries/bass.h"             // bass
#include "AvId3Reader.h"                // ID3TAGS
#include "badSongs.h"                   // badsongs form
#include "Folders.h"                    // copy folder to another folder
#include <dirent.h> 	                // copy folders
#include "artists.h"                    // artists form
#include "AllSong.h"
#include "PlayDatesInfo.h"

	  // ============ MY FUNCTIONS ============
void loadFonts();                                 // loads all fonts
void freeFonts();                                 // unload all fonts
void checkFilesExisting();                        // checks whether all necessary files exists
void restoreMform();                             // maximize / restore mform
void maximizeMform();
void addSubVol(short choice);           	      // +- volume by 20%
void myShowModal(TForm *form);                    // shows form modal
UnicodeString intToUS(const int &integer);  	// conver int to US string
UnicodeString changeSlashes(UnicodeString str, char newSlashChar);
extern void openFolderSelect(UnicodeString path); // Open Folder and sel file

	  // ============ MY VARIABLES ============
Playlists pl;              // main instance of Playlists class
AddingFiles af;            // main instance of AddingFiles class
std::vector <Songs> songs;                       // main info about songs in playlist
std::vector <TButton*> plButtons;                // SCouner playlists
std::vector <UINT> matches;                      // for F3 search

bool emptySpaceClicked = false;
bool programIsCreating = true;
bool maximized = false;
bool renaming = false;                            // is playlist going to be renamed?
bool addingNewPl = false;                         // is user adding new pl? (for rearrange plBtns)
bool dblPress = false;                           // did user press btn twice? (like 'B' for pl 'best')
bool dblPressGlobal = false;
bool lWinPressing = false;                          // is win key pressing?
int device = -1;   	 							  // Default Sounddevice
short lastVolume;                                 // last volume of player (in procents 0-100%)
short plColWidthMin = 11;       // don't make hor.scrollbar for listview!
short plColWidthMax = 25;
FocusBtnState currFocusBtnState;    // which state has btn [FOCUS]?
UnicodeString adurText;                           // last [0:32] condition
WORD lastPressedKey = 0;                          // memorizes last pressed key (used in dblKeyPress)
WORD lastPressedGlKey = 0;                        // same, but for playlists.
DoublePress doubleKeyCatching;

extern std::vector <Shortcut> shortcutsData;     // here stored data with shorcuts
extern ReasonToDelete currReason;               // curr state for deleteBadSongs form
extern UINT notFound;                  // how many songs weren't found in 1st step?
extern std::vector <AllSong> allSongs;


//---------------------------------------------------------------------------
__fastcall Tmform::Tmform(TComponent* Owner)                                     // constructor
	: TForm(Owner)
{
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

	loadFonts();
	DragAcceptFiles(mform->Handle, true);             // allow Drag&Drop
  BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, true);      // checks if default sounddevice
  BASS_Init(device,44100,0,0,0);                      // has changed  + INIT

	AllSong::loadFromFile();    // load data from AllSongs.txt

	pl.loadPlaylistSettings();
	pl.loadPlaylist(pl.activePlaylist);               // load last playlist condition
	SHAutoComplete(editQSearch->Handle, SHACF_AUTOAPPEND_FORCE_OFF); // allow ctrl+backspace  F3 Search
	pl.loadForOtherForms('m');
	pl.loadPlaylistButtons();
	pl.refreshSongCounter("comm");
	restoreFromTray();                                // unload all global shortcuts
	timerAutoSave->Interval = 1000 * (60 * 30);  // save every 10 minutes

#ifdef _DEBUG
    mform->Caption = "SCounter Test";
#endif

	togCheckingMode->Hint = "Turn off to disable saving play/skip counts";
}
//---------------------------------------------------------------------------

void  __fastcall Tmform::WMDropFiles(TWMDropFiles &Message)                         		   // Drag&Drop
{
//clock_t start, stop; double TIMe; start = clock();
	disableMainForm(true);                          // don't allow user close program

	int firstFreeIndex = songs.size();                // memorize last count of playlist
	af.firstAddedInd = firstFreeIndex;
	pl.saveSelection();
	mform->labelPlDuration->Caption = "Adding files...";
    mform->labelPlCount->Caption = "";
  /////////// Drag & Drop
	UINT FileCount = DragQueryFile((HDROP) Message.Drop, 0xFFFFFFFF, NULL, 0);
	UnicodeString FileName;
	int FileLength;
 for(UINT i=0; i < FileCount; i++)
 {
	FileName.SetLength(MAX_PATH);
	FileLength = DragQueryFile((HDROP)Message.Drop, i, FileName.c_str(), FileName.Length());
	FileName.SetLength(FileLength);

	af.process_dropped_filename(FileName);               // here things happen
 }
	DragFinish((HDROP) Message.Drop);
 /////////// Drag & Drop

	playlist->Items->Count = songs.size();               // refresh playlist
    pl.firstInactive = songs.size();

	disableMainForm(true);                          // allow user close program
	af.loadAddedSongsData(firstFreeIndex);     	    // start loading data

//stop = clock(); TIMe = (double)(stop-start)/CLOCKS_PER_SEC; ShowMessage(TIMe);
}

//---------------------------------------------------------------------------

void __fastcall Tmform::FormClose(TObject *Sender, TCloseAction &Action)                    // On Close
{
    pl.saveEverything();		// save data.
//    Shortcut::unregisterAllShortcuts(); 	// unregister all shortcuts   NOPE!!! SEE ONDESTROY FUNC

    freeFonts();
	pl.stop();
	BASS_Free();
}
//---------------------------------------------------------------------------

void __fastcall Tmform::playlistData(TObject *Sender, TListItem *Item)                      // fill playlist
{                                                                                           // data
   Item->Caption = songs[Item->Index].name;
   Item->SubItems->Add(songs[Item->Index].duration);
   if(songs[Item->Index].duration != "")
   	  Item->SubItems->Add(songs[Item->Index].listenings);
   Item->SubItems->Add("");
}
// BUTTONS             BUTTONS              BUTTONS           BUTTONS             BUTTONS

void __fastcall Tmform::playbtnClick(TObject *Sender)                            	       // [PLAY]
{  pl.playPauseResume();                                                   		        }
//---------------------------------------------------------------------------
void __fastcall Tmform::stopbtnClick(TObject *Sender)                             	       // [STOP]
{  pl.stop();    											                            }
//---------------------------------------------------------------------------
void __fastcall Tmform::prevbtnClick(TObject *Sender)                              		   // [PREVIOUS]
{  pl.prevNext(-1);	    																}
//---------------------------------------------------------------------------
void __fastcall Tmform::nextbtnClick(TObject *Sender)                                      // [NEXT]
{  pl.prevNext(1);      												}
//---------------------------------------------------------------------------
void __fastcall Tmform::shufflebtnClick(TObject *Sender)                                   // [SHUFFLE]
{
	pl.shuffleClick(false);
	pl.makeVisibleCurrSong(3,0);
	if(editQSearch->Focused()) {
		matches.clear();
		playlist->SetFocus();
	}
}
//---------------------------------------------------------------------------
void __fastcall Tmform::findDoublesbtnClick(TObject *Sender)                               // [FIND DOUBLES]
{  pl.findDoubles(true);                                                        		    }
//---------------------------------------------------------------------------
void __fastcall Tmform::focusbtnMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{                                                                                          // [FOCUS]
   if(mform->Visible)
   		playlist->SetFocus();

   if(X > 37) return;              // GUaRD. This function for some reason calls by popupItems too.
                                   // We prevent that
   switch(currFocusBtnState)
   {
	   case FocusPlaying:
	   {
			if(pl.stopped)         // GUARD
				return;
			int index = pl.existsInPlaylist("int", false, -1
				, *&(playingSongInfo->Items->Strings[1]), *&(playingSongInfo->Items->Strings[2]));

			if(index != -1) {
				pl.actPlayingSong = index;
                pl.playingSong = index;
				playlist->Items->Item[index]->MakeVisible(true);
            }
       }
	   break;

	   case FocusSelected:
			if (pl.selectedSong != -1) playlist->Items->Item[pl.selectedSong]->MakeVisible(true);
	   break;

	   case FocusSCS:
			if(shufflebtn->Tag && songs.size())
			{
				 if(pl.shuffleCurrSong[filterbtn->Tag] == 0)  playlist->Items->Item[0]->MakeVisible(true);
				 else playlist->Items->Item[pl.shuffleCurrSong[filterbtn->Tag] -1]->MakeVisible(true);
			}
	   break;
   }
}
//---------------------------------------------------------------------------
void __fastcall Tmform::filterbtnClick(TObject *Sender)                                    // [FILTER] pop-up
{
   for(int i=0; i < 4; i++)
   {
	 TMenuItem *popUpItem = filterPopUp->Items->Items[i];
	 if(filterbtn->Tag == popUpItem->Tag) { popUpItem->Checked = true; popUpItem->Enabled = false; }
	 else  { popUpItem->Checked = false; popUpItem->Enabled = true; }
   }

   filterPopUp->Popup(mform->Left+filterbtn->Left +10, mform->Top+filterbtn->Top +45);
}
//---------------------------------------------------------------------------
void __fastcall Tmform::popPlItemPopup(TObject *Sender)              					   // [PL ITEM] r-click
{   refreshPlItemPopMenu();            }
//---------------------------------------------------------------------------
void __fastcall Tmform::filterMenuItemClick(TObject *Sender)                       		   // [FILTER] onItemClick
{
   TMenuItem *ClickedItem = dynamic_cast <TMenuItem *> (Sender);

   if(ClickedItem->Tag == 9)                  // user wants to enter settings window
   { setForm->treeV->Selected = setForm->treeV->Items->Item[3];
	 setForm->PageControl1->ActivePageIndex = 3;
	 myShowModal(setForm); return; }

   if(filterbtn->Tag == ClickedItem->Tag) return;  // don't trigger this second time.
   filterbtn->Tag = ClickedItem->Tag;
   if(!shufflebtn->Tag)					       // when shuffe is OFF
	  pl.filter(ClickedItem->Tag);
   else							 		       // when shuffe is ON
	  pl.shuffleClick(true);

	pl.refreshSongCounter("comm");
	pl.makeVisibleCurrSong(3,0);
    pl.refreshPlDuration();
}
//---------------------------------------------------------------------------
void __fastcall Tmform::clearbtnClick(TObject *Sender)                                     // [CLEAR]
{
   //pl.stop();         // IS THAT DANGEROUS?
   pl.falseStop();

   pl.selectedSong = -1;
   songs.clear(); playlist->Items->Count = 0;
   pl.firstInactive = 0;

   TSpeedButton *btn = dynamic_cast <TSpeedButton*> (Sender);
   if(btn->Caption == "Clear")  pl.refreshSongCounter("comm");

   for(int i=0; i < 5; i++)
	 { pl.started[i] = 0; if(shufflebtn->Tag && filterbtn->Tag == i) pl.started[i] = 1; }
   std::fill(pl.shuffleCurrSong, pl.shuffleCurrSong+5, 0); currCounter->Caption = 0;
}
//---------------------------------------------------------------------------              // [COUNT US]
void __fastcall Tmform::countusbtnClick(TObject *Sender)
{
   UINT amount=0, index;

   index = ListView_GetNextItem(mform->playlist->Handle, -1, LVNI_SELECTED);
   while(index != -1)
   {
	 amount += songs[index].listenings;
	 index = ListView_GetNextItem(mform->playlist->Handle, index, LVNI_SELECTED);
   }
                                                                  // show message!
   UnicodeString mes = L"You have listened these songs\n"; mes += amount; mes+=L"\n times";
   messageForm->showMessage(mes, 'i', 666);
}
//---------------------------------------------------------------------------            // [DELETE BAD SONGS]
void __fastcall Tmform::dbsClick(TObject *Sender)
{
	currReason = Twin;
	myShowModal(formBadSongs);
}
//---------------------------------------------------------------------------            // [COUNTER]
void __fastcall Tmform::counterClick(TObject *Sender)
{  myShowModal(Listenings);                                               }
//---------------------------------------------------------------------------
void __fastcall Tmform::muteClick(TObject *Sender)                                      // [MUTE]
{
	if(volume->Position)                                // if tag == 1 (sound is ON)
	{ BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 0);
	  lastVolume = volume->Position;
	  volume->Position = 0;                          // mute!
	  mute->Picture->LoadFromFile("res/img/muted.png");							}
	else
	{ BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 100*lastVolume);
	  volume->Position = lastVolume;
	  mute->Picture->LoadFromFile("res/img/unmuted.png");                       }
}
//---------------------------------------------------------------------------
void __fastcall Tmform::volumeMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) // [VOLUMEBAR]
{
	if(GetKeyState(VK_LBUTTON) & 0x100)           // if left mouse button is down
	{
	   volume->Position = X*100/volume->Width;
	   float vol = volume->Position;  vol /= 100;
	   BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 10000*vol);
	   if(!volume->Position)        // muted
		  mute->Picture->LoadFromFile("res/img/muted.png");
	   else
	   {  mute->Picture->LoadFromFile("res/img/unmuted.png");
	   lastVolume = volume->Position;}
	}
}
void __fastcall Tmform::volumeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)
{
   volumeMouseMove(NULL, Shift, X, Y);
}
//---------------------------------------------------------------------------
void addSubVol(short choice)
{
   if(choice == -1)     // substraction
   {
	   if(mform->volume->Position == 0) return;
	   if(lastVolume - 20 > 0)
	   {
		   lastVolume -= 20;
		   mform->volume->Position -= 20;
		   BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 100*lastVolume);
	   }
	   else                                                   // user MUTED!
	   {
		   mform->mute->Picture->LoadFromFile("res/img/muted.png");
		   mform->volume->Position = 0;
		   BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 0);
	   }
   }
   else                // addition
   {
	   if(lastVolume + 20 < 100 || !mform->volume->Position)
	   {
		   if(mform->volume->Position == 0)       // if player was muted
		   {   mform->mute->Picture->LoadFromFile("res/img/unmuted.png");
			   lastVolume = 0;
		   }
		   lastVolume += 20;
		   mform->volume->Position += 20;
		   BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 100*lastVolume);
	   }
	   else
	   {
		   lastVolume = 100;
		   mform->volume->Position = 100;
		   BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 10000);
	   }
   }
}
//---------------------------------------------------------------------------             // [SETTINGS]
void __fastcall Tmform::settingsClick(TObject *Sender)
{  myShowModal(setForm); }
//---------------------------------------------------------------------------
void Tmform::restoreFromTray()                          					// restore from tray
{
	tray->Visible = false;
	Show();
	Application->BringToFront();
	SetActiveWindow(Application->Handle);            // only this matters
	mform->SetFocus();
	if(pl.terminatingSong) {
		if(mform->nextbtn->Tag ==2) { mform->nextbtn->Tag = 1; myShowModal(messageForm); }
		else  messageForm->Visible = true;
	}

    for(UINT i=0; i < shortcutsData.size(); i++)         // unregister all shortcuts
		for(UINT j=0; j < plButtons.size(); j++)
			if(shortcutsData[i].vkCode && shortcutsData[i].functionName == plButtons[j]->Caption)
				shortcutsData[i].unregisterShortcut(shortcutsData[i]);
}
//---------------------------------------------------------------------------
void __fastcall Tmform::trayClick(TObject *Sender)                            // [TRAY] click
{
	if(timerDblClick->Enabled) { timerDblClick->Tag = 1; return; }   // it's double-click!
	timerDblClick->Enabled = true;      	  // play/pause
}
//---------------------------------------------------------------------------
void __fastcall Tmform::trayDblClick(TObject *Sender)                         // [TRAY] DBL click
{  timerDblClick->Tag = 1;  Sleep(130);  restoreFromTray();   }         // restore
//---------------------------------------------------------------------------
void __fastcall Tmform::traySettingsClick(TObject *Sender)                    // [TRAY] SETTINGS click
{    myShowModal(setForm);}
//---------------------------------------------------------------------------
void __fastcall Tmform::Exit1Click(TObject *Sender)      					  // [TRAY] EXIT btn
{
	trayPopup->Tag = 1;
	mform->Close();
}
//---------------------------------------------------------------------------

	 // BUTTONS  (end)           BUTTONS    (end)          BUTTONS    (end)       BUTTONS    (end)         BUTTONS

void __fastcall Tmform::timerTimer(TObject *Sender)                                         // TIMER
{
	UnicodeString Adur = pl.refreshingDuration();               // refresh 00:44 adur
	if(Adur != "0" && Adur != "00:00" && pl.stopped == false)
	   adur->Caption = Adur;

	if(pl.iteration() == true)                                  // if 75% was listened
	   pl.iterate();
}
//---------------------------------------------------------------------------

	 // HOTKEYS 					HOTKEYS            HOTKEYS                HOTKEYS                   HOTKEYS

void __fastcall Tmform::playlistKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(!lWinPressing && Key == vkLWin)       // remember lwin key state
		lWinPressing = true;

	if(containsOnly(Shift, Alt) && Key == vkF4) Exit1Click(NULL);	       // [ALT + F4] !!!
	if(containsOnly(Shift, Ctrl) && Key == vkA) playlist->SelectAll();    // [CTRL + A]
	if(Key == vkReturn && playlist->SelCount)  pl.refreshSong();       // [ENTER]
	if(Key == vkF3)       editQSearch->SetFocus();                     // [F3 SEARCH]
	if(Key == vkEscape && setForm->minimizeOnEsc->Checked)
		minimize();			                         					// [ESCAPE]

	if(shufflebtn->Enabled && Key == vkDelete && containsOnly(Shift, ModifierKey::None))
		af.deleteFiles();    // [DELETE]  mform is disabled, thus disable this func, 'cus  it's dangerous.

	if(Key == vkControl // DON'T ALLOW JUST CTRL, SHIFT, WIN, ALT TO STEP HERE
	   || Key == vkShift
	   || Key == vkMenu
	   || Key == vkLWin)
         return;

	UnicodeString functionName;							// FIND OUT FUNCTION NAME OF SHORTCUT
	for(int i=0; i < shortcutsData.size(); i++)
	{
		TShiftState shortcutShift;                          // in settings (Alt + Ctrl + C)
		if(shortcutsData[i].alt)   shortcutShift << ssAlt;
		if(shortcutsData[i].ctrl)  shortcutShift << ssCtrl;
		if(shortcutsData[i].shift) shortcutShift << ssShift;


		bool playlistHotkey = false;        // is that hotkey for playlist?
		for(UINT j=0; j < plButtons.size(); j++)
			if(plButtons[j]->Caption == shortcutsData[i].functionName)
			{
			   if(Key == shortcutsData[i].vkCode
			   && ( (Shift == shortcutShift && lWinPressing == shortcutsData[i].win)  // 'Ctrl + Shift + A'
				   || containsOnly(Shift, ModifierKey::None) ) )             // 'A' for AllSongs
				playlistHotkey = true;
            }

		if((Key == shortcutsData[i].vkCode
		   && Shift == shortcutShift
		   && lWinPressing == shortcutsData[i].win)    // we found pressed combination
		   || playlistHotkey
		   || (Key == shortcutsData[i].vkCode
				&& (containsOnly(Shift, ModifierKey::None) || containsOnly(Shift, ModifierKey::Shift))
				&& shortcutsData[i].functionName == "Focus playing/selected/SCS song" ))
		{                                                              // in the vector!
		   if(!shortcutsData[i].vkCode || (shortcutsData[i].global && !playlistHotkey))
				return;  // we don't  care about global ones

		   functionName = shortcutsData[i].functionName;  // we know which function to call!
		   break;
		}
	}

  if(shufflebtn->Enabled)                    // shortcuts, which can be used only if mform is "enabled"
  {
	if(functionName == "Create new playlist")       btnAddClick(NULL);
	if(functionName == "Previous playlist page")    btnLeftClick(NULL);
	if(functionName == "Next playlist page")        btnRightClick(NULL);
	if(functionName	== "["+all->Caption+"]")		filterMenuItemClick(all);         // FILTERS
	if(functionName	== "["+less5->Caption+"]")		filterMenuItemClick(less5);
	if(functionName	== "["+from5to20->Caption+"]")	filterMenuItemClick(from5to20);
	if(functionName	== "["+more20->Caption+"]")		filterMenuItemClick(more20);
	if(functionName	== "["+more100->Caption+"]")	filterMenuItemClick(more100);
	if(functionName == "Delete bad songs")          dbsClick(mform);
	if(functionName == "Shuffle")                   shufflebtnClick(NULL);
	if(functionName == "Count us")                  if(playlist->SelCount > 1) countusbtnClick(NULL);
	if(functionName == "Find twins")                pl.findDoubles(true);
	if(functionName == "Open file location (of selected song)") if(playlist->SelCount == 1) openFolderSelect(songs[pl.selectedSong].path);

	if(functionName == "Add artists to playlist")   btnAddArtistsClick(NULL);
	if(functionName == "Refresh selected song")     Refresh1Click(NULL);
	if(functionName == "Delete playing song")       btnDelPlayingSongClick(NULL);


	bool modKey1and2Pressed = containsOnly(Shift, setForm->cboxMod1->ItemIndex, setForm->cboxMod2->ItemIndex);
							  // detect if modkey1 and modkey2 were pressed.
	doubleKeyCatching = DoublePress::Local;
	for(UINT i=0; i < plButtons.size(); i++)               // PLAYLISTS
	{
		if(functionName == plButtons[i]->Caption)
		{
			if(containsOnly(Shift, ModifierKey::None))
			{
				dblPress = false; 					// DETECTING DBL PRESS
				if(Key == lastPressedKey) {
					dblPress = true;
                    lastPressedKey = 0; 	// reset last pressed key
                }
				else {
					lastPressedKey = Key;  // memorize this key as last pressed
					timerDblPress->Enabled = true;
				}
			}


			if(dblPress                                  // copy SELECTED SONGS (or PLAYING SONG)
			   || modKey1and2Pressed)
			{
			   TMenuItem *menuItem = new TMenuItem(this);
			   menuItem->Caption = plButtons[i]->Caption;
			   if(dblPress)
				   menuItem->Tag = 2; // playing song only

			   if(menuItem->Tag == 0 && pl.activePlaylist == menuItem->Caption)
			   {                         // e.g. ignore "ctrl + shift + A" if user already in "AllSongs"
				   delete menuItem;
                   return;
               }

			   hiddenItemClick(menuItem);		// copy that
			}
		}

	}
  }

	if(functionName == "Play, pause, resume")  		pl.playPauseResume();
	if(functionName == "Previous song")             prevbtnClick(NULL);
	if(functionName == "Next song")                 nextbtnClick(NULL);
	if(functionName == "Focus playing/selected/SCS song"
		&& !mform->WindowState
		&& Shift == TShiftState() << ssShift)
	{
		WCHAR *currState = focusbtn->Caption.SubString1(1,1).c_str();

		switch (*currState)
		{   				//   P, S, SCS
			case L'':   playingItemClick(selectedItem);           break;        // playing song
			case L'':   playingItemClick(scsItem);          	   break;        // selected song
			case L'':   playingItemClick(playingItem);            break;        // SCS
		}
	}
	else if(functionName == "Focus playing/selected/SCS song" && !mform->WindowState)
		focusbtnMouseDown(NULL,0,TShiftState(0),0,0);
	if(functionName == "Listenings window")         counterClick(NULL);
	if(functionName == "Mute/Unmute")               muteClick(NULL);
	if(functionName == "Volume down 20%")           addSubVol(-1);
	if(functionName == "Volume up 20%")             addSubVol(1);
	if(functionName == "Settings")                  settingsClick(NULL);
	if(functionName == "Hide / Show SCounter")      if(setForm->minimizeOnEsc->Checked) minimize();
	if(functionName == "Terminating Song")          nextbtnMouseDown(0, 0, (TShiftState)0, 0, -22);

	if(functionName == "Play next song automatically (ON/OFF)") cboxAutoNextClick(NULL);
	if(functionName == "Open playing song destination folder")  btnOpenDirPlSongClick(NULL);

//	if(functionName == "Quick search")              editQSearch->SetFocus();  // NOT FINISHED  ??
//	if(functionName == "Previous match (Quick search F3)")
//	if(functionName == "Next match (Quick search F3)")

}

void __fastcall Tmform::playlistKeyPress(TObject *Sender, System::WideChar &Key)   // turn off searching in
{                                                                                  // ListView
	Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall Tmform::playlistDblClick(TObject *Sender)               // [DBL CLICK]
{   if(!emptySpaceClicked) pl.refreshSong();	}
//---------------------------------------------------------------------------
void __fastcall Tmform::ds(TObject *Sender)
{
	if(adurText != adur->Caption)
	{  prBar->Position = pl.progressBar(); adurText = adur->Caption; }  // progressBar animation
}
//---------------------------------------------------------------------------

	// HOTKEYS 					HOTKEYS            HOTKEYS                HOTKEYS                   HOTKEYS


void __fastcall Tmform::playlistChange(TObject *Sender, TListItem *Item, TItemChange Change)  // on Change
{
	if(playlist->SelCount)                 			 // selected files by user
		pl.selectedSong = playlist->Selected->Index;

	if(playlist->SelCount < 2) countusbtn->Enabled = false;
	else                       countusbtn->Enabled = true;
}

//---------------------------------------------------------------------------

void __fastcall Tmform::playlistDrawItem(TCustomListView *Sender, TListItem *Item          // on DrawItem
, TRect &Rect, TOwnerDrawState State)
{
	TListView *ListView = dynamic_cast<TListView*>(Sender);
	TCanvas *Canvas = Sender->Canvas;
	int index = Item->Index;

	Canvas->Brush->Color = RGB(25,29,34);               // LV usual color
	Canvas->Font->Color = RGB(210,210,210);             // font (WHITE) for usual songs

	if(playlist->SelCount && playlist->Items->Item[Item->Index]->Selected)
	   Canvas->Brush->Color = RGB(75,75,75);    // selectedSong (GRAY)

	if(shufflebtn->Tag && index < pl.shuffleCurrSong[filterbtn->Tag])
	{
	  Canvas->Brush->Color = RGB(63,90,60);          // listened songs DIM GREEN
	  Canvas->Font->Color =  RGB(0,0,0);             // + font (BLACK)
	}

	if(playlist->Items->Item[index]->Selected)
	  Canvas->Font->Color = RGB(210,210,210);     // selectedSongs (font) (WHITE)

	if(shufflebtn->Tag                    			  // SCS color
	 && (index == pl.shuffleCurrSong[filterbtn->Tag] -1 || (!index && !pl.shuffleCurrSong[filterbtn->Tag])))
	{
		if(Canvas->Brush->Color != 0x009B9B00 && index < pl.firstInactive)  //  (don't change brush color if
		    Canvas->Brush->Color = RGB(25,100,25);     // VERY DARK GREEN   //   this song is playing)

		if(index == 0 && !playlist->Items->Item[index]->Selected) // first item should have black font (fixed bug)
            Canvas->Font->Color = clBlack;

		if(index -1 >= 0)
			playlist->Items->Item[index-1]->Update();  // fixing prev item
	}

    if(index >= pl.firstInactive) 					        	// inactive songs
		Canvas->Font->Color = RGB(0,0,0);           			// (font) BLACK

	if(!pl.stopped && songs[index].name == playingSongInfo->Items->Strings[0])   // indeed actPlayingSong
	{
		Canvas->Brush->Color = 0x00909000;     // LIGHT BLUE

		if(playlist->Items->Item[index]->Selected)  // if selected
			 Canvas->Font->Color = clWhite;
		else
			 Canvas->Font->Color = clBlack;
	}

	Canvas->FillRect(Rect);
	Canvas->Brush->Style = bsClear;
	Rect.Right = ListView->Columns->Items[0]->Width;

	UnicodeString text1;                                       // set text of 1 column (name)
	if( songNameShouldBeHidden(index) )
		text1 = "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"; // hide name
	else
		text1 = songs[index].name;       // show name

	Canvas->TextRect(Rect, Rect.Left+7, Rect.Top+1, text1);

	for (int col = 0; col < 2; col++)                           // draw the columns
	{
	  Rect.Left = Rect.Left + ListView->Columns->Items[col]->Width + 1;
	  Rect.Right = Rect.Left + ListView->Columns->Items[col + 1]->Width;

	  UnicodeString text;									   // set text of 2,3 column (name)
	  if(songNameShouldBeHidden(index))
		  text = "- - -";                      // hide unlistened songs =)
	  else
		  text = ListView->Items->Item[index]->SubItems->Strings[col];  // don't hide


	  short center = Rect.Left + (Rect.Right - Rect.Left - Canvas->TextWidth(text))/2;
	  Canvas->TextRect(Rect, center , Rect.Top+1, text);
	}
//    Canvas->Free();
}

//---------------------------------------------------------------------------
void __fastcall Tmform::mouseEnter(TObject *Sender)                                        // Mouse Enter
{  ((TButton*)Sender)->Font->Color = 0x001EC823;             }  // GREEN

void __fastcall Tmform::mouseLeave(TObject *Sender)                                        // Mouse Leave
{
   if(pl.terminatingSong) return;                                 // if user terminates!
   if(((TButton*)Sender)->Tag == 1 && ((TButton*)Sender)->Name !=L"filterbtn")
	  ((TButton*)Sender)->Font->Color = clBlack;                  // BLACK
   else ((TButton*)Sender)->Font->Color = 0x00DCDCDC;             // WHITE-GRAY
}
//---------------------------------------------------------------------------
void __fastcall Tmform::nextbtnMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)                                                          // on Right Click [NEXT]
{
													  // from other places
	if(pl.playing && mform->Tag && (Button == TMouseButton::mbRight || Y == -22))  // terminating song
	{
		pl.terminatingSong = true;
        bool tempShowTheEnd = pl.showTheEnd; // save user value
		pl.showTheEnd = false;
		tray->IconIndex = 3;               // trayIcon terminating play

														// show the message
		messageForm->btnAbort->Visible = true;
		if(!pl.showTermSongMes 					   //  don't show the message!!
		  || (!mform->Visible || !mform->Active))
        {
			nextbtn->Tag = 2;     // NEXTBTN tag '2' means, that SCounter hides immediately
			messageForm->showMessage("SCounter will close when this song ends.", 't', 2);
			mform->minimize();
		}
		else
		    messageForm->showMessage("SCounter will close when this song ends.", 't', 2);

		pl.showTheEnd = tempShowTheEnd; // restore user value
	}
}
//---------------------------------------------------------------------------


void __fastcall Tmform::WMSysCommand(TMessage &Msg)                             // on minimize, close
{	unsigned int const sys_code = Msg.WParam & 0xFFF0;

	if(sys_code == SC_MINIMIZE)
	{
		// ignore windows minimizing  onMinimize + onClose+minimize
		if(setForm->toTrayGroup->ItemIndex %2 == 1)      // on 1 and 3
		{
			minimize();
			Msg.Result = 0;
			return;
		}
		else {             // register all pl shortcuts
		   for(UINT i=0; i < shortcutsData.size(); i++)                 // register all plGlobal shortcuts
				for(UINT j=0; j < plButtons.size(); j++)
					if(shortcutsData[i].vkCode && shortcutsData[i].functionName == plButtons[j]->Caption)
						shortcutsData[i].registerShortcut(shortcutsData[i]);
        }
	}

	if(Msg.Msg == WM_SYSCOMMAND && Msg.WParam ==SC_KEYMENU)
		 return;     // disable ALT key

	TForm::Dispatch(&Msg);


//	else if(sys_code == SC_MAXIMIZE)              // user maximized wnd
//	else if(sys_code == SC_RESTORE)              // user restoredd wnd
}
//---------------------------------------------------------------------------
void Tmform::minimize()                             				// simulates user Minimize click
{                      // onMinimize + onMin and onClose
   if(setForm->toTrayGroup->ItemIndex != 0)  // DINT???
   {
	   if(pl.playing) {
		  if(pl.terminatingSong)  mform->tray->Icon->LoadFromFile("res/img/tray terminating.ico");
		  else                    mform->tray->Icon->LoadFromFile("res/img/tray playing.ico");
       }

	   else if(!pl.stopped)  {
		  if(pl.terminatingSong)  mform->tray->Icon->LoadFromFile("res/img/tray terminatingPause.ico");
		  else                    mform->tray->Icon->LoadFromFile("res/img/tray paused.ico");
	   }
	   else                     mform->tray->Icon->LoadFromFile("res/img/tray default.ico");

	   Hide();
	   tray->Visible = true;
   }
   else    Application->Minimize();

   for(UINT i=0; i < shortcutsData.size(); i++)                 // register all plGlobal shortcuts
		for(UINT j=0; j < plButtons.size(); j++)
			if(shortcutsData[i].vkCode && shortcutsData[i].functionName == plButtons[j]->Caption)
                shortcutsData[i].registerShortcut(shortcutsData[i]);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::FormCloseQuery(TObject *Sender, bool &CanClose)      // prevent from closing if traying
{          // ignore windows closing
	 if(mform->Tag == 0) { CanClose = false; return; }   // mform is disabled, can't close
	 if(setForm->toTrayGroup->ItemIndex > 1 && trayPopup->Tag != 1) // tray tag = 1 means close SCounter
	 {
		CanClose = false;
	    minimize();
	 }                                                      // PROGRAM CAN CLOSE
	 else { CanClose = true; af.programIsClosing = true; }  // do what windows wants
}

//---------------------------------------------------------------------------





void loadFonts()                                                              // loads my fonts.
{
   AddFontResourceW(L"res/fonts/buttons.ttf");
   AddFontResourceW(L"res/fonts/symbola.ttf");
   AddFontResourceW(L"res/fonts/counter.ttf");
   AddFontResourceW(L"res/fonts/playingSong.ttf");
   AddFontResourceW(L"res/fonts/small numbers.ttf");
   AddFontResourceW(L"res/fonts/SCounterFont.ttf");
}
void freeFonts()
{
	RemoveFontResourceW(L"res/fonts/buttons.ttf");
	RemoveFontResourceW(L"res/fonts/symbola.ttf");
	RemoveFontResourceW(L"res/fonts/counter.ttf");
	RemoveFontResourceW(L"res/fonts/playingSong.ttf");
	RemoveFontResourceW(L"res/fonts/small numbers.ttf");
	RemoveFontResourceW(L"res/fonts/SCounterFont.ttf");
}

// =====



void __fastcall Tmform::editQSearchChange(TObject *Sender)                 // quick search F3
{
	if(editQSearch->Text.Length() < 2 || editQSearch->Text == "Quick search [F3]"
	   || editQSearch->Text == "")
	{  labelQSearch->Visible = false;    return;   }   // GUARD  + hide label
	else labelQSearch->Visible = true;  // show label

	matches.clear();

	playlist->Selected = false;       // clear selection
	pl.selectedSong = -1;

	for(int i=0; i < songs.size(); i++)
	{
	   if(songs[i].name.LowerCase().Pos(editQSearch->Text.LowerCase()))    // there is such song!
	   {
		  if(matches.size() == 0) {                     // select first song
			 playlist->ItemIndex = i;
			 playlist->Items->Item[i]->MakeVisible(true);
		  }
		  matches.push_back(i);    // add to vector
	   }
	}

	if(matches.size())   		// change label
	{
	   labelQSearch->Caption = "1 / ";
	   labelQSearch->Caption = labelQSearch->Caption + matches.size();
	   labelQSearch->Font->Color = clWhite;
	}
	else
	{
	  labelQSearch->Caption = "no matches";
	  labelQSearch->Font->Color = 0x000934FF;    // almost red
	}
}
//---------------------------------------------------------------------------

void __fastcall Tmform::editQSearchEnter(TObject *Sender)               // on Search Enter
{
   if(editQSearch->Text == "Quick search [F3]") editQSearch->Text = ""; // clear edit
   labelQSearch->Visible = true;                                        // show label
   labelQSearch->Caption = "";

   editQSearch->Font->Color = clWhite;                  // white "active" font
}
//---------------------------------------------------------------------------

void __fastcall Tmform::editQSearchKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(Key == vkEscape  || Key == vkReturn)                      // exit QSearch
	{
	   playlist->SetFocus();
	   labelQSearch->Visible = false;   // hide label
	   if(Key == vkReturn && playlist->SelCount == 1 && editQSearch->Text.Length() > 1
		  && editQSearch->Text != "Quick search [F3]")
		  pl.refreshSong();
	}

	if(Key == vkF1 || Key == vkF2)
	{
		int index=0;

		if(playlist->SelCount == 1)          // find index in matches vector
		{
		   ptrdiff_t pos = find(matches.begin(), matches.end(), playlist->ItemIndex) - matches.begin();
		   if(pos < matches.size()) index = pos;    // founded!
           else return;

		   playlist->Selected = false;
		}
		else return;

		if(Key == vkF1 && index > 0)			     index --;
		if(Key == vkF2 && index < matches.size() -1) index ++;

		playlist->ItemIndex = matches[index];                 // select prev/next
		playlist->Items->Item[matches[index]]->MakeVisible(true);

		labelQSearch->Caption = index+1;                      // change caption
		labelQSearch->Caption = labelQSearch->Caption + " / ";
		labelQSearch->Caption = labelQSearch->Caption + matches.size();
	}

    if(Key == vkF3 && labelQSearch->Caption == "") editQSearchChange(NULL);   // search entered on F3
}
//---------------------------------------------------------------------------

void __fastcall Tmform::editQSearchExit(TObject *Sender)     // F3 search on loosing focus
{
	 if(editQSearch->Text == "")
		editQSearch->Text = "Quick search [F3]";
	 editQSearch->Font->Color = clInactiveCaption; // grey font
	 labelQSearch->Visible = false;   // hide label

			// clear F3 matches except last selected!
	 int lastMatch = -1;
	 for(int i=0; i < matches.size(); i++)
		if(pl.selectedSong == matches[i])
		{	lastMatch = matches[i]; break; }

	 matches.clear();
	 if(lastMatch > -1)
		matches.push_back(lastMatch);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos,
		  bool &Handled)
{                                                                    // scroll playlist panel
	POINT mousePos;
	short borderWidth =  (Width - ClientWidth) /2;
	mousePos.x = MousePos.x -Left - borderWidth;
	mousePos.y = MousePos.y -Top  -(Height - ClientHeight - borderWidth) ;
                                                                              // scrolling playlist pages
	if( shufflebtn->Enabled
	 && mousePos.x > panelMain->Left && mousePos.x < panelMain->Left + panelMain->Width
	 && mousePos.y > panelMain->Top && mousePos.y < panelMain->Top + panelMain->Height)
	{
		if(WheelDelta < 0)
			btnRightClick(NULL);
		else
			btnLeftClick(NULL);

        Handled = true;

		return;
    }

	int step = 10;
	if(!btnInvisible->Focused() && !playlist->Focused())      // scrolling playlist
	{
	   if(WheelDelta < 0)	playlist->Scroll(0, step);     // scroll down
	   if(WheelDelta > 0) 	playlist->Scroll(0, -step);    // up
	}
}
//---------------------------------------------------------------------------

void __fastcall Tmform::ApplicationEvents1Message(tagMSG &Msg, bool &Handled)  // ON MESSAGE
{
	if(Msg.message == WM_KEYDOWN)     // WM_KEYDOWN
	{
		switch(Msg.wParam)
		{
			case vkTab:                                     // TAB KEY
				 shufflebtnClick(NULL);
				 Handled = true;
				 return;

			case vkLWin:                    // lwinkey is pressing
				lWinPressing = true;
			break;
		}
	}

	if(Msg.message == WM_KEYUP && Msg.wParam == vkLWin) // lwinkey no longer pressing
		lWinPressing = false;

	if (Msg.message == WM_HOTKEY)        // HOTKEY
	{
		switch(Msg.wParam)    // this is shortcut ID
		{
			case 1:
			case 401:
			case 201:   playbtnClick(NULL);  				  break;           // Play, pause, resume
			case 2:
			case 402:
			case 202:   if(!pl.terminatingSong) prevbtnClick(NULL);   break;   // Previous song
			case 3:
			case 203:
			case 403:   if(!pl.terminatingSong) nextbtnClick(NULL);   break;   // Next song
			case 4:
			case 204:  	focusbtnMouseDown(NULL,0,TShiftState(0),0,0);    break;  // Focus playing song
			case 5:
			case 205:   if(!Listenings->Visible) counterClick(NULL);
						else Listenings->Close(); break;		   // Listenings window
			case 8:
			case 208:   muteClick(NULL);       break;          // Mute/Unmute
			case 9:
			case 209:   addSubVol(-1);         break;          // Volume down 20%
			case 10:
			case 210:   addSubVol(1);          break;          // Volume up 20%
									  // 15,16,17 is for Quick Search F3
			case 18:
			case 218:                                          // Hide / Show SCounter
			{
					if(!mform->Visible) restoreFromTray(); 		   // Trayed?    - restore!
			   else if(!mform->Active) Application->Restore();     // Taskbared? - restore!
					else  minimize();            // - hide!
			}
			break;

			case 19:                                           // Terminating Song
			case 219:     if(!pl.terminatingSong)
				nextbtnMouseDown(0, 0, (TShiftState)0, 0, -22);
			break;
			case 24:                                           // enable/disable auto Next
			case 224:
				cboxAutoNextClick(NULL);
			break;

			case 26:                                           // open playing song dest folder
			case 226:
				btnOpenDirPlSongClick(NULL);
			break;


		  if(shufflebtn->Enabled)       // shortcuts, which can be used only if mform is "enabled"
		  {
			case 6:
			case 206:   shufflebtnClick(NULL);           // Shuffle
			break;
//			case 7:
//			case 207:   if(playlist->SelCount > 1)
//						countusbtnClick(NULL); 		           // Count us   (not accesible)
//			break;

			case 11:
			case 211:   if(!Form2->Visible) findDoublesbtnClick(mform); break;     // Find twins
			case 12:
			case 212:   dbsClick(mform);       break;          // Delete bad songs
			case 13:
			case 213:   if(!setForm->Visible) settingsClick(NULL);   break;          // Settings
			case 14:
			case 214:   if(playlist->SelCount == 1)            // Open file location (of selected song)
						openFolderSelect(songs[pl.selectedSong].path);
			break;
			case 20:                                           // add new playlist
			case 220:
				btnAddClick(NULL);
			break;                                             // next, prev playlist page
			case 21:
			case 221:   btnLeftClick(NULL);     break;
			case 22:
			case 222:   btnRightClick(NULL);    break;
			case 23:                                           // refresh playing song duration
			case 223:
				Refresh1Click(NULL);
			break;

			case 25:                                           // delete playing song
			case 225:
				btnDelPlayingSongClick(NULL);
			break;

			case 27:                                           // add artists
			case 227:
				if(!formArtists->Visible)
					btnAddArtistsClick(NULL);
			break;

			case 29:
			case 229:   filterMenuItemClick(all);     break;    // fitler 1
			case 30:
			case 230:   filterMenuItemClick(less5);   break;    // filter 2
			case 31:
			case 231:   filterMenuItemClick(from5to20);  break; // filter 3
			case 32:
			case 232:   filterMenuItemClick(more20);  break;    // filter 4
			case 33:
			case 233:   filterMenuItemClick(more100); break;    // filter 5

			default:                                            // PLAYLISTS GLOBAL SHORTCUTS
			{
				if (!shufflebtn->Enabled || Msg.wParam < 300 || mform->Active)   // GUARDS
					return;

				doubleKeyCatching = DoublePress::Global;

				timerDblPress->Enabled = true;
				if (Msg.wParam == lastPressedGlKey) {
					dblPressGlobal = true;
					lastPressedGlKey = 0;

					TMenuItem *menuItem = new TMenuItem(this);
					menuItem->Caption = plButtons[Msg.wParam -300 -35]->Caption;
					menuItem->Tag = 2; // playing song only

					 hiddenItemClick(menuItem);		// copy that to pressed pl
				}
				else {
					dblPressGlobal = false;
					lastPressedGlKey = Msg.wParam; // memorize this pl hotkey
				}
			}
		 }
	   }
	}


//	else if(Msg.message == 0)
//	{                                                // user runned another SCounter
//	   ShowMessage("message received!!!");
//	   if(!mform->Visible) trayClick(NULL); 		   // Trayed
//	   else if(!mform->Active) Application->Restore();
//	}
}
//---------------------------------------------------------------------------

void fixPlaylistHeight()
{
	TListView &playlist = *mform->playlist;
    playlist.Height = playlist.VisibleRowCount *17 +2;  // set perfect height

	int space = -1; // space between playlist and plLabel

	do
	{
		int decrIncrBy = 17;     // decrease size

		space = mform->plLabel->Top - (playlist.Top + playlist.Height);

		if(space < 0)
			playlist.Height -= decrIncrBy;
		else if(space > 17)
			playlist.Height += decrIncrBy;

	} while (space < 0 || space > 17);  // playlist collides with plLabel or playlist not high enough
}


void maximizeMform()                               // maximize mform
{
	mform->Height = Screen->WorkAreaHeight;    // mform high as possible
	mform->Top = 0;

	fixPlaylistHeight();

	mform->btnMaximize->Caption = L""; // minimize icon
	mform->btnMaximize->Hint = "Restore SCounter";
}

void restoreMform()
{
	mform->ClientHeight = 486;       // default restored mform height.
//	mform->playlist->Height = 7*17 +2;        // 17 WORKING

	fixPlaylistHeight();

	mform->btnMaximize->Caption = L""; // maximize icon
	mform->btnMaximize->Hint = "Maximize SCounter";
}

UnicodeString changeSlashes(UnicodeString str, char newSlashChar)
{                                         // pass '/' newSlashChar to get str with '/' only
										  // pass '\' newSlashChar to get str with '\' only
										  // pass '\\' newSlashChar to get str with '\\' only
	char oldSlashChar = '\\';
	if(newSlashChar == '\\') oldSlashChar = '/';

	UINT posInStr = str.Pos(oldSlashChar);
	while(posInStr)
	{
		str.Delete(posInStr, 1);                      // delete oldChar
		str.Insert(newSlashChar, posInStr);           // insert newChar
		posInStr = str.Pos(oldSlashChar);             // search next oldChar
	}

	return str;
}

void __fastcall Tmform::debugbtnClick(TObject *Sender)
{
//    af.copyToPlaylist(plButtons[0]->Caption, songs);



//	ShowMessage(ListView_GetExtendedListViewStyle(playlist->Handle));
//		ListView_SetExtendedListViewStyle(playlist->Handle,
//		ListView_GetExtendedListViewStyle(playlist->Handle) & !LVS_EX_INFOTIP); // disable hints
}
//---------------------------------------------------------------------------

void __fastcall Tmform::Show1Click(TObject *Sender)                        // tray [SHOW] click
{   restoreFromTray(); }
//---------------------------------------------------------------------------

// 000000000000000000000000000 PLAYLIST PANEL 0000  BEGIN  000000000000000000000000000000 //

void Tmform::addPlaylist(UnicodeString Caption)                // ADD NEW PLAYLIST
{
   pl.activePlaylist = pl.delNonPathChars(Caption);    // change active playlist

   int prevPlaylistInd;                                // SAVE prev playlist
   for(int i=0; i < plButtons.size(); i++)
	   if(plButtons[i]->Font->Color == clLime) prevPlaylistInd = i;
   pl.savePlaylist(plButtons[prevPlaylistInd]->Caption);

   clearbtnClick(clearbtn);                        	// CLEAR playlist
   plButtons[prevPlaylistInd]->Font->Color = clWhite;

   pl.savePlaylist(Caption);                           // SAVE new playlist

   TButton *tempBtn = new TButton(mform);    	  // create btn
   tempBtn->Parent = mform;
   tempBtn->Caption = Caption;      						  // caption
   tempBtn->Width = Canvas->TextWidth(tempBtn->Caption) + 7*2;// width

   tempBtn->Height = 22;                                      // pos
   tempBtn->Top = -200;
   tempBtn->Tag = -3;

   tempBtn->StyleElements = TStyleElements(seBorder);         // appearance
   tempBtn->Font->Name = "Microsoft Sans Serif";
   tempBtn->Font->Color = clLime;

   tempBtn->OnEnter = btnDefaultEnter;     				      // Inherit methods
   tempBtn->OnExit  = btnDefaultExit;
   tempBtn->PopupMenu = popRClick;
   tempBtn->OnMouseDown = btnDefaultMouseDown;

   if(Caption == "All songs")
		plButtons.insert(plButtons.begin(), tempBtn);
   else
		plButtons.push_back(tempBtn);                             // push to the vector

   rearrangePlButtons();                                     // and rearrange
}

// =================================================

void __fastcall Tmform::timerMoveTimer(TObject *Sender)         // turn the page
{
	short tagDirection = btnLeft->Tag;

	if(timerMove->Tag == 0)                                   // active page
	{
		int pages = 0, newPage = plButtons[0]->Left/(7 - panelMain->Width);
		if(tagDirection == 0)  newPage --;
		else                   newPage ++;
		for(int i=0; i < plButtons.size(); i++)     // count pages count
			if(plButtons[i]->Tag >= 0) pages ++;

													// LEFT & RIGTH GUARD
		if(tagDirection == 0 && newPage < 0
		|| tagDirection == 1 && newPage >= pages)
		{
			timerMove->Enabled = false;
			return;
		}

		changeColorOfArrows(tagDirection);         // color of arrows
	}


	switch(tagDirection)
	{
		case 0:                                               // prev page
			for(int i=0; i < plButtons.size(); i++) plButtons[i]->Left += 32;
		break;

		case 1:                                               // next page
			for(int i=0; i < plButtons.size(); i++) plButtons[i]->Left -= 32;
		break;
	}
	timerMove->Tag ++;

	if(timerMove->Tag == (panelMain->Width +16)/ 32)           // finish moving
	{
		if(tagDirection) for(int i=0; i < plButtons.size(); i++) plButtons[i]->Left += 16;
		else             for(int i=0; i < plButtons.size(); i++) plButtons[i]->Left -= 16;
		timerMove->Tag = 0;
		timerMove->Enabled = false;
	}
}

//---------------------------------------------------------------------------

void Tmform::changeColorOfArrows(short input)                                   // change colors of arow btns
{                                                             // number of pages
	short pages = 0;  for(int i=0; i < plButtons.size(); i++) if(plButtons[i]->Tag >= 0) pages ++;
	short currPage = plButtons[0]->Left/(7 - panelMain->Width); // currPage
	if(!input) currPage --;  else if(input == 1)  currPage ++;

    labelPlPage->Caption = "page: " + intToUS(currPage +1) + '/' + intToUS(pages);
													// LEFT arrow
	if(currPage == 0) btnLeft->Font->Color = clInactiveCaption;
	else              btnLeft->Font->Color = 0x001EC823;  // GREEN
													// RIGHT arrow
	if(currPage == pages-1 || pages == 1) btnRight->Font->Color = clInactiveCaption;
	else              				      btnRight->Font->Color = 0x001EC823;  // GREEN
}

void Tmform::renamePlaylist(UnicodeString Caption)       		 // RENAME PLAYLIST
{
    for(int i=shortcutsData.size() -1; i >= 0 ; i--)    // rename also shortcuts
		if(shortcutsData[i].functionName == plButtons[popRClick->Tag]->Caption)
			shortcutsData[i].functionName = Caption;

	UnicodeString oldName = pl.getSettingsPath();
				  oldName += "playlists/";
				  oldName += pl.delNonPathChars(plButtons[popRClick->Tag]->Caption);
	UnicodeString temp = oldName;
	UnicodeString newName = pl.getSettingsPath();
				  newName += "playlists/";
				  newName += pl.delNonPathChars(Caption);

	plButtons[popRClick->Tag]->Caption = Caption;
	plButtons[popRClick->Tag]->Width   = Canvas->TextWidth(Caption) + 7*2; // width
	rearrangePlButtons();
	pl.activePlaylist = pl.delNonPathChars(Caption);

	if(!MoveFileW(oldName.c_str(), newName.c_str()))          // rename folder!
		ShowMessage("Couldn't rename the folder with old playlist name!"
				   "\nError: " + intToUS(GetLastError()) );
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnDefaultEnter(TObject *Sender)             // on Getting focus
{
	for(int i=0; i < plButtons.size(); i++)             // no one playlist is focused?
	{
	  if(plButtons[i]->Focused())         // we don't leave plPanel
		 plButtons[i]->Font->Color = clLime;     // green
	  else plButtons[i]->Font->Color = clWhite;
	}
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnDefaultExit(TObject *Sender)              // on Loosing focus
{
   	TButton *tempBtn = dynamic_cast <TButton *> (Sender);

	tempBtn->Font->Color = clWhite;

	for(int i=0; i < plButtons.size(); i++)             // no one playlist is focused?
	{
	  if(plButtons[i]->Focused())         // we don't leave plPanel
		 break;

	  if( i == plButtons.size() -1) tempBtn->Font->Color = clLime;  // "save focus"
	}
}
//---------------------------------------------------------------------------
void Tmform::rearrangePlButtons()                              // change positions of buttons
{
   TButton *currBtn, *prevBtn;     // currBtn->Tag 0,1,2,3,4 etc. for "pages". -1, -2 = rows
   short space = 7;       // space between panel border and button

   std::vector <TButton*> plButtonsCopy;        // COPIES OF PLBUTTONS
   for(UINT i=0; i < plButtons.size(); i++)
   {
	   TButton *tempBtn = new TButton(mform);
       tempBtn->Parent = NULL;
	   tempBtn->Tag  = plButtons[i]->Tag;
	   tempBtn->Left = plButtons[i]->Left;
	   tempBtn->Top  = plButtons[i]->Top;
	   tempBtn->Width  = plButtons[i]->Width;
	   tempBtn->Height = plButtons[i]->Height;
	   tempBtn->Font->Color = plButtons[i]->Font->Color;
	   plButtonsCopy.push_back(tempBtn);
   }

   for(int i=0; i < plButtonsCopy.size(); i++)     // reset all Buttons
   {
	  plButtonsCopy[i]->Left = space;
	  plButtonsCopy[i]->Tag = -3;
	  plButtonsCopy[i]->Top = space;
   }
   plButtonsCopy[0]->Tag = 0;


   for(int i=1, pages=1; i < plButtonsCopy.size(); i++)
   {
		prevBtn = plButtonsCopy[i-1]; currBtn = plButtonsCopy[i];
																   // it fits in 1st row
		if( (prevBtn->Tag >= 0  ||  prevBtn->Tag == -1)
		 && prevBtn->Left + prevBtn->Width + space*2 + currBtn->Width < panelMain->Width*pages)
		{
			currBtn->Left = prevBtn->Left + prevBtn->Width + space;
			currBtn->Top  = prevBtn->Top;
			currBtn->Tag = -1;  // we now in 1st row
		}
		else if(prevBtn->Tag == -1                                        // it fits in 2nd row
		|| prevBtn->Left + prevBtn->Width + space*2 + currBtn->Width < panelMain->Width*pages)
		{
			if(prevBtn->Tag == -1) currBtn->Left = panelMain->Width*(pages-1) + space;
			else currBtn->Left = prevBtn->Left + prevBtn->Width + space;

			currBtn->Top = space + currBtn->Height + space;     // put down (2nd row)
			currBtn->Tag = -2;  // we now in 2nd row
		}
		else
		{                               // new page
			currBtn->Tag  = pages;
			currBtn->Left = space + panelMain->Width*pages;
			currBtn->Top  = space;

			pages ++;
		}
   }

   short currPage = 0;
   for(UINT i=0; i < plButtonsCopy.size(); i++)                               // find currPage (currPage is page
   {                                                                      // with active playlist)
	   if(plButtonsCopy[i]->Font->Color == clLime)   // find active pl
	   {
		   currPage = plButtonsCopy[i]->Left/panelMain->Width;
		   break;
	   }
   }

   for(int i=0; i < plButtonsCopy.size(); i++)                       // turn back to active page
		plButtonsCopy[i]->Left -= panelMain->Width * currPage;

   for(int i=plButtonsCopy.size() -1; i > -1; i--)              // APPLY CHANGES TO ORIGINAL BTNS
   {
	   plButtons[i]->Tag  = plButtonsCopy[i]->Tag;
	   plButtons[i]->Left = plButtonsCopy[i]->Left;
	   plButtons[i]->Top  = plButtonsCopy[i]->Top;
	   plButtons[i]->Width  = plButtonsCopy[i]->Width;
	   plButtons[i]->Height = plButtonsCopy[i]->Height;
	   plButtons[i]->Font->Color = plButtonsCopy[i]->Font->Color;
	   if(plButtons[i]->Parent == mform)        // for adding playlists only
            plButtons[i]->Parent = panelMain;
	   delete plButtonsCopy[i];
   }

   plLabel->Caption = pl.getActivePlFullName();

   changeColorOfArrows(-1);  // usual case
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnDefaultMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)
{
	TButton *tempBtn = dynamic_cast <TButton*> (Sender);   // which playlist btn
	unsigned short index;                                  // was clicked ?
	for(int i=0; i < plButtons.size(); i++)
		if(tempBtn->Caption == plButtons[i]->Caption)
			  index = i;
//LMOUSE
	if(Button == TMouseButton::mbLeft)                  // playlist Click!!!
	{
		if(!editQSearch->Focused() && mform->Active)
			 playlist->SetFocus();
													   // GUARD (when this pl already active)
		if(pl.activePlaylist == pl.delNonPathChars(tempBtn->Caption))
			 return;
		else                                           // change ALL plButtons colors
		{
			for(UINT i=0; i < plButtons.size(); i++)
				plButtons[i]->Font->Color = clWhite;
            tempBtn->Font->Color = clLime;
        }

		pl.savePlaylist(pl.activePlaylist);  // save prev. PL
		pl.activePlaylist = pl.delNonPathChars(tempBtn->Caption);
		pl.loadPlaylist(pl.activePlaylist);  // load clicked PL

		if(!shufflebtn->Tag)					       // when shuffe is OFF
			pl.filter(filterbtn->Tag);
		else							 		       // when shuffe is ON
			pl.shuffleClick(true);

		pl.refreshSongCounter("comm");
		pl.makeVisibleCurrSong(3,0);

		if(tempBtn->Left < 0 || tempBtn->Left > panelMain->Width - 30)        // turn to that page
			rearrangePlButtons();
	}


//RMOUSE
	if(Button == TMouseButton::mbRight)
	{
		if(tempBtn->Caption == "All songs")            // it's "all songs" playlist, disable popup.
			tempBtn->PopupMenu = NULL;

		popRClick->Tag = index;                        // make tag to know which pl was clicked
	}
}
//---------------------------------------------------------------------------

				//       BUTTONS
void __fastcall Tmform::btnAddClick(TObject *Sender)         // [ADD NEW PLST]
{   formPlaylists->Caption = "Adding new playlist";
	formPlaylists->editName->Text = ""; myShowModal(formPlaylists);   }
//---------------------------------------------------------------------------
void __fastcall Tmform::Rename1Click(TObject *Sender)                 // [RENAME popup]
{
	formPlaylists->Caption = "Renaming playlist";
	formPlaylists->editName->Text = plButtons[popRClick->Tag]->Caption;
    renaming = true;
	myShowModal(formPlaylists);
}
//---------------------------------------------------------------------------
void __fastcall Tmform::Delete1Click(TObject *Sender)                 // DELETE PLAYLIST
{                                                                     // [DELETE popup]
	if(plButtons.size() == 1)                                  // don't delete last playlist
	{  ShowMessage("SCounter should have at least 1 playlist.");  return;  }

	UnicodeString deletingPlName = plButtons[popRClick->Tag]->Caption;  // used for deleting shortcut from vec
	bool actPlDeleting = 0;                                   // are we deleting active playlist?
	if(plButtons[popRClick->Tag]->Font->Color == clLime)
		actPlDeleting = 1;

	UnicodeString path = pl.getSettingsPath();            // deleting folder
				  path += "playlists/";
				  path += pl.delNonPathChars(plButtons[popRClick->Tag]->Caption);
                  path += '\0';

	SHFILEOPSTRUCT f = {0, FO_DELETE, path.c_str(), 0 ,
		FOF_NOCONFIRMATION|FOF_SIMPLEPROGRESS, false, 0, 0};
	if(SHFileOperation(&f))                                     // try, if fails - show error
		ShowMessage("Can't delete folder with this playlist!");

	delete plButtons[popRClick->Tag];                        // and delete it from vector
	plButtons.erase(plButtons.begin() + popRClick->Tag);


    	    				// activate previous pl
	if(actPlDeleting)
	{
		UINT newIndex;
		if(popRClick->Tag -1 >= 0) newIndex = popRClick->Tag -1;
		else                       newIndex = 0;

		plButtons[newIndex]->Font->Color = clLime;
        pl.activePlaylist = pl.delNonPathChars(plButtons[newIndex]->Caption);
		pl.loadPlaylist(pl.activePlaylist); 		   // load prev PL

        if(!shufflebtn->Tag)					       // when shuffe is OFF
			pl.filter(filterbtn->Tag);
		else							 		       // when shuffe is ON
			pl.shuffleClick(true);

		pl.makeVisibleCurrSong(3,0);
	}

	for(int i=shortcutsData.size() -1; i >= 0 ; i--)    // delete also shortcuts
		if(shortcutsData[i].functionName == deletingPlName)
			shortcutsData.erase(shortcutsData.begin() + i);

	for(UINT i=0, row = 35; i < plButtons.size(); i++, row++)     // this is awful code
		for(UINT j=0; j < shortcutsData.size(); j++)
		{
			if(plButtons[i]->Caption == shortcutsData[j].functionName)
                shortcutsData[j].row = row;
        }

	rearrangePlButtons();
}
//---------------------------------------------------------------------------
															 // CHANGE TAG FOR TURNING PAGES
void __fastcall Tmform::btnRightClick(TObject *Sender)
{
   if(timerMove->Enabled) return;
   btnLeft->Tag = 1; 			// tag for next page
   timerMove->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnLeftClick(TObject *Sender)
{
	if(timerMove->Enabled) return;
	btnLeft->Tag = 0;           // tag for next page
	timerMove->Enabled = true;
}
//---------------------------------------------------------------------------



// 000000000000000000000000000 PLAYLIST PANEL 0000  END  00000000000000000000000000000000 //



void Tmform::refreshPlItemPopMenu()
{											  // enable /disable open folder item
   if(playlist->SelCount == 1)  popPlItem->Items->Items[2]->Enabled = true ;
   else 						popPlItem->Items->Items[2]->Enabled = false ;

   popPlItem->Items->Items[0]->Clear();                              // load playlists to popUp for songs
   for(int i=0; i < plButtons.size(); i++)
   {
		if(pl.activePlaylist == pl.delNonPathChars(plButtons[i]->Caption)) continue;  // don't add active Pl

		TMenuItem *tempMItem = new TMenuItem (this);
		tempMItem->Caption = plButtons[i]->Caption;      // pl name
		tempMItem->OnClick = hiddenItemClick;            // on click

		popPlItem->Items->Items[0]->Add(tempMItem);
   }
}


void __fastcall Tmform::hiddenItemClick(TObject *Sender)               // on pl Item popup click
{                                                                      // COPY SONGS TO PLAYLIST
     pl.savePlaylist(pl.activePlaylist);       // save active playlist if copying to actPlaylist

	 TMenuItem *menuItem = dynamic_cast <TMenuItem*> (Sender);
	 std::vector <Songs> tempSongs;           // it'll be send to copyToPlaylist()
	 Songs singleSong("1", 0);                                                    // our SINGLE song
	 if(menuItem->Tag == 2) {
		if(pl.stopped)        // GUARD (if tag == 2 and no playing song)
			return;
		else
			singleSong = pl.existsInScounter(playingSongInfo->Items->Strings[1]
						, playingSongInfo->Items->Strings[2]);
     }

	 if(menuItem->Tag == 2 && singleSong.path == "0")     // SINGLE song NOT FOUND IN SCOUNTER!
	 {
		messageForm->showMessage("Song not found in SCounter!", 'e', 2);
		return;
	 }
// BAD CODE 2.1.1
//	 else if(menuItem->Tag == 2                           // SINGLE song already exists in dest pl
//	 && pl.existsInPlaylist("bool", -1, singleSong.artist, singleSong.title, menuItem->Caption))
//	 {  messageForm->playBeep('w'); return; }


	 if(menuItem->Tag == 2)  {
// BAD CODE 2.1.1
//		 if(pl.activePlaylist == pl.delNonPathChars(menuItem->Caption)) {
//			pl.addToSongs(singleSong);        // is it curr playlist?
//			messageForm->playBeep('i');       // add to "songs" instead
//			return;
//		 }

		 tempSongs.push_back(singleSong);
	 }
	 else {							 // if called by RClick in "playlist"
		 std::vector <UINT> selectedSongs = af.getSelectedSongs();  // get sel.indexes
		 for(int i = 0; i < selectedSongs.size(); i++) tempSongs.push_back(songs[selectedSongs[i]]);
	 }


	 af.copyToPlaylist(menuItem->Caption, tempSongs);
	 delete menuItem;
}
//---------------------------------------------------------------------------





void __fastcall Tmform::timerDblClickTimer(TObject *Sender)           // TRAY CLICK is that DBL click?
{                                    // 0 = single-click,  1 = double click
	if(!timerDblClick->Tag) pl.playPauseResume();

	timerDblClick->Tag = 0;
	timerDblClick->Enabled = false;
}
//---------------------------------------------------------------------------



void Tmform::disableMainForm(bool strongDisabling)                 // disable form
{
	panelMain->Enabled = !panelMain->Enabled;       // second phase (loading data 60 secs)
	disableBtnPanel(btnDelPlayingSong);  // disable it (and it enables once adding files is done)


	shufflebtn->Enabled = !shufflebtn->Enabled;
	filterbtn->Enabled = !filterbtn->Enabled;
	dbs->Enabled = !dbs->Enabled;
	btnSortPlBy->Enabled = !btnSortPlBy->Enabled;
	findDoublesbtn->Enabled = !findDoublesbtn->Enabled;
	settings->Enabled = settings->Enabled;
    togCheckingMode->Enabled = !togCheckingMode->Enabled;
	clearbtn->Enabled = !clearbtn->Enabled;
	if(!shufflebtn->Tag)
		btnSortPlBy->Enabled = !btnSortPlBy->Enabled;

	if(strongDisabling){                     // first phase (addings paths 3 secs)
		mform->Tag = !mform->Tag;            // don't allow user to close the program
		playlist->Enabled = !playlist->Enabled;
		editQSearch->Enabled = !editQSearch->Enabled;
	}
}

//---------------------------------------------------------------------------


void __fastcall Tmform::delayTimerTimer(TObject *Sender)  // this timer using for loading playlist 2nd time.
{
	delayTimer->Enabled = false;       // disable this timer immediately, so this method won't happen twice

	//================================ 		// if user closed SCounter before all data was loaded
	if(af.firstLoadingInd != -1) af.loadAddedSongsData(af.firstLoadingInd);
	else {

		af.shuffleAddFiles(songs.size());
		if(shufflebtn->Tag) pl.sortSongs(2 + filterbtn->Tag, false); 		   // if shuffle was ON
								// filter all songs again, from 0
		pl.filter(filterbtn->Tag);

		if(!programIsCreating) pl.makeVisibleCurrSong(3,0);
        else { programIsCreating = false; pl.loadLastSelSong(); } // load LAST selectedSong


	//================================   // check whether all songs exists in AllSongs
//		  std::vector <UnicodeString> tempVect;               // indexation of AllSongs.txt data
//		  for(UINT i=0; i < af.dataAllSongs->Count; i+=4)
//			tempVect.push_back(af.dataAllSongs->Strings[i].LowerCase());     // artists
//		  af.Indexation(tempVect, af.artAS, af.artIndexesAS);
	// NEW CODE
		AllSong::makeIndexation(LetterCase::Lower);


		std::vector <Songs*> notFoundSongs;
		pl.saveSelection();
		for(UINT i=0; i < songs.size(); i++)
			if(!pl.existsInAllSongs("bool", true, songs[i].artist, songs[i].title)
			  && !songs[i].IHaveBadTags(songs[i].name, songs[i].artist, songs[i].title))   // song not found in
			{                                                                               // allSongs
				if(!notFoundSongs.size())
					playlist->Selected = false;

				notFoundSongs.push_back(&songs[i]);
				playlist->Items->Item[i]->Selected = true;
            }

		if(notFoundSongs.size())
		{
			currReason = NotFoundInAllSongs;  // set tag and show form with bad songs.
            formBadSongs->receiveFiles(notFoundSongs);
			myShowModal(formBadSongs);
        }

	 //================================     // refresh listenings of loading pl
		pl.refreshListeningsForLoadingPl();
	}
}
//---------------------------------------------------------------------------




void __fastcall Tmform::btnCopyPlSongClick(TObject *Sender)     // [COPY PLAYING SONG TO AN.PL]
{
	TPopupMenu *tempPop = new TPopupMenu(mform);        // MAKE POPUP WITH PLAYLISTS
	tempPop->AutoHotkeys = maManual;

	for(int i=0; i < plButtons.size(); i++)        	// load playlists to popUp for songs
	{
		TMenuItem *tempMItem = new TMenuItem (this);
		tempMItem->Caption = plButtons[i]->Caption;      // pl name
		tempMItem->Tag = 2;                              // tag (2 means it will be triggered by btnCPS)
		tempMItem->OnClick = hiddenItemClick;            // on click

		tempPop->Items->Add(tempMItem);
	}
													  // pop up!
	POINT point;  (GetCursorPos(&point));
	tempPop->Popup(point.x, point.y);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::timerSwimArtTimer(TObject *Sender)
{
  TPanel *Panel = artistPan;               // define your panel
  TLabel *Text = artist;              // define your text
    TTimer *TTimer = timerSwimArt;

	int textRight = Text->Left + Text->Width;
	int step = 1;
	int waitingTime = 70;
	int space = 5;
						                                 // GUARD
	if(Text->Width + space*2 <= Panel->Width)            // it fits.
	{
		Text->Left = 0;
		TTimer->Enabled = false;
		return;
	}
	else if(Text->Tag == 2000)  Text->Tag = waitingTime;   // "on create timer"


	if(Text->Tag == 0 && textRight > Panel->Width - space)  // move left
	  step *= -1;
	else if(Text->Tag < waitingTime && Text->Left < 0)      // wait
	{ Text->Tag ++; return; }
	else if(Text->Left <= 0)                                // move right
	  step *= 1;
	else                                                    // wait
	{ Text->Tag --; return; }

	Text->Left += step;           // move caption!
	Text->Refresh();
}
//---------------------------------------------------------------------------

void __fastcall Tmform::timerSwimTitTimer(TObject *Sender)
{
  TPanel *Panel = titlePan;               // define your panel
  TLabel *Text = title;              // define your text
  TTimer *TTimer = timerSwimTit;

	int textRight = Text->Left + Text->Width;
	int step = 1;
	int waitingTime = 70;
	int space = 5;
														 // GUARD
	if(Text->Width + space*2 <= Panel->Width)            // it fits.
	{
		Text->Left = 0;
		TTimer->Enabled = false;
		return;
	}
	else if(Text->Tag == 2000)  Text->Tag = waitingTime;   // "on create timer"


	if(Text->Tag == 0 && textRight > Panel->Width - space)  // move left
	  step *= -1;
	else if(Text->Tag < waitingTime && Text->Left < 0)      // wait
	{ Text->Tag ++; return; }
	else if(Text->Left <= 0)                                // move right
	  step *= 1;
	else                                                    // wait
	{ Text->Tag --; return; }

	Text->Left += step;           // move caption!
	Text->Refresh();
}
//---------------------------------------------------------------------------


void __fastcall Tmform::timerSwimAlbTimer(TObject *Sender)
{
  TPanel *Panel = panelAlbum;               // define your panel
  TLabel *Text = album;              // define your text
  TTimer *TTimer = timerSwimAlb;

	int textRight = Text->Left + Text->Width;
	int step = 1;
	int waitingTime = 70;
	int space = 5;
														 // GUARD
	if(Text->Width + space*2 <= Panel->Width)            // it fits.
	{
		TTimer->Enabled = false;
		return;
	}
	else if(Text->Tag == 2000) Text->Tag = waitingTime;   // "on create timer"


	if(Text->Tag == 0 && textRight > Panel->Width - space*2)// move left
	  step *= -1;
	else if(Text->Tag < waitingTime && Text->Left < 0)      // wait
	{ Text->Tag ++; return; }
	else if(Text->Left <= space)                            // move right
	  step *= 1;
	else                                                    // wait
	{ Text->Tag --; return; }

	Text->Left += step;           // move caption!
	Text->Refresh();
}
//---------------------------------------------------------------------------



void __fastcall Tmform::Refresh1Click(TObject *Sender)           // [REFRESH DUR]
{
	std::vector <UINT> selectedSongs = af.getSelectedSongs();
	if (!selectedSongs.size() ) return;

	for(int i=0; i < selectedSongs.size(); i++)
	{
		HSTREAM tempStream = BASS_StreamCreateFile(false, songs[selectedSongs[i]].path.c_str(), 0,0,0);
		songs[selectedSongs[i]].duration = songs[0].getDuration(tempStream, 1);
		BASS_StreamFree(tempStream);
	}
	playlist->UpdateItems(selectedSongs[0], selectedSongs[selectedSongs.size()-1]);
}
//---------------------------------------------------------------------------



void __fastcall Tmform::playlistMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{                                                                     // was it clicked on empty space?
	 if(Y/16 <= playlist->Items->Count)  emptySpaceClicked = false;
	 else { emptySpaceClicked = true; pl.selectedSong = -1; }
                                                                     // pop up plItem
	 if(Button == TMouseButton::mbRight && !emptySpaceClicked        // (only if mform is "Enabled")
	   && shufflebtn->Enabled)
		playlist->PopupMenu->Popup(X+Left+playlist->Left, Y+Top+playlist->Top +(mform->Height-ClientHeight));

														 // [FOCUS] Middle button click
	 if(Button == TMouseButton::mbMiddle && Shift.Contains(ssShift))
	 {
		WCHAR *currState = focusbtn->Caption.SubString1(1,1).c_str();

		switch (*currState)
		{   				//   P, S, SCS
			case L'':   playingItemClick(selectedItem);           break;        // playing song
			case L'':   playingItemClick(scsItem);          	   break;        // selected song
			case L'':   playingItemClick(playingItem);            break;        // SCS
		}
     }
	 else if(Button == TMouseButton::mbMiddle &&  !Shift.Contains(ssAlt) && !Shift.Contains(ssCtrl))
		focusbtnMouseDown(NULL,0,TShiftState(0),0,0);
}
//---------------------------------------------------------------------------


void __fastcall Tmform::togCheckingModeClick(TObject *Sender)     // [TOGGLE CHECHING MODE]
{
	playlist->SetFocus();	// don't focus me (pl btn)!
}
//---------------------------------------------------------------------------




void __fastcall Tmform::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{                                                                         // [FOCUS] Middle button click
	if(Button == TMouseButton::mbMiddle && Shift.Contains(ssShift))
	{
        WCHAR *currState = focusbtn->Caption.SubString1(1,1).c_str();

		switch (*currState)
		{   				//   P, S, SCS
			case L'':   playingItemClick(selectedItem);           break;        // playing song
			case L'':   playingItemClick(scsItem);          	   break;        // selected song
			case L'':   playingItemClick(playingItem);            break;        // SCS
		}
	}
	else if(Button == TMouseButton::mbMiddle && !Shift.Contains(ssAlt) && !Shift.Contains(ssCtrl))
		focusbtnMouseDown(NULL,0,TShiftState(0),0,0);
}
//---------------------------------------------------------------------------



void __fastcall Tmform::btnSortPlByClick(TObject *Sender)
{
	pmenuSortBy->Popup(Left + btnSortPlBy->Left +8
	, Top + btnSortPlBy->Top -pmenuSortBy->Items->Count*8  +4);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::asdf1Click(TObject *Sender)
{
	matches.clear();        // don't mess with f3 search
														  // define which popupItem was clicked
	TMenuItem *popupItem = dynamic_cast <TMenuItem *> (Sender);
    short sortBy;
	if(popupItem->Caption == "name") sortBy = -2;
	else              			     sortBy = -1;

	pl.saveSelection();  int tempIndex = pl.actPlayingSong;

	pl.sortSongs(sortBy, 1);                         // sort by path | name
	for(int i=0; i < songs.size(); i++) songs[i].indexNorm = i;     // fix indexes norm
	pl.filter(filterbtn->Tag);                   // and filter this

	pl.restoreSelection();                                   // saving actPlayingSong
	pl.actPlayingSong = tempIndex; pl.playingSong = tempIndex; playlist->UpdateItems(tempIndex,tempIndex);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnMaximizeClick(TObject *Sender)
{
	if(!maximized) maximizeMform();
	else restoreMform();  // default height for normal state

	maximized = !maximized;
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnMinusClick(TObject *Sender)
{
	playlist->Height --;
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnPlusClick(TObject *Sender)
{
	playlist->Height ++;
}
//---------------------------------------------------------------------------




void __fastcall Tmform::btnMaximizeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,int X, int Y)
{
	TPanel *clickedPanel = dynamic_cast <TPanel*> (Sender);
	clickedPanel->BevelOuter = bvLowered;
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnMaximizeMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	 TPanel *clickedPanel = dynamic_cast <TPanel*> (Sender);
	 clickedPanel->BevelOuter = bvRaised;
}
//---------------------------------------------------------------------------

void __fastcall Tmform::cboxAutoNextClick(TObject *Sender)
{
	if(cboxAutoNext->Font->Color == 0x001EC823)   cboxAutoNext->Font->Color = 0x00DCDCDC;
	else   cboxAutoNext->Font->Color = 0x001EC823;
}
//---------------------------------------------------------------------------


void __fastcall Tmform::btnDelPlayingSongClick(TObject *Sender)
{
 	if(pl.stopped)         // GUARD
		return;

/* TODO -cbug :
How do i restore playing song index after playing song became inactive?
- answer: make a pointer to the song that should be next. If something happens
to it, just go from 0 and don't care.
*/
	int indexInPl = pl.existsInPlaylist("int", false, -1
	 , *&(playingSongInfo->Items->Strings[1]), *&(playingSongInfo->Items->Strings[2]));
																					  // GUARD
	if(indexInPl == -1) {
		ShowMessage("Playing song doesn't exist in current playlist");
		return;
	}

	// from this line SONG WILL BE DELETED
	pl.saveSelection();     // save selection  (because when song becomes inactive it uses
							// selection trick)
	bool songWasInactive = indexInPl >= pl.firstInactive;

	playlist->ClearSelection();
	playlist->ItemIndex = indexInPl;   // select playing song
	af.deleteFiles();                  // delete it.

	if(songWasInactive) {     // PLAYING SONG was inactive
		pl.restoreSelection();   // restore selection  (because when song becomes inactive it uses
								 // selection trick)
    	playlist->Selected->MakeVisible(true);
	}
}
//---------------------------------------------------------------------------

void Tmform::enableBtnPanel(TPanel *panel)               // enable / disable panels (that look like btns)
{
	panel->Enabled = true;
	panel->Font->Color = 0x00DCDCDC;    // white
    panel->BevelOuter = bvRaised;
}


void Tmform::disableBtnPanel(TPanel *panel)
{
	panel->Enabled = false;
	panel->Font->Color = clInactiveCaption;   // gray
	panel->BevelOuter = bvLowered;
}


void __fastcall Tmform::playingItemClick(TObject *Sender)
{
	TMenuItem *clickedItem = dynamic_cast <TMenuItem*> (Sender);

	for(UINT i=0; i < 3; i++)               // enable and uncheck all items
	{
		pmenuFocusBtn->Items->Items[i]->Checked = false;
		pmenuFocusBtn->Items->Items[i]->Enabled = true;
	}
    clickedItem->Checked = true;        // disable clicked item
	clickedItem->Enabled = false;

	currFocusBtnState = clickedItem->Tag;   // change state

	switch (currFocusBtnState)                  // change state of current state focusbtn
	{
		 case FocusPlaying:
		 {                                              //   P, S, SCS
			  focusbtn->Caption = L'';
			  focusbtn->Hint = "Show playing song";
		 }
		 break;

		 case FocusSelected:
		 {
			  focusbtn->Caption = L'';
			  focusbtn->Hint = "Show selected song";
		 }
		 break;

		 case FocusSCS:
		 {
			  focusbtn->Caption = L'';
			  focusbtn->Hint = "Show shuffle current song";
		 }
		 break;
	}
}
//---------------------------------------------------------------------------

UnicodeString intToUS(const int &integer)  	// conver int to US string
{
	return UnicodeString(integer);
}

void __fastcall Tmform::btnOpenDirPlSongClick(TObject *Sender)
{                                                            // find playing song in scounter
	if(pl.stopped)         // GUARD
		return;

	Songs playingSong = pl.existsInScounter(playingSongInfo->Items->Strings[1], playingSongInfo->Items->Strings[2]);
	if(playingSong.path == "0") {               	// song not found!
		ShowMessage("This song not found in SCounter!");
		return;
	}

	openFolderSelect(playingSong.path);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::timerAutoSaveTimer(TObject *Sender)    // makes back every N minutes
{
	 if(setForm->Tag == FileDialogState::Showing)     // don't save, if user chooses folder
		return;

//	 static int a = 0;
//	 a++;
//     mform->Caption = a;

	 const maxBackupCount = 5;

     pl.saveEverything();

	 SrcFolder sourceSet(pl.getSettingsPath() + "settings");   // create objects
	 SrcFolder sourcePl(pl.getSettingsPath() + "playlists");

	 UnicodeString backupPath = pl.getSettingsPath() + "BACKUP";
	 if(!DirectoryExists(backupPath))
		 CreateDir(backupPath);

	 std::vector <UnicodeString> dirNames = SrcFolder::getFileNames(backupPath);
	 int dirCount = dirNames.size();   // how many dirs already exist in BACKUP dir?
     int tempInt = dirCount;
	 if(dirCount == maxBackupCount) tempInt = maxBackupCount -1;
	 backupPath += "\\" + intToUS(0) + intToUS(tempInt +1)       // name of new folder
			+ "__" + Date().FormatString("dd.mm.yyyy") + "__" + findRepChar(Time(), ':', '.');
			
	 if(dirCount < 5) {
		CreateDir(backupPath);
	 }
	 else if(dirNames[0].Pos("01__") || dirNames[0].Pos("02__") || dirNames[0].Pos("03__")) {
//		lb->Items->Add(dirNames[0]);   DEBUG ONLY
		dirNames[0] += '\0';
		SHFILEOPSTRUCT f = {0, FO_DELETE, dirNames[0].c_str(), 0 ,
		FOF_NOCONFIRMATION|FOF_SIMPLEPROGRESS, false, 0, 0};
        dirNames.erase(dirNames.begin());
		if(SHFileOperation(&f)) {                   // error!
		   ShowMessage("Can't delete folder!\n" + dirNames[0]);
		   return;
		}

		for(UINT i=0; i < dirNames.size(); i++)       // fix folder names
		{
			UnicodeString dirName = ExtractFileName(dirNames[i]);
			UnicodeString newName = ExtractFileDir(dirNames[i]);
			dirName.Delete1(2,1);
			dirName.Insert1(intToUS(i+1), 2);
			newName = newName + '\\' + dirName;

			MoveFileW(dirNames[i].c_str(), newName.c_str());          // rename folder!
        }
		
		CreateDir(backupPath);
	 }
	 else {
        ShowMessage("Can't delete folder because its name wrong: \n" + dirNames[0]);
		   return;
     }

	 sourceSet.copyToAnotherFolder(backupPath);
	 sourcePl.copyToAnotherFolder(backupPath);

	 if(setForm->Visible)                      // if user is watching "backup" page in settings
          setForm->refreshBackupList();		   // refresh all avaible backups
}
//---------------------------------------------------------------------------


void __fastcall Tmform::btnInvisibleClick(TObject *Sender)
{
    timerAutoSaveTimer(NULL);
}
//---------------------------------------------------------------------------

void __fastcall Tmform::copyToPlClick(TObject *Sender)
{
    // see "hidden item click"
}
//---------------------------------------------------------------------------

void __fastcall Tmform::btnAddArtistsClick(TObject *Sender)
{
    pl.savePlaylist(pl.activePlaylist);  // save to file, so formArtist can load it for indexation
	myShowModal(formArtists);
}
//---------------------------------------------------------------------------


void __fastcall Tmform::playlistKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
//	 if(Key == vkLWin)                         //
//        lWinPressing = false;
}
//---------------------------------------------------------------------------

void __fastcall Tmform::timerDblPressTimer(TObject *Sender)
{
	switch (doubleKeyCatching)
	{
		case DoublePress::Local:
		{
			if(!dblPress)                   // it is SINGLE-press!
			{
				for(UINT i=0; i < shortcutsData.size(); i++)    // find playlist for this KEY
				{
					if(shortcutsData[i].vkCode == lastPressedKey) {
						for(UINT j=0;  j < plButtons.size(); j++)
							if (shortcutsData[i].functionName == plButtons[j]->Caption)    // switch playlist
								btnDefaultMouseDown(plButtons[j], TMouseButton::mbLeft, TShiftState(), 0, 0);
					}
				}
			}

			lastPressedKey = 0;     // forget about key, that was pressed more than 1 sec before
		}
		break;

		case DoublePress::Global:
		{
			if(!dblPressGlobal)                   // it is SINGLE-press!
			{
				for(UINT i=0; i < shortcutsData.size(); i++)    // find playlist for this KEY
				{
					if(shortcutsData[i].row == lastPressedGlKey -300) {
						for(UINT j=0;  j < plButtons.size(); j++)
							if (shortcutsData[i].functionName == plButtons[j]->Caption) // switch playlist
								btnDefaultMouseDown(plButtons[j], TMouseButton::mbLeft, TShiftState(), 0, 0);
					}
				}
			}


			lastPressedGlKey = 0;     // forget about key, that was pressed more than 1 sec before
		}
		break;
	}

    dblPress = false;  					// reset dblPress value
	timerDblPress->Enabled = false; 	// disable timer
}
//---------------------------------------------------------------------------


bool Tmform::containsOnly (const TShiftState &state, ModifierKey key1, ModifierKey key2)
{                                                 // returns true if only Ctrl was pressed e.g.
	bool pressedModKeys[4];

	if (state.Contains(ssCtrl)) pressedModKeys[ModifierKey::Ctrl] = true;
	else                        pressedModKeys[ModifierKey::Ctrl] = false;

	if (state.Contains(ssShift)) pressedModKeys[ModifierKey::Shift] = true;
	else                         pressedModKeys[ModifierKey::Shift] = false;

	if (state.Contains(ssAlt))  pressedModKeys[ModifierKey::Alt] = true;
	else                        pressedModKeys[ModifierKey::Alt] = false;

	if (lWinPressing)  		pressedModKeys[ModifierKey::Win] = true;
	else                        pressedModKeys[ModifierKey::Win] = false;

	bool requiredKeys[4] = {0,0,0,0};
	for(int i=0; i < 4; i++)
		if(i == key1 || i == key2)
			requiredKeys[i] = true;


	bool pressedKeysCorrect = true;
	for(int i=0; i < 4; i++)
		if(requiredKeys[i] != pressedModKeys[i])
            pressedKeysCorrect = false;

	return pressedKeysCorrect;                // required keys pressed correctly
}
void __fastcall Tmform::FormDestroy(TObject *Sender)
{
	 Shortcut::unregisterAllShortcuts(); 	// unregister all shortcuts
}
//---------------------------------------------------------------------------

void __fastcall Tmform::Opensourcefolder1Click(TObject *Sender)
{
	openFolderSelect(songs[pl.selectedSong].path);
}
//---------------------------------------------------------------------------


bool Tmform::songNameShouldBeHidden(const int &indexInSongs)   // should song has name " ------ "?
{           // read CAREFULLY THIS CONDITION, IT'S A LITTLE BIT TRICKY

										// if one condition is true, we can show the title
   if(!shufflebtn->Tag          			// shuffle is OFF
	 || indexInSongs <= pl.shuffleCurrSong[filterbtn->Tag] -1 // song is lower than SCS
	 || indexInSongs >= pl.firstInactive    // song is INACTIVE
	 || (!pl.stopped && songs[indexInSongs].name == playingSongInfo->Items->Strings[0])) // song is PLAYING
		return false;       //   "Muse - Uno"

                                      // program will get here only if shuffle is ON!!!
   if(indexInSongs == pl.selectedSong)     //  F3 SEARCH
   {
	   for(UINT i=0; i < matches.size(); i++)
		   if(indexInSongs == matches[i])
			   return false;    //   "Muse - Uno"
   }


	return true;        // "--------------------"   hide title of the song.
}


void __fastcall Tmform::ApplicationEvents1Deactivate(TObject *Sender)
{
	lWinPressing = false;             // SCounter loses focus. So, user used hot-key with "lwin"
}
//---------------------------------------------------------------------------





void myShowModal(TForm *form)                     // shows form modal
{
//	mform->OnDeactivate(NULL);
	form->ShowModal();
}
void __fastcall Tmform::FormDeactivate(TObject *Sender)
{
    lWinPressing = false;       // lwin key
}
//---------------------------------------------------------------------------

void __fastcall Tmform::timerLWinStateTimer(TObject *Sender)
{
	 if(GetKeyState(vkLWin) < 0)
		 lWinPressing = true;
	 else
		 lWinPressing = false;
}
//---------------------------------------------------------------------------










void __fastcall Tmform::plLabelClick(TObject *Sender)
{
	rearrangePlButtons();
}
//---------------------------------------------------------------------------


bool PlayingSong::songIsPlaying()
{
	if(BASS_ChannelIsActive(pl.stream) == BASS_ACTIVE_STOPPED) // channel is STOPPED / INACTIVE
		return false;
	else
        return true;
}

UnicodeString PlayingSong::name()
{
	return mform->playingSongInfo->Items->Strings[0];
}

UnicodeString PlayingSong::artist()
{
	return mform->playingSongInfo->Items->Strings[1];
}

UnicodeString PlayingSong::title()
{
	return mform->playingSongInfo->Items->Strings[2];
}


//
//void __fastcall Tmform::btnStealAllSongsClick(TObject *Sender)
//{
//
//clock_t start, stop; double TIMe; start = clock();
//
//    af.loadAllSongsData();
//	for(UINT i=0; i < af.dataAllSongs->Count; i+=4)
//	{
//		UnicodeString artist = af.dataAllSongs->Strings[i];
//		UnicodeString title =af.dataAllSongs->Strings[i+1].SubString1(artist.Length() + 4, 9999);
//		UnicodeString birthDay = af.dataAllSongs->Strings[i+2];
//		UnicodeString nullDate = "09.09.9999 01:00";
//
//		allSongs.push_back(AllSong(&artist,
//								   &title,
//								   &birthDay,
//								   &nullDate,
//								   &nullDate,
//								   af.dataAllSongs->Strings[i+3].ToInt(),
//								   0) );
//	}
//
//
//stop = clock(); TIMe = (double)(stop-start)/CLOCKS_PER_SEC; ShowMessage(TIMe);
//}
//---------------------------------------------------------------------------




