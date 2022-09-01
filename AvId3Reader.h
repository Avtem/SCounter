//===========================================================//
//  This library was created to read MP3 tags.               //
//  Supported tags: ID3 v2.4 (UTF 8 only)                    //
//                                                           //
//  started: 16.02.2019                                      //
//  finished: 17.02.2019                                     //
//===========================================================//
//  16.03.2019 - fixed bug when file coudn't be opened       //
//===========================================================//

#ifndef AVID3READERH
#define AVID3READERH

// INCLUDES   // ======= // =======
#include <vcl.h>


struct ID3Header                           // ID3 header (10 bytes)
{
   unsigned char id[3];           // "ID3"
   unsigned char ver;             // $04
   unsigned char rev;             // $00
   unsigned char flags;           // %abcd0000
   uint8_t tSize[4];     // 4bytes * %0xxxxxxx
};
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

class ID3Tags                              // MAIN CLASS
{
public:
              // TAGS
	UnicodeString artist;         // Metallica
	UnicodeString title;          // Fade to Black
	UnicodeString album;          // Ride The Lightning
	UnicodeString track;          // 4
	UnicodeString year;           // 1984
//    UINT duration;                // 343 seconds

	UINT tagsSize;         			  // size of tags (in bytes)
 /////

	ID3Tags(UnicodeString path, bool checkFileExisting);              // constructor
		// reads all tags and saves to object proporties

	void setAllTags(std::string constStr);
		// sets all object proporties
};

#endif

