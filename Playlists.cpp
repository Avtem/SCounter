#include "Playlists.h"

// ============== my includes ===================== //
#include "ssetings.h"
#include <locale>
#include <fstream>
#include "AvId3Reader.h"             // my id3 v2.4 tags ^^
#include <dirent.h>                  // for reading file names from folder
#include "PlaylistWindow.h"          // ampersandChanger()
#include "Songs.h"
#include "enums.h"                   // enums
#include "Unit1.h"
#include "AllSong.h"
#include "PlayDatesInfo.h"


bool songEnded = false;  // used only endSong()
const int skippingMinTime = 300;
std::vector <int> lastSongs;
extern std::vector <TButton*> plButtons;                // SCouner playlists
extern short plColWidthMin;
extern short plColWidthMax;
extern AddingFiles af;
extern bool maximized;
extern void maximizeMform();                             // maximize mform
extern void  restoreMform();
extern FocusBtnState currFocusBtnState;       // which state has btn [FOCUS]?
extern bool badSongsCalling;                  // using for "findDoubles"
extern UnicodeString intToUS(const int &integer);  	// conver int to US string
extern std::vector <AllSong> allSongs;   // here we store all data.
extern std::vector <UnicodeString> indArtists; 		   // INDEXATION AS
extern std::vector <std::vector <int> > indArtIndexes; // INDEXATION AS

Playlists::Playlists()                                                                     // just constructor
{
   selectedSong = -1;    // index of currently selected song
   playingSong = -1;     // index of currently playing song
   actPlayingSong = -1;  // index of currently playing song
   playing = false;      // is player playing?
   stopped = true;
   iterated = false;
   terminatingSong = false;     // are we closing when song ends?
   showTheEnd = true;    // show message (the end)

   playlistData = new TStringList;

   UnicodeString path = getSettingsPath(); path += L"settings/config.ini";
   iniConfig = new TIniFile(path);
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::findDoubles(bool show)                                                 // on findDoublesClick
{
	{
	  std::vector <UnicodeString> tempVector;                  // Indexation of songs
	  for(int i=0; i < songs.size(); i++)
		 tempVector.push_back(songs[i].artist.LowerCase());

	  af.Indexation(tempVector, af.artBef, af.artIndexesBef);
	}

	if(!badSongsCalling)
		af.sameAdded.clear();           // cccclear!
	af.samePlaylist.clear();
	af.samePlaylist2.clear();


  bool inArtistsVect = false;
	for(int i=0; i < songs.size(); i++)   // check EVERY element from playlist
	{
	  for(int z=0; z < af.sameAdded.size(); z++)      // skip already founded doubles
		if(songs[i].path == af.sameAdded[z])  { inArtistsVect = true; break; }

	  if(inArtistsVect == false)
	  {
		UnicodeString origArt  = songs[i].artist.LowerCase();
		UnicodeString origName = songs[i].name.LowerCase(); // LOWERCASE!!!

		  for(int j=0; j < af.artBef.size(); j++)          // seeking for number in vector
		  {
			  if(origArt == af.artBef[j])
			  {
				for(int k=0; k < af.artIndexesBef[j].size(); k++)     // check ALL indexes
				{
				  if(i < af.artIndexesBef[j][k] &&
				  origName == songs[af.artIndexesBef[j][k]].name.LowerCase())  // SAME SONG!!!
				  {
					  af.samePlaylist.push_back(i);  							  // index of first file
					  af.sameAdded.push_back(songs[af.artIndexesBef[j][k]].path); // path of second file

						af.samePlaylist2.push_back(af.artIndexesBef[j][k]);       // for DeleteBadSongs
				  }                                           // no break here!!
				}
				break;
			  }
		  }
	  }

	  inArtistsVect = false;
	}
																 // and show duplicates
	   Form2->Label1->Caption = "Original song:"; Form2->Label2->Caption = "Duplicate:";
	if(show)
	{
	  if(af.samePlaylist.size() == NULL)
	     messageForm->showMessage(L"There are no twins", 'i', 2);

	  af.showSameSongs();
    }
	   Form2->Label1->Caption = "Song in playlist:"; Form2->Label2->Caption = "Adding song:";
}

/////////////////////////////////////////////////////////////////////////////////////////

UnicodeString Playlists::getLastPlName()    				// gets "My playlist" without \/ * : ...
{
   TStringList *plName = new TStringList;                             // last playlist name
   plName->LoadFromFile(getSettingsPath() + "playlists/lastPlaylistName");

   return plName->Strings[0];
}


/////////////////////////////////////////////////////////////////////////////////////////

UnicodeString Playlists::delNonPathChars(UnicodeString name)  const   // deletes chars   \/ * : ?  <> " |
{                                                               	  //  from string
   for(int i=name.Length(); i >= 1 ; i--)
   {
	   if(name.SubString(i, 1) == "\\"
	   || name.SubString(i, 1) == "/"
	   || name.SubString(i, 1) == "*"
	   || name.SubString(i, 1) == ":"
	   || name.SubString(i, 1) == "?"
	   || name.SubString(i, 1) == "<"
	   || name.SubString(i, 1) == ">"
	   || name.SubString(i, 1) == "\""
	   || name.SubString(i, 1) == "|"
	   || name.SubString(i, 1) == "."   // dots!  (added later than other chars)
	   || name.SubString(i, 1) == " ")  // + spaces!!!

			name.Delete(i, 1);
   }

   return name;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::loadPlaylist(UnicodeString name)	        	 				 		   // loads playlist
{
    // name should be without <>|*?/\"
   UnicodeString folderPath = getSettingsPath() + "playlists/" + name + "/";
   int firstInactiveCopy = pl.firstInactive;

   mform->clearbtnClick(NULL);      // clear playlist
   playlistData->Clear();
   if(!FileExists(folderPath + "playlistData"))
   {
	   ShowMessage("Can not load the playlist");
       return;
   }
   playlistData->LoadFromFile(folderPath + "playlistData");          // load data from .txt

   int loadItems=7;
   for(int i=0; i < playlistData->Count; i+=loadItems)
   {
	   Songs newSong(playlistData->Strings[i]              // title
					,playlistData->Strings[i+1]            // artist
					,playlistData->Strings[i+2]            // path
					,playlistData->Strings[i+3]            // duration
					,playlistData->Strings[i+4]            // date
					,playlistData->Strings[i+5].ToInt()    // listenings
					,playlistData->Strings[i+6].ToInt() ); // indexNorm

	   songs.push_back(newSong);
   }

   if(firstInactiveCopy <= songs.size())         // check whether first inactive is correct.
	  pl.firstInactive = firstInactiveCopy;      // if it is not - set to 0 (because in some cases SCounter
   else                                          // won't run)
      pl.firstInactive = 0;

   TStringList *tempList = new TStringList;             // SCS data
   tempList->LoadFromFile(folderPath + "SCSdata");
   for(int i=0; i < tempList->Count; i+=2)
   {
	  started[i/2] = tempList->Strings[i].ToInt();              //  started[5]         // AA 11 22 33 44
	  shuffleCurrSong[i/2] = tempList->Strings[i+1].ToInt();    //  shuffleCurrS[5]
   }
   delete tempList;

										   // order[5] indexes
   for(int i=0; i < 5; i++)
   {
	   if(started[i])
	   {
		 UnicodeString path = folderPath + "orders/";
		 path += i;

		 playlistData->Clear();
		 playlistData->LoadFromFile(path);

		 for(int j=0; j < playlistData->Count; j++)
			songs[j].order[i] = playlistData->Strings[j].ToInt();
	   }
   }

   if (songs.size() == 0) shuffleCurrSong[mform->filterbtn->Tag] = 0;      // SCS 288
   else mform->currCounter->Caption = shuffleCurrSong[mform->filterbtn->Tag];

   mform->playlist->Items->Count = songs.size();           // show playlist
   makeVisibleCurrSong(0,0);

   mform->delayTimer->Enabled = true;	 //  refresh shuffle indexes + check listenings
   mform->plLabel->Caption = getActivePlFullName();

																				// playlist width
   if(songs.size() > mform->playlist->VisibleRowCount) mform->playlist->Column[3]->Width = plColWidthMin;
   else 										 	   mform->playlist->Column[3]->Width = plColWidthMax;
}

/////////////////////////////////////////////////////////////////////////////////////////


void Playlists::savePlaylist(UnicodeString name)    							           // saves playlist
{                                                                                          // + order, SCS
   name = delNonPathChars(name);      // delete <>|*?/\"
   UnicodeString folderPath = getSettingsPath() + "playlists/" + name + "/";

   if(!DirectoryExists(folderPath))    			// create "Default" folder
	  CreateDir(folderPath);
   if(!DirectoryExists(folderPath + "orders"))  // create "orders" folder
	  CreateDir(folderPath + "orders");

   playlistData->Clear();

   for(int i=0; i < songs.size(); i++)
   {
	  playlistData->Add(songs[i].title);
	  playlistData->Add(songs[i].artist);
	  playlistData->Add(songs[i].path);
	  playlistData->Add(songs[i].duration);
	  playlistData->Add(songs[i].date);
	  playlistData->Add(songs[i].listenings);
	  playlistData->Add(songs[i].indexNorm);
   }
   playlistData->SaveToFile(folderPath + "playlistData", TEncoding::UTF8);


   TStringList *tempList = new TStringList;             // SCS data
   for(int i=0; i < 5; i++)
   {
	  tempList->Add(started[i]+0);       //  started[5]         // 00 11 22 33 44
	  tempList->Add(shuffleCurrSong[i]); //  shuffleCurrS[5]
   }
   tempList->SaveToFile(folderPath + "SCSdata");
   delete tempList;

  for(int i=0; i < 5; i++)                                    // order[5]
  {
	 if(started[i])
	 {
	   name = folderPath + "orders/";  name += i;

	   playlistData->Clear();
	   for(int j=0; j < songs.size(); j++)                // fill with order indexes
			playlistData->Add(songs[j].order[i]);

	   playlistData->SaveToFile(name);
	 }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

UnicodeString Playlists::refreshingDuration()          								   // refresh adur [00:34]
{
	if(songEnded)
	{
        songEnded = false;
		endSong();
    }

	if(playingSong > -1 || (BASS_ChannelIsActive(stream) && actPlayingSong == -1) ) //show currDuration (0:43)
	  return songs[0].getDuration(stream, 0);
	else return "0";
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::makeVectorOfRandInts(std::vector <int> &givenVector       // Fill given vector by random ints
, int firstIndex, int lastIndex, int srandValue)
{
   givenVector.clear();

   std::vector <int> tempVector;                    // fill vector like: {-2,-1,0,1,2,3,4}
   for(int i = firstIndex; i < lastIndex; i++)
		tempVector.push_back(i);

		std::vector <int> tempVector2; // 2-nd part of vector
		std::vector <int> lastIndexes; // here indexes for "inserting" songs
		if(lastSongs.size())
		{
		   std::sort(lastSongs.begin(), lastSongs.end());
		   for(int i=0; i < lastSongs.size(); i++)
		   { tempVector2.push_back(i);
			 tempVector.erase(tempVector.begin()); }
		}


   srand(time(NULL) + srandValue);

   int range = tempVector.size();

   for(int i=0; i < tempVector2.size(); i++)
   {
		int randIndex = random(range);
		lastIndexes.push_back(tempVector[randIndex]);
		tempVector[randIndex] = tempVector[range-1];
		range--;
   }

   for(int j=0; j < tempVector2.size(); j++) // join tempV1+tempV2
	   tempVector[j+tempVector.size()-tempVector2.size()] = tempVector2[j];

   range = tempVector.size();
   for(int i=0; i < tempVector.size(); i++)
   {
		int randIndex = random(range);
		givenVector.push_back(tempVector[randIndex]);
		tempVector[randIndex] = tempVector[range-1];
		range--;
   }

		for(int i=0; i < lastSongs.size(); i++)
		   givenVector.insert(givenVector.begin()+lastSongs[i], lastIndexes[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::filter(short choice)                                                  // filters playlist
{
   short currTag = mform->filterbtn->Tag;
   mform->currCounter->Caption = shuffleCurrSong[currTag];

   if(!af.deleting) falseStop();                     // if af.deleteFiles calls, don't stop

   int min = filterMin[choice-1], max = filterMax[choice-1];   // set MIN and MAX
   int count = songs.size();           // count of playlist
   mform->playlist->Selected = false;
   selectedSong = -1;

   TMenuItem *popUpItem = mform->filterPopUp->Items->Items[choice];

   mform->filterbtn->Caption = popUpItem->Caption;
   mform->filterbtn->Tag = popUpItem->Tag;

   if(mform->shufflebtn->Tag) mform->currCounter->Caption = shuffleCurrSong[choice];

	 if(choice == 0 && mform->shufflebtn->Tag) { firstInactive = songs.size(); sortSongs(2, true); return; }
	 if(choice == 0)     { firstInactive = songs.size(); sortSongs(0, true); return; }


   if(!mform->shufflebtn->Tag) sortSongs(0, false);      // if shuffle is OFF
   firstInactive = 0;

   std::vector <int> tempFilteredOrder;
   for(int i=0; i < count; i++)         // filtering
   {
	  if(songs[i].listenings < min               // bad song
	  || songs[i].listenings > max)
	  {
		  tempFilteredOrder.push_back(i);
	  }
	  else
	  {                                          // good song
		  tempFilteredOrder.insert(tempFilteredOrder.begin()+firstInactive, i);
		  firstInactive ++;
	  }
   }

   std::vector<int> copyOfFilteredOrder (songs.size());            // rearrange
   for(int i=0; i < tempFilteredOrder.size(); i++)
	   copyOfFilteredOrder[tempFilteredOrder[i]] = i;

   for(int i=0; i < tempFilteredOrder.size(); i++)
	   songs[i].indexFiltered = copyOfFilteredOrder[i];

   sortSongs(1, true);                        // sort songs

   if(mform->editQSearch->Focused()) mform->playlist->SetFocus();            // disable f3 search
   mform->labelQSearch->Visible = false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::filterCopy(short choice)                                              // filters Copy method
{
   if(!choice) return;          // choice = 1,2,3,4 only        // IS THAT DANGEROUS?

   int min = filterMin[choice-1], max = filterMax[choice-1];   // set MIN and MAX
   int count = songs.size();           // count of playlist

   std::vector <int> tempFilteredOrder;

   for(int i=0, z=0; i < count; i++)         // filtering
   {
	  if(songs[i].listenings < min                // bad song
	  || songs[i].listenings > max)
		  tempFilteredOrder.push_back(i);
	  else										  // good song
	  {
		  tempFilteredOrder.insert(tempFilteredOrder.begin()+z, i);
		  z++;
      }
   }

   std::vector<int> copyOfFilteredOrder (songs.size());            // rearrange
   for(int i=0; i < tempFilteredOrder.size(); i++)
	   copyOfFilteredOrder[tempFilteredOrder[i]] = i;

   for(int i=0; i < tempFilteredOrder.size(); i++)
	   songs[i].indexFiltered = copyOfFilteredOrder[i];

   sortSongs(1, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

// "LOAD EVERYTHING"
void Playlists::loadPlaylistSettings()										     // this function for OnCreate
{
   lastVolume = iniConfig->ReadInteger("userSettings", "volume", 100);                    // volume
   mform->volume->Position = lastVolume; BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 100*lastVolume);

   currFocusBtnState = iniConfig->ReadInteger("userSettings", "focusBtnState", 0);     // focus btn
   mform->playingItemClick(mform->pmenuFocusBtn->Items->Items[currFocusBtnState]);

   af.firstLoadingInd = iniConfig->ReadInteger("playlistSettings", "firstLoadingInd", -1);// was pl. loaded?
   af.firstAddedInd = iniConfig->ReadInteger("playlistSettings", "firstAddedInd", -1);    // first D&D index
   mform->filterbtn->Tag = iniConfig->ReadInteger("playlistSettings", "filterTag", 0);    //  filter TAG
   for(int i=0; i < 4; i++)
   {
	 UnicodeString string = L"filterMin"; string += i+1;
	   filterMin[i] = iniConfig->ReadInteger("userSettings", string, 0);
				   string = L"filterMax"; string += i+1;
	   filterMax[i] = iniConfig->ReadInteger("userSettings", string, 0);
   }
   refreshFilterCap();                                                                    // filter CAPTION
   mform->filterbtn->Caption = mform->filterPopUp->Items->Items[mform->filterbtn->Tag]->Caption;

   TMenuItem *popUpItem = mform->filterPopUp->Items->Items[mform->filterbtn->Tag];        // caption of Filter
   mform->filterbtn->Caption = popUpItem->Caption;
   firstInactive = iniConfig->ReadInteger("playlistSettings", "firstInactive", -1);       // firstInactive

   if(iniConfig->ReadBool("userSettings", "autoNext", 1))     // autoNext
		mform->cboxAutoNext->Font->Color = 0x001EC823; // set green
   else mform->cboxAutoNext->Font->Color = 0x00DCDCDC;

   if(iniConfig->ReadBool("playlistSettings", "on", false))                               // if shuffle was ON
   {  mform->shufflebtn->Font->Color = 0x001EC823;               // shuffle GREEN FONT
	  mform->shufflebtn->Tag = 1;                                // shuffle TAG
	  mform->currCounter->Visible = true;                        // currCounter - VISIBLE
	  mform->currCounter->Caption = shuffleCurrSong[mform->filterbtn->Tag];} // currCounter - value

   if(Date().FormatString("dd.mm.yyyy") == iniConfig->ReadString("playlistSettings", "today", 0)) // todays listen.
	  todayListenings = iniConfig->ReadInteger("playlistSettings", "todayListenings", 0);
   else    // it is another day
   {
	 todayListenings = AllSong::getAllListenings(); // af.getAllList(); NEW CODE
	 iniConfig->WriteString("playlistSettings", "today", Date().FormatString("dd.mm.yyyy")); // today's date
	 iniConfig->WriteInteger("playlistSettings", "todayListenings", todayListenings);	   // + listenings
   }

   activePlaylist = getLastPlName();  											  // active playlist name
   showTermSongMes = iniConfig->ReadBool("userSettings", "showTermSongMes", 1);   // showTermSongMes
   if(mform->shufflebtn->Tag) mform->btnSortPlBy->Enabled = false;            // dis. sort btn

	PlayDatesInfo::loadData(getSettingsPath() + "settings/PlayDatesInfo.big");
}
void Playlists::loadForOtherForms(char form)                                               // happens later
{                                                                                 // always show mes
   if(form == 's')
   {
	   setForm->neverAnPPR->Checked = iniConfig->ReadBool("userSettings", "alwaysShowFNFPPR", 1);
	   if(!setForm->neverAnPPR->Checked)
		   setForm->dontAnPPR->Checked = false;

	   // show "End of playlist" msg
	   pl.showTheEnd = iniConfig->ReadBool("userSettings", "showEndPlMsg", 1);
	   setForm->cboxShowEndPlMsg->Checked = pl.showTheEnd;
	   // all songs pl
	   setForm->cboxAllPl->Checked = iniConfig->ReadBool("userSettings", "allSongsPlaylist", 1);
	   // show tray?
	   setForm->toTrayGroup->ItemIndex = iniConfig->ReadInteger("userSettings", "toTray", 1);
//	   setForm->restoreGroup->ItemIndex = iniConfig->ReadInteger("userSettings", "restoreOn", 0);
	   setForm->toTrayGroupClick(NULL);         // enable/disable restore on click

	   setForm->minimizeOnEsc->Checked = iniConfig->ReadInteger("userSettings", "minimizeOnEsc", 1);
	   Shortcut::loadShortcuts();    // load hotkeys data
                                                              // copy selected songs to choosen pl mod keys
	   setForm->cboxMod1->ItemIndex = iniConfig->ReadInteger("userSettings", "modifierKey1", 0);
	   setForm->cboxMod2->ItemIndex = iniConfig->ReadInteger("userSettings", "modifierKey2", 1);
   }
   if(form == 'm')
   {    												// BTN COLORS
	   mform->dbs->Font->Color = 0x00DCDCDC;  // white
	   mform->countusbtn->Font->Color = 0x00DCDCDC;
	   mform->findDoublesbtn->Font->Color = 0x00DCDCDC;
	   mform->clearbtn->Font->Color = 0x00DCDCDC;
	   mform->btnSortPlBy->Font->Color = 0x00DCDCDC;
	   mform->btnMaximize->Font->Color = 0x00DCDCDC;
	   mform->mute->Picture->LoadFromFile("res/img/unmuted.png");

	   maximized = iniConfig->ReadBool("userSettings", "maximized", 0);      // was window minimized / maximized?
	   if(maximized)  maximizeMform();
	   else           restoreMform();
	   mform->Left = iniConfig->ReadInteger("userSettings", "mformLeft", 200);  // restore mform pos
	   mform->Top  = iniConfig->ReadInteger("userSettings", "mformTop", 200);

		mform->tray->Icons = new TImageList(mform->tray);      // load tray icons
		TIcon *MyIcon = new TIcon;
		MyIcon->LoadFromFile("res/img/tray default.ico");      // 0 index
		mform->tray->Icon->Assign(MyIcon);
		mform->tray->Icons->AddIcon(MyIcon);

		MyIcon->LoadFromFile("res/img/tray playing.ico");      // 1 index
		mform->tray->Icons->AddIcon(MyIcon);
		MyIcon->LoadFromFile("res/img/tray paused.ico");       // 2 index
		mform->tray->Icons->AddIcon(MyIcon);
		MyIcon->LoadFromFile("res/img/tray terminating.ico");  // 3 index
		mform->tray->Icons->AddIcon(MyIcon);
		MyIcon->LoadFromFile("res/img/tray terminatingPause.ico");  // 4 index
		mform->tray->Icons->AddIcon(MyIcon);

	   mform->timerDblClick->Interval = GetDoubleClickTime(); // set timer dbl click interval
	   mform->timerDblPress->Interval = GetDoubleClickTime(); // set timer dbl click interval

	   if(GetDoubleClickTime() -150 > 200)
		   mform->timerDblPress->Interval -= 150;

	   Application->HintPause = 200;   Application->HintHidePause = 200000;      // hint
   }
}

void Playlists::savePlaylistSettings()										     // this function for OnClose
{
   iniConfig->WriteBool("playlistSettings", "on", mform->shufflebtn->Tag);                 // shuffleTAG
   iniConfig->WriteInteger("playlistSettings", "firstLoadingInd", af.firstLoadingInd);     // was pl. loaded?
   iniConfig->WriteInteger("playlistSettings", "firstAddedInd", af.firstAddedInd);         // first D&D index
   iniConfig->WriteInteger("playlistSettings", "filterTag", mform->filterbtn->Tag);        // filterTAG
   iniConfig->WriteInteger("playlistSettings", "firstInactive", firstInactive);            // firstInactive

   // autoNext
   iniConfig->WriteBool("userSettings", "autoNext", mform->cboxAutoNext->Font->Color == 0x001EC823);

   iniConfig->WriteBool("userSettings", "showEndPlMsg", showTheEnd);   // show msg "End of playlist"
   iniConfig->WriteBool("userSettings", "showTermSongMes", showTermSongMes); 			   // showTermSongMes
   iniConfig->WriteBool("userSettings", "alwaysShowFNFPPR", setForm->neverAnPPR->Checked); // always show mes?
   iniConfig->WriteBool("userSettings", "maximized", maximized);       		    	       // maximized/minim
   iniConfig->WriteBool("userSettings", "allSongsPlaylist", setForm->cboxAllPl->Checked);  // all songs pl
   iniConfig->WriteInteger("userSettings", "volume", lastVolume);                          // volume
   iniConfig->WriteInteger("userSettings", "toTray", setForm->toTrayGroup->ItemIndex);     // toTray?
   iniConfig->WriteInteger("userSettings", "restoreOn", setForm->restoreGroup->ItemIndex); // restoreON?
   iniConfig->WriteInteger("userSettings", "minimizeOnEsc", setForm->minimizeOnEsc->Checked); // minim.onESC?
   iniConfig->WriteInteger("userSettings", "mformLeft", mform->Left);                      // save pos mform
   iniConfig->WriteInteger("userSettings", "mformTop",  mform->Top);
   iniConfig->WriteInteger("userSettings", "focusBtnState", currFocusBtnState);     // focus btn
   iniConfig->WriteInteger("userSettings", "modifierKey1", setForm->cboxMod1->ItemIndex);  // mod keys
   iniConfig->WriteInteger("userSettings", "modifierKey2", setForm->cboxMod2->ItemIndex);

   setForm->savebtnClick(NULL);     // save hotkeys grid to .txt file

   TStringList *plName = new TStringList;                             // last playlist name
   plName->Add(activePlaylist);
   plName->SaveToFile(getSettingsPath() + "playlists/lastPlaylistName", TEncoding::UTF8);

   for(int i=0; i < 4; i++)
   {                                                                                       // filter values
	 UnicodeString string = L"filterMin"; string += i+1;
	   iniConfig->WriteInteger("userSettings", string, filterMin[i]);
				   string = L"filterMax"; string += i+1;
	   iniConfig->WriteInteger("userSettings", string, filterMax[i]);
   }

   if(playingSong > -1)
   {
	 iniConfig->WriteInteger("playlistSettings", "playingSong", playingSong);              // playingSong
	 iniConfig->WriteInteger("playlistSettings", "selectedSong", -1);
   }
   else
   {
	 iniConfig->WriteInteger("playlistSettings", "selectedSong", selectedSong);            // selectedSong
	 iniConfig->WriteInteger("playlistSettings", "playingSong", -1);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::loadLastSelSong() // what index was when scounter was closed in earlier instance?
{
	   playingSong  = iniConfig->ReadInteger("playlistSettings", "playingSong", -1);          // playingSong
	   selectedSong = iniConfig->ReadInteger("playlistSettings", "selectedSong", -1);         // selectedSong
	   makeVisibleCurrSong(0,0);
}

void Playlists::refreshSongCounter(String input)              // comm - for common counter
{                                                             // , curr - for current pos. in shuffle playlist
	int count = songs.size();

    if(input == "curr")
	{
	   if (count == 0)
		   shuffleCurrSong[mform->filterbtn->Tag] = 0;
	   else if (playingSong < count)
		   shuffleCurrSong[mform->filterbtn->Tag] ++;

	   mform->currCounter->Caption = shuffleCurrSong[mform->filterbtn->Tag];
	}
	else if(input == "comm")
	{
	   if(mform->filterbtn->Caption == "All")
	   {
		  mform->commCounter->Caption = count;
		  mform->letterF->Visible = false;
	   }
	   else
	   {
		 mform->commCounter->Caption = firstInactive;
		 mform->letterF->Visible = true;
	   }
	   mform->labelPlCount->Caption = pl.firstInactive;     // pl count 200/5000
	   mform->labelPlCount->Caption = mform->labelPlCount->Caption + "/";
	   mform->labelPlCount->Caption	= mform->labelPlCount->Caption + songs.size();
	   refreshPlDuration();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::shuffleClick(bool programmatically) 									    // [onShuffleClick]
{
	falseStop();                                        // clearing info from previous playlist
	if(!programmatically)  mform->btnSortPlBy->Enabled = !mform->btnSortPlBy->Enabled;
	mform->playlist->Selected = false;             // selectings are not actual for any new playlist.
	selectedSong = -1;

	short currTag = mform->filterbtn->Tag;
	if(!mform->shufflebtn->Tag || programmatically)      // turn shuffle ON
	{
	   mform->shufflebtn->Font->Color = 0x001EC823;   mform->shufflebtn->Tag = 1;
	   mform->currCounter->Visible = true;

	   if(started[currTag] == false)                    // creates new order for shuffle
	   {
		 sortSongs(0, false);

		 started[currTag] = true;
		 shuffleCurrSong[currTag] = 0;

		 std::vector <int> tempOrder;                         // making shuffle order for active filter
		 makeVectorOfRandInts(tempOrder, 0, songs.size(), 0);
		 for(int i=0; i < tempOrder.size(); i++)
		 	songs[i].order[currTag] = tempOrder[i];
	   }
	   sortSongs(2+currTag, false);                             // sort by order[filter]
	}
	else if (mform->shufflebtn->Tag == true)
	{
	   mform->shufflebtn->Font->Color = clBlack;   mform->shufflebtn->Tag = 0;
	   mform->currCounter->Visible = false;

	   sortSongs(0, false);
	}

											   // and apply current filter
	mform->currCounter->Caption = shuffleCurrSong[currTag];
	if(currTag == 0 && programmatically)
	{    firstInactive = songs.size();
		 mform->filterbtn->Caption = "All";
		refreshSongCounter("comm");
	}
	if(currTag == 0) { mform->playlist->UpdateItems(0, songs.size()); return; }

	mform->filterbtn->Tag = -1;                        // reset filter tag
	filter(currTag);                                   // filter

	refreshSongCounter("comm"); // TEMP????
}

void Playlists::endSong()
{
	if(playingSong == -1 && selectedSong >= firstInactive - 1)      // for realTimeFilter
	   stop();                                           	// song was last active song and it ended.


	   // if playlist ends (commCounter = filtered)
	if(playingSong == mform->commCounter->Caption.ToInt() - 1)
	{
	   if(mform->cboxAutoNext->Font->Color == 0x001EC823) theEndM();        // GREEN CLR
	   else stop();                                                // close program

	   if(terminatingSong)
	   {
			messageForm->Close();
			playPauseResume();
			mform->Exit1Click(mform);
	   }
	}
	else     // if song ends...
	{
	   if(mform->cboxAutoNext->Font->Color == 0x001EC823)   // AutoNext is ON
	   {
		  if(songs.size() && firstInactive != 0              // always try to play next song
			&& actPlayingSong == -1 && playingSong == -1  // all this just changes PLAYING SONG index
			&& selectedSong >= firstInactive)
		  {
			  if(mform->shufflebtn->Tag)    // SHUFFLE
			  {             // it can switch to next SCS
				  if(shuffleCurrSong[mform->filterbtn->Tag] +1 < firstInactive)
					 playingSong = shuffleCurrSong[mform->filterbtn->Tag];
				  else {
					 playingSong = 0;              // needs more check
					 playPauseResume();
					 goto jump;
				  }
			  }
			  else                        // NON-SHUFFLE
			  {
				  playingSong = 0;
				  playPauseResume();           // needs more check
				  goto jump;
			  }
		  }

		  prevNext(1);
	   }
	   else                       // AutoNext is OFF
		  stop();

	   jump: // goto!
	   if(terminatingSong)
	   {
			playPauseResume();
			mform->Exit1Click(NULL);  // close program
	   }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK Playlists::onSongEnded(HSYNC handle, DWORD channel, DWORD data, void *user )
{                                       //     == here was before
	songEnded = true;
}

void Playlists::playPauseResume()                                           // when "play" button is clicked
{
	if(songs.size() || BASS_ChannelIsActive(stream))
	{
		if(playingSong >= firstInactive                                    // ignore inactive songs
		 || (actPlayingSong < 0 && selectedSong >= firstInactive && !BASS_ChannelIsActive(stream)
			 && !(firstInactive != 0 && playingSong == 0   // super exclusive case
				  && mform->cboxAutoNext->Font->Color == 0x001EC823) ) )
		  return;

		int tempActSong = actPlayingSong;

		  if(mform->playlist->SelCount > 1)                 // GUARD
		  {   if(actPlayingSong != -1) selectedSong = -1;   // multi-select case
			  else selectedSong = 0;
		  }

		if(playing == true && playingSong == actPlayingSong               // PAUSE song
		 && BASS_ChannelIsActive(stream) == BASS_ACTIVE_PLAYING )   // song is NOT ending (playing)
		{
			BASS_ChannelPause(stream);
			mform->playbtn->Caption = L"⏵";  // pause *II*
			if(pl.terminatingSong) mform->tray->IconIndex = 4;   // terminating pause
			else                   mform->tray->IconIndex = 2;
			mform->timer->Enabled = false;
			playing = false;
			return;
		}
		else if(BASS_ChannelIsActive(stream) != BASS_ACTIVE_PAUSED       // CREATE NEW STREAM (play NEW song)
		|| playingSong != actPlayingSong)
		{
		   if(actPlayingSong == -1 && selectedSong == -1) playingSong = 0;
		   else if(playingSong == -1 || playingSong == selectedSong)
			  playingSong = selectedSong;

		   ID3Tags tags(songs[playingSong].path, true);

           skipping();    // important function. count skipping
		   BASS_StreamFree(stream);                            // play song
		   stream = BASS_StreamCreateFile(false, songs[playingSong].path.c_str(),0,0,0);

		   BASS_ChannelSetSync(stream, BASS_SYNC_END, 0, onSongEnded, 0);
		   iterated = false;

		   if(!stream)                                         // has user deleted/moved song?
		   {
			   UnicodeString tempUS = "File not found: \n\n";  tempUS += songs[playingSong].path;

			   mform->playlist->Selected = false; selectedSong = playingSong;
			   mform->playlist->Items->Item[selectedSong]->Selected = true;

			   af.deleteFiles();
			   stop();

			   if(setForm->dontAnPPR->Checked)
			   messageForm->showMessage(tempUS, 'e', 33);       // show the message

			   playPauseResume();         // if it calls it should ALWAYS play.
			   return;
		   }

		   mform->playingSongInfo->Clear();             // fill playingSongInfo
		   mform->playingSongInfo->Items->Add(tags.artist + " - " + tags.title);
		   mform->playingSongInfo->Items->Add(tags.artist);
		   mform->playingSongInfo->Items->Add(tags.title);


		   if(songs[playingSong].duration == "") {             // if tags aren't loaded - load!
			  af.loadSongFullInfo(songs[playingSong]);
			  mform->playlist->UpdateItems(playingSong, playingSong);
		   }

											   // fill data about playling song
//		   if(mform->tray->Visible)
//		   	  mform->tray->Visible = true;  WHAT IS THIS???
		   mform->tray->Hint = PlayingSong::name();
		   mform->tray->Refresh();
		   mform->artist->Caption = "  " + formPlaylists->ampersandChanger(tags.artist, 2);    //// artist
		   mform->artist->Tag = 2000;
		   mform->timerSwimArt->Enabled = true;
		   mform->title->Caption  = "  " + formPlaylists->ampersandChanger(tags.title, 2);     //// title
		   mform->title->Tag = 2000;
		   mform->timerSwimTit->Enabled = true;
														   //// album              // it fits
		   if(mform->Canvas->TextWidth(tags.album) + 5*2 <= mform->panelAlbum->Width)
			  mform->album->Left = (mform->panelAlbum->Width - mform->Canvas->TextWidth(tags.album)) /2;
		   else  mform->album->Left = 5;
		   mform->album->Caption = formPlaylists->ampersandChanger(tags.album, 2);
		   mform->album->Tag = 2000;
		   mform->timerSwimAlb->Enabled = true;
		   mform->year->Caption   = tags.year;
		   mform->adur->Caption   = "00:00";
		   mform->cdur->Caption   = songs[0].getDuration(stream, 1);
		   mform->counter->Caption= songs[playingSong].listenings;
		   mform->btnCopyPlSong->Enabled = true;

		   mform->enableBtnPanel(mform->btnOpenDirPlSong);  // open file dir.
		   if(mform->shufflebtn->Enabled)   // enable, only if mform is "Enabled"
			   mform->enableBtnPanel(mform->btnDelPlayingSong); // enable btn delCurrPlSong

		   playing = true;
		   actPlayingSong = playingSong;
		}
		  BASS_ChannelPlay(stream, false);                   // RESUME song
		  mform->timer->Enabled = true;
		  mform->playbtn->Caption = L"⏸";    // play *>*
			if(pl.terminatingSong) mform->tray->IconIndex = 3; // terminating play
			else                   mform->tray->IconIndex = 1;
		  stopped = false;
		  playing = true;


		if(mform->shufflebtn->Tag && shuffleCurrSong[mform->filterbtn->Tag] == playingSong
		  && songs[playingSong].name == PlayingSong::name())
		  refreshSongCounter("curr");

		  if(tempActSong > -1)                                    // mark with green color
			 mform->playlist->Items->Item[tempActSong]->Update(); // + unmark previous played song
		  if(actPlayingSong > -1)
			 mform->playlist->Items->Item[actPlayingSong]->Update();

		mform->playlist->Items->BeginUpdate();
//		mform->playlist->Refresh();  // refresh all playing/ unplaying songs
		mform->playlist->UpdateItems(mform->playlist->TopItem->Index, mform->playlist->TopItem->Index
										+ mform->playlist->VisibleRowCount);
		mform->playlist->Items->EndUpdate();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::prevNext(int choice)                                            // plays previous/next song
{
  if(songs.size() == 0                                                 // GUARD #1
  || choice ==  1 && playingSong == -1 && selectedSong >= firstInactive - 1
  || choice == -1 && selectedSong >= firstInactive && actPlayingSong < 0 )
      return;

	if(playingSong == -1 && selectedSong <= firstInactive)
	  playingSong = selectedSong;

	int actualCount = mform->commCounter->Caption.ToInt();

	if(actualCount                                             // (playlist ends)
	&& mform->shufflebtn->Tag && choice == 1   			       // user wants refresh
	&& shuffleCurrSong[mform->filterbtn->Tag] == actualCount   // shuffle order!
	&& playingSong == actualCount - 1)
	   { theEndM(); return; }

  if( choice == -1 && playingSong < 1                                  // GUARD #2
	||choice ==  1 && playingSong >= mform->commCounter->Caption.ToInt() - 1)
	   return;


	mform->timer->Enabled = false;
	switch(choice)
	{
		case -1:
		  playingSong --;
		break;

		case 1:
		  if(mform->shufflebtn->Tag && shuffleCurrSong[mform->filterbtn->Tag] == 0)
		  {
			  actPlayingSong = -1;
				 playingSong = 0;
			  break;
          }

		  playingSong ++;
		break;
	}
	playPauseResume();

	if((mform->shufflebtn->Tag && choice == 1  			      						 // refresh currCounter
	&& shuffleCurrSong[mform->filterbtn->Tag] == playingSong
	&& songs[playingSong].name == PlayingSong::name()) 							 // next
	|| (choice == -1 && shuffleCurrSong[mform->filterbtn->Tag] == playingSong))      // prev
		  refreshSongCounter("curr");
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::refreshSong()                                                       // for [ENTER + DBLCLICK]
{                                                        // ignore inactive
	if(firstInactive != songs.size() && selectedSong >= firstInactive) return;

	if(playingSong == selectedSong)  stop();

	playingSong = selectedSong;
	playPauseResume();
												  // refresh currCounter
	if(mform->shufflebtn->Tag && mform->currCounter->Caption.ToInt() == selectedSong)
		 refreshSongCounter("curr");
}

/////////////////////////////////////////////////////////////////////////////////////////

bool Playlists::iteration()                                                // only checks if 75% was listened
{
	if(iterated == true) return false;

	QWORD lengthA, lengthC;
	lengthA = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
	lengthC = BASS_ChannelGetLength  (stream, BASS_POS_BYTE);

	float durA = BASS_ChannelBytes2Seconds(stream, lengthA);  // to seconds
	float durC = BASS_ChannelBytes2Seconds(stream, lengthC);

	float songPersantage = 0.75;
#ifdef _DEBUG
    songPersantage = 0.4;
#endif

	if(durA > durC * songPersantage) { iterated = true; return true; }
	else return false;
}                                                                               // changes counter everywhere
void Playlists::iterate()
{                                                 // IGNORE SONGS WITH BAD TAGS!
	 if(songs[0].IHaveBadTags(PlayingSong::name(), PlayingSong::artist(), PlayingSong::title()))
		 return;
									// find and save listenings in "AllSongs"
	 AllSong *playingAllSong = AllSong::find(PlayingSong::artist(), PlayingSong::title());
	 if(playingAllSong == NULL)  // song doesn't exist in allSongs
		return;

	 playingAllSong->incrementPlayCount();
	 PlayDatesInfo::addPlayDate(playingAllSong->id(), Now());

	 int lis = playingAllSong->playCount();      // "active" counter
	 short currTag = mform->filterbtn->Tag;

//start/////////// ARE YOU INTERRUPTING IN OTHER PLAYLISTS? ////// or too fat for current filter?

	if(mform->Tag && existsInPlaylist("bool", false, -1, PlayingSong::artist(), PlayingSong::title()) )
	{
	 for(int i=1; i < 5; i++)         // check yourself in every shuffle playlist!
	 {
		if(shuffleCurrSong[i] && (lis-1 == filterMax[i-1]))              // decrement shuffleCurrSong[i]
		{
		   pl.sortSongs(2+i, false);
		   pl.filterCopy(i);
		   for(int j=0; j < songs.size(); j++)
			  if(PlayingSong::name() == songs[j].name)  // song has been found
			  {
				  if(j < shuffleCurrSong[i]) shuffleCurrSong[i] --;
				  break;
			  }
		}

		if(i==4) {          // clean after yourself

		   if(!mform->shufflebtn->Tag)  pl.sortSongs(0, false);
		   else     pl.sortSongs(2 + currTag, false);

		   if(currTag) pl.filterCopy(currTag);         // filter, if it's not "All"
		}
	 }
	}
//end/// ARE YOU INTERRUPTING IN OTHER PLAYLISTS? //////

	 mform->counter->Caption = lis;                 // COUNTER caption

										   // "PASSIVE" counter in playlist
	 int realIndex = existsInPlaylist("int", false, -1, PlayingSong::artist(), PlayingSong::title());

	 if(realIndex > -1) {
		 songs[realIndex].listenings = lis;
		 mform->playlist->UpdateItems(realIndex, realIndex);
     }
															   // this song is no longer in our family
	 if(mform->Tag && realIndex > -1   // this song exists in pl.
	 && (currTag && lis - 1 == filterMax[currTag-1]))
	 {
		 if(actPlayingSong == -1 || PlayingSong::name() != songs[actPlayingSong].name)
		 {
			for(int i=0; i < songs.size(); i++)                    // find out real index   // here was " i < firstInactive"
			   if(PlayingSong::name() == songs[actPlayingSong].name)
			   {  actPlayingSong = i;  break;   }
		 }

		filterCopy(currTag);       // move it down
		if(firstInactive > 0) { firstInactive --; refreshSongCounter("comm"); }     // refresh commCounter

		selectedSong = actPlayingSong;                       // select previous
		mform->playlist->Selected = false;
		actPlayingSong = -1; playingSong = -1;
		if(selectedSong > 0) { selectedSong --; mform->playlist->ItemIndex = selectedSong; }
		else {  mform->playlist->ItemIndex = 0; selectedSong = -1; }
	 }

	 mform->currCounter->Caption = shuffleCurrSong[currTag];  // refresh SCS caption

	 // refresh listenings window                                          // artist
	 if(Listenings->Visible == true)
		 Listenings->iterate(PlayingSong::artist());

     saveEverything();     // save all data
}

/////////////////////////////////////////////////////////////////////////////////////////

int Playlists::progressBar()                     	// returns % of listened song for progressBar animation
{
   if(BASS_ChannelIsActive(stream) == BASS_ACTIVE_STOPPED) return 0;  // GUARD

   QWORD actLength, comLength;
	actLength = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);    // 0 = active duration [0:20]
	comLength = BASS_ChannelGetLength  (stream, BASS_POS_BYTE);    // 1 = common duration [4:21]

   return actLength*1000/comLength;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::stop()																       // stops playing
{
	skipping();    // important function. count skipping

	BASS_ChannelStop(stream);
	BASS_StreamFree(stream);
	mform->tray->Hint = "SCounter";

	mform->timer->Enabled = false;
	mform->artist->Caption = "  Artist";       // clear data about playling song
	mform->title->Caption  = "  Title";
	mform->album->Caption = "Album";
    mform->year->Caption = "Year";
	mform->adur->Caption   = "00:00";
	mform->cdur->Caption   = "00:00";
	mform->counter->Caption= "counter";
	mform->playbtn->Caption= L"⏵";
	mform->tray->IconIndex = 0;
    mform->album->Left = 96;		// set left to default
	stopped = true;
	playing = false;

	int tempActPlayingSong = actPlayingSong;

	actPlayingSong = -1;
	if(playingSong > -1 && mform->playlist->Items->Item[playingSong])
		mform->playlist->Items->Item[playingSong]->Update();
	playingSong = -1;

	if(songs.size() && !mform->playlist->SelCount)   // makeVisibleCurrSong
	{  selectedSong = tempActPlayingSong;  mform->playlist->ItemIndex = selectedSong; }

	mform->btnCopyPlSong->Enabled = false;
	mform->disableBtnPanel(mform->btnDelPlayingSong); // disable btn delCurrPlSong
    mform->disableBtnPanel(mform->btnOpenDirPlSong);
}
void Playlists::falseStop()                                          // when clicked shuffle, filter and etc.
{
	actPlayingSong = -1;
	playingSong = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::makeVisibleCurrSong(short choice, int firstFreeInd)	// user should know which song is playing...
{
	int index = 0;             // MAIN INDEX. IT WILL BE SELECTED AND SHOWN.

	switch(choice)
	{
		case 0:                                                            //// on create
		  if(songs.size() == 0) selectedSong = -1;
		  else if(!mform->shufflebtn->Tag)             // if shuffle is OFF
		  {
			 if(playingSong > -1)
			 {
			   selectedSong = playingSong;
			   playingSong = -1;
			   mform->playlist->Items->Item[selectedSong]->Selected = true;
			   mform->playlist->Items->Item[selectedSong]->MakeVisible(true); }
			 else if(selectedSong > -1)
			 {
				mform->playlist->Items->Item[selectedSong]->Selected = true;
				mform->playlist->Items->Item[selectedSong]->MakeVisible(true);
			 }
			 else if(firstInactive || songs.size())
			 {
				selectedSong = 0;
				mform->playlist->Items->Item[selectedSong]->Selected = true;
			 }
		  }
		  else                                           // if shuffle is ON
		  {
			 if(firstInactive)
			 {
			   actPlayingSong = shuffleCurrSong[mform->filterbtn->Tag] - 1;
               if(actPlayingSong == -1) actPlayingSong = 0;
			   playingSong = actPlayingSong;
			   mform->playlist->Selected = false; selectedSong = -1;
			   mform->playlist->Items->Item[actPlayingSong]->MakeVisible(true);
             }
			 else if(songs.size())                                   // all songs are inactive
			 {
				mform->playlist->Items->Item[0]->Selected = true;
				mform->playlist->Items->Item[0]->MakeVisible(true);
			 }
		  }
		break;

		case 1:                                                           // theEndM
			playingSong = 0; actPlayingSong = 0; selectedSong = -1;
			mform->playlist->Selected = false;
			mform->playlist->Items->Item[0]->MakeVisible(true);
			mform->playlist->Items->Item[0]->Update();
		break;

		case 2:                                                          // deleteSongs
			if(selectedSong >= 0) index = selectedSong;
		break;

		case 3:                                                          // on shuffleClick && filterClick
		{
			if(songs.size() == 0) selectedSong = -1;
			else if(!mform->shufflebtn->Tag)  // turn shuffle OFF
			{
				if(actPlayingSong == -1 && PlayingSong::songIsPlaying())  // find which song is playing
				{
				   int realIndex = existsInPlaylist("int", false, -1, PlayingSong::artist(),
													PlayingSong::title());

				   if(realIndex >= firstInactive) goto end; // playing song is inactive
				   else if(realIndex > -1) { index = realIndex; actPlayingSong = index; playingSong = index; }
				   else  mform->playlist->ItemIndex = 0;
				}
				else
				{
				   end:           //goto!
				   selectedSong = index;
				   mform->playlist->ItemIndex = index;
				}
				mform->playlist->Items->Item[index]->MakeVisible(true);
			}
			else                        // turn shuffle ON
			{
				actPlayingSong = shuffleCurrSong[mform->filterbtn->Tag] - 1;
				if(actPlayingSong == -1 && firstInactive) actPlayingSong = 0;
				playingSong = actPlayingSong;
				if(playingSong == -1 && songs.size())
				{
				  selectedSong = 0; mform->playlist->Items->Item[selectedSong]->Selected = true;
				  mform->playlist->Items->Item[0]->MakeVisible(true);
				}
				else mform->playlist->Items->Item[playingSong]->MakeVisible(true);
			}
		}
		break;

		case 4:                                                     // ADDING FILES
		{
		  if(firstFreeInd == songs.size() || mform->shufflebtn->Tag == 1)  // D&D didn't add any songs
		  {                                                                // or shuffle == ON
			 restoreSelection();        // restore users' selections
			 if(savedSelectedSongs.size())  choice = -1;    // go to the end of method.
		  }
		  else
		  {
			 index = firstFreeInd;
			 selectedSong = index;
		  }
		}
		break;
	}

	if(choice == 2 || choice == 4) {
        mform->playlist->Selected = false;                		   // reset selection
		mform->playlist->ItemIndex = index;				           // select
		mform->playlist->Items->Item[index]->MakeVisible(true);    // make visible
    }


	if(mform->playlist->SelCount == 1)
	   mform->playlist->Items->Item[mform->playlist->Selected->Index]->Focused = true;  // for arrows navigate
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::theEndM()												// happens when shuffle playlist ends
{
	short currFilterTag = mform->filterbtn->Tag;

	if( (mform->commCounter->Caption.ToInt()                   // GUARD
	&& playingSong == mform->playlist->Items->Count - 1
	&& mform->shufflebtn->Tag
	&& shuffleCurrSong[currFilterTag] < playingSong)
	|| !mform->commCounter->Caption.ToInt()
	|| mform->shufflebtn->Tag && shuffleCurrSong[currFilterTag] < mform->commCounter->Caption)
	   {  stop(); return; }

	stop();

	if(showTheEnd && terminatingSong == false)
	   messageForm->showMessage(L"The end of playlist!", 'E', 2);

	if(mform->shufflebtn->Tag)       // refresh order for shuffle
	{
	   started[currFilterTag] = false;

	   short proc = mform->commCounter->Caption.ToInt()*0.3;           // don't repeat last songs!
	   if(proc >= 15)
		 for(int i = 0; i < proc && i < 1000; i++)
			lastSongs.push_back(songs[mform->commCounter->Caption.ToInt()-1-i].indexNorm);

	   shuffleClick(true);
	   lastSongs.clear();
	}

	makeVisibleCurrSong(1,0);

	// if(autoNext is GREEN)
	if(mform->cboxAutoNext->Font->Color == 0x001EC823) playPauseResume();
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::refreshFilterCap()                                              // refreshes captions in mform
{
   UnicodeString caption;

   for(int i=0; i < 4; i++)
   {
		if(filterMin[i] == filterMax[i])                           // = 1
		{
			caption = "= ";
			caption += filterMin[i];
		}
		else if(filterMin[i] == 0)                                 //  ≤ 3
		{
			caption = L"≤ ";
			caption += filterMax[i];
		}
		else if(filterMax[i] == 2147483647)                        //  ≥ 200
		{
			caption = L"≥ ";
			caption += filterMin[i];
		}
		else                                                       // 29-170
		{
			caption = filterMin[i];
			caption += " - ";
			caption += filterMax[i];
		}

		mform->filterPopUp->Items->Items[i+1]->Caption = caption;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::saveSelection()                                             // memorizes all selected songs
{
	if(!mform->playlist->SelCount) return;             // GUARD

	savedSelectedSongs.clear();                         // adds selectedSongs to the vector
	savedSelectedSongs = af.getSelectedSongs();
}
/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::restoreSelection()                                           // restores selected songs
{
	mform->playlist->Selected = false; // reset whatever was before

    if(!savedSelectedSongs.size())   return;    	// GUARD

	selectedSong = savedSelectedSongs[0];

	for(int i=0; i < savedSelectedSongs.size(); i++)
		 mform->playlist->Items->Item[savedSelectedSongs[i]]->Selected = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



///########################## sorting of vectors ########################################

struct byIndexNorm
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.indexNorm < rx.indexNorm;
	}
};
struct byIndexFiltered
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.indexFiltered < rx.indexFiltered;
	}
};
struct byOrder0
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.order[0] < rx.order[0];
	}
};
struct byOrder1
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.order[1] < rx.order[1];
	}
};
struct byOrder2
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.order[2] < rx.order[2];
	}
};
struct byOrder3
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.order[3] < rx.order[3];
	}
};
struct byOrder4
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.order[4] < rx.order[4];
	}
};
struct byPath
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.path.LowerCase() < rx.path.LowerCase();
	}
};
struct byName
{
	bool operator()( const Songs& lx, const Songs& rx ) const {
		return lx.name.LowerCase() < rx.name.LowerCase();
	}
};

///########################## sorting of vectors ########################################


/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::sortSongs(short choice, bool displayChanges)
{
	switch(choice)
	{
	   case -2:  std::sort(songs.begin(), songs.end(), byName());        // sort by name (Artist - Title)
	   break;

	   case -1:  std::sort(songs.begin(), songs.end(), byPath());        // sort by path
	   break;

	   case 0:   std::sort(songs.begin(), songs.end(), byIndexNorm());
	   break;

	   case 1:   std::sort(songs.begin(), songs.end(), byIndexFiltered());
	   break;

	   case 2:   std::sort(songs.begin(), songs.end(), byOrder0());
	   break;

	   case 3:   std::sort(songs.begin(), songs.end(), byOrder1());
	   break;

	   case 4:   std::sort(songs.begin(), songs.end(), byOrder2());
	   break;

	   case 5:   std::sort(songs.begin(), songs.end(), byOrder3());
	   break;

	   case 6:   std::sort(songs.begin(), songs.end(), byOrder4());
	   break;
	}

	if(displayChanges)
	   mform->playlist->UpdateItems(0, songs.size()-1);                // and display the changes in playlist
}


/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::loadPlaylistButtons()                                    // loads buttons to the panelMain
{
   for(UINT i=0; i < plButtons.size(); i++)      // clear previous pl btns
       delete plButtons[i];
   plButtons.clear();

   TStringList *strList = new TStringList;
   strList->LoadFromFile(getSettingsPath() + "settings" + "/plButtonsData", TEncoding::UTF8);

   for(int i=0; i < strList->Count; i+=7)
   {
	   TButton *btn = new TButton (mform->panelMain);             // create btn and load proporties
	   btn->Parent = mform->panelMain;
	   btn->StyleElements =  TStyleElements(seBorder);
	   btn->OnEnter = mform->btnDefaultEnter;
	   btn->OnExit  = mform->btnDefaultExit;
	   btn->PopupMenu = mform->popRClick;
	   btn->OnMouseDown = mform->btnDefaultMouseDown;
	   btn->Font->Name = "Microsoft Sans Serif";

	   btn->Caption = strList->Strings[i];                  	 // caption
	   btn->Width   = strList->Strings[i+1].ToInt();             // width
	   btn->Height  = strList->Strings[i+2].ToInt();             // pos
	   btn->Top     = strList->Strings[i+3].ToInt();
	   btn->Left    = strList->Strings[i+4].ToInt();
	   btn->Tag     = strList->Strings[i+5].ToInt();
	   btn->Font->Color = strList->Strings[i+6].ToInt();
	   plButtons.push_back(btn);
   }
   mform->changeColorOfArrows(-1);
   mform->plLabel->Caption = getActivePlFullName();

   delete strList;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::savePlaylistButtons()                                    // saves buttons to the panelMain
{
   TStringList *strList = new TStringList;

   for(int i=0; i < plButtons.size(); i++)
   {
	   strList->Add(plButtons[i]->Caption);      				  // caption
	   strList->Add(plButtons[i]->Width);					      // width

	   strList->Add(plButtons[i]->Height);                         // pos
	   strList->Add(plButtons[i]->Top);
	   strList->Add(plButtons[i]->Left);
	   strList->Add(plButtons[i]->Tag);
	   strList->Add(plButtons[i]->Font->Color);
   }

   strList->SaveToFile(getSettingsPath() + "settings" + "/plButtonsData", TEncoding::UTF8);
   delete strList;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::refreshPlDuration()                                     // 05:16:59 for pl
{
	if(af.firstLoadingInd != -1)              // PL IS NOT LOADED
		return;

	UINT days = 0, hours=0, mins=0, secs=0;     // how many minutes to listen all playlist?

	for(int i=0; i < firstInactive; i++)
	{                                                                            // seconds
	   secs += songs[i].duration.SubString(songs[i].duration.Length() -1, 2).ToInt();
	   mins += songs[i].duration.SubString(1, songs[i].duration.Pos(":") -1).ToInt();
	}

	mins += secs/60;     // 1087 mins
	hours = mins/60;     // 18 hours
	mins  %= 60;         // 15 mins
	secs  %= 60;         // 45 secs

	days  = hours/24;    // 0 days
    hours = hours%24;    // 18 hours

	UnicodeString temp;
	if(days < 10) { temp = "0"; temp += days; }
	else 		  temp = days;
	temp += " ";

	if(hours < 10) temp += "0";
	temp += hours;
	temp += ":";

	if(mins < 10) temp += "0";
	temp += mins;
	temp += ".";

	if(secs < 10) temp += "0";
	temp += secs;

	mform->labelPlDuration->Caption = temp;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::refreshListeningsForLoadingPl() 		   //	check listenings before load pl
{
//	{                                               // indexation of AllSongs.txt data
//	   std::vector <UnicodeString> tempVect;
//	   for(int i=0; i < af.dataAllSongs->Count; i+=4)
//			tempVect.push_back(af.dataAllSongs->Strings[i].LowerCase());      // artists
//	   af.Indexation(tempVect, af.artAS, af.artIndexesAS);
//	}
//	 NEW CODE
	AllSong::makeIndexation(LetterCase::Lower);

	for(int i=0; i < songs.size(); i++)        // check EVERY song
	{
		for(int j=0; j < indArtists.size(); j++)    // NEW CODE
		{                                     // indArtists[j] (already lowercased)
			if(songs[i].artist.LowerCase() == indArtists[j])   // artist == artist
			{
                Application->ProcessMessages();
				for(int k=0; k < indArtIndexes[j].size(); k++)
				{              // k < indArtIndexes[j].size()  				// name == name


//				   if(songs[i].name.LowerCase() == af.dataAllSongs->Strings[af.artIndexesAS[j][k]*4+1])
//				   {							   // list == list
//					   if(songs[i].listenings != af.dataAllSongs->Strings[af.artIndexesAS[j][k]*4+3].ToInt())
//						   songs[i].listenings = af.dataAllSongs->Strings[af.artIndexesAS[j][k]*4+3].ToInt();
//					   break;
//				   }

				   // NEW CODE
				   if(songs[i].name.LowerCase() == allSongs[indArtIndexes[j][k]].name().LowerCase())
				   {                         // play count in AllSongs.txt always RIGHT
					   songs[i].listenings = allSongs[indArtIndexes[j][k]].playCount();
                       break;
				   }
                }
				break;
			}
        }
	}
}


/////////////////////////////////////////////////////////////////////////////////////////

int Playlists::existsInPlaylist(std::string returns, bool usingIndexation, int knownIndex
	, const UnicodeString &artist, const UnicodeString &title)     // function #1
{
	int indexInPl = -1;									// INT TO RETURN

	if(!usingIndexation)						// W/OUT INDEXATION
	{
		for(int i=0; i < songs.size(); i++)
			if(artist.LowerCase() == songs[i].artist.LowerCase()
			&&  title.LowerCase() == songs[i].title.LowerCase() )
			{
				if(knownIndex == -1 || knownIndex != i)
				{  indexInPl = i; break;  }
			}

	}
	else if(usingIndexation)
	{											// WITH INDEXATION
		for(int i=0; i < af.artBef.size(); i++)
		{
			if(artist.LowerCase() == af.artBef[i])   // same artist
			{
				for(int j=0; j < af.artIndexesBef[i].size(); j++)
				{														// same title
					if(title.LowerCase() == songs[af.artIndexesBef[i][j]].title.LowerCase() )
					{
						if(knownIndex == -1 || knownIndex != af.artIndexesBef[i][j])
						{  indexInPl = af.artIndexesBef[i][j]; break;  }
					}
				}

				break;
			}
		}

	}


	if(returns == "bool") indexInPl ++;					// RETURN
	return indexInPl;
}
		  // ===========
int Playlists::existsInPlaylist(std::string returns, int knownIndex
	, const UnicodeString &artist, const UnicodeString &title, UnicodeString plName)     // function #2
{
	int indexInPl = -1;									// INT TO RETURN
	plName = delNonPathChars(plName);

	TStringList *plSongs = new TStringList;           // W/OUT INDEXATION
	plSongs->LoadFromFile(getSettingsPath() + "playlists/" + plName + "/playlistData");

	for(int i=0; i < plSongs->Count; i+=7)
		if(artist.LowerCase() == plSongs->Strings[i+1].LowerCase()
		&&  title.LowerCase() == plSongs->Strings[i].LowerCase() )
		{
		   if(knownIndex == -1 || knownIndex != i)   // returns like songs[IndexInPl]
		   {  indexInPl = i/7; break;  }
		}

	delete plSongs;

	if(returns == "bool") indexInPl ++;					// RETURN
	return indexInPl;
}

int Playlists::existsInPlaylist(std::string returns                                     // function #3
	, const UnicodeString &artist, const UnicodeString &title, TStringList *plData)
{
	int indexInPlData = -1;									// INT TO RETURN

                                                        // USING INDEXATION
	for(int i=0; i < af.artBef.size(); i++)     // look through all artists
	{
		if(artist.LowerCase() == af.artBef[i].LowerCase())
		{
			for(int j=0; j < af.artIndexesBef[i].size(); j++)  // look through all titles
			{
				if(title.LowerCase() == plData->Strings[af.artIndexesBef[i][j] *7].LowerCase())
				{       	// SAME SONG!!!
					 indexInPlData =  af.artIndexesBef[i][j];
					 break;
                }
			}
			break;
		}
	}

	if(returns == "bool") indexInPlData ++;
	return indexInPlData;
}

/////////////////////////////////////////////////////////////////////////////////////////

Songs Playlists::existsInScounter(UnicodeString artist, UnicodeString title)
{                                // checks for song in all playlists (starts from activePlaylist)
	int Index = existsInPlaylist("int", false, -1, artist, title);
	if(Index > -1)             // found in active playlist!
		return songs[Index];

	for(int i=0; i < plButtons.size(); i++)
	{
		if(activePlaylist == delNonPathChars(plButtons[i]->Caption)) continue; // don't check active pl

		Index = existsInPlaylist("int", -1, artist, title, plButtons[i]->Caption);
		if(Index > -1) {                 // SONG FOUND!
			  TStringList *list = new TStringList;
			  list->LoadFromFile(getSettingsPath() + "playlists/" + delNonPathChars(plButtons[i]->Caption)
			  + "/playlistData");

			  Songs tempSong(list->Strings[Index*7],  // title
							 list->Strings[Index*7+1],  // artist
							 list->Strings[Index*7+2],  // path
							 list->Strings[Index*7+3],  // duration
							 list->Strings[Index*7+4],  // date
							 list->Strings[Index*7+5].ToInt(),   // listen.
							 list->Strings[Index*7+6].ToInt() ); // indexNorm
			  delete list;
			  return tempSong;
		}
	}


	Songs emptySong("0",0);
	return emptySong;           // no such song in SCounter.
}


/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::addToSongs(Songs song)                   		// adding new song to the Songs vector
{
	song.indexNorm = songs.size();                            // indexNorm
	for(int i=0; i < 5; i++) song.order[i] = songs.size();    // order[5]

	songs.push_back(song);                                    // add to the Songs vector
	mform->playlist->Items->Count = songs.size();

	short currFilter = mform->filterbtn->Tag;                 // "press" filterBtn
	mform->filterbtn->Tag = -1;
	mform->filterMenuItemClick(mform->filterPopUp->Items->Items[currFilter]);
}
/////////////////////////////////////////////////////////////////////////////////////////

int Playlists::existsInAllSongs(const std::string &returningValue, const bool &usingIndexation
  , const UnicodeString &artist, const UnicodeString &title) const  // check whether given song exists in AllSongs file
{
    int returningIndex = -1;    // -1 if file not found

	if(!usingIndexation)                                      // Without Indexation
	{
		for(UINT i = 0; i < allSongs.size(); i++)   // NEW CODE =
		{                    // lowCase name
//			UnicodeString temp = (artist.LowerCase() + " - " + title.LowerCase());
//			if(af.dataAllSongs->Strings[i+1] == temp)
//				returningIndex = i;

			// NEW CODE
			AllSong *asFound = AllSong::find(artist.LowerCase(), title.LowerCase());
			if(asFound)
			   returningIndex = asFound->index();
		}
	}
	else {                                                    // Using Indexation
		for(UINT i=0; i < indArtists.size(); i++)
		{                // NEW CODE = allSongs.size(), i++

									  // af.artAS[i] is lowerCased
			if(artist.LowerCase() == indArtists[i])   // artist == artist
			{                        // indArtist[i] already lowercased
				for(UINT j=0; j < indArtIndexes[i].size(); j++)
				{                    // indArtIndexes[i].size()


//					UnicodeString AllSongsTitle =              // DON'T NEED ALL OF THIS IN NEW CODE
//					   af.dataAllSongs->Strings[af.artIndexesAS[i][j] *4 +1].LowerCase() // title
//					   .SubString0(3 +artist.Length(), title.Length());

					   // NEW CODE
					if(title.LowerCase() == allSongs[indArtIndexes[i][j]].title().LowerCase())
					{
						returningIndex = indArtIndexes[i][j];
						break;
					}
//					if(title.LowerCase() == AllSongsTitle)      // same title
//					{
//					   returningIndex = af.artIndexesAS[i][j];
//					   break;
//					}
				}

				break;
			}

        }
	}


	if(returningValue == "bool") returningIndex ++;
	return returningIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////


UnicodeString Playlists::getSettingsPath()     // returns "C:/Users/.../SCounter/"
{
//	#ifdef _DEBUG
//		 return L"C:/Users/Avtem/AppData/Roaming/SCounterTest/";
//	#endif

	PWSTR path = NULL;
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);

	UnicodeString temp = path;
	temp += "\\SCounter\\";

	return temp;
}
/////////////////////////////////////////////////////////////////////////////////////////

void Playlists::saveEverything()                       // saves all "saveable" data
{
	savePlaylist(activePlaylist);               // save playlist
	PlayDatesInfo::saveData(getSettingsPath() + "settings/PlayDatesInfo.big");
	savePlaylistSettings();
	savePlaylistButtons();

	UnicodeString path = getSettingsPath(); path += "settings/AllSongs.txt";
//	af.dataAllSongs->SaveToFile(path, TEncoding::UTF8);   // save data to AllSongs.txt
	//NEW CODE
    AllSong::saveToFile();
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

UnicodeString Playlists::getActivePlFullName() const      // gets name, like "\\\ WOW ///"
{
//	if(activePlaylist == "")              // DINT?
//		return "";

	for(int i=0; i < plButtons.size(); i++)
		if(delNonPathChars(plButtons[i]->Caption) == activePlaylist)
			return plButtons[i]->Caption;

	for(int i=0; i < plButtons.size(); i++)                              // if we haven't find the match
		if(plButtons[i]->Font->Color == clLime)
			return plButtons[i]->Caption;

    return "";
}

/////////////////////////////////////////////////////////////////////////////////

void Playlists::refreshAllSCounterFilters(Filter filter)    // loads all playlists and deletes SCS data.
{
	TStringList *list = new TStringList(this);       // filter All can not be reset.

	for(int i=0; i < plButtons.size(); i++)          // load every pl
	{
		UnicodeString plPath = getSettingsPath() + "playlists/" + delNonPathChars(plButtons[i]->Caption)
												 + "/SCSdata";
		if(!FileExists(plPath))    // GAURD
			continue;

		list->LoadFromFile(plPath);

		list->Strings[filter*2]    = 0;        // reset.
		list->Strings[filter*2 +1] = 0;

		list->SaveToFile(plPath);
	}

	delete list;
}

UINT Playlists::getPos() const
{
	QWORD bytes = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);

	return BASS_ChannelBytes2Seconds(stream, bytes) *1000;
}


void Playlists::skipping()
{
	if(!PlayingSong::songIsPlaying()
	   || iterated
	   || getPos() < skippingMinTime) // counts amount msecs when "next | prev | stop" was clicked
		return;

	 AllSong *skippingAllSong = AllSong::find(PlayingSong::artist(), PlayingSong::title());
	 if(skippingAllSong == NULL)
		return;

	skippingAllSong->incrementSkipCount();
    PlayDatesInfo::addSkipDate(skippingAllSong->id(), Now());
}
