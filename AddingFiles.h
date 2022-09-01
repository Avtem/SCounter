#ifndef ADDINGFILES_H
#define ADDINGFILES_H

#include <vcl.h>
#include "libraries/bass.h"
#include <vector>
#include "Songs.h"
#include "Playlists.h"
#include "sameFileExists.h"
#include "Unit1.h"
#include "llistenings.h"
#include "message.h"

class  Playlists;
extern Playlists pl;
extern std::vector <Songs> songs;

class AddingFiles
{
  bool deleting;                                     // don't stop song, if deletingSongs() func. used in pl.filter()
  bool programIsClosing;                             // program is going to close
  int firstLoadingInd;    // -1 if pl loaded 		 // first index to continue loading data (tag, doubles, duration, etc.)
  int firstAddedInd;                                 // which songs.size was before D&D?
  HSTREAM stream;                                    // here song's stream
//  TStringList *dataAllSongs;   					     // all data from AllSongs.txt
  // NEW CODE (DON'T NEED THIS ANYMORE)

  std::vector <UnicodeString> artBef;                // indexation of playlist(BEFORE Drag&Drop)
  std::vector <std::vector <int> > artIndexesBef;    //

  // DON'T NEED THIS ANYMORE
//  std::vector <UnicodeString> artAS;                 // indexation of AllSongs.txt   - Metallica
//  std::vector <std::vector <int> > artIndexesAS;     // [13, 14, 292, 293]    !!! multiply by 4!!!

  std::vector <int> samePlaylist;     				 // index of the original song
  std::vector <UnicodeString> sameAdded;      	     // for same songs (filePath)
  std::vector <int> samePlaylist2;     				 // index of twin song

public:
				 // DON'T NEED THIS ANYMORE
//  ~AddingFiles() { delete dataAllSongs; } //

  friend class Tmform;
  friend class Playlists;
  friend class TListenings;
  friend class TsetForm;
  friend class Songs;
  friend class TForm2;
  friend class TformBadSongs;
  friend class TformArtists;

  void process_dropped_filename(UnicodeString &FileN);
	   // Drag & Drop continuation

  bool isFolder(UnicodeString &filePath);
	   // is that folder?

  bool associationOk(UnicodeString &filePath);
	 // does player support this extension?

  void addThisFile(UnicodeString &filePath);
	 // add this file to list

  void shuffleAddFiles(int firstFreeIndPl);
	 // on adding new files when shuffle is ON

  void Indexation(std::vector <UnicodeString> &inputVec, std::vector <UnicodeString> &artists
  , std::vector <std::vector <int> > &artistsInd);
	 // index received list of files

  void showSameSongs();
	 // shows same song if it is already in playlist

  std::vector <UINT> getSelectedSongs();
	 // gets indexes of all selected songs in pl by user

  void copyToPlaylist(UnicodeString plName, std::vector <Songs> &movingSongs);
	 // copies passes songs to plName

  void messUpTheVector(UINT begin, std::vector <int> &givenVector);
	 // [0,1,2,  7,8,5,9,4,3]    begin = 3

  void loadSongFullInfo(Songs &song);
	 // load tags, checks doubles, etc. (one song)
  void loadAddedSongsData(UINT firstIndex);
	 // load tags, checks doubles, etc. (many songs, after D&D)

  void rearrangePlaylist();
	 // happens when songs are added and their data are loaded


	 // DON'T NEED THIS ANYMORE
  void loadAllSongsData();                                             // ALLSONGS
//	  loads aaaallll songs.
	 // DON'T NEED THIS ANYMORE
//  void addToAllsongs(int indexInPl);
	 // adds (only one) song info to "dataAllSongs" with name, date of adding and listenings

  void AllSongRevision();// DON'T NEED THIS ANYMORE
	 // deletes duplicates from AllSongs.txt

  void deleteFiles();
	 // deleting selected files by user

//  UINT getAllList(); NEW CODE (DON'T NEED THIS)
	 // shows all listenings for all time at this moment (2523)


};


#endif
