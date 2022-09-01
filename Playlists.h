#ifndef PLAYLISTS_H
#define PLAYLISTS_H

// ============== my includes ===================== //
#include <vcl.h>
#include <vector>
#include <algorithm>
#include <inifiles.hpp>

#include "Unit1.h"
#include "Songs.h"
#include "AddingFiles.h"
#include "message.h"
#include "enums.h"

class AddingFiles;

extern AddingFiles af;
extern std::vector <Songs> songs;
extern short lastVolume;
extern unsigned int todayListenings;
extern unsigned int startList;
extern std::vector <int> lastSongs;


class  Playlists
{
	int playingSong;         // index of currently playing song
	int actPlayingSong;      // index of currently playing song (shows real playing song)
	int selectedSong;        // index of currently selected song
	int firstInactive;       // index of first inactive song
	bool playing;            // is player playing?
	bool stopped;            // check if playback was stopped
	bool iterated;           // was 70% of song listened?
	bool started[5];         // was one of shuffle playlists started?
	bool terminatingSong;    // is that last song in this session?
    bool showTermSongMes;    // show term.song message or not?
	bool showTheEnd;         // show message ("the end of pl")?
	int shuffleCurrSong[5];  // current song in shuffle (for user, starting from 1. For me from 0)
                             // SCS starts from 1 from 1 from 1!!!
	int filterMin[4];        // for filter
	int filterMax[4];        // for filter
	std::vector <UINT> savedSelectedSongs;  // memorize selected songs
	std::vector <UnicodeString> playlists; // all playlists in SCounter
    UnicodeString activePlaylist;          // which playlist is active? Without \/ *: ...

	TStringList *playlistData;  // data from visible LV
    TIniFile *iniConfig;     // INI for shuffle
	HSTREAM stream;          // for actual playing song

public:

  friend class Tmform;
  friend class TmessageForm;
  friend class AddingFiles;
  friend class TsetForm;
  friend class TformPlaylists;
  friend class TformBadSongs;
  friend class TformArtists;
  friend void CALLBACK onSongEnded(HSYNC handle, DWORD channel, DWORD data, void *user);
  friend struct PlayingSong;        // for playingSongInfo only

  Playlists();         // just constructor

  void loadPlaylist(UnicodeString name);
	 // loads playlist
  void savePlaylist(UnicodeString name);
	 // saves playlist

  void findDoubles(bool show);
	 // on findDoublesClick

  void makeVectorOfRandInts(std::vector <int> &givenVector, int firstIndex, int lastIndex,int srandValue);
	 // Fill given vector by random ints

  void sortSongs(short choice, bool displayChanges);
     // sorts vector of songs. 0 = by indexNorm, 1 = by indexShuffle

  void filter(short choice);       // 0 = All, 1 = 1-st filter and etc.
	 // filters playlist
  void filterCopy(short choice);
	 // filters Copy method

  UnicodeString getActivePlFullName() const;
	 // gets name, like "\\\ WOW ///"

  void loadPlaylistSettings();
	 // this function for OnCreate
  void loadForOtherForms(char form);
     // for other forms
  void savePlaylistSettings();
	 // this function for OnClose
  UnicodeString getSettingsPath();
	 // returns "C:/Users/.../Scounter/settings"


  void refreshSongCounter(String input);
	 // comm - for common counter, curr - for current pos. in shuffle playlist
  void refreshPlDuration();
     // 05:16:59 for pl

  bool iteration();
	// only checks if 75% was listened

  void iterate();
	// change counter everywhere

  void shuffleClick(bool programmatically);
	// [onShuffleClick]

  void makeVisibleCurrSong(short choice,  int firstFreeInd);
	// user should know which song is playing...

  int progressBar();
	// returns % of listened song for progressBar animation

  void refreshFilterCap();
	// refreshes captions in mform

  void saveSelection();
	// memorizes all selected songs
  void restoreSelection();
	// restores selected songs

  void loadPlaylistButtons();
	// loads buttons to the panelMain
  void savePlaylistButtons();
	// saves buttons to the panelMain
  void saveEverything();
	// saves all "saveable" data

  void refreshAllSCounterFilters(Filter filter);
  	// loads all playlists and deletes SCS data.

  void loadLastSelSong();
	// what index was when scounter was closed in earlier instance?

  UnicodeString delNonPathChars(UnicodeString name) const;
	// deletes chars   \/ * : ?  <> " |    from string

  UnicodeString getLastPlName();
	// gets "My playlist" without \/ * : ...

  void refreshListeningsForLoadingPl();
	// check listenings before load pl

  int existsInPlaylist(std::string returns, bool usingIndexation, int knownIndex  // for current pl only
	, const UnicodeString &artist, const UnicodeString &title);
  int existsInPlaylist(std::string returns, int knownIndex                     // function #2
	, const UnicodeString &artist, const UnicodeString &title, UnicodeString plName) ;     // for choosen playlist
  int existsInPlaylist(std::string returns                                     // function #3
	, const UnicodeString &artist, const UnicodeString &title, TStringList *plData);       // for choosen playlist
	// returns -1 (or "false") if song not found
	// for searching doubles - pass index to "knownIndex" parameter else pass "-1"

  Songs existsInScounter(UnicodeString artist, UnicodeString title);
	// checks for song in all playlists (starts from activePlaylist)


// ======= = = = = = ALL SONGS =================== ///
  int existsInAllSongs(const std::string &returningValue, const bool &usingIndexation
  						, const UnicodeString &artist, const UnicodeString &title) const;
    // check whether given song exists in AllSongs file

  void addToSongs(Songs song);
	// adding new song to the Songs vector

							  // PLAYING FEATURES

  UnicodeString refreshingDuration();
	 // refresh adur [00:34]

  void refreshSong();
     // for [ENTER + DBLCLICK]

  void playPauseResume();
	 // when "play" button is clicked

  void stop();
	 // stops playing
  void falseStop();
     // when clicked shuffle, filter and etc.

  void prevNext(int choice);
	 // plays previous/next song

  void theEndM();
	 // happens when shuffle playlist ends

  void skipping();
	 // counts amount msecs when "nextbtn" was clicked

  UINT getPos() const;
	 // returns position of HSTREAM in milliseconds

  static void CALLBACK onSongEnded(HSYNC handle, DWORD channel, DWORD data, void *user );
  void endSong();
	 // makes things when song ends
};


#endif
