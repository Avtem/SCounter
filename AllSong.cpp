#include "AllSong.h"

	  // ============ MY INCLUDES ============
#include "Unit1.h"  // so we can use pl,af
#include "Playlists.h"
#include "AddingFiles.h"
#include "enums.h"
#include <math.h>

      // ============ MY FUNCTIONS ============


	  // ============ MY VARIABLES ============
std::vector <AllSong> allSongs;   // here we store all data.
std::vector <UnicodeString> indArtists;  		// INDEXATION
std::vector <std::vector <int> > indArtIndexes; // INDEXATION

extern Playlists pl;
extern AddingFiles af;
extern UnicodeString intToUS(const int &integer);  	// conver int to US string

// INIT STATIC VALUES
UnicodeString getFilePath()
{  return pl.getSettingsPath() + "settings/AllSongsPC.txt"; }
UnicodeString AllSong::m_path = getFilePath();  // path to "allsongs.txt" file
const UnicodeString AllSong::m_dateFormat = "dd.mm.yyyy hh:nn";
const UnicodeString AllSong::m_dateFormatSimple = "dd.mm.yyyy";
const TDateTime AllSong::m_nullDate(9999,9,9);  	  // "--.--.----"
const WCHAR AllSong::m_separator = WCHAR(0xF00F);
unsigned int AllSong::mFirstFreeId = 256;   // 0-255 reserved

////////////////////////////////

AllSong::AllSong(const UnicodeString &artist, const UnicodeString &title)   // FOR BRAND NEW SONGS
   : m_artist(artist), m_title(title), m_birthDay(Now()),       // everything is 0
	 m_lastPlayed(m_nullDate), m_lastSkipped(m_nullDate), m_playCount(0), m_skipCount(0)
{
	mId = mFirstFreeId;
	mFirstFreeId ++;
}

AllSong::AllSong(const UnicodeString *artist, const UnicodeString *title,   // FOR LOADING
		const UnicodeString *birthDay, const UnicodeString *lastPlayed, const UnicodeString *lastSkipped,
		const int &playCount, const int &skipCount, const int &id)
	: m_artist(*artist), m_title(*title), m_playCount(playCount), m_skipCount(skipCount),
      mId(id)
{
	if(*birthDay != "NBD")
		m_birthDay = TDateTime(*birthDay);
	if(*lastPlayed != "NPD" && *lastPlayed != "--no play date--")
		m_lastPlayed = TDateTime(*lastPlayed);
	if(*lastSkipped != "NSD" && *lastSkipped != "--no skip date--")
		m_lastSkipped = TDateTime(*lastSkipped);
}

UnicodeString *const AllSong::takePart(TStringList *const list, const int &iterator)
{
	UnicodeString *str = new UnicodeString();
	*str = list->Strings[iterator].SubString1(1,list->Strings[iterator].Pos1(m_separator) -1);

	list->Strings[iterator] = list->Strings[iterator].Delete(1, str->Length() +1);

	return str;
}

void AllSong::loadFromFile()
{
	if(!FileExists(m_path))                  // GUARD
	{
		ShowMessage("Can not open file \"AllSongsPC.txt\"");
		mform->Exit1Click(NULL); // critical error. Exit
	}

	TStringList *list = new TStringList;
	list->LoadFromFile(m_path, TEncoding::UTF8);

    allSongs.clear();
	for(UINT i=6; i < list->Count; i++)
	{
		UnicodeString *artist = takePart(list, i);
		UnicodeString *title  = takePart(list, i);
		UnicodeString *birthDay = takePart(list, i);
		UnicodeString *lastPlayed = takePart(list, i);
		UnicodeString *lastSkipped = takePart(list, i);
		UnicodeString *playCount = takePart(list, i);
		UnicodeString *skipCount = takePart(list, i);
		UnicodeString *id = takePart(list, i);
		unsigned int idUint = id->ToInt();

	   // getting firstFreeId from AllSongs.txt
		if(id->ToInt() >= mFirstFreeId)
			mFirstFreeId = id->ToInt();

		allSongs.push_back(AllSong(artist,  	  // artist
								   title,  		  // title
								   birthDay,      // birthDay
								   lastPlayed,    // lastPlayed
								   lastSkipped,   // lastSkipped
								   playCount->ToInt(),     // playCount
								   skipCount->ToInt(),    // skipCount
								   idUint));           // id

		delete artist, title, birthDay, lastPlayed, lastSkipped, playCount, skipCount, id;
	}

	mFirstFreeId ++;

	delete list;
}

void AllSong::saveToFile()
{
	if(!FileExists(m_path))                  // GUARD
	{
		ShowMessage("Can not open file \"AllSongsPC.txt\"");
		mform->Exit1Click(NULL); // critical error. Exit
	}

	TStringList *list = new TStringList;

	list->Add("AllSongsInfo: " + UnicodeString(allSongs.size()));
	list->Add("SCounterBirth: ");// + m_ScounterBirthDay().FormatString(m_dateFormat));
	list->Add("Reserved.");    list->Add("Reserved.");    list->Add("Reserved.");
	list->Add("");

	UnicodeString line;
	UnicodeString emptyPlayDate = "NPD"; emptyPlayDate += m_separator;
	UnicodeString emptySkipDate = "NSD"; emptySkipDate += m_separator;
	#ifdef DEBUG
		ShowMessage("RUNNING DEBUG BUILD");
	#endif

	for(UINT i=0; i < allSongs.size(); i++)
	{
		line = allSongs[i].m_artist; line += m_separator;
		line += allSongs[i].m_title; line += m_separator;
		line += allSongs[i].m_birthDay.FormatString(m_dateFormat) + m_separator;
		if(allSongs[i].m_lastPlayed.Val == 0)
			line += emptyPlayDate;
		else
			line += allSongs[i].m_lastPlayed.FormatString(m_dateFormat) + m_separator;
		if(allSongs[i].m_lastSkipped.Val == 0)
			line += emptySkipDate;
		else
			line += allSongs[i].m_lastSkipped.FormatString(m_dateFormat) + m_separator;
		line += intToUS(allSongs[i].m_playCount) + m_separator;
		line += intToUS(allSongs[i].m_skipCount) + m_separator;
		line += intToUS(allSongs[i].mId)         + m_separator;

		list->Add(line);
	}

	list->SaveToFile(m_path, TEncoding::UTF8);

	delete list;
}

// static
AllSong* AllSong::find(const UnicodeString &artist, const UnicodeString &title)
{
	for(UINT i=0; i < allSongs.size(); i++)
		if(allSongs[i].artist().LowerCase() == artist.LowerCase()
		  && allSongs[i].title().LowerCase() == title.LowerCase())
				return &allSongs[i];

	return NULL;      // nothing found
}

void AllSong::makeIndexation(LetterCase Case)
{
	indArtists.clear(); indArtIndexes.clear();

	std::vector <UnicodeString> artistList;            // indexation of AllSongs.txt data

	if(Case == LetterCase::Lower)
		for(UINT i=0; i < allSongs.size(); i++)
			artistList.push_back(allSongs[i].artist().LowerCase());      // artists
	else
		for(UINT i=0; i < allSongs.size(); i++)
			artistList.push_back(allSongs[i].artist());    			     // Artists

	af.Indexation(artistList, indArtists, indArtIndexes);
}

void AllSong::incrementPlayCount()
{
	m_playCount ++;
	updatePlayDate(); // and also update date, because it's Necessary
}

void AllSong::incrementSkipCount()
{
	m_skipCount ++;
	updateSkipDate();  // and also update date, because it's Necessary
}

void AllSong::updatePlayDate()
{
	m_lastPlayed = Now();
}

void AllSong::updateSkipDate()
{
	m_lastSkipped = Now();
}

const int AllSong::index() const  // find index of 'this' in allSongs vector
{
	for(UINT i=0; i < allSongs.size(); i++)
		if(this->m_artist == allSongs[i].artist())
			return i;

    return -1; // nothing found
}

const UnicodeString &AllSong::birthDateShort() // "09.01.2013" format
{
	m_birthDayStr = m_birthDay.FormatString(m_dateFormatSimple);
	return m_birthDayStr;
}

// static
int AllSong::bytesRequiredForIds()  // how many bytes are needed for storing ids?
{
	return int(log(float(mFirstFreeId)) /log(256.0f) +1);
}

const UINT AllSong::getAllListenings()
{
	UINT count = 0;
	for(UINT i=0; i < allSongs.size(); i++)
		count += allSongs[i].m_playCount;

	return count;
}

//static
AllSong *AllSong::find(const unsigned int &id)
{
    for(UINT i=0; i < allSongs.size(); i++)
		if(id == allSongs.at(i).mId)
			return &allSongs[i];

	return NULL;      // nothing found
}

void AllSong::makeRevision()    // deletes all duplicates in allSongs
{
	// indexation of AllSongs.txt data
   AllSong::makeIndexation(LetterCase::Lower);

   std::vector <UINT> badSongs;       // indexes to delete data from AllSongs
   bool alreadyChecked = false;

   for(UINT i=0; i < indArtists.size(); i++)
   {
	  for(UINT j=0; j < indArtIndexes[i].size(); j++)
	  {
		 for(UINT k=j+1; k < indArtIndexes[i].size(); k++)
		 {
			for(int z=badSongs.size()-1; z > -1 && badSongs[z] > indArtIndexes[i][0]; z--)
			  if(indArtIndexes[i][k] == badSongs[z])
				{ alreadyChecked = true;  break; }    // we already checked this song.

			if(alreadyChecked) { alreadyChecked=0; continue; }

			UnicodeString name1 = allSongs[indArtIndexes[i][j]].m_artist + " - "
								+ allSongs[indArtIndexes[i][j]].title();
			UnicodeString name2 = allSongs[indArtIndexes[i][k]].m_artist + " - "
								+ allSongs[indArtIndexes[i][k]].title();
			if(name1.LowerCase() == name2.LowerCase())
				badSongs.push_back(indArtIndexes[i][k]);       // punish double!
		 }

	  }
   }

//	  mform->bs->Caption = badSongs.size();                 // DEBUG       // DEBUG              // DEBUG

   std::sort(badSongs.begin(), badSongs.end());                    // delete doubles
   for(int i = badSongs.size()-1; i > -1; i--)
	  allSongs.erase(allSongs.begin() +badSongs[i]);
}

const UnicodeString &AllSong::birthDate()       // "full" format
{
	m_birthDayStr = m_birthDay.FormatString(m_dateFormat);
	return m_birthDayStr;
}


const UnicodeString &AllSong::name()
{
	m_name = m_artist + " - " + m_title;
	return m_name;
}

