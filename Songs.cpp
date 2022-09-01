#include "Songs.h"

    // INCLUDES ===============//
#include "AvId3Reader.h"
#include "AddingFiles.h"
#include "AllSong.h"

extern AddingFiles af;
extern std::vector <AllSong> allSongs;
extern std::vector <UnicodeString> indArtists;  	   // INDEXATION
extern std::vector <std::vector <int> > indArtIndexes; // INDEXATION


Songs::Songs(const UnicodeString &givenFileName, const UINT &IndexNorm)    // NEW CONSTRUCTOR
{                                                                                // for adding new files
	path = givenFileName;              // "C:/music/Metallica"
//	fileName = ExtractFileName(givenFileName.Delete(givenFileName.Length()-3, 4));
	fileName = ExtractFileName(givenFileName.SubString(1, givenFileName.Length()-4));   // 08. Until it sleeps
	name = fileName;                   // (08. Until it sleeps - bef D%D) Metallica - Until it sleeps - aft.D%D
	indexNorm = IndexNorm;             // indexNorm
	listenings = 0;                    // fill that!

    selected = false;                  // don't leave empty variables!!! it's very dangerous!
	indexFiltered = 0;
	std::fill(order, order + 5, 0);
}

Songs::Songs(const UnicodeString &Title, const UnicodeString &Artist, const UnicodeString &Path  // OLD CONSTRUCTOR
	, const UnicodeString &Duration, const UnicodeString &Date
	, const unsigned short &Listenings, const int &IndexNorm)
{
	title = Title;
	artist = Artist;
    name = Artist + " - " + Title;
	path = Path;
	duration = Duration;
	date = Date;
	listenings = Listenings;
	indexNorm = IndexNorm;
	indexFiltered = 0;
	std::fill(order, order + 5, 0);
    selected = false;
}

////////////////////////////////////////////////////////////////////////////////////

UnicodeString Songs::getDuration(HSTREAM &streamIN, int choice)                             // gets duration of the song.   [0:20][04:21]
{
	QWORD length;
	if(choice == 0) length = BASS_ChannelGetPosition(streamIN, BASS_POS_BYTE);    // 0 = active duration [0:20]
	else            length = BASS_ChannelGetLength  (streamIN, BASS_POS_BYTE);    // 1 = common duration [4:21]

	int dur = BASS_ChannelBytes2Seconds(streamIN, length);    // gotten duration
															  // to seconds
	UnicodeString duration;
	if(dur/60 < 10) duration += "0";    // mins
	duration = duration + dur/60 + ":";
	if(dur%60 < 10) duration += "0";    // secs
	duration += dur%60;

	return duration;
}

////////////////////////////////////////////////////////////////////////////////////

void Songs::getListeningAndDate(Songs &songToCheck)
	   // Returns listenings. Pass string to set date. USES INDEXATION allSongs.txt FROM AF!
{
	if(IHaveBadTags(songToCheck.name, songToCheck.artist, songToCheck.title))   {
		songToCheck.date = "--.--.---- --:--";
		songToCheck.listenings = 0;
		return;
	}

	for(int i=0; i < indArtists.size(); i++)
	{    // NEW CODE  indArtists.size();

		if(songToCheck.artist.LowerCase() == indArtists[i])   // artist == artist
		{                                   // indArtists[i]
			for(int j=0; j < indArtIndexes[i].size(); j++)
			{               // indArtIndexes[i].size();                                   // name == name
			   if(songToCheck.name.LowerCase() == allSongs[indArtIndexes[i][j]].name().LowerCase())
			   {                                  // name.LowerCase();
				   songToCheck.date       = allSongs[indArtIndexes[i][j]].birthDate();
											  //allSongs[indArtIndexes[i][j].getDate();
				   songToCheck.listenings = allSongs[indArtIndexes[i][j]].playCount();
											 //allSongs[indArtIndexes[i][j].getPlayCount();
				   return;
			   }
			}
			break;
		}
	}
		 // if song got here, it means it wasn't in allSongs data

//	af.addToAllsongs(songToCheck.indexNorm);
	// NEW CODE
	if(mform->togCheckingMode->State == tssOn) {
       allSongs.push_back(AllSong(songToCheck.artist, songToCheck.title));
		songs[songToCheck.indexNorm].date = allSongs[allSongs.size() -1].birthDate();
		songs[songToCheck.indexNorm].listenings = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////

bool Songs::IHaveBadTags(const UnicodeString &name, const UnicodeString &artist, const UnicodeString &title)
	   // if songs has "=== NO TAGS V2" or smth like this, returns true.
{
	if(artist == "=== NO TAGS V2"
	 ||artist == "=== FILE NOT FOUND"
	 ||artist == "=== CONVERT TO TAG V2.4"
	 ||artist == "=== NO ARTIST"
	 ||artist == "=== SPACE ON THE BEG."
	 ||title  == "=== NO TITLE"
	 ||title  == "=== SPACE ON THE BEG.")
		return true;

    return false;
}
////////////////////////////////////////////////////////////////////////////////////
