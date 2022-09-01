//---------------------------------------------------------------------------
#pragma hdrstop
#include "ssetings.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TsetForm *setForm;


	  // ============ MY INCLUDES ============
#include "Folders.h"    // get backup folder names + copy folders
#include "enums.h"      // enums
#include "AllSong.h"

      // ============ MY FUNCTIONS ============


	  // ============ MY VARIABLES ============
bool panelClick = false;                      // don't check for limit on panelClick
short maxFilterValue = 200;                   // for thumbs in filter edit window
short oldModkey1Index;                        // memorizes last index of cboxMod1 & 2
short oldModkey2Index;
struct Shortcut;
std::vector <Shortcut> shortcutsData;           // here stored data with shorcuts
std::vector<int> indexx;                      // chooses items to display
extern Playlists pl;
extern bool programIsCreating;
extern std::vector <TButton*> plButtons;                // SCouner playlists
extern UnicodeString intToUS(const int &integer);  	// conver int to US string
extern bool lWinPressing;                          // is win key pressing?
extern std::vector <AllSong> allSongs;   // here we store all data.

//---------------------------------------------------------------------------
__fastcall TsetForm::TsetForm(TComponent* Owner)
	: TForm(Owner)
{
	SHAutoComplete(search->Handle, SHACF_AUTOAPPEND_FORCE_OFF); // allow ctrl+backspace
	for(UINT i=0; i < PageControl1->PageCount; i++)      // hide tabs
		PageControl1->Pages[i]->TabVisible = false;

	int jump = 10;                       // set a little bit lower those components (hotkeys page)
	Label7->Top += jump;
	cboxMod1->Top += jump;
	cboxMod2->Top += jump;
	Label8->Top += jump;
	minimizeOnEsc->Top += jump;
}
//---------------------------------------------------------------------------


bool deleteFolder(const UnicodeString &path)
{
	 if(!DirectoryExists(path))                 // GUARD
		 return false;


	 SHFILEOPSTRUCT f = {0, FO_DELETE, (path + '\0').c_str(), 0 ,
	 FOF_NOCONFIRMATION|FOF_SIMPLEPROGRESS, false, 0, 0};

	 if(SHFileOperation(&f)) {                   // error!
		ShowMessage("Can't delete folder!\n" + path);
		return false;
	 }

	 return true;
}


WCHAR* GetKeyName(unsigned int virtualKey)
{
	unsigned int scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

    // because MapVirtualKey strips the extended bit for some keys
    switch (virtualKey)
    {
		case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
		case VK_PRIOR: case VK_NEXT: // page up and page down
		case VK_END: case VK_HOME:
		case VK_INSERT: case VK_DELETE:
		case VK_DIVIDE: // numpad slash
		case VK_NUMLOCK:
		{
			scanCode |= 0x100; // set extended bit
			break;
		}
	}


	LONG lParamValue = (scanCode << 16);
	static WCHAR name[1024];

	int result = GetKeyNameTextW(lParamValue, name, 1024);
	if (result > 0)
		return &name[0];
	else
		return L"Error";
}

void TsetForm::deleteData()                                                             // [DELETE]
{                                                                                       // songs from AllSongs
   int lastSelSong = ListView_GetNextItem(listView->Handle, -1, LVNI_SELECTED);
   std::vector<int> selIndexes;
   while(true)
   {
	  selIndexes.push_back(lastSelSong);
	  if(ListView_GetNextItem(listView->Handle, lastSelSong, LVNI_SELECTED) != -1)
		 lastSelSong = ListView_GetNextItem(listView->Handle, lastSelSong, LVNI_SELECTED);
	  else break;
   }

   listView->Selected = false;
   listView->Items->Count = listView->Items->Count - selIndexes.size();

   std::vector <UnicodeString> tempVecOfArtists;     // and make indexation of curr Playlist
   for(int i=0; i < songs.size(); i++)
		tempVecOfArtists.push_back(songs[i].artist.LowerCase());    // artists
   af.Indexation(tempVecOfArtists, af.artBef, af.artIndexesBef);

   if(indexx.size())                                                // if F3 filter was ON
   {
	  for(int i=0; i < selIndexes.size(); i++)
         selIndexes[i] = indexx[selIndexes[i]];
   }

   std::vector<int> playlistIndexes;
   for(int i = selIndexes.size() - 1; i > -1; i--)
   {
	  for(int j=0; j < af.artBef.size(); j++)                             // delete from playlist
	  {
//		  if(af.dataAllSongs->Strings[selIndexes[i]*4].LowerCase() != af.artBef[j].LowerCase()) continue;
		  // NEW CODE
		  if(allSongs[selIndexes[i]].artist().LowerCase() != af.artBef[j].LowerCase()) // artist == artist
			 continue;

		  for(int k=0; k < af.artIndexesBef[j].size(); k++)
		  {

//			  if(af.dataAllSongs->Strings[selIndexes[i]*4 + 1]
//			 == songs[af.artIndexesBef[j][k]].name.LowerCase())
			 // NEW CODE
			 if(allSongs[selIndexes[i]].title().LowerCase()== songs[af.artIndexesBef[j][k]].title.LowerCase())
			 {
				 playlistIndexes.push_back(af.artIndexesBef[j][k]);
				 goto end;
             }
		  }
	  }
	  end:

//	  for(int j=0; j < 4; j++)                  // delete from dataAllSongs
//		 af.dataAllSongs->Delete(selIndexes[i]*4);
		 // NEW CODE
	  for(short j=0; j < 4; j++) 0;
		 allSongs.erase(allSongs.begin() + selIndexes[i]);
   }

   mform->playlist->Selected = false;
   for(int i=0; i < playlistIndexes.size(); i++)
   { songs[playlistIndexes[i]].selected = true;
	 mform->playlist->Items->Item[playlistIndexes[i]]->Selected = true; }
   af.deleteFiles();

   UnicodeString text = playlistIndexes.size();
   text += " song(songs) has been deleted from your playlist";
   messageForm->showMessage(text, 'i', 2);

   searchChange(NULL);
   listView->UpdateItems(0, listView->Items->Count-1);
}


void __fastcall TsetForm::FormShow(TObject *Sender)                                        // on Show
{
   sg->Col = 1; sg->Row = 1;    // select first active cell in hotkeys

 /////
   int firstRowInd = 0;                                       // find index and add filters captions
   for(int i=0; i < sg->RowCount; i++)
		if("   FILTERS" == sg->Cells[0][i])
		    firstRowInd = i+1;
   for(int i=0; i < 5; i++)
   {
	   sg->Cells[0][firstRowInd] = "[" + mform->filterPopUp->Items->Items[i]->Caption + "]";
	   firstRowInd ++;
   }
 ////                            		// add playlists captions and hotkeys
	   firstRowInd ++;  		// because "    PLAYLIST"  row
	   UINT firstRowIndCopy = firstRowInd;
	   sg->RowCount = firstRowInd + plButtons.size();      // set SG rowCount coordinately to playlists count

   for(UINT i=firstRowInd; i < sg->RowCount; i++)      // clear all playlist cells
   {
	   for(UINT j=1; j < 5; j++)
			sg->Cells[j][i] = "";
   }
   for(UINT i=0; i < plButtons.size(); i ++)           // fill playlists names
   {
	   sg->Cells[0][firstRowInd] = plButtons[i]->Caption;
	   firstRowInd ++;
   }
   for(UINT i=firstRowIndCopy; i < sg->RowCount; i++)     //  and fill playlists assigned keys
   {
		for(UINT j=0; j < shortcutsData.size(); j++)
			if(sg->Cells[0][i] == shortcutsData[j].functionName && shortcutsData[j].vkCode)
			{
				sg->Cells[shortcutsData[j].col][i] = shortcutsData[j].keysCaption;
                shortcutsData[j].row = i;  // fix row (because they do change often)
            }
   }

 /////                                            // check whether playlist "All songs" enabled.
   programIsCreating = true;
   if(plButtons[0]->Caption == "All songs")       // if something gone wrong
		cboxAllPl->Checked = true;
   else
		cboxAllPl->Checked = false;
   programIsCreating = false;
 //////

   refreshBackupList();                 // shows all avaible backups

   treeV->SetFocus();
   search->Text = L"Search ... [F3]";

   listView->Items->Count = allSongs.size();
   // NEW CODE
}
//---------------------------------------------------------------------------

void TsetForm::refreshBackupList()            // shows all avaible backups
{
   std::vector <UnicodeString> backupNames
	 = SrcFolder::getFileNames(pl.getSettingsPath() + "BACKUP");
   lvBackup->Clear();
   for(UINT i=0; i < backupNames.size(); i++) {
	   lvBackup->Items->Add();
	   UnicodeString caption = ExtractFileName(findRepChar(backupNames[i], '_', ' '));
	   UnicodeString time = findRepChar(caption.SubString0(caption.Length() - 8, 8), '.', ':');
	   caption.Delete0(caption.Length() - 8, 8);
	   caption += time;

	   lvBackup->Items->Item[i]->Caption = caption;
   }
}


void __fastcall TsetForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)         // on Form4 keyDown
{
	if (Key == vkEscape && !sg->EditorMode)	setForm->Close();  // [ESC]   close the form
	else if (Key == vkF3 && PageControl1->ActivePageIndex == 1) searchClick(NULL);
}
//---------------------------------------------------------------------------              // on Form4 KeyPress
void __fastcall TsetForm::FormKeyPress(TObject *Sender, System::WideChar &Key)
{
	if(PageControl1->ActivePage->Name == "hotKeysTab" || Key == vkEscape)
	   Key = 0;                // STOP BEEPING!   +  reset entering StringGrid via keyDown
}
//---------------------------------------------------------------------------
void __fastcall TsetForm::listViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)    // listView KeyDown
{
   if(Key == vkDelete && listView->SelCount) {                       // [DELETE]  - for ever?
      messageForm->playBeep('w');
	  if(MessageBoxW(NULL, L"SCounter will forget about this(these) song(s) FOREVER."
	  "\nListenings, date of adding will be deleted.\nDelete?", L"Attention", MB_YESNO)
	  == IDYES)
	    deleteData();
   }
   if(Shift.Contains(ssCtrl) && Key == vkA) listView->SelectAll();  // [CTRL+A]
}
//---------------------------------------------------------------------------              // searchBox KeyDown


void __fastcall TsetForm::savebtnClick(TObject *Sender)
{
   TStringList *hotKeyboardData = new TStringList;
   hotKeyboardData->Add(sg->ColCount);                          // saving data to TXT
   hotKeyboardData->Add(sg->RowCount);

   for(int i=0; i < sg->ColCount; i++)
   {
	   for(int j=0; j < sg->RowCount; j++)
	   {
		  hotKeyboardData->Add(sg->Cells[i][j]);
	   }
   }

   UnicodeString path = pl.getSettingsPath(); path += "settings/hotKeysData.txt";
   hotKeyboardData->SaveToFile(path, TEncoding::UTF8);
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::listViewData(TObject *Sender, TListItem *Item)           // onData
{
   int index = Item->Index;
   if(indexx.size())    // user selected indexes
	   index = indexx[Item->Index];
//   if(indexx.size() == 0)                // DON'T NEED THIS IN NEW CODE
//	   index = Item->Index*4;
//   else
//	   index = indexx[Item->Index]*4;


   // NEW CODE
   Item->Caption = allSongs[index].name();       // name
   Item->SubItems->Add(allSongs[index].playCount());  // listenings
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::FormCreate(TObject *Sender)                             // onCreate
{
  panel->Caption = mform->filterPopUp->Items->Items[1]->Caption;     // caption of filter
  MenuItemClick(first); panel->Caption = mform->filterPopUp->Items->Items[1]->Caption;

  PageControl1->ActivePageIndex = 0;   // Allsongs editor

  treeV->Selected = treeV->Items->Item[0];  // select first node

  pl.loadForOtherForms('s');                // load settings

   TStringList *hotKeyboardData = new TStringList;            // loading shortcuts from TXT
   hotKeyboardData->LoadFromFile(pl.getSettingsPath() + "settings/hotKeysData.txt");

   short colCountTxt = hotKeyboardData->Strings[0].ToInt();
   short rowCountTxt = hotKeyboardData->Strings[1].ToInt();
   sg->RowCount = rowCountTxt;

   for(int i=0; i < colCountTxt; i++)
   {
	   for(int j=0; j < rowCountTxt; j++)
	   {
		  sg->Cells[i][j] = hotKeyboardData->Strings[i*rowCountTxt+j+2];
	   }
   }

  aboutText->Caption =
  "Version: 2.2.0     Release date: 04.06.2020	"
  "\nCreated by:																		"
  "\n                                                                                   "
  "\nThanks to Dima Nikolaevich for giving me music knowing inspiration.                "
  "\nThanks to Miroslaw Zelent for teaching me.                                         "
  "\nThanks to Jaroslaw Chladowicz for preventing SCounter be in sketch for ever.       "
  "\nThanks to Tanya. I even don't know how strongly she changed my life.               "
  "\n                                                                                   "
  "\nThank you for using this program. This makes me happy.                             "
  ;

  v1_0label->Caption =
L"► Release date: 28.12.2018"            // v1.0
L"\n\nFirst release version. It was pretty awful =)";

                                                                // v1.1
  v1_1label1->Caption =
L"► Release date: 13.01.2019 														      "
L"\n\n    Fixed:                                                                          "
L"\n•  A lot of HotKeys bugs                                                              "
L"\n•  Volume-bar bug (it changed volume only on click&&move)                             "
L"\n•  \"Filter\" bug (user could choose currently active filter second time)             "
L"\n•  Bug with deleted songs in playlist                                                 "
L"\n•  \"Delete bad songs\" bug                                                           "
L"\n•  Little bugs in \"Allsongs editor\"                                                 "
L"\n•  \"Filter\" bug (SCS got wrong when playing song \"got out\" from other filters)    "
L"\n\n    Improved:                      						                          "
L"\n•  Opening folders in \"Same file exists\" window                                     "
L"\n•  Playing continues when last song in playlist ends                                  "
L"\n•  Playing doesn't stop on filter && shuffle click                                    "
L"\n•  .exe file can be renamed                                                           "
L"\n•  Fonts loads into memory (it was necessery to have SCounter fonts on your OS)       "
L"\n•  Pressing 'esc' in Settings does not beep anymore                                   "
L"\n\n    Added:                                                                          "
L"\n•  Ssounds for messages                                                                "
L"\n•  Unplayed songs are hidden in shuffle mode                                          "
L"\n•  \"About\" page                                                                     "
L"\n•  Export/import listenings feature                                                   "
L"\n•  Real time filtering                                                                "
L"\n•  Listenings change in real time in \"Listenings\" window                            "
;

   v1_2_0label->Caption =                                      // v1.2.0
L"► Release date: 13.02.2019                                                              "
L"\n                                                                                      "
L"\n    Fixed:                                                                            "
L"\n•  Istaller bug (\"Choose installation directory\" skipped if SCounter was installed)  "
L"\n•  Import listenings bug (Today && current listenings got all listenings from .ssf file (1500 e.g.))   "
L"\n•  Messages bugs (Some windows were too narrow to read the message)                    "
L"\n•  Busy SCounter (It was impossible to delete file in win file explorer when "
L"\"Show same songs\" window was active)                                                 "
L"\n																					  "
L"\n    Improved:                                                                         "
L"\n•  Little bugs and made some improvements                                              "
L"\n•  Improved import and export dialog windows (those dialogs weren't 100% save +info messages were added) "
L"\n•  Hint time appearing(500ms-200ms) + hint time showing(5sec-infinity)                 "
L"\n•  Adding files + \"Delete Bad Songs\" (after adding files SCounter was focused on playing song "
L"\n  +DBS cleared selection if were no bad songs)  										  "
L"\n•  Saving data (SCounter didn't work with UAC in directory \"C:/Program files\")       "
L"\n                                                                                      "
L"\n	Added:                                                                            "
L"\n•  CTRL+A works in allsongs editor                                                     "
L"\n•  History of versions                                                                 "
L"\n•  Remembering last SCounter position                                                  "
L"\n•  Don't annoy me again! feature for some messages                                     "
L"\n•  Running guard (doesn't run SCounter without nesessary files)                        "
L"\n•  \"TerminatingSong\" feature                                                         "
L"\n•  Minimizing to tray                                                                  "
L"\n•  Quick search                                                                        "
L"\n•  User can modify filter values                                                       "
;

   v1_2_1label->Caption =                      // v1.2.1
L"► Release date: 14.02.2019                                                              "
L"\n                                                                                      "
L"\n    Fixed:                                                                            "
L"\n•  5 little bugs"
;

   v1_2_2label->Caption =                      // v1.2.2
L"► Release date: 16.02.2019                                                              "
L"\n                                                                                      "
L"\n    Fixed:                                                                            "
L"\n•  7 little bugs																	      "
L"\n•  Real-time filtering bug															  "
L"\n                                                                                      "
L"\n    Added:                                                                            "
L"\n•  4 global keys                        					                              "
;

   v2_0_0label->Caption =                      // v2.0.0
L"► Release date: 23.03.2019                                                              "
L"\n                                                                                      "
L"\n    Legend:                                                                                "
L"\n-  \" \" - window                                                                  "
L"\n-  [ ] - function or feature                                                            "
L"\n-  SCS = shuffle current song                                                          "
L"\n                                                                                        "
L"\n    Fixed:                                                                               "
L"\n•  5 little bugs                                                                       "
L"\n•  Bug in real-time refreshing in \"Listenings\" (if sorting messed up when song counter iterated)"
L"\n•  1 deleting bug (if playing song was deleted in pause, SCS didn't decrement)         "
L"\n                                                                                       "
L"\n    Improved:                                                                           "
L"\n•  [Do not repeat last songs] increased 300-1000 songs                                 "
L"\n•  [Quick search] shows founded song name in shuffle                                   "
L"\n•  [Terminating song] SCounter can hide immediately if users wants to                  "
L"\n•  \"Find twins\" doesn't flick anymore                                                "
L"\n•  SCounter doesn't 'jump' anymore (when program is starting)                           "
L"\n•  Tag support: SCounter reads ID3 tag v.2.2, v.2.3, v.2.4 (ISO-8859-1 and UTF-8 only) "
L"\n•  Adding files algorythm has been totally changed and improved                        "
L"\n•  Artis/Title/Album moves, if it doesn't fit a panel                                  "
L"\n                                                                                       "
L"\n    Added:                                                                              "
L"\n•  Preventing opening second instance of SCounter                                      "
L"\n•  \"Listenings/Counter\" window opens/closes via \"C\" key                            "
L"\n•  Tray icon restores after \"explorer.exe\" crash                                     "
L"\n•  Album + year panels                                                                 "
L"\n•  Hint with playing song name in tray                                                 "
L"\n•  Playlists                                                                           "
L"\n•  Required time to listen all songs in current playlist                               "
L"\n•  Tray icons (playing, paused) + play/pause on tray icon click                        "
L"\n•  [Sort playlist by path] function                                                    "
L"\n•  Playlists!!!                                                                        "
L"\n•  Modyfing hotkeys                                                             "
;

	v2_1_0label->Caption =                      // v2.1.0
L"► Version 2.1.0                                                                          "
L"\n► Release date: 03.07.2019                                                              "
L"\n																						   "
L"\nLegend:                                                                                    "
L"\n  "" - window                                                                            "
L"\n  [] - function or feature                                                               "
L"\n  SCS = Shuffle current song                                                             "
L"\n																						   "
L"\n   Fixed:                                                                                  "
L"\n•  9 little bugs                                                                           "
L"\n•  DBL-click and right click doesn't work in playlist, if user clicks on empty space       "
L"\n•  After creating new playlist it jumps toward it (turns next playlist page if new playlist "
L"\ndoesn't fit current page)                                                                  "
L"\n																						   "
L"\n   Improved:                                                                               "
L"\n•  Name of playing song is showing instead \"------\"                                        "
L"\n•  SCS and playing songs now selected with different colors                                "
L"\n•  Tray icon becomes orange when [Terminating song] is going to close SCounter             "
L"\n•  [Copy to another playlist] doesn't add songs that already exists in that playlist       "
L"\n•  [Jump to selected || playing || SCS] (now user can choose which song will be shown)     "
L"\n•  [Delete bad songs] user can see all deleting songs and choose beetwen deleting and      "
L"\nnot deleting those bad songs                                                               "
L"\n•  Auto save - all data saves when any song passed 75%                                     "
L"\n•  Export / Import - now SCounter exports / imports all user settings                      "
L"\n•  Playlist flicking issue has been removed                                                "
L"\n•  Songs highlighting logic                                                                "
L"\n•  \"Delete bad songs\" now shows which songs are bad and what the issue                   "
L"\n																						   "
L"\n   Added:                                                                                  "
L"\n•  [Refresh song duration] (useful if song has been cut in audio redactor e.g.)            "
L"\n•  Message warning in \"AllSongs editor\". (if user deletes data from AllSongs)              "
L"\n•  Progress bar for [Loading files data]                                                   "
L"\n•  [Auto next] - user can choose what happens when song ends (play next or do nothing)     "
L"\n•  [Sort songs by name]                                                                    "
L"\n•  [Play next song automatically]                                                          "
L"\n•  [Maximize / Restore SCounter]                                                           "
L"\n•  [Delete playing song]                                                                   "
L"\n•  [Open playing song destination folder]                                                  "
L"\n•  [Backup] allows user restore early saved settings                                       "
L"\n•  [Add artist to playlists]                                                               "
L"\n•  [Starting guard] (checks whether all necessary files exist)                             "
L"\n•  Playlists with assignable hotkeys                                                       "
L"\n•  Maaaaaany new hotkeys                                                                   "
  ;


	v2_1_1label->Caption =                      // v2.1.1
L"► Version 2.1.1                                                            "
L"\n► Release date: 08.07.2019                                                 "
L"\n                                                                           "
L"\nLegend:                                                                    "
L"\n	"" - window                                                              "
L"\n	[] - function or feature                                                 "
L"\n	SCS = Shuffle current song                                               "
L"\n                                                                           "
L"\n   Fixed:                                                                  "
L"\n•  listening window closes without modifier keys                           "
L"\n•  import/export settings bug (importing could cause program to never run) "
L"\n•  playlist hight wasn't accurate after run and importing settings         "
L"\n•  playlist shortcuts such as: 'Ctrl + Shift + C'                          "
L"\n•  playlist horizontal bar appearing when adding new files via \"Add artists\""
L"\n•  6 other bugs                                                            "
L"\n                                                                              "
L"\n   Improved:                                                                  "
L"\n•  changed positions of playlist panel labels                                 "
L"\n•  [Scounter launcher]                                                        "
L"\n•  playlist buttons have artifacts when renaming, adding and deleting         "
L"\n                                                                              "
L"\n   Added:                                                                     "
L"\n•  playlist pages label                                                       "
	;

		v2_1_2label->Caption =                      // v2.1.2
L"► Version 2.1.2                                                              "
L"\n► Release date: 09.09.2019                                                 "
L"\n                                                                           "
L"\nLegend:                                                                    "
L"\n	"" - window                                                              "
L"\n	[] - function or feature                                                 "
L"\n	SCS = Shuffle current song                                               "
L"\n                                                                           "
L"\n   Fixed:                                                                   "
L"\n•  issue with naming playlist with dots '.'                                     "
L"\n•  song could switch to next when CPU was loaded more than 80%                    "
L"\n•  [add artists] didn't refresh playlist song count and playlist time duration     "
L"\n•  horizontall bar showed up while adding new files                                 "
L"\n•  when shuffle is on and SCS = 0, font always remained white                        "
L"\n•  modifying filter caused changes only in current playlist, instead of changing all playlists  "
L"\n•  6 other bugs                                                              "
L"\n                                                                                        "
L"\n   Improved:                                                                            "
L"\n•  info sound: when any songs were copied/added, it emits sound 'success', "
L"\nif songs already were in destination playlist, it emits sound 'warning'               "
L"\n•  in \"Add artists\" when user presses 'Enter', new artists are added and \"Add artist\" closes"
L"\n                                                                                        "
L"\n   Added:                                                                               "
L"\n•  label that displays current playing playlist name + [jump to curr playlist page]     "
L"\n•  playlist pages now scrollable with mouse over playlist panel                         "
L"\n•  'Ctrl+A' works in \"Add artists\"                                                      "
	;

			v2_2_0label->Caption =                      // v2.2.0
L"► Version 2.2.0                                                            "
L"\n► Release date: 04.06.2020                                                 "
L"\n                                                                           "
L"\nLegend:"
L"\n	"" - window"
L"\n	[] - function or feature"
L"\n	SCS = Shuffle current song"
L"\n"
L"\n   Fixed:"
L"\n•  2 little bugs"
L"\n•  F3 search can search again after pressing Shuffle button"
L"\n•  all opened files/folders now are always closed (redundant BACKUP folders won't create anymore)"
L"\n•  [Current session listenings] now shows correct listening count after [Import settings]"
L"\n•  songs with duration < 4 seconds now are counting."
L"\n•  deleting playing song when it goes in [inactive] section of playlist breaks [auto-next] playing(just stops)"
L"\n•  playCount iteration algorithm (play count could drastically mess up in previous versions)"
L"\n•  SCounter will not close if it is a last song and [Terminating song] is active"
L"\n"
L"\n   Improved:"
L"\n•  autobackup saves data every 30 min instead of 10 min"
L"\n•  deleting playing song will not stop playback"
L"\n"
L"\n   Added:"
L"\n•  SCounter now counts skipped songs as well"
L"\n•  SCounter remembers date and time of every single listening and skipping"
L"\n•  You can enable/disable showing message \"The end of playlist\""
	;
}

//---------------------------------------------------------------------------

void __fastcall TsetForm::searchClick(TObject *Sender)
{
   if(search->Text == L"Search ... [F3]") search->Text = L"";
   search->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TsetForm::searchChange(TObject *Sender)
{
   if(search->Text.Length() == 0 || search->Text == L"Search ... [F3]")   // GUARD
   {
	 indexx.clear();
	 listView->Items->Count = allSongs.size();
	 // NEW CODE

	 listView->UpdateItems(0, listView->Items->Count);
	 return;
   }


   listView->Items->Count = 0;
   UINT count=0;
   indexx.clear();
   for(int i=0; i < allSongs.size(); i++)
				  // allSongs.size();

	   // NEW CODE
	   if(allSongs[i].name().LowerCase().Pos1(search->Text.LowerCase()))
//	   if(af.dataAllSongs->Strings[i*4 + 1].Pos(search->Text.LowerCase()))
	   {
		  count++;
		  indexx.push_back(i);
	   }
   listView->Items->Count = indexx.size();
}

//---------------------------------------------------------------------------

void __fastcall TsetForm::exportbtnClick(TObject *Sender)                          // [EXPORT BTN]
{
	 UnicodeString currDir = GetCurrentDir();

	 TCHAR path[_MAX_PATH] = _T("");    // get desktop directory
	 SHGetSpecialFolderPath(NULL, path, CSIDL_DESKTOP, FALSE);
	 saveD->InitialDir = path;

	 saveD->FileName = L"SCounter settings (" + Date().FormatString("dd.mm.yyyy") + ")";

     setForm->Tag = FileDialogState::Showing;
	 if( saveD->Execute() )
	 {
		 pl.saveEverything();      // save current settings!!!!

		 SrcFolder src(pl.getSettingsPath());          // create Folder object
		 CreateDir(saveD->FileName);                   // create empty dest folder
		 src.copyToAnotherFolder(saveD->FileName);     // copy!
	 }

     setForm->Tag = FileDialogState::NotShowing;
	 SetCurrentDir(currDir); // turn back our .exe directory!
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::importbtnClick(TObject *Sender)                         // [IMPORT BTN]
{
	UnicodeString currDir = GetCurrentDir();

	TCHAR path[_MAX_PATH] = _T("");    // get desktop directory
	SHGetSpecialFolderPath(NULL, path, CSIDL_DESKTOP, FALSE);
    openD->DefaultFolder = path;

	openD->FileName = L"";
    setForm->Tag = FileDialogState::Showing;
	if(openD->Execute())
	{
		//  show warn message to user
		if(IDNO == MessageBoxW(NULL, L"You are going to restore settings. All current settings will be lost."
		"\nContinue?", L"", MB_YESNO)) {
			SetCurrentDir(currDir); // turn back our .exe directory!
			setForm->Tag = FileDialogState::NotShowing;
			return;
        }
                                                              // it's not scounter settings folder!!!
	  if(!DirectoryExists(openD->FileName + "\\settings") || !DirectoryExists(openD->FileName + "\\playlists"))
	  {
        setForm->Tag = FileDialogState::NotShowing;
		messageForm->showMessage("This folder does not seem to be proper SCounter settings folder.", 'e', 2);
		return;
	  }

		 SrcFolder settings(openD->FileName + "\\settings");          // create Folder object
		 SrcFolder playlists(openD->FileName + "\\playlists");          // create Folder object
		 SrcFolder backup(openD->FileName + "\\BACKUP");          // create Folder object

//		 deleteFolder(pl.getSettingsPath() + "settings");       // delete old folders
//		 deleteFolder(pl.getSettingsPath() + "playlists");
		 if(backup.objectIsValid)
			deleteFolder(pl.getSettingsPath() + "BACKUP");

		 CreateDir(pl.getSettingsPath() + "settings");          // create empty dirs
		 CreateDir(pl.getSettingsPath() + "playlists");
		 if(backup.objectIsValid)
		 	CreateDir(pl.getSettingsPath() + "BACKUP");

		 settings.copyToAnotherFolder(pl.getSettingsPath());     // copy!  to %Appdata%
		 playlists.copyToAnotherFolder(pl.getSettingsPath());
		 if(backup.objectIsValid)
			 backup.copyToAnotherFolder(pl.getSettingsPath());

       	Shortcut::unregisterAllShortcuts();

//		af.loadAllSongsData();                            // load data from AllSongs.txt
		AllSong::loadFromFile();
		// NEW CODE

		pl.loadPlaylistSettings();
		pl.loadPlaylist(pl.activePlaylist);               // load last playlist condition
		pl.loadForOtherForms('m');
		pl.loadForOtherForms('s');
		pl.loadPlaylistButtons();
		pl.refreshSongCounter("comm");
        startList = AllSong::getAllListenings();  // set current session listenings

		 messageForm->showMessage("Settings restored successfully!", 'i', 2);     // show message (with beep)
	}

    setForm->Tag = FileDialogState::NotShowing;
	SetCurrentDir(currDir); // turn back our .exe directory!
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::treeVChange(TObject *Sender, TTreeNode *Node)             // onChange
{
	 PageControl1->ActivePageIndex = treeV->Selected->Index;
}
//---------------------------------------------------------------------------




void __fastcall TsetForm::panelMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)                                                       // [CHOOSEFILTER POPUP]
{
   POINT point;  (GetCursorPos(&point));

   for(int i=0; i < 4; i++)                      // copy popup captions from mform
	 myOwnPopup->Items->Items[i]->Caption = mform->filterPopUp->Items->Items[i+1]->Caption;

   myOwnPopup->Popup(point.x, point.y);
}
//---------------------------------------------------------------------------


void __fastcall TsetForm::MenuItemClick(TObject *Sender)                   // on MenuItemClick
{
 panelClick = true;
	TMenuItem *ClickedItem = dynamic_cast <TMenuItem *> (Sender);
    panel->Tag     = ClickedItem->Tag;
	panel->Caption = ClickedItem->Caption;

    minEdit->Text = pl.filterMin[ClickedItem->Tag];       // show filter on rangeBar

	if(pl.filterMax[ClickedItem->Tag] == 2147483647)
	   maxbtnClick(NULL);       // for activate on change
	else maxEdit->Text = pl.filterMax[ClickedItem->Tag];

 panelClick = false;
	if(!panel->Focused() && setForm->Visible)   panel->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::thumbRMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
   if(GetKeyState(VK_LBUTTON) & 0x100)                          // dragging thumbs
   {
	  TImage *img = dynamic_cast <TImage *> (Sender);
	  TEdit *edit;

	  switch(img->Tag)
	  {
		case 0:          // left thumb moving
		{
			int pix = X + thumbL->Left - rangeBar->Left;
			if(pix < 1)                                    // limiting
			{ minEdit->Text = 0; thumbL->Left = rangeBar->Left - thumbL->Width+1; break; }

			if(pix > thumbR->Left - rangeBar->Left)
			{
			  if(maxEdit->Text=="Max") minEdit->Text = maxFilterValue;
			  else minEdit->Text = maxEdit->Text;     // value for filter
			  thumbL->Left  = thumbR->Left - thumbR->Width; break; }

			thumbL->Left = X + thumbL->Left-thumbL->Width;
			minEdit->Text = (pix*maxFilterValue) / rangeBar->Width;  // value for filter
		}
		break;

		case 1:        // right thumb moving
		{
			int pix = X + thumbR->Left - rangeBar->Left;

			if(pix < thumbL->Left+thumbL->Width - rangeBar->Left)        // limiting
			{ maxEdit->Text = minEdit->Text;
			thumbR->Left = thumbL->Left + thumbL->Width; break; }

			if(pix > rangeBar->Width -2)//+ thumbR->Width)
			{ maxEdit->Text = maxFilterValue;     // value for filter
			  thumbR->Left  = rangeBar->Left + rangeBar->Width -2; break; }

			thumbR->Left = X + thumbR->Left;
			maxEdit->Text = (pix*maxFilterValue) / rangeBar->Width;  // value for filter
		}
		break;
	  }
	  panel->SetFocus();
   }
																			 // move snake!!!
   if(maxEdit->Text.Pos("x"))  moveSnake(true);
   else moveSnake(false);
}
//---------------------------------------------------------------------------
void __fastcall TsetForm::maxbtnClick(TObject *Sender)               // [MAXBTN]
{
   maxEdit->Text = "Max";
   panel->SetFocus();
   thumbR->Left = rangeBar->Width + rangeBar->Left - 2;   // move thumb
   moveSnake(true);                  				      // resize snake
}
//---------------------------------------------------------------------------

void TsetForm::moveSnake(bool wMax) 				          // resizing snake
{
  if(minEdit->Text != "" && maxEdit->Text != "")           // enable "apply btn"?
  {
	 if(minEdit->Text.ToInt()  != pl.filterMin[panel->Tag]  // yes, user changed value
	 || maxEdit->Text == "Max" && pl.filterMax[panel->Tag] != 2147483647
	 || (maxEdit->Text != "Max" && (maxEdit->Text.ToInt() != pl.filterMax[panel->Tag]) ) )
	 {      if(!applybtn->Enabled) applybtn->Enabled = true;   }
	 else { if(applybtn->Enabled)  applybtn->Enabled = false;  }
  }

   snake->Left = thumbL->Left+thumbL->Width - rangeBar->Left;       // LEFT

   if(!wMax && minEdit->Text != "" && maxEdit->Text != ""
   && (minEdit->Text.ToInt() == maxEdit->Text.ToInt()
   || ( minEdit->Text.ToInt() == maxEdit->Text.ToInt()-1 )
   || ( minEdit->Text.ToInt() == maxFilterValue )))
   { snake->Width = 1; return; }

   if(wMax) snake->Width = rangeBar->Width;                      // RIGHT (WIDTH)
   else  snake->Width = rangeBar->Width - (rangeBar->Width - (thumbR->Left - rangeBar->Left)) //right square
	  - (thumbL->Left+thumbL->Width - rangeBar->Left);
}

void __fastcall TsetForm::minEditChange(TObject *Sender)
{
  bool withMax = false;
  if(maxEdit->Text == "Max")
  {	withMax = true;  maxEdit->ReadOnly = true; }
  else  maxEdit->ReadOnly = false;

  if(! (GetKeyState(VK_LBUTTON) & 0x100))                                // minEdit + maxEdit change
  {
	   TEdit *edit = dynamic_cast <TEdit*> (Sender);
	   TImage *img;
	   short rangeBarLeft = rangeBar->Left;
														 // GUARD
	   if(edit->Text == "" || (edit->Name == "maxEdit" && withMax))
		   return;


	   if(edit->Name == "minEdit")                       // LEFT thumb!
	   {
			img = thumbL;
			rangeBarLeft -= img->Width;

			if(!panelClick && !withMax && (edit->Text.ToInt() >= maxEdit->Text.ToInt())
			||  withMax && (minEdit->Text.ToInt() >= maxFilterValue))
			{
			   if(withMax) edit->Text = maxFilterValue;
			   else edit->Text = maxEdit->Text;

			   img->Left = thumbR->Left - img->Width;
			   goto jump;
			}
	   }
	   else
	   {
		  img = thumbR;                                  // RIGHT thumb!

		  if(!panelClick)
		  {
			if(maxEdit->Text.ToInt() < minEdit->Text.ToInt())   return;
			if(maxEdit->Text.ToInt() > maxFilterValue) {  maxEdit->Text = maxFilterValue; rangeBarLeft -= 2; }
		  }
	   }

		 {                                          // change position(thumb->Left)
		   float multiplier = (float) maxFilterValue/rangeBar->Width;
		   img->Left = rangeBarLeft + edit->Text.ToInt() / multiplier;
		 }

	jump:
	   edit->SelStart = 3;
  }

  moveSnake(withMax);
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::minEditExit(TObject *Sender)        // on loose focus
{
	TEdit *edit = dynamic_cast <TEdit*> (Sender);      // max lost focus
	TImage *img;
	short rangeBarLeft = rangeBar->Left;

	if(edit->Name == "maxEdit")  img = thumbR;        // set thumb
	else     { img = thumbL; rangeBarLeft -= thumbL->Width; }

	if(edit->Text == "")
	edit->Text = (img->Left - rangeBarLeft) *maxFilterValue / rangeBar->Width;
                                                                  // little GUARD (very rare)
	if(maxEdit->Text != "Max"
       && (minEdit->Text.ToInt() > maxEdit->Text.ToInt()) ) maxEdit->Text = minEdit->Text;
}
//---------------------------------------------------------------------------


void __fastcall TsetForm::applybtnClick(TObject *Sender)     // [APPLY BTN]
{
	panel->SetFocus();

	if(minEdit->Text != "" && maxEdit->Text == "Max" && minEdit->Text.ToInt() == 0)   // is that "All" filt?
	{
	   messageForm->showMessage("That filter already exists. It calls \"All\"", 'i', 2);
	   return;
    }


	messageForm->playBeep('w');									// message
	UnicodeString temp = L"All playlists will lose shuffle data in \"" + panel->Caption + "\" filter. Continue?";
  // if(!pl.started[panel->Tag + 1] ||
  												   // APPLY NEW FILTER!
	if(Application->MessageBoxW(temp.c_str(), L"", MB_YESNO) == IDYES)
	{
		short tag = panel->Tag + 1;  // which filter is going to change?

		pl.shuffleCurrSong[tag] = 0;
		pl.started[tag] = 0;
		for(int i=0; i < songs.size(); i++)
			songs[i].order[tag] = 0;
        pl.refreshAllSCounterFilters((Filter)tag);    // loads all playlists and deletes SCS data.

		pl.filterMin[tag-1] = minEdit->Text.ToInt();
		if(maxEdit->Text != "Max") pl.filterMax[tag-1] = maxEdit->Text.ToInt();
		else pl.filterMax[tag-1] = 2147483647;
          pl.refreshFilterCap();

		if(tag == mform->filterbtn->Tag) // that filter is running
		{
			mform->filterbtn->Tag = -1;     // because filterMenuItemClick has "sameTagGuard"
			mform->filterMenuItemClick(mform->filterPopUp->Items->Items[tag]);
		}


		UINT indexTemp;    // shortcutsData has "function name" property. We're gonna refresh it
		String newCaption = mform->filterPopUp->Items->Items[tag]->Caption;
		for(int i=0; i < shortcutsData.size(); i++)
			if(shortcutsData[i].functionName == "["+ panel->Caption +"]")
			{
				shortcutsData[i].functionName = "["+ newCaption +"]";
//				indexTemp = i;  // what is this?????
			}

		panel->Caption = newCaption;
//		shortcutsData[indexTemp].functionName = "["+ newCaption +"]"; // what is this?????
	}

	applybtn->Enabled = false;      // it sets new filter, thus we can't apply same values!
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::twoHundbtnClick(TObject *Sender)
{                                                            // [200 btn]
   maxEdit->Text = 200;
   panel->SetFocus();
   thumbR->Left = rangeBar->Width + rangeBar->Left - 2;   // move thumb
   moveSnake(true);                  				      // resize snake
}
//---------------------------------------------------------------------------



void __fastcall TsetForm::toTrayGroupClick(TObject *Sender)
{
//	if(toTrayGroup->ItemIndex > 0) restoreGroup->Enabled = true;
//	else                           restoreGroup->Enabled = false;
}
//---------------------------------------------------------------------------


                                                                           // scroll!!
void __fastcall TsetForm::FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta,
          TPoint &MousePos, bool &Handled)
{
	if(PageControl1->ActivePage->Caption == "About")
	{
		short step = 15;
		WheelDelta = -WheelDelta; // -120 = scrolling up, +120 = scrolling down


		if(WheelDelta < 0) collapsingGroup->VertScrollBar->Position -= step;
		else               collapsingGroup->VertScrollBar->Position += step;
	}
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::CategoryPanel2Expand(TObject *Sender)
{
   collapsingGroup->VertScrollBar->Position -= 16 ;
}
//---------------------------------------------------------------------------

void Shortcut::registerShortcut(Shortcut shortcut)     // REGISTER GLOBAL SHORTCUT!
{
	if(!shortcut.global || !shortcut.vkCode) 	    // GUARD
		return;

	int identifier = shortcut.row;
	if(shortcut.col == 4) identifier += 200;      // +200 for SECOND global shortcut
	if(shortcut.row > 34) identifier += 300;      // +300 for PLAYLISTS global shortcut
    if(shortcut.col == 0) identifier += 400;      // +400 for DEFAULT gl.shortcuts

	short VKCode = shortcut.vkCode;

	UINT alt=0, ctrl=0, shift=0, win =0;
	if(shortcut.alt) 	alt = 1;
	if(shortcut.ctrl) 	ctrl = 2;
	if(shortcut.shift) 	shift = 4;
	if(shortcut.win)    win = 8;

	if(!RegisterHotKey(Application->Handle, identifier , alt | ctrl | shift | win, VKCode))
	{
        UnicodeString error;
		if(GetLastError() == ERROR_HOTKEY_ALREADY_REGISTERED)         // DEBUG
			error = "Already registred";
		else
			error = GetLastError();

//		ShowMessage("Cannot register hotkey. \nKey id: " + intToUS(identifier)
//			+ "\nShortcut: " + shortcut.keysCaption + "\nFunction: " + shortcut.functionName
//			+ "\nError: " + error);

		return;
	}
}

void Shortcut::unregisterShortcut(Shortcut shortcut)     // UNREGISTER GLOBAL SHORTCUT!
{
   if(!shortcut.global) return;    // GUARD

	int identifier = shortcut.row;
	if(shortcut.col == 4) identifier += 200;
	else if(shortcut.row > 34) identifier += 300; 		// playlists
    else if(shortcut.col == 0) identifier += 400;      // +400 for DEFAULT gl.shortcuts

	if(!UnregisterHotKey(Application->Handle, identifier)
		&& GetLastError() != 87
		&& GetLastError() != ERROR_HOTKEY_NOT_REGISTERED) {   // try to unregister
        ;   // do nothing! (for now)
//		ShowMessage("Could not unregister shortcut with id: " + intToUS(identifier)        // ERROR!
//			+ "\nShortcut: " + shortcut.keysCaption + "\nFunction: " + shortcut.functionName);
//		if(GetLastError() == ERROR_HOTKEY_NOT_REGISTERED)          // DEBUG
//			ShowMessage (GetLastError());
	}
}

void Shortcut::loadShortcuts()
{
	TStringList *strList = new TStringList;
	strList->LoadFromFile(pl.getSettingsPath() + "settings/ShortcutsData", TEncoding::UTF8);

    shortcutsData.clear();		// CLEEEAR@@!!!

	for(int i=0; i < strList->Count; i +=14)
	{
		Shortcut tempShortcut ( strList->Strings[i].ToInt()    // sequence same as defined in struct
							   ,strList->Strings[i +1].ToInt()
							   ,strList->Strings[i +2].ToInt()
							   ,strList->Strings[i +3].ToInt() +0
							   ,strList->Strings[i +4].ToInt() +0
							   ,strList->Strings[i +5].ToInt() +0
							   ,strList->Strings[i +6].ToInt() +0
							   ,strList->Strings[i +7].ToInt() +0
							   ,strList->Strings[i +8]   			 // "play, pause, resume"
							   ,strList->Strings[i +9]);   		     // "Alt + S"

		shortcutsData.push_back(tempShortcut);
	}

	Shortcut::registerAllShortcuts();

    mform->restoreFromTray();   // unregister all plGlobal shortcuts

}
void Shortcut::saveShortcuts()
{
	TStringList *strList = new TStringList;

	for(int i=0; i < shortcutsData.size(); i ++)
	{
		strList->Add(shortcutsData[i].vkCode);            // sequence same as defined in struct
		strList->Add(shortcutsData[i].col);
		strList->Add(shortcutsData[i].row);
		strList->Add(shortcutsData[i].shift +0);
		strList->Add(shortcutsData[i].ctrl +0);
		strList->Add(shortcutsData[i].alt +0);
        strList->Add(shortcutsData[i].win +0);
		strList->Add(shortcutsData[i].global +0);
		strList->Add(shortcutsData[i].functionName);  // terminationSong
		strList->Add(shortcutsData[i].keysCaption);   // "Alt + S"

		strList->Add("=== reserved");
		strList->Add("=== reserved");
		strList->Add("=== reserved");
		strList->Add("=== reserved");
	}

	strList->SaveToFile(pl.getSettingsPath() + "settings/shortcutsData", TEncoding::UTF8);
}

//---------------------------------------------------------------------------
																	  // just constructor
Shortcut::Shortcut(UINT VkCode, int Col, int Row, bool Shift, bool Ctrl, bool Alt, bool Win,  bool Global
	, UnicodeString FunctionName, UnicodeString KeysCaption)
{
	 vkCode = VkCode;
	 col = Col;
	 row = Row;
	 shift = Shift;
	 ctrl= Ctrl;
	 alt = Alt;
     win = Win;
	 global = Global;
	 functionName = FunctionName;
	 keysCaption = KeysCaption;

}

//---------------------------------------------------------------------------

int Shortcut::findByColRow(int Col, int Row)   // returns index ind vector (-1 if not found)
{
	for(int i=0; i < shortcutsData.size(); i++)
	{
		if(shortcutsData[i].col == Col && shortcutsData[i].row == Row)
			return i;                               // found!
	}

	return -1;          // not found!
}

//---------------------------------------------------------------------------

void __fastcall TsetForm::sgSetEditText(TObject *Sender, int ACol, int ARow, const UnicodeString Value)
{
	 btnX->SendToBack();                                          // on EXIT editing
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::sgGetEditText(TObject *Sender, int ACol, int ARow, UnicodeString &Value)
{                                                                         // on cell ENTER
    bool plCellClicked = false;
	for(int i=0; i < plButtons.size(); i++)             // pl shortcuts
		if(sg->Cells[0][ARow] == plButtons[i]->Caption && ACol > 2)
			plCellClicked = true;

	if( ACol == 1 && ARow == 4                                       // + mouseScrollbtn
	 || ACol == 1 && sg->Cells[0][ARow] == "Hide / Show SCounter"    // + Esc
	 || sg->Cells[0][ARow] == "   FILTERS"
	 || sg->Cells[0][ARow] == "   PLAYLISTS"
	 || sg->Cells[0][ARow] == "Quick search"
	 || sg->Cells[0][ARow] == "Previous match (Quick search F3)"
	 || sg->Cells[0][ARow] == "Next match (Quick search F3)"
	 || sg->Cells[0][ARow] == "Focus playing/selected/SCS song" && ACol != 2
	 || sg->Cells[0][ARow] == "Shuffle" && ACol == 1                // Shuffle TAB
	 || sg->Cells[0][ARow] == "Count us" && ACol > 2
	 || sg->Cells[0][ARow] == "Find twins" && ACol > 2
	 || sg->Cells[0][ARow] == "Delete bad songs" && ACol > 2
	 || sg->Cells[0][ARow] == "Open file location (of selected song)" && ACol > 2
	 || sg->Cells[0][ARow] == "Create new playlist" && ACol > 2
	 || sg->Cells[0][ARow] == "Previous playlist page" && ACol > 2
	 || sg->Cells[0][ARow] == "Next playlist page" && ACol > 2
	 || sg->Cells[0][ARow] == "Add artists to playlist" && ACol > 2
	 || plCellClicked)
	{   UnicodeString temp = sg->Cells[ACol][ARow];     // keep cells' text
		sg->EditorMode = false;
		sg->Cells[ACol][ARow] = temp;
		return; }


	btnX->Left = sg->CellRect(ACol, ARow).right - btnX->Width + sg->Left +2;
	btnX->Top  = sg->CellRect(ACol, ARow).top + sg->Top +2;

	sg->Refresh();
	btnX->Visible = true;
	btnX->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::sgDblClick(TObject *Sender)
{
	 POINT point;                                                // Double-click  StringGrid
	 GetCursorPos(&point);
	 int mouseLeft = point.x - setForm->Left - sg->Left - PageControl1->Left
	 - PageControl1->ActivePage->Left -10;
	 int mouseTop  = point.y - setForm->Top -  sg->Top  - PageControl1->Top
	 - (setForm->Height - setForm->ClientHeight);

	 for(int col=0; col < sg->ColCount; col++)                      // find out which cell was clicked
	 {
		  for(int row=0; row < sg->RowCount; row++)
		  {
			  TRect rect = sg->CellRect(col, row);
			  if( mouseLeft > rect.left && mouseLeft < rect.right
			   && mouseTop >  rect.top  && mouseTop  < rect.bottom)
			  {
				 if(!col || !row)      // don't enter if fixed row/col
					 return;

				 if(sg->Cells[0][row] == "Focus playing/selected/SCS song" && col != 2)   // + mouseScrollbtn
				 { messageForm->showMessage("Only 'Key 2' assignable for this function.", 'i', 2);
				   return; }

				 if( sg->Cells[0][row] == "Hide / Show SCounter" && col == 1   // + Esc
				  || sg->Cells[0][row] == "Shuffle" && col == 1                // Shuffle TAB
				  || sg->Cells[0][row] == "   FILTERS"
				  || sg->Cells[0][row] == "   PLAYLISTS"
				  || sg->Cells[0][row] == "Quick search"
				  || sg->Cells[0][row] == "Previous match (Quick search F3)"
				  || sg->Cells[0][row] == "Next match (Quick search F3)")
				 { messageForm->showMessage("This shortcut is not assignable.", 'i', 2);
				   return; }

				 if(col > 2 && (                                  // global shortcuts
					 sg->Cells[0][row] == "Count us"
				  || sg->Cells[0][row] == "Find twins"
				  || sg->Cells[0][row] == "Delete bad songs"
				  || sg->Cells[0][row] == "Open file location (of selected song)"
				  || sg->Cells[0][row] == "Create new playlist"
				  || sg->Cells[0][row] == "Previous playlist page"
				  || sg->Cells[0][row] == "Next playlist page"
				  || sg->Cells[0][row] == "Add artists to playlist"))
                 { messageForm->showMessage("Global shortcut for this function cannot be set.", 'i', 2);
				   return; }

				 for(int i=0; i < plButtons.size(); i++)             // pl shortcuts
					if(sg->Cells[0][row] == plButtons[i]->Caption && col > 2)
					{
						ShowMessage("Playlist can have only 2 hotkeys."
						"\nThe double-pressed key will add playing song to according playlist."
						"\nPressing the key with modifier key1 && modifier key2 will add selected songs"
						" to according playlist."
						"\n\nIf SCounter is minimized, pressing mod.key1 && mod.key2 with the key will"
						"switch to according playlist."
						"\nIf SCounter is minimized, shortcut above pressed twice will copy"
						" playing song to according playlist.");
                        return;
                    }

				 sg->Col = col;
				 sg->Row = row;
				 sg->EditorMode = true;

				 return;
			  }
		  }
	 }
}
//---------------------------------------------------------------------------

UnicodeString giveCellNameShift(bool alt, bool ctrl, bool shift, bool win)
{
	UnicodeString str = "";
	std::string strings[4] = { "Alt", "Ctrl", "Shift", "Win" };
	bool  bools[4] =		 { alt,    ctrl,   shift,   win  };

	for(int i=0; i < 4; i++)
	{
		if(!bools[i]) continue;           // make "Ctrl + Shift + " string
		str += strings[i].c_str();
		str += " + ";
	}

	return str;
}

void __fastcall TsetForm::sgKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(!lWinPressing && Key == vkLWin)       // memorize lwin key state
		lWinPressing = true;

	if(Key == vkEscape) { sg->EditorMode = false; return; }  // exit cell editing

	if(sg->Focused() && !sg->EditorMode)                     // allow navigation keys to navigate
	{
		if(Key == vkHome && Shift.Contains(ssCtrl))     // proper Home functioning
			sg->Row = 1;
		if(Key == vkHome)
			sg->Col = 1;
		return;
    }

	if(!sg->EditorMode) { Key = 0; return; }                 // another exit
	if(Key == vkMenu || Key == vkShift || Key == vkControl || Key == vkLWin) {
		Key = 0;
		return;     // It's not "D, F, 7" btn!
    }


	for(UINT i=0; i < plButtons.size(); i++)      // if user tries to assing pl shortcut with MOD key
	{
	   const UnicodeString &selHotkey = sg->Cells[0][sg->Row];

	   if( (selHotkey == "Focus playing/selected/SCS song" || selHotkey == plButtons[i]->Caption )
		&& (Shift.Contains(ssCtrl)|| Shift.Contains(ssShift) || Shift.Contains(ssAlt) || lWinPressing))
		{
			messageForm->showMessage("You can assign only \"one-key\" shortcut for this function.", 'i', 2);
			return;
		}
    }


	bool assigningPlKey = false;
    UnicodeString cellName;
	cellName = giveCellNameShift(Shift.Contains(ssAlt), Shift.Contains(ssCtrl)
			  , Shift.Contains(ssShift), lWinPressing);

	UnicodeString name = GetKeyName(Key);                                        // get vkCode as string
		if(Key == vkMediaPlayPause) name = L"Play Media";
		if(Key == vkMediaStop)      name = L"Stop Media";
		if(Key == vkMediaPrevTrack) name = L"Previous Media";
		if(Key == vkMediaNextTrack) name = L"Next Media";




	for(UINT i=0; i < shortcutsData.size(); i++)                // is that shortcut already using?
	{
		UnicodeString cellNameAdditional;         // second "cell name" 'Shift + B' for pl e.g.

		if(shortcutsData[i].functionName == "Focus playing/selected/SCS song") // user is assigning NORM shortcut
			cellNameAdditional = giveCellNameShift(false, false, true, false);
		else if(sg->Cells[0][sg->Row] == "Focus playing/selected/SCS song")    // user is assigning "Shift+F"
			cellNameAdditional = "Shift + " + name;
		else
		{                                                // is this "PLAYLIST" type?
			for(UINT j=0; j < plButtons.size(); j++)
			{
				if(shortcutsData[i].functionName == plButtons[j]->Caption) { // user is assigning NORM shortcut
					std::vector <bool> shiftVec = getShiftNameForPlaylists(false, false, false, false);
					cellNameAdditional = giveCellNameShift(shiftVec[0], shiftVec[1], shiftVec[2], shiftVec[3]);
				}
				else if (sg->Row > 34 )  // <- this is very bad code. 35 row and higher - playlists
				{                        // user is assigning PLAYLIST hotkey
					std::vector <bool> modKeys = getShiftNameForPlaylists(false,false,false,false);
					cellNameAdditional = giveCellNameShift(modKeys[0], modKeys[1], modKeys[2], modKeys[3])
															 + name;
					assigningPlKey = true;
                }
			}
		}

                                                              // SHORTCUT IS ALREADY USED!! SHOW MSG!
		if(shortcutsData[i].vkCode      // (check only active shortcuts)
		  && (cellName + name == cellNameAdditional + shortcutsData[i].keysCaption // user is assigning NORM shortcut
		  || cellName + name == shortcutsData[i].keysCaption                       // user is assigning NORM shortcut
		  || cellNameAdditional == shortcutsData[i].keysCaption ))   // user is assigning "Shift+F" OR playlist HOTKEY
		{
			 messageForm->showMessage("["+cellName+name+"]" + "  already using for: "
			 + shortcutsData[i].functionName, 'i',2);
			 return;
		}
	}

	if (name != "Error" && sg->EditorMode)
	{
		sg->Cells[sg->Col][sg->Row] = cellName + name;           // ADD HOTKEY TO THE VEC (OR EDIT EXISTING)

		int shortcutIndexVec = shortcutsData[0].findByColRow(sg->Col, sg->Row);

		Shortcut tempShortcut(0,0,0,0,0,0,0,0,0,0);
		if(shortcutIndexVec == -1) {
			shortcutIndexVec = shortcutsData.size();
			shortcutsData.push_back(tempShortcut);
		}
		else
			shortcutsData[shortcutIndexVec] = tempShortcut; // overwrite old shortcut

		if(Shift.Contains(ssAlt))    shortcutsData[shortcutIndexVec].alt   = true;   // if mod.key is pressed
		else                         shortcutsData[shortcutIndexVec].alt   = false;
		if(Shift.Contains(ssCtrl))   shortcutsData[shortcutIndexVec].ctrl  = true;
		else                         shortcutsData[shortcutIndexVec].ctrl  = false;
		if(Shift.Contains(ssShift))  shortcutsData[shortcutIndexVec].shift = true;
		else                         shortcutsData[shortcutIndexVec].shift = false;
		if(lWinPressing)             shortcutsData[shortcutIndexVec].win   = true;
		else                         shortcutsData[shortcutIndexVec].win   = false;

		if(assigningPlKey)                                  // for pl hotkeys only
			shortcutsData[shortcutIndexVec].assignModKeys();

		shortcutsData[shortcutIndexVec].col = sg->Col;           // apply info to the objects
		shortcutsData[shortcutIndexVec].row = sg->Row;
		shortcutsData[shortcutIndexVec].vkCode = Key;
		if(sg->Col > 2 || assigningPlKey) shortcutsData[shortcutIndexVec].global = true;
		else            shortcutsData[shortcutIndexVec].global = false;
		shortcutsData[shortcutIndexVec].keysCaption = cellName + name;
		shortcutsData[shortcutIndexVec].functionName = sg->Cells[0][sg->Row];
                                                                                       // GLOBAL
		shortcutsData[0].unregisterShortcut(shortcutsData[shortcutIndexVec]);      // unregister shortcut
		shortcutsData[0].registerShortcut(shortcutsData[shortcutIndexVec]);        // register shortcut
	}

	sg->EditorMode = false;
	Key = 0;
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::btnXMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)
{
   if(Button != TMouseButton::mbLeft) return;

   sg->EditorMode = false;              	// exit editor
   sg->Cells[sg->Col][sg->Row]  = "";       // reset caption
															  // deactevate shortcut!
   int index = shortcutsData[0].findByColRow(sg->Col, sg->Row);

   while(index > -1) {
	   shortcutsData[0].unregisterShortcut(shortcutsData[index]);
	   shortcutsData.erase(shortcutsData.begin() + index);
       index = shortcutsData[0].findByColRow(sg->Col, sg->Row);
    }

   btnX->SendToBack();
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::sgSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{                                                                 // 0 COLUMN IS FIXED!
	if(!ACol)  CanSelect = false;
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::FormClose(TObject *Sender, TCloseAction &Action)  // on Close
{
   Shortcut::saveShortcuts();
}
//---------------------------------------------------------------------------



void insertRow(int index, TStringGrid *grid)
{
   grid->RowCount ++;
   for(int i=grid->RowCount -1; i > index; i--)
	  grid->Rows[i] = grid->Rows[i -1];
   grid->Rows[index]->Clear();
}
void __fastcall TsetForm::btnAddRowClick(TObject *Sender)
{
   insertRow(sg->Row, sg);
}
//---------------------------------------------------------------------------

void DeleteRowFromStringGrid(TStringGrid *Grid, int ARow)
{
		int count = Grid->RowCount;

		if ( count - Grid->FixedRows <= 1 ) return;

		// --------------------------- ------------------------
        for ( ARow; ARow < count; ARow++ )
        {
                Grid->Rows[ARow] = Grid->Rows[ARow+1];
        }
        // --------------------------------------------------------------------------

        Grid->RowCount--;
        Grid->SetFocus();
}
void __fastcall TsetForm::btnDeleteRowClick(TObject *Sender)
{
   DeleteRowFromStringGrid(sg, sg->Row);
}
//---------------------------------------------------------------------------







void __fastcall TsetForm::lvBackupChange(TObject *Sender, TListItem *Item, TItemChange Change)
{
	if(!btnRestoreBackup->Enabled && lvBackup->SelCount)
		btnRestoreBackup->Enabled = true;
	else if(btnRestoreBackup->Enabled && !lvBackup->SelCount)
		btnRestoreBackup->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::btnRestoreBackupClick(TObject *Sender)
{
	//  show warn message to user
	if(IDNO == MessageBoxW(NULL, L"You are going to restore settings. All current settings will be lost."
	"\nContinue?", L"", MB_YESNO))
		return;

	std::vector <UnicodeString> backupNames = SrcFolder::getFileNames(pl.getSettingsPath() + "BACKUP");

	SrcFolder setDir(backupNames[lvBackup->ItemIndex] + "\\settings");
	SrcFolder plDir(backupNames[lvBackup->ItemIndex] + "\\playlists");

	setDir.copyToAnotherFolder(pl.getSettingsPath());
	plDir.copyToAnotherFolder(pl.getSettingsPath());

   	Shortcut::unregisterAllShortcuts();

//	af.loadAllSongsData();                            // load data from AllSongs.txt
	AllSong::loadFromFile();
	// NEW CODE
	pl.loadPlaylistSettings();
	pl.loadPlaylist(pl.activePlaylist);               // load last playlist condition
	pl.loadForOtherForms('m');
    pl.loadForOtherForms('s');
	pl.loadPlaylistButtons();
	pl.refreshSongCounter("comm");

	messageForm->showMessage("Settings restored succesfully!", 'i', 2);
}
//---------------------------------------------------------------------------


void __fastcall TsetForm::cboxAllPlClick(TObject *Sender)
{                             // checked property changes BEFORE this function
	 if(programIsCreating)
		return;

	 if(plButtons.size() == 1 && !cboxAllPl->Checked) {      // don't delete last playlist
		   programIsCreating = true;
			cboxAllPl->Checked = true;
			messageForm->showMessage("SCounter should have at least 1 playlist.", 'i', 1);
           programIsCreating = false;
			return;
	 }

	 if(cboxAllPl->Checked) {                   // enable allSongs pl
		  mform->addPlaylist("All songs");
	 }
	 else  {
		 mform->popRClick->Tag = 0;                        // make tag to know which pl was clicked
		 mform->Delete1Click(NULL);
	 }

}
//---------------------------------------------------------------------------



void __fastcall TsetForm::WMSysCommand(TMessage &Msg)
{

}
//---------------------------------------------------------------------------



void __fastcall TsetForm::sgKeyPress(TObject *Sender, System::WideChar &Key)
{
	 ShowMessage("gotcha!");
}
//---------------------------------------------------------------------------

void __fastcall TsetForm::cboxMod1Select(TObject *Sender)
{                                                        // prevent 2 same indexes
	TComboBox *currCbox = dynamic_cast <TComboBox*> (Sender);
	TComboBox *oppositeCbox;

	if(currCbox->Name == "cboxMod1") {                // assigning cboxes mod1 and mod2
		currCbox = cboxMod1;
		oppositeCbox = cboxMod2;
    }
	else {
		currCbox = cboxMod2;
		oppositeCbox = cboxMod1;
    }


	if(currCbox->ItemIndex == oppositeCbox->ItemIndex)        // change opposite cbox item
	{
		if(oppositeCbox->ItemIndex +1 < oppositeCbox->Items->Count)
			oppositeCbox->ItemIndex ++;
		else
			oppositeCbox->ItemIndex --;
	}

    bool shiftStates[4] = {0,0,0,0};                        // get modkey1+modkey2 states
	for(int i=0; i < 4; i++)
		if(i == cboxMod1->ItemIndex || i == cboxMod2->ItemIndex)
			shiftStates[i] = true;

	UnicodeString shiftCaption =                                       // get "Ctrl + Shift +" caption
		giveCellNameShift(shiftStates[2], shiftStates[0], shiftStates[1], shiftStates[3]);


	bool newShortcutsOk = true;
	for(UINT i=0; i < shortcutsData.size(); i++)            // CHECK FOR SAME SHORTCUTS
	{
		if(shortcutsData[i].vkCode && shortcutsData[i].row > 34) // it's active shortcut for pl
		{
			UnicodeString cellName = shiftCaption + shortcutsData[i].keysCaption;
			for(UINT j=0; j < shortcutsData.size(); j++)
				if(cellName == shortcutsData[j].keysCaption                             // same keysCaption!
				   && shortcutsData[j].vkCode && i != j && shortcutsData[j].row < 34 )  // some guards
				{
					newShortcutsOk = false;
//					ShowMessage(shortcutsData[j].functionName + " cellName: " + cellName);  	//DEBUG
                }
		}
	}

	if(!newShortcutsOk)            // oops, user can't change mod keys to these values, 'cus they're using
	{
		cboxMod1->ItemIndex = oldModkey1Index;    // restore values
		cboxMod2->ItemIndex = oldModkey2Index;

		messageForm->showMessage("These modifier keys cannot be set, because some function(s) using them",
								'e',2);
		return;
	}

										 // UNREGISTER / REGISTER ALL PL HOTKEYS
	for(UINT i=0; i < shortcutsData.size(); i++)
	{
		if(shortcutsData[i].vkCode && shortcutsData[i].row > 34)
		{
			shortcutsData[i].assignModKeys();  // change shift states

			if(!mform->Visible) {
				shortcutsData[i].unregisterShortcut(shortcutsData[i]);
				shortcutsData[i].registerShortcut(shortcutsData[i]);
            }
		}
    }
}
//---------------------------------------------------------------------------



void __fastcall TsetForm::sgMouseWheelDown(TObject *Sender, TShiftState Shift, TPoint &MousePos,
          bool &Handled)
{
	short scrollBy = 4;
	 if(sg->Row +scrollBy < sg->RowCount)
		sg->Row += scrollBy;

}
//---------------------------------------------------------------------------


void __fastcall TsetForm::sgMouseWheelUp(TObject *Sender, TShiftState Shift, TPoint &MousePos,
		  bool &Handled)
{
	short scrollBy = -4;
	 if(sg->Row +scrollBy > 0)
		sg->Row += scrollBy;
}
//---------------------------------------------------------------------------


std::vector <bool> TsetForm::getShiftNameForPlaylists(bool userPressedAlt, bool userPressedCtrl
													, bool userPressedShift, bool userPressedWin)
{
	std::vector <bool> returnVec;

	if (setForm->cboxMod1->ItemIndex == ModifierKey::Alt       // Alt
	  ||setForm->cboxMod2->ItemIndex == ModifierKey::Alt)
		returnVec.push_back(true);
	else
		returnVec.push_back(userPressedAlt);

	if (setForm->cboxMod1->ItemIndex == ModifierKey::Ctrl       // ctrl
	  ||setForm->cboxMod2->ItemIndex == ModifierKey::Ctrl)
		returnVec.push_back(true);
	else
		returnVec.push_back(userPressedCtrl);


	if (setForm->cboxMod1->ItemIndex == ModifierKey::Shift       // shift
	  ||setForm->cboxMod2->ItemIndex == ModifierKey::Shift)
		returnVec.push_back(true);
	else
		returnVec.push_back(userPressedShift);


	if (setForm->cboxMod1->ItemIndex == ModifierKey::Win       // Win
	  ||setForm->cboxMod2->ItemIndex == ModifierKey::Win)
		returnVec.push_back(true);
	else
		returnVec.push_back(userPressedWin);

	return returnVec;
}


void Shortcut::assignModKeys()     // this function for playlists only. It gets shift  states from setForm
{
	bool shiftStates[4] = {false,false,false,false};

	shiftStates[setForm->cboxMod1->ItemIndex] = true;
    shiftStates[setForm->cboxMod2->ItemIndex] = true;

	  this->ctrl = shiftStates[0];
	  this->shift = shiftStates[1];
	  this->alt = shiftStates[2];
	  this->win = shiftStates[3];
}

void __fastcall TsetForm::cboxMod1Enter(TObject *Sender)
{
	 oldModkey1Index = cboxMod1->ItemIndex;        // memorize this indexes for onSelect()
     oldModkey2Index = cboxMod2->ItemIndex;
}
//---------------------------------------------------------------------------


void Shortcut::registerAllShortcuts()
{
   	for(UINT i=0; i < shortcutsData.size(); i++)
		if(shortcutsData[i].vkCode && shortcutsData[i].global)
			shortcutsData[0].registerShortcut(shortcutsData[i]);
}

void Shortcut::unregisterAllShortcuts()
{
	for(UINT i=0; i < shortcutsData.size(); i++)
		if(shortcutsData[i].vkCode && shortcutsData[i].global)
			shortcutsData[0].unregisterShortcut(shortcutsData[i]);
}



void __fastcall TsetForm::listViewDrawItem(TCustomListView *Sender, TListItem *Item, TRect &Rect,
          TOwnerDrawState State)
{
	TListView *lv = dynamic_cast<TListView*>(Sender);
	TCanvas *canvas = Sender->Canvas;
	int index = Item->Index;

	int indexInAllS = Item->Index;   // NEW CODE indexInAllS = index;
	if(indexx.size())
		indexInAllS = indexx[index];
//	if(indexx.size() == 0)
//	   indexInAllS = index*4;
//	else
//	   indexInAllS = indexx[index]*4;

	canvas->Brush->Color = RGB(25,29,34);               // LV usual color
	canvas->Font->Color = RGB(210,210,210);             // font (WHITE) for usual songs

	if(lv->SelCount && lv->Items->Item[index]->Selected)
	   canvas->Brush->Color = RGB(75,75,75);    // selectedSong (GRAY)

	canvas->FillRect(Rect);
	canvas->Brush->Style = bsClear;
	Rect.Right = lv->Columns->Items[0]->Width;

	UnicodeString text1 = allSongs[indexInAllS].name();              // set text of 1 column (name)
	// NEW CODE
	canvas->TextRect(Rect, Rect.Left+7, Rect.Top+1, text1);

						  // draw a column
	Rect.Left = Rect.Left + lv->Columns->Items[0]->Width + 1;
	Rect.Right = Rect.Left + lv->Columns->Items[1]->Width;

	UnicodeString text;									   // set text of 2,3 column (name)
	text = allSongs[indexInAllS].playCount();
		// text = allSongs[indexInAllS].playCount();

	short center = Rect.Left + (Rect.Right - Rect.Left - canvas->TextWidth(text))/2;
	canvas->TextRect(Rect, center , Rect.Top+1, text);
}
//---------------------------------------------------------------------------




void __fastcall TsetForm::cboxShowEndPlMsgClick(TObject *Sender)
{
    // show/don't show msg "End of playlist"
	pl.showTheEnd = !pl.showTheEnd;
}
//---------------------------------------------------------------------------

