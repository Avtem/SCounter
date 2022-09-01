#ifndef ALLSONG_H
#define ALLSONG_H

	  // ============ MY INCLUDES ============
#include <vcl.h>  // many useful stuff here
#include <vector> // vector.
#include "enums.h"
#include "Songs.h"

	  // ============ MY FUNCTIONS ============


	  // ============ MY VARIABLES ============


class AllSong
{
	unsigned int mId;
	UnicodeString m_artist;
	UnicodeString m_title;
	TDateTime m_birthDay;
	UnicodeString m_birthDayStr;  // TEMP VARIABLE (for getter only)
	UnicodeString m_name; 	      // TEMP VARIABLE (for getter only)
	TDateTime m_lastPlayed;
	TDateTime m_lastSkipped;
	int m_playCount;
	int m_skipCount;

//	static TDateTime m_ScounterBirthDay();              // TODO
//	static UnicodeString getScounterBirthDayStr();

    static unsigned int mFirstFreeId;
	static const TDateTime m_nullDate;  	  // "09.09.9999"
	static const UnicodeString m_dateFormat;  // "dd.mm.yyyy hh:nn";
   	static const UnicodeString m_dateFormatSimple;  // "dd.mm.yyyy"
	static UnicodeString m_path;  			  // path to "allsongs.txt" file
	static const WCHAR m_separator;          // 0xF00F

public:
	AllSong(const UnicodeString &artist, const UnicodeString &title);  // FOR BRAND NEW
	AllSong(const UnicodeString *artist, const UnicodeString *title,   // FOR LOADING
		const UnicodeString *birthDay, const UnicodeString *lastPlayed,
		const UnicodeString *lastSkipped, const int &playCount, const int &skipCount,
		const int &id);

	static void loadFromFile();
	static void saveToFile();
	static const UINT getAllListenings();
    static AllSong *find(const unsigned int &id);
	static AllSong *find(const UnicodeString &artist, const UnicodeString &title);
	static UnicodeString * const takePart(TStringList *const list, const int &iterator);
	static void makeIndexation(LetterCase Case);
	static void makeRevision();   // deletes all duplicates in allSongs
    static int bytesRequiredForIds();  // how many bytes are needed for storing ids?

	void incrementPlayCount();
	void updatePlayDate();
	void incrementSkipCount();
	void updateSkipDate();

// GETTERS
    const unsigned int &id() const { return mId; }
	const UnicodeString &artist() const { return m_artist; }
	const UnicodeString &title()  const { return m_title;  }
	const UnicodeString &name();  // depricated
	const int &playCount()  const { return m_playCount;  }
	const int index() const;
	const UnicodeString &birthDate();      // "full" format
	const UnicodeString &birthDateShort(); // "09.01.2013" format
};

#endif
