//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "llistenings.h"
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "ssetings.h"   // for Shortcut struct
#include "AllSong.h"


std::vector <ListeningItem> litems;         // for listview of artists
short currColumn = 0;                       // active sorting column

TListenings *Listenings;
UINT startList;                               // count of listenings on the start program
UINT todayListenings;                         // listened songs for beginning of day
extern std::vector <Shortcut> shortcutsData;  // here stored data with shorcuts
extern bool lWinPressing;                     // is win key pressing?
extern std::vector <AllSong> allSongs;
extern std::vector <UnicodeString> indArtists; 		   // INDEXATION AS
extern std::vector <std::vector <int> > indArtIndexes; // INDEXATION AS

//---------------------------------------------------------------------------
__fastcall TListenings::TListenings(TComponent* Owner)
	: TForm(Owner)
{
}

//---------------------------------------------------------------------------
																			// just constructor
ListeningItem::ListeningItem(UnicodeString ar, int lis, UnicodeString d, unsigned short day)
{
	artist = ar;
	listenings = lis;
	date = d;
	days = day;
}

//---------------------------------------------------------------------------

void __fastcall TListenings::FormShow(TObject *Sender)                                     // onShow
{
//	{                                               // indexation of AllSongs.txt data
//	  std::vector <UnicodeString> tempVect;
//		for(int i=0; i < af.dataAllSongs->Count; i+=4)
//			tempVect.push_back(af.dataAllSongs->Strings[i]);     // Metallica
//	  af.Indexation(tempVect, af.artAS, af.artIndexesAS);
//	}
	// NEW CODE
    AllSong::makeIndexation(LetterCase::Sentence);  // lowercased = false!!!!!

	litems.clear();
	numberOfArtists->Caption = indArtists.size();     // caption
							   // indArtists.size()
	UnicodeString firstDate, firstDateEver=L"99.99.9999";

	for(int i=0; i < indArtists.size(); i++)          // building the vector
	{               // NEW CODE indArtists.size()

		UINT listenings = 0;
		for(int j=0; j < indArtIndexes[i].size(); j ++) // count listenings of this artist + find first date
		{               // indArtIndexes[i].size()

		   listenings += allSongs[indArtIndexes[i][j]].playCount();
						 // allSongs[indArtIndexes[i][j]].playCount

		   if(j)
			   firstDate = compare2dates(firstDate,
					allSongs[indArtIndexes[i][j-1]].birthDateShort());
													   // allSongs[indArtIndexes[i][j-1].birthDateStr()
		   else firstDate = allSongs[indArtIndexes[i][0]].birthDateShort();   // only one song of this artist
							// allSongs[indArtIndexes[i][0]].birthDateStr()
		}
		firstDateEver = compare2dates(firstDateEver, firstDate);

		ListeningItem litem(indArtists[i], listenings, countDays(firstDate)
						   // indArtists[i]
		, difftime(Date().Val, StrToDate(firstDate).Val));   // add to the vector
		litems.push_back(litem);
    }
	totalSongsListenings->Caption = AllSong::getAllListenings();  // caption totalSongsList
				   /// NEW CODE // AllSong::getAlllist()
    firstSongEver->Caption = firstDateEver;           // caption firstDateEver


   lvListenings->Column[0]->Tag = 0;
   ListeningItem it(0,0,0,0);
   it.sortArtists(0);

   lvListenings->Items->Count = litems.size();                   // show changes

   if(totalSongsListenings->Caption.ToInt() <= startList )   // current session listenings
		startListenings->Caption = 0;
   else
   		startListenings->Caption = totalSongsListenings->Caption.ToInt() - startList;

																 // today listenings
   if(totalSongsListenings->Caption.ToInt() <= todayListenings)
		todayList->Caption = 0;
   else
   	    todayList->Caption = totalSongsListenings->Caption.ToInt() - todayListenings;
}

//---------------------------------------------------------------------------

void TListenings::iterate(UnicodeString artist)       						// when iteration happens
{
	startListenings->Caption = startListenings->Caption.ToInt() +1;
	todayList->Caption = todayList->Caption.ToInt() +1;
	totalSongsListenings->Caption = totalSongsListenings->Caption.ToInt() +1;

	 // find in listview and iterate;
	for(int i=0; i < litems.size(); i++)
		if(litems[i].artist == artist)
		{ litems[i].listenings ++; lvListenings->UpdateItems(i,i); break;  }
}

//---------------------------------------------------------------------------

void __fastcall TListenings::lvListeningsData(TObject *Sender, TListItem *Item)             // on Data
{
	int index = Item->Index;
	Item->Caption = litems[index].artist;
	Item->SubItems->Add(litems[index].listenings);
	Item->SubItems->Add(litems[index].date);
}

//---------------------------------------------------------------------------

void __fastcall TListenings::lvListeningsColumnClick(TObject *Sender, TListColumn *Column)  // on columnClick
{
   if(currColumn != Column->Index)
   {                                                           // reset all captions
	   lvListenings->Column[0]->Caption = "Artist";      lvListenings->Column[0]->Tag = 0;
	   lvListenings->Column[1]->Caption = "Listenings";  lvListenings->Column[1]->Tag = 0;
	   lvListenings->Column[2]->Caption = "Added";       lvListenings->Column[2]->Tag = 0;
   }
   else Column->Tag = !Column->Tag;
   currColumn = Column->Index;

   switch(Column->Index)
   {
	   case 0:
		  if(!Column->Tag)
		  lvListenings->Column[0]->Caption = "Artist                                                  [A-Z]";
		  else
		  lvListenings->Column[0]->Caption = "Artist                                                  [Z-A]";
	   break;

	   case 1:
		  if(!Column->Tag)
		  lvListenings->Column[1]->Caption = "Listenings [0-9]";
		  else
		  lvListenings->Column[1]->Caption = "Listenings [9-0]";
	   break;

	   case 2:
		  if(!Column->Tag)
		  lvListenings->Column[2]->Caption = "Added [newest on top]";
		  else
		  lvListenings->Column[2]->Caption = "Added [oldest on top]";
	   break;

   }

   ListeningItem it(0,0,0,0);
   it.sortArtists(Column->Index);
}
//---------------------------------------------------------------------------

void __fastcall TListenings::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	for(UINT i = 0; i < shortcutsData.size(); i++)
	{
		TShiftState shortcutShift;                          // in settings (Alt + Ctrl + C)
		if(shortcutsData[i].alt)   shortcutShift << ssAlt;
		if(shortcutsData[i].ctrl)  shortcutShift << ssCtrl;
		if(shortcutsData[i].shift) shortcutShift << ssShift;


		if(shortcutsData[i].functionName == "Listenings window"         // user pressed "closing" shortcut
           && Key == shortcutsData[i].vkCode
		   && (Shift == shortcutShift && lWinPressing == shortcutsData[i].win) )
            this->Close();
	}

	if (Key == vkEscape) Listenings->Close();   // [ESC]
}

void __fastcall TListenings::FormKeyPress(TObject *Sender, System::WideChar &Key)
{     Key = 0;   }// do not beep



void __fastcall TListenings::FormCreate(TObject *Sender)
{                                                                                     // on Create
   startList = AllSong::getAllListenings();  //NEW CODEaf.getAllList();
}

//---------------------------------------------------------------------------

UnicodeString TListenings::compare2dates(const UnicodeString &date1, const UnicodeString &date2)
// compares dates. Returns earliest
{
	if(date1.SubString(7,4).ToInt() == date2.SubString(7,4).ToInt())   // years
	{
		if(date1.SubString(4,2).ToInt() == date2.SubString(4,2).ToInt())  // months
		{
			if(date1.SubString(1,2).ToInt() < date2.SubString(1,2).ToInt())
			     return date1;
			else return date2;
		}
		else if(date1.SubString(4,2).ToInt() < date2.SubString(4,2).ToInt())
			 return date1;
		else return date2;

	}
	else if (date1.SubString(7,4).ToInt() < date2.SubString(7,4).ToInt())    // years
		 return date1;
	else return date2;
}

//---------------------------------------------------------------------------

UnicodeString TListenings::countDays(UnicodeString d)                         // how many days ago was added?
{
	TDateTime date = StrToDate(d);
	TDateTime today = Date();      // today, 1-st parameter

	unsigned short days = difftime(today.Val, date.Val);
	UnicodeString daysString;

	if(days == 0)
	   return L"Today";
	else if(days == 1)
	   return L"Yesterday";
	else if(days < 31)
	{
		daysString = days;
		daysString += " days";
	}
	else if(days < 365)
	{
		if((daysString = days/30) == 1) daysString += " month ";
		else daysString += " months ";

		if(days%30 == 1) daysString += " 1 day ";
		else if(days%30 > 0)
		{ daysString += days%30;  daysString += " days ";  }
	}
	else
	{
		if((daysString = days/365) == 1) daysString += " year ";
		else daysString += " years ";

		short months = (days%365)/30;
		if(months) daysString += months;

		if(months == 1)
		  daysString += " month ";
		else if(months)
		  daysString += " months ";

		if(days%30 == 1) daysString += " 1 day";
		else if(days%30)
		{
		  daysString += days%30;
		  daysString += " days";
        }
	}

	return daysString + L" ago";
}


///########################## sorting of columns ########################################

struct byArtists
{
	bool operator()( const ListeningItem& lx, const ListeningItem& rx ) const {
		if(!Listenings->lvListenings->Column[currColumn]->Tag)
			 return lx.artist.LowerCase() < rx.artist.LowerCase();
		else return lx.artist.LowerCase() > rx.artist.LowerCase();
	}
};
struct byListenings
{
	bool operator()( const ListeningItem& lx, const ListeningItem& rx ) const {
		if(!Listenings->lvListenings->Column[currColumn]->Tag) return lx.listenings < rx.listenings;
		else return lx.listenings > rx.listenings;
	}
};
struct byDate
{
	bool operator()( const ListeningItem& lx, const ListeningItem& rx ) const {
		if(!Listenings->lvListenings->Column[currColumn]->Tag) return lx.days < rx.days;
		else return lx.days > rx.days;
	}
};
///########################## sorting of columns ########################################


///////////////////////////////////////////////////////////////////////////////////////////

void ListeningItem::sortArtists(short choice)
{
	switch(choice)
	{
	   case 0:   std::sort(litems.begin(), litems.end(), byArtists());
	   break;

	   case 1:   std::sort(litems.begin(), litems.end(), byListenings());
	   break;

	   case 2:   std::sort(litems.begin(), litems.end(), byDate());
	   break;
	}
											  // and display the changes in listenings
	Listenings->lvListenings->UpdateItems(0, Listenings->lvListenings->Items->Count -1);
}






//---------------------------------------------------------------------------

