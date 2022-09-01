//---------------------------------------------------------------------------

#pragma hdrstop

#include "PlayDatesInfo.h"
#include <fstream> // file io
#include <string> // strings
#include "AllSong.h"

//---------------------------------------------------------------------------
std::vector<SingleDay> PlayDatesInfo::mData;


struct KeyValue {
	std::string key;
	std::string value;

	KeyValue(const char *key, const char *value)
		:key(key), value(value)
	{}

	// retreive string as <Key>value
	std::string asString() {
		return "<" + key + ">" + value;
	}

	static std::string valueOf(const std::vector<KeyValue> &vec, std::string key) {
		for(int i=0; i < vec.size(); i++)
			if(key == vec[i].key)
				return vec[i].value;

		return "";
    }
};

	// All uints are Big endian (MSB)
	void uintToCharArray(const unsigned int &number, const short byteCount, unsigned char *array) {
		for(int i=0; i < byteCount; i++)
			array[i] = (0xFF & (number >> 8 *(byteCount-1 -i)));
	}
	unsigned int uintFromCharArray(const unsigned char * const byteArray, short byteCount) {
		unsigned int number = 0;
		for(int i=0; i < byteCount; i++)
			number += byteArray[i] << 8 *(byteCount -1 -i);

        return number;
	}

std::string makeInfoString() {
	std::string str("");

	std::vector<KeyValue> keyAndValues;
	keyAndValues.push_back(KeyValue("File name", 				"PlayDatesInfo"));
	keyAndValues.push_back(KeyValue("API version of this file", "1.0.0"));
	keyAndValues.push_back(KeyValue("Play entries", AnsiString(PlayDatesInfo::totalPlayCount()).c_str()));
	keyAndValues.push_back(KeyValue("Skip entries",	AnsiString(PlayDatesInfo::totalSkipCount()).c_str()));
	keyAndValues.push_back(KeyValue("AllSongId ByteCount",	AnsiString(AllSong::bytesRequiredForIds()).c_str()));
	// end of all tags
	keyAndValues.push_back(KeyValue("INFO END", "\n"));

	for(int i=0; i < keyAndValues.size(); i++)
		str += keyAndValues[i].asString();

	return str;
}

// this function just tries to read all infotags from an AvtemFile. Don't try to understand this func.
void readInfoTags(std::vector<unsigned char> &fileInVector, std::vector<KeyValue> &keyValues) {

	std::string infoLine = "";
	for(unsigned int i=0; i < fileInVector.size(); i++) {
		// read whole infoline
		if(fileInVector.at(i) == '\n') {
        	// delete infoTags from the buffer!
			fileInVector.erase(fileInVector.begin(), fileInVector.begin() +i +1);
			break;
		}
		infoLine += fileInVector.at(i);
	}

	std::string key = "";
    std::string value = "";
	for(int pos=0; pos < infoLine.length(); pos++) {
		// okay, we found beginning of a key!
		if(infoLine[pos] == '<') {
			key = "";
			while(infoLine[++pos] != '>') {
				key += infoLine[pos];
			}

			value = "";
			while(infoLine[++pos] != '<')
				value += infoLine[pos];

			pos --;
			keyValues.push_back(KeyValue(key.c_str(), value.c_str()));
        }
	}
}

void PlayDatesInfo::loadData(UnicodeString filePath)
{
    // create file if doesn't exist
	if(!FileExists(filePath)) {
		ShowMessage("File " + filePath + " doesn't exist!");
        return;
    }

	//open file
	std::ifstream stream;
	stream.open(filePath.c_str(), std::ios::binary);
	if(stream == NULL) {
		ShowMessage("Couldn't open file for reading! \n" + filePath);
        return;
	}

	int c;
	std::vector<unsigned char> buffer;
	while ((c = stream.get()) != EOF)
		buffer.push_back(c);

	std::vector<KeyValue> infoTags;
	readInfoTags(buffer, infoTags); // DELETES TAGS FROM DATA!!!

    // reading all info tags...
	UnicodeString temp(KeyValue::valueOf(infoTags, "AllSongId ByteCount").c_str());
	int AllSongIdSize = temp.ToInt();
	temp = (KeyValue::valueOf(infoTags, "Play entries").c_str());
	int playCount = temp.ToInt();
	temp = (KeyValue::valueOf(infoTags, "Skip entries").c_str());
	int skipCount = temp.ToInt();
	if(KeyValue::valueOf(infoTags, "API version of this file") != "1.0.0") {
		ShowMessage("Nooo, file version is not 1.0.0!\n" + filePath);
		return;
    }

	int DAY;
	int ENTRY_COUNT;
	int ALLSONG_ID;
	int TIME_SINCE;
	int pos = 0; // for moving in buffer
	TDateTime day1970(1970, 1, 1);
	TDateTime tDay;
    unsigned char *dynArray = new unsigned char[AllSongIdSize];

	PlayDatesInfo::mData.clear();

	for(int i=0; i < playCount + skipCount; 0)
	{
		void (*currentFunc)(const unsigned int &, const TDateTime &)
			= i < playCount ? &PlayDatesInfo::addPlayDate : &PlayDatesInfo::addSkipDate;

		unsigned char arr2bytes[2] = {buffer.at(pos), buffer.at(pos+1)};
		pos += 2;
		DAY = uintFromCharArray(arr2bytes, 2);
		tDay = TDateTime(day1970.Val + DAY);

		arr2bytes[0] = buffer.at(pos);
		arr2bytes[1] = buffer.at(pos+1);
		ENTRY_COUNT = uintFromCharArray(arr2bytes, 2);
		pos += 2;

		// add all entries for the day
		for(int j=0; j < ENTRY_COUNT; j++) {
			for(int m=0; m < AllSongIdSize; m++)
				dynArray[m] = buffer.at(pos +m);
			pos += AllSongIdSize;
			ALLSONG_ID = uintFromCharArray(dynArray, 2);

			arr2bytes[0] = buffer.at(pos);
			arr2bytes[1] = buffer.at(pos+1);
			TIME_SINCE = uintFromCharArray(arr2bytes, 2);
			pos += 2;

			currentFunc(ALLSONG_ID, TDateTime(tDay.Val +TDateTime(TIME_SINCE /60, TIME_SINCE%60, 0, 0).Val));
            i++;
		}
	}
    delete [] dynArray;

	stream.close();

	return;
    // just debug stuff for displaying when i listened and skipped songs
	for(int i=0; i < mData.size(); i++) {
		AllSong *as = NULL;
		UnicodeString str = "";

		str = "PLAYED: \n";
		for(int j=0; j < mData.at(i).playDates.size(); j++) {
			as = AllSong::find(mData.at(i).playDates.at(j).allSongId);

			UnicodeString asName = mData.at(i).playDates.at(j).allSongId;
			if(as)
				asName = as->name();
			str += asName +" = " + mData.at(i).playDates.at(j).date.DateTimeString() + '\n';
		}
		ShowMessage(str);

		str = "SKIPPED: \n";
		for(int j=0; j < mData.at(i).skipDates.size(); j++) {
			as = AllSong::find(mData.at(i).skipDates.at(j).allSongId);

			UnicodeString asName = mData.at(i).skipDates.at(j).allSongId;
			if(as)
				asName = as->name();
			str += asName +" = " + mData.at(i).skipDates.at(j).date.DateTimeString() + '\n';
		}
		ShowMessage(str);
	}
}

void PlayDatesInfo::saveData(UnicodeString filePath)
{
	// create file if doesn't exist

	std::fstream stream;
	stream.open(filePath.c_str(), std::ios::out | std::ios::binary);

	if(!stream.good()) {
		ShowMessage("Couldn't open file: " + filePath);
        return;
	}

	// write info tags
	stream << makeInfoString();

	for(int v=0; v < 2; v++)  // write play, then skip entries
	{
		std::vector <SingleEntry> vec;

		for(int i=0; i < mData.size(); i++)
		{
			vec = (v == 0) ? mData[i].playDates : mData[i].skipDates;

            // this day doesn't have play dates OR skip dates
			if(vec.size() == 0)
				continue;

			const int idByteCount = AllSong::bytesRequiredForIds();
			unsigned char arr2bytes[2];
			unsigned char *arr4bytes = new unsigned char[idByteCount];  // for ALLSONG_ID only

            // #1 DAYS since 1970
			uintToCharArray(mData[i].getDaysSince1970(), 2, arr2bytes);
			stream.write(arr2bytes, 2);

			// #2 ENTRY_COUNT for the day
			uintToCharArray(vec.size(), 2, arr2bytes);
			stream.write(arr2bytes, 2);

			for(int j=0; j < vec.size(); j++) {
				// #A.1 ALLSONG_ID
				uintToCharArray(vec[j].allSongId, idByteCount, arr4bytes);
				stream.write(arr4bytes, idByteCount);

                // #A.2 MINUTE_COUNT since 0:00 of the day
				uintToCharArray(vec[j].minCountSincMidnight(), 2, arr2bytes);
				stream.write(arr2bytes, 2);
			}
            delete [] arr4bytes;
		}
	}

	stream.close();
}

SingleDay *PlayDatesInfo::findDayInData(const TDateTime &date)
{
	for(int i=0; i < mData.size(); i ++) {
		if(int(mData[i].day) == int(date))
			return &mData[i];
	}

	return NULL;
}

// static
void PlayDatesInfo::addPlayDate(const unsigned int &allSongId, const TDateTime &time)
{
	SingleDay *singleDay = findDayInData(time);
	if(singleDay == NULL) {
		mData.push_back(SingleDay(time));
		singleDay = &mData.back();
	}

	singleDay->addPlayTime(allSongId, time);
}

// static
void PlayDatesInfo::addSkipDate(const unsigned int &allSongId, const TDateTime &time)
{
	SingleDay *singleDay = findDayInData(time);
	if(singleDay == NULL) {
		mData.push_back(SingleDay(time));
		singleDay = &mData.back();
	}

	singleDay->addSkipTime(allSongId, time);
}

// static
uint64 PlayDatesInfo::totalPlayCount()  // play count in database
{
	uint64 count = 0;
	for(int i=0; i < mData.size(); i++)
		count += mData[i].playDates.size();

	return count;
}

// static
uint64 PlayDatesInfo::totalSkipCount()  // play count in database
{
	uint64 count = 0;
	for(int i=0; i < mData.size(); i++)
		count += mData[i].skipDates.size();

	return count;
}
#pragma package(smart_init)
