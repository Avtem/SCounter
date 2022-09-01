#include "AvId3Reader.h"   // HEADER FILE
#include <vcl.h>           // for TFileStream and many other func.
#include <string>          // for std::string
#include <locale>          // for UTF-16



// CONSTANTS // ======= // =======
// all begin with "const"
const std::string constNOTAGS     = "=== NO TAGS";        		 // file doesn't have any tags
const std::string constNOTAGS1    = "=== NO TAGS V1";     		 // file doesn't have tags v.1
const std::string constNOTAGS2    = "=== NO TAGS V2";     		 // file doesn't have tags v.2
//const std::string constNOTAGS24   = "=== NO TAGS V2.4";    // file doesn't have tags v.2.4
const std::string constSPACEONBEG = "=== SPACE ON THE BEG.";	 // tag begins with space " Metallica"
const std::string constNOTFOUND   = "=== FILE NOT FOUND";  		 // file doesn't exist
const std::string constCONVERTTO24= "=== CONVERT TO TAG V2.4";   // conver to v2.4
const short int   constFRAMESIZE  = 10;                    		 // tag header size v.2





int Sync_safe_to_int( uint8_t* sync_safe )              // 4 bytes to 1 int
{
	uint32_t byte0 = sync_safe[0];
	uint32_t byte1 = sync_safe[1];
	uint32_t byte2 = sync_safe[2];
	uint32_t byte3 = sync_safe[3];

	return byte0 << 21 | byte1 << 14 | byte2 << 7 | byte3;
}

  // ============================

void ID3Tags::setAllTags(std::string constStr)                    // sets all object proporties
{
	artist = constStr.c_str();
	title  = constStr.c_str();
	album  = constStr.c_str();

	year = "----";
	track = '-';
    tagsSize = 0;
}

	 // ============================
                                                                // CONSTRUCTOR
ID3Tags::ID3Tags(UnicodeString path, bool checkFileExisting)
{
	if(checkFileExisting)  							   // checks for file existing
	{
		if(!FileExists(path))
		{ setAllTags(constNOTFOUND); return; }
	}
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                                         			  // checks ID3 v2.4 tag existing
	TFileStream *file = new TFileStream(path, fmOpenRead | fmShareDenyNone);
	ID3Header id3header;

	file->Read(&id3header, constFRAMESIZE);           // what version and existing
	if(id3header.id[0] != 'I' || id3header.id[1] != 'D' || id3header.id[2] != '3')
//	  || id3header.ver != 4)
	{
		if (id3header.id[0] != 'I')	  setAllTags(constNOTAGS2);       // if no v2 tags
//		else  		 setAllTags(constCONVERTTO24);  // doesn't support earliest version for now... =(

		delete file;
		return;
	}

	tagsSize = Sync_safe_to_int(id3header.tSize);
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
														// reads all existing tags
	uint8_t *allTags = new uint8_t [tagsSize];  		// read tags (byte array)
	file->Read(allTags, tagsSize);
	delete file;

	for(int i=0, tagsCount=0; i < tagsSize; i++)                     // search for TAG HEADERS
	{
		if(allTags[i] == 84)  // 'T'
		{
			if(allTags[i+1] == 73 && allTags[i+2] == 84 && allTags[i+3] == 50    // title   TIT2
			 ||allTags[i+1] == 80 && allTags[i+2] == 69 && allTags[i+3] == 49    // artist  TPE1
			 ||allTags[i+1] == 65 && allTags[i+2] == 76 && allTags[i+3] == 66    // album   TALB
			 ||allTags[i+1] == 82 && allTags[i+2] == 67 && allTags[i+3] == 75    // track   TRCK
//			 ||allTags[i+1] == 76 && allTags[i+2] == 69 && allTags[i+3] == 78    // durat   TLEN
			 ||allTags[i+1] == 68 && allTags[i+2] == 82 && allTags[i+3] == 67)   // year    TDRC
			{
                std::string frameName;              // gets name
				for(int j=0; j < 4; j++) frameName += allTags[i+j];

				uint8_t encoding = allTags[i+10];   // gets encoding

                uint8_t temp[4];                    // gets size
				for(int j=0; j < 4; j++) temp[j] = allTags[i+j+4];
				UINT frameSize = Sync_safe_to_int(temp);

				std::string tempString;             // gets string (title, album, etc.)
				for(int j=0; j < frameSize-1; j++) tempString += allTags[j+i+11];

				UnicodeString returningString;  // here tag "string"
				switch(encoding)
				{
					case 0:         // ISO-8859-1   for smaller than $80
					{
						returningString = tempString.c_str();
					}
					break;

					case 1:         // UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM
					case 2:         // UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM
					{
						setAllTags(constCONVERTTO24);  			 // doesn't support utf-16 for now... =(
						tagsCount ++;     // adds founded tag
						if(tagsCount == 5)   // BREAKS LOOP
                            break;
						continue;
					}

					case 3:         // UFT-8 unicode in v2.4
					{
						int wchars_num = MultiByteToWideChar(CP_UTF8, 0, tempString.c_str(), -1, NULL, 0);
						wchar_t *wstr = new wchar_t[wchars_num];
						MultiByteToWideChar(CP_UTF8, 0, tempString.c_str(), -1, wstr, wchars_num);

						returningString = wstr;

						delete[] wstr;
					}
				}
																// GUARD "SPACE"
				if(returningString.SubString(0, 1) == " ")
				{
					if(frameName == "TDRC")       returningString = "----"; // year
					else if (frameName == "TRCK") returningString = "-";    // track
					else 		returningString = constSPACEONBEG.c_str();
                }

															  // sets tags
				if(frameName == "TALB")      album  = returningString;
				else if(frameName == "TIT2") title  = returningString;
				else if(frameName == "TPE1") artist = returningString;
				else if(frameName == "TRCK") track  = returningString;
//                else if(frameName == "TLEN") ShowMessage("we have duration!");//duration  = returningString.ToInt();
				else if(frameName == "TDRC") year   = returningString.SubString(0,4);

				tagsCount ++;   // adds founded tag
				if(tagsCount == 5) break;             // BREAKS LOOP
			}
		}
	}
											  // GUARD "EMPTY TAG"
	for(int i=0; i < 5; i++)
	{
		if(!artist.Length()) artist = "=== NO ARTIST";
		if(!title.Length())   title = "=== NO TITLE";
		if(!album.Length())   album = "=== NO ALBUM";
		if(!year.Length())    year  = "----";
		if(!track.Length())   track = "-";
	}
}
