#ifndef SONGS_H
#define SONGS_H

#include <vcl.h>
#include "libraries/bass.h"
//#include <string>

class Songs
{
public:

   UnicodeString fileName;                   // 08. Until it sleeps
   UnicodeString name;                       // name used for displaying in PLAYLIST
   UnicodeString title;                      // Until it sleeps
   UnicodeString artist;                     // Metallica    !!!(was changed)
   UnicodeString path;                       // E:/music/metallica/other/untilitsleeps.mp3
   unsigned short listenings;                // 7 (listenings)
   UnicodeString duration;                   // 04:23
   UnicodeString date;                       // 02.07.2018    when song was firstly added to the SCounter?
//   UnicodeString lastListening;              // 14.03.2019 13:43 - when user listened last time this song?

   int indexNorm;                            // normal playlist + All filter
   int indexFiltered;                        // normal playlist + ANY filter
   int order[5];                             // every shuffle order is different
   bool selected;


   Songs(const UnicodeString &givenFileName = "0", const UINT &IndexNorm = 0);    // NEW CONSTRUCTOR

   Songs(const UnicodeString &Title, const UnicodeString &Artist, const UnicodeString &Path
	, const UnicodeString &Duration, const UnicodeString &Date                   //  CONSTRUCTOR FOR LOADING
	, const unsigned short &Listenings, const int &IndexNorm);


   UnicodeString getDuration(HSTREAM &streamIN, int choice);
	   // gets duration of the song.   0 = [0:20]  1 = [04:21]

   void getListeningAndDate(Songs &songToCheck);
	   // Returns listenings. Date will be set in passed song. USES INDEXATION FROM AF!

   bool IHaveBadTags(const UnicodeString &name, const UnicodeString &artist, const UnicodeString &title);
       // if songs has "=== NO TAGS V2" or smth like this, returns true.
};



#endif
