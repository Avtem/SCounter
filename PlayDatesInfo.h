//---------------------------------------------------------------------------

#ifndef PlayDatesInfoH
#define PlayDatesInfoH
//---------------------------------------------------------------------------
#include <vcl.h>  // for many useful stuff
#include <vector> // stl vectors

#define uint64 unsigned long long

struct SingleEntry {
	unsigned int allSongId;  // id in AllSongs data
	TDateTime date; // date of playing / skipping

	unsigned long long minCountSincMidnight() {
		// amount of Mins since the day of listening/skippp
		unsigned short hours, mins, secs;
		date.DecodeTime(&hours, &mins, &secs, &secs);
		int a = hours;
		return hours*60 + mins;
	}
};
struct SingleDay {
	TDateTime day;

	std::vector <SingleEntry> playDates;
	std::vector <SingleEntry> skipDates;

	SingleDay(const TDateTime &Day)
		:day(Day)
	{}

	void addPlayTime(const unsigned int &allSongId, const TDateTime &date) {
		playDates.push_back(SingleEntry());
		playDates.back().allSongId = allSongId;
		playDates.back().date = date;
	}
	void addSkipTime(const unsigned int &allSongId, const TDateTime &date) {
		skipDates.push_back(SingleEntry());
		skipDates.back().allSongId = allSongId;
		skipDates.back().date = date;
	}

	unsigned long long getDaysSince1970() {
		// amount of DAYS since 1970
		return int(day.Val) - int(TDateTime(1970,1,1).Val);
	}
};

class PlayDatesInfo
{
	static std::vector<SingleDay> mData;

public:
	static void loadData(UnicodeString filePath);
	static void saveData(UnicodeString filePath);

	static void addPlayDate(const unsigned int &allSongId, const TDateTime &time);
	static void addSkipDate(const unsigned int &allSongId, const TDateTime &time);

	static SingleDay *findDayInData(const TDateTime &date);

	void getLastPlayDate();
	void getLastSkipDate();

	static uint64 totalPlayCount();  // play count in database
	static uint64 totalSkipCount();  // skip count in database
};


#endif
