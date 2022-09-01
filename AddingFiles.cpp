#include "AddingFiles.h"

// ============== my includes ===================== //
#include <dirent.h>                  // for D&D (opens folders)
#include "AvId3Reader.h"             // my id3 v2.4 tags ^^
#include "ssetings.h"               // for copyToAnotherPl() func
#include "AllSong.h"


extern void myShowModal(TForm *form);                     // shows form modal
UINT firstFreeIndCopyToPl = -1;  // this will fix function af.shuffleAdd(firstFreeIndex)
extern short plColWidthMin;
extern short plColWidthMax;
extern std::vector <AllSong> allSongs;   // here we store all data.
extern std::vector <UnicodeString> indArtists;  		// INDEXATION
extern std::vector <std::vector <int> > indArtIndexes;  // INDEXATION

void AddingFiles::process_dropped_filename(UnicodeString &FileN)                           // Drag & Drop continuation
{
	wDIR *wdir = wopendir(FileN.c_str()); // well, what did we receive here?
	struct wdirent *ent;
	UnicodeString directory;

  //=============================================== if we have a folder
	if(wdir)  // it's a folder!
	{
		while ((ent=wreaddir(wdir)) != NULL)  	// reading all files
		{
			if ( !wcscmp(ent->d_name, L".") || !wcscmp(ent->d_name, L"..") )
			 ;  // do not add this shit to fileList
			else
			{
				directory = FileN + "\\" + ent->d_name;

				if(isFolder(directory) == true) // were folders in dropped folder?
				   process_dropped_filename(directory);
				else                            // we had no folders in folder
				   addThisFile(directory);
			}
		}
		wclosedir(wdir);
	}
  //=============================================== if we have just files
	else
	{
		addThisFile(FileN);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

bool AddingFiles::isFolder(UnicodeString &filePath)                                          // is that folder?
{
	wDIR *openedDir = wopendir(filePath.c_str());
	if(openedDir) {
		wclosedir(openedDir);
		return true;      // it's folder
    }

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

bool AddingFiles::associationOk(UnicodeString &filePath)                                     // does player support this extension?
{
	if(ExtractFileExt(filePath) == ".mp3"
	|| ExtractFileExt(filePath) == ".wav"
	|| ExtractFileExt(filePath) == ".ogg")
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::addThisFile(UnicodeString &filePath)                          // add this file to playlist
{
   Application->ProcessMessages();
   if(associationOk(filePath) == false) return;

   Songs newSong(filePath, songs.size());      // add to "songs" vector
   songs.push_back(newSong);

   if(songs.size()%10 == 0)                    // sexy "progress bar"
   {   																	// playlist width
	   if(songs.size() > mform->playlist->VisibleRowCount) mform->playlist->Column[3]->Width = plColWidthMin;
	   else 										 	   mform->playlist->Column[3]->Width = plColWidthMax;

	   mform->playlist->Items->Count = songs.size();
	   mform->commCounter->Caption = songs.size();
   }
}

//////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::Indexation(std::vector <UnicodeString> &inputVec,   // metallica       // input vector (lower cased)
std::vector <UnicodeString> &artists, std::vector <std::vector <int> > &artistsIndexes)  // output vector
{                           // lowercased!
	artists.clear();                   // clear output vectors
	artistsIndexes.clear();

	bool inArtistsIndexes = false;

	for(int i=0; i < inputVec.size(); i++)
	{												// current song in playlist
		for(int j=artists.size()-1; i!=0 && j > -1; j--)
		{
			 if(inputVec[i] == artists[j])
			 {
				artistsIndexes[j].push_back(i);      // add index of same artist
				break;
			 }

			 if(j == 0) // it's passed vector of artists!
				 inArtistsIndexes = false;
		}

		if(inArtistsIndexes == false)              			// CREATE NEW ARTIST VECTOR
		{
			 artists.push_back(inputVec[i]);      // metallica
			 std::vector <int> temp;              // create vector for indexes of (metallica)
			 temp.push_back(i);                   // first index of new art.
			 artistsIndexes.push_back(temp);
		}
		inArtistsIndexes = true;
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::showSameSongs()
{
	if(!samePlaylist.size())  return;   // there is no doubles

	messageForm->playBeep('w');         // beep

	Form2->fixed->Tag = 0;              // set captions for first couple
	Form2->fixedClick(NULL);

	myShowModal(Form2);
}

////////////////////////////////////////////////////////////////////////////////////////


// NEW CODE DON'T NEED THIS
//void AddingFiles::loadAllSongsData()
//{
//	dataAllSongs = new TStringList;
//
//	UnicodeString path = pl.getSettingsPath(); path += "settings/AllSongs.txt";
//	dataAllSongs->LoadFromFile(path.c_str());
//}



	 // NEW CODE DON'T NEED THIS
//void AddingFiles::addToAllsongs(int indexInPl)                  // adds (only one) song info to "dataAllSongs"
//{                                                               // with name, date of adding and listenings
//	if(mform->togCheckingMode->State == tssOff) return;   // cheching mode!!!
//	dataAllSongs->Add(songs[indexInPl].artist);
//	dataAllSongs->Add(songs[indexInPl].name.LowerCase());
//	dataAllSongs->Add(Date().FormatString("dd.mm.yyyy"));
//	dataAllSongs->Add("0");      // listenings
//
//	songs[indexInPl].date = Date().FormatString("dd.mm.yyyy");
//	songs[indexInPl].listenings = 0;
//}


////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::shuffleAddFiles(int firstFreeIndPl)		// on adding new files
{                                                                    // AFTER THIS FUNCTION
   if(firstFreeIndCopyToPl != -1)                                    // PLAYLIST IS MESSED UP
	   firstFreeIndPl = firstFreeIndCopyToPl;

   for(int i=0; i < 5; i++)
     if(pl.started[i])
   	  for(int j=firstFreeIndPl; j < songs.size(); j++)           // add new "order numbers"
		 songs[j].order[i] = j;                                  // [5, 6, 7, 8]

   for(int i=0; i < 5; i++)
   {
	  if(!pl.started[i]) continue;    // Don't create anything if shuffle wasn't started!

	  pl.sortSongs(2+i, false);
	  pl.filterCopy(i);                               // make "good" order [0,1,2,3,4,5,6,7]
	  for(int j=0; j < songs.size() && i!=0 ; j++)
	  	songs[j].order[i] = j;


	  std::vector <int> pieceOfNewOrder;
	  for(int j = pl.shuffleCurrSong[i]; j < songs.size(); j++)  // [3, 4, 5, 6, 7, 8]
		 pieceOfNewOrder.push_back(j);

	  std::vector <int> tempVect;                                // [0,4,2,5,1,3,6] indexes
	  pl.makeVectorOfRandInts(tempVect, 0, songs.size() - pl.shuffleCurrSong[i], i);

	  for(int j=pl.shuffleCurrSong[i], f=0; j < songs.size(); j++, f++)
		 songs[j].order[i] = pieceOfNewOrder[tempVect[f]];
   }
}

////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::deleteFiles()											// deleting selected files by user
{
	// GUARD
   if(!mform->Tag || mform->playlist->SelCount == 0 || !mform->playlist->SelCount)
		return;

   int countBefore = 0;                                // number of songs before deleting song
   bool inDanger = false, inRealDanger = false;
   short currTag = mform->filterbtn->Tag;
   int playingSongCopy = pl.actPlayingSong;
   bool playingCopy = pl.playing;

															// mark selected songs in vector
   std::vector <UINT> selectedSongs = af.getSelectedSongs();
   for(int i=0; i < selectedSongs.size(); i++)
   		songs[selectedSongs[i]].selected = true;

   for(int i=0; i < playingSongCopy; i++)               // how many songs before PlayingSong?
	  if(songs[i].selected) countBefore ++;

   if(playingSongCopy > -1 && songs[playingSongCopy].selected)    // is song in danger?
   {
	  inDanger = true;
	  countBefore ++;

	  // commented. Don't stop playing song when it's deleted
      // since 2.2.0
//	  if(PlayingSong::songIsPlaying()
//	   && PlayingSong::name() == songs[pl.actPlayingSong].name)
//		inRealDanger = true;
   }

   for(int i=0; i < 5; i++)                               // fix orders
   {
	   if(!pl.started[i]) continue;

	   pl.sortSongs(2+i,false);                               // refresh SCS

	   if(i != 0)
	   	   pl.filterCopy(i);

	   int before = 0;
	   for(int j=0; j < pl.shuffleCurrSong[i]; j++)
		  if(songs[j].selected)
			before ++;

	   pl.shuffleCurrSong[i] -= before;

	   ///////////////////////////////////

	   pl.sortSongs(2+i,false);
	   for(int j=0, selSongsBef=0; j < songs.size(); j++)
	   {
		   if(songs[j].selected) selSongsBef++;
		   else songs[j].order[i] -= selSongsBef;
	   }
   }

   int lastSelSong = selectedSongs[selectedSongs.size() -1];
   lastSelSong	-= mform->playlist->SelCount;


   for(int i=songs.size()-1; i > -1 ; i--)                  // DELETE SONGS!
	  if(songs[i].selected)
		  songs.erase(songs.begin()+i);

   pl.sortSongs(0,false);                                 // fix indexNorm
   for(int i=0; i < songs.size(); i++)
	   songs[i].indexNorm = i;


 // == -- == -- == -- == -- == -- == -- == -- == -- == -- == -- == -- == -- == -- == --

   mform->filterbtn->Tag = -1;                                // update all stuff
   if(mform->shufflebtn->Tag) pl.sortSongs(2+currTag,false);  // TURN BACK playlist order
   deleting = true;                                           // if af.deleteFiles calls, don't stop playing
   pl.filter(currTag);

   mform->playlist->Items->Count = songs.size();              // display changes
   pl.refreshSongCounter("comm");

   if(!inDanger)                                              // playing song is safe
   {
	  pl.playingSong -= countBefore;
	  pl.actPlayingSong = pl.playingSong;
   }
   else                                                       // song in danger!
   {
	   pl.playingSong -= countBefore - 1;
	   pl.actPlayingSong = -2;

	   if(pl.playingSong == pl.firstInactive && pl.firstInactive > 0)
		  pl.playingSong = pl.firstInactive - 1;     // there is no next song, thus, play last in PL

	   if(inRealDanger)                                     // play (or pause)
	   {
		  pl.playPauseResume();                      // is it safe? i just want to prevent stopping
		  if(!playingCopy) {       // song is paused // after deleting playing song
			 pl.playPauseResume();
			 pl.shuffleCurrSong[currTag] --;
		  }
	   }

	   if(!pl.firstInactive && inRealDanger)              // there are no active songs
	   {
			pl.stop();
			pl.makeVisibleCurrSong(2,0);
	   }
	   else if (!inRealDanger)
	   {
			pl.falseStop();
			goto jump;
	   }
   }

   if(!inDanger && songs.size())           				      // if player wasn't playing
   {
	   jump:                                                      // select next song after deleting
	   pl.selectedSong = lastSelSong + 1;
	   if(pl.selectedSong == songs.size()) pl.selectedSong --;
	   pl.makeVisibleCurrSong(2,0);
   }

   deleting = false;

   if(songs.size() > mform->playlist->VisibleRowCount) mform->playlist->Column[3]->Width = plColWidthMin;
   else 										 	   mform->playlist->Column[3]->Width = plColWidthMax;
}


////////////////////////////////////////////////////////////////////////////////////////
	// NEW CODE DON'T NEED THIS
//void AddingFiles::AllSongRevision()								     // deletes duplicates from AllSongs.txt
//{
//	{                                               // indexation of AllSongs.txt data
//	  std::vector <UnicodeString> tempVect;
//	  for(int i=0; i < dataAllSongs->Count; i+=4)
//	  tempVect.push_back(dataAllSongs->Strings[i].LowerCase());     // artists
//	  Indexation(tempVect, artAS, artIndexesAS);
//	}
//
//   std::vector <int> badSongs;       // indexes to delete data from AllSongs
//   bool alreadyChecked = false;
//
//   for(int i=0; i < artAS.size(); i++)
//   {
//	  for(int j=0; j < artIndexesAS[i].size(); j++)
//	  {
//		 for(int k=j+1; k < artIndexesAS[i].size(); k++)
//		 {
//			for(int z=badSongs.size()-1; z > -1 && badSongs[z] > artIndexesAS[i][0]; z--)
//			  if(artIndexesAS[i][k] == badSongs[z])
//				{ alreadyChecked=1;  break; }    // we already checked this song.
//
//			if(alreadyChecked) { alreadyChecked=0; continue; }
//
//			if(dataAllSongs->Strings[artIndexesAS[i][j]*4+1]
//			== dataAllSongs->Strings[artIndexesAS[i][k]*4+1])
//			{
//				badSongs.push_back(artIndexesAS[i][k]);       // punish double!
//			}
//		 }
//
//	  }
//   }
//
////	  mform->bs->Caption = badSongs.size();                 // DEBUG       // DEBUG              // DEBUG
//
//   std::sort(badSongs.begin(), badSongs.end());                    // delete doubles
//   for(int i=badSongs.size()-1; i > -1; i--)
//   {
//	  for(int j=3; j > -1; j--)
//		 dataAllSongs->Delete(badSongs[i]*4);
//   }
//}

////////////////////////////////////////////////////////////////////////////////////////

// NEW CODE DON'T NEED THIS
//UINT AddingFiles::getAllList() 									 // shows all listenings for all time (2523)
//{
//	UINT list = 0;
//
//	for(int i=0; i < dataAllSongs->Count; i+=4)
//		 list += dataAllSongs->Strings[i+3].ToInt();
//
//	return list;
//}

////////////////////////////////////////////////////////////////////////////////////////

std::vector <UINT> AddingFiles::getSelectedSongs() 		// gets indexes of all selected songs in pl by user
{
   std::vector<UINT> tempVec;
   if(!mform->playlist->SelCount) return tempVec;    // 0 songs selected
														 // get first selected item
   int lastSelSong = ListView_GetNextItem(mform->playlist->Handle, -1, LVNI_SELECTED);

   while(true)
   {
	  tempVec.push_back(lastSelSong);
														 // get next selected item
	  if(ListView_GetNextItem(mform->playlist->Handle, lastSelSong, LVNI_SELECTED) != -1)
		 lastSelSong = ListView_GetNextItem(mform->playlist->Handle, lastSelSong, LVNI_SELECTED);
	  else break;
   }

   return tempVec;
}

////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::copyToPlaylist(UnicodeString plName, std::vector <Songs> &movingSongs)
{                                     // SCounterTest/playlists/
	UnicodeString path = pl.getSettingsPath() + "playlists/" + pl.delNonPathChars(plName) + "/";

	TStringList *plData = new TStringList;              	// pl data    LOAD
	plData->LoadFromFile(path + "playlistData");        	// + first free index
	UINT firstFreeIndex = plData->Count / 7;

	TStringList *scsData = new TStringList;             	// SCS data   LOAD
	scsData->LoadFromFile(path + "SCSdata");
	bool Started[5];
	int  scs[5];
	for(int i=0; i < scsData->Count; i+=2)
	{
	   Started[i/2] = scsData->Strings[i].ToInt();     //  started[5]
	   scs[i/2]     = scsData->Strings[i+1].ToInt();   //  scs[5]
	}

	  ////&&&&&&&&&&&&&&&&&&&&&& LOOKING FOR DOUBLES &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

  std::vector <UnicodeString> plArtists;          // indexation of loaded playlist
   for(int i=0; i < plData->Count; i+=7)  plArtists.push_back(plData->Strings[i+1]);
  Indexation(plArtists, artBef, artIndexesBef);

  for(int i=movingSongs.size() -1; i > -1; i--)       // delete doubles
  {
	   if(pl.existsInPlaylist("bool", movingSongs[i].artist, movingSongs[i].title, plData))
		  movingSongs.erase(movingSongs.begin() + i);
  }

	  ////&&&&&&&&&&&&&&&&&&&&&& LOOKING FOR DOUBLES &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

//    bool anotherException = false; // do we copy
	if(movingSongs.size())         // user wants to know, if everything gone good.
		messageForm->playBeep('i');
	else
	{
		messageForm->playBeep('w');   // oops, it already was there!
        delete plData, scsData;
        return;
    }

	TStringList *orderData = new TStringList;        	    //  orders[5]       LOAD
	for(int i=0; i < 5; i++)                                //                & SAVE
		if(Started[i])
		{
			orderData->Clear();
			orderData->LoadFromFile(path + "orders/" + i);
			for(int j=0; j < movingSongs.size(); j++) orderData->Add(firstFreeIndex +j);

			orderData->SaveToFile(path + "orders/" + i);
		}

	if(plName == pl.activePlaylist) {                  // add new songs to ACTIVE playlist

		for(UINT i=0; i < movingSongs.size(); i++)       // fix indexNORM
			movingSongs[i].indexNorm = i + firstFreeIndex;


	 firstFreeIndCopyToPl = songs.size();    // this will fix function

										  // af.shuffleAdd(firstFreeIndex) (in delayTimer, after loop below)
		for(UINT i=0; i < movingSongs.size(); i++)
			songs.push_back(movingSongs[i]);

																							// playlist width
	   if(songs.size() > mform->playlist->VisibleRowCount) mform->playlist->Column[3]->Width = plColWidthMin;
	   else 										 	   mform->playlist->Column[3]->Width = plColWidthMax;

	   mform->playlist->Items->Count = songs.size();


		mform->delayTimerTimer(NULL);
	    mform->playlist->Refresh(); // fix artifacts
	 firstFreeIndCopyToPl = -1;  // reset the value, so it won't mess when af.shuffleAdd(firstFreeIndex)
	}                            // will be called with other func.
	else {
		for(int i=0; i < movingSongs.size(); i++)     // add new songs to NOT ACTIVE playlist
		{
		   plData->Add(movingSongs[i].title);
		   plData->Add(movingSongs[i].artist);
		   plData->Add(movingSongs[i].path);
		   plData->Add(movingSongs[i].duration);
		   plData->Add(movingSongs[i].date);
		   plData->Add(movingSongs[i].listenings);
		   plData->Add(i + firstFreeIndex);
		}
		plData->SaveToFile(path + "playlistData", TEncoding::UTF8);
	}


	if(plName == pl.activePlaylist)
	{
		pl.refreshSongCounter("curr");           // refresh all stuff, that show count of songs.
		pl.refreshSongCounter("comm");
		pl.refreshPlDuration();
    }

	delete plData;
	delete scsData;
	delete orderData;
}

////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::messUpTheVector(UINT begin, std::vector<int> &givenVector) // [0,1,2,  7,8,5,9,4,3]
{
	if(begin >= givenVector.size()) return; 		// GUARD

	std::vector <int> rightChunk;                           // fill [3,4,5,6,7,8,9]
	for(int i=begin; i < givenVector.size(); i++) rightChunk.push_back(givenVector[i]);

	std::vector <int> randIndexes;                         // make indexes for rChunk [0,1,2 ... 6]
	pl.makeVectorOfRandInts(randIndexes, 0, givenVector.size() - begin, 0);

	std::vector <int> tempVector = rightChunk;              // mess up rChunk [8,3,5,7,9,4,6]
	for(int i=0; i < rightChunk.size(); i++)
		rightChunk[randIndexes[i]] = tempVector[i];

	for(int i=begin; i < givenVector.size(); i++)
		givenVector[i] = rightChunk[i-begin];
}

////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::loadSongFullInfo(Songs &song)                    // load tags, checks doubles, etc. (one song)
{
	stream = BASS_StreamCreateFile(false, song.path.c_str(),0,0,0);    // 3:43 duration
	song.duration = song.getDuration(stream, 1);
	BASS_StreamFree(stream);

	ID3Tags tag(song.path, false);                                     // artist, title
	song.artist = tag.artist;
	song.title  = tag.title;

	// I DON'T USE THIS ANYMORE (MAKES SONG NAME "=== NO TAGS V2" ISTEAD OF "=== NO TAGS V2 - === NO TAGS V2"
//	if(song.artist.SubString(1,3) == "===")        // name is BAD TAG
//	{
//		if(song.artist != "=== NO TAGS V2"
//		 &&song.artist != "=== FILE NOT FOUND"
//		 &&song.artist != "=== CONVERT TO TAG V2.4")
//			goto jump;        					 // it has good name
//		else song.name = song.artist;    // it has bad name
//	}
//	else  { jump:
	   song.name = song.artist + " - " + song.title;
//	}

	song.getListeningAndDate(song);								   // listenings, date
}

////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::loadAddedSongsData(UINT firstIndex)          // load tags, checks doubles, etc.
{
	mform->disableMainForm(false);
	firstLoadingInd = firstIndex;
	mform->labelPlDuration->Caption = "Loading data...";


//	{                                               // indexation of AllSongs.txt data
//	  std::vector <UnicodeString> tempVect;         // because it's going to read all listenings + dates
//	  for(int i=0; i < dataAllSongs->Count; i+=4)
//		tempVect.push_back(dataAllSongs->Strings[i].LowerCase());     // artists
//	  Indexation(tempVect, artAS, artIndexesAS);
//	}
    AllSong::makeIndexation(LetterCase::Lower);

	for(int i=firstIndex; i < songs.size(); i++, firstLoadingInd++)
	{
		Application->ProcessMessages();
		if(programIsClosing) return;

		loadSongFullInfo(songs[i]);
		mform->playlist->UpdateItems(i,i);              // update item

		mform->labelPlCount->Caption = (i - firstAddedInd)*100 / (songs.size() - firstAddedInd);
		mform->prbarDataLoaded->Position = mform->labelPlCount->Caption.ToInt()*10; // progress bar 58%
       	mform->labelPlCount->Caption = mform->labelPlCount->Caption + "%";    // 58% label progress
	}

	rearrangePlaylist();    // all data is loaded. Now make usable playlist!
	mform->disableMainForm(false);
		if(!pl.stopped)                                   // if song is playing, restore btnDelPlSong enabled state
			mform->enableBtnPanel(mform->btnDelPlayingSong);
}

////////////////////////////////////////////////////////////////////////////////////////

void AddingFiles::rearrangePlaylist() 			// happens when songs are added and their data are loaded
{
	short currFilterTag = mform->filterbtn->Tag;
	deleting = true;        // don't trigger "falseStop()
	mform->prbarDataLoaded->Position = 0; // disable progress bar 58%

	std::vector <UnicodeString> tempVecOfArtists;     // Indexation of curr Playlist
	for(int i=0; i < firstAddedInd; i++)
		tempVecOfArtists.push_back(songs[i].artist.LowerCase());    // artists.lowercase
	Indexation(tempVecOfArtists, artBef, artIndexesBef);




	UINT doublesCount = 0;						   // LOOKING FOR DOUBLES!
	sameAdded.clear();
    samePlaylist.clear();
	for(int i=songs.size() -1; i >= firstAddedInd; i--)
	{
		int exists = pl.existsInPlaylist("int", true, i, songs[i].artist, songs[i].title);
		if(exists > -1)                              // it already exists in playlist!!!
		{
			 sameAdded.push_back(songs[i].path);         // adding song
			 songs.erase(songs.begin() + i);             // delete added double!!!
			 doublesCount ++;
			 samePlaylist.push_back(exists);             // song in playlist
		}
	}
	mform->playlist->Items->Count -= doublesCount;      // for best perfomance

	pl.sortSongs(0, false);                             // fix indexNorm after deleting doubles
	for(UINT i=0; i < songs.size(); i++)
        songs[i].indexNorm = i;


	if(setForm && setForm->cboxAllPl->Checked
	  && pl.activePlaylist != "Allsongs"   // copy songs to All Songs pl
	  && mform->togCheckingMode->State == tssOn)
	{
		std::vector <Songs> addedSongs = songs;
		if(firstAddedInd != 0)                                   // delete songs that already were in pl
			addedSongs.erase(addedSongs.begin(), addedSongs.begin() + firstAddedInd -1);
		copyToPlaylist("All songs", addedSongs);
	}

   shuffleAddFiles(firstAddedInd);					  // make indexes for orders, if shuffle was ON
   if(mform->shufflebtn->Tag) 		   // if shuffle was ON, then sort
	pl.sortSongs(2 + currFilterTag, false); pl.sortSongs(2 + currFilterTag, false);

   mform->filterbtn->Tag = -1;                                  // filter all songs again, from 0
   pl.filter(currFilterTag);

   firstLoadingInd = -1;    	   // playlist loaded successfully
   pl.refreshSongCounter("comm");  // AFTER FILTERING!!!
   pl.makeVisibleCurrSong(4, firstAddedInd);
   firstAddedInd = -1;
   deleting = false;

																				// playlist width
	if(songs.size() > mform->playlist->VisibleRowCount) mform->playlist->Column[3]->Width = plColWidthMin;
	else 										 		mform->playlist->Column[3]->Width = plColWidthMax;

//   AllSongRevision();                                // delete duplicates in AllSongs
   // NEW CODE
    AllSong::makeRevision();

   	showSameSongs();    // show, which songs user was trying to add
}

////////////////////////////////////////////////////////////////////////////////////////
