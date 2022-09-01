//---------------------------------------------------------------------------

#ifndef llisteningsH
#define llisteningsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>

#include <vector>
#include "AddingFiles.h"

class AddingFiles;
class ListeningItem;
extern AddingFiles af;
extern std::vector <ListeningItem> litems;         // for listview of artists
extern short currColumn;                       // active sorting column

//---------------------------------------------------------------------------
class TListenings : public TForm
{
__published:	// IDE-managed Components
	TListView *lvListenings;
	TPanel *firstSongEver;
	TPanel *totalSongsListenings;
	TPanel *Panel3;
	TPanel *Panel4;
	TPanel *Panel5;
	TPanel *numberOfArtists;
	TPanel *Panel7;
	TPanel *startListenings;
	TPanel *Panel6;
	TPanel *todayList;
	TPanel *Panel9;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall lvListeningsData(TObject *Sender, TListItem *Item);
	void __fastcall lvListeningsColumnClick(TObject *Sender, TListColumn *Column);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
private:	// User declarations
	UnicodeString countDays(UnicodeString d);                               // how many days ago was added?
    UnicodeString compare2dates(const UnicodeString &date1, const UnicodeString &date2);  // compares dates.
public:		// User declarations
	__fastcall TListenings(TComponent* Owner);
    void iterate(UnicodeString artist);             // when iteration happens
};
//---------------------------------------------------------------------------

class ListeningItem                  // MY CLASS
{
	UnicodeString artist;             // 30 Seconds to Mars
	int listenings;                   // 243
	UnicodeString date;               // 2y 4m 24d ago
	unsigned short days;              // 135

public:
	friend struct byArtists;
	friend struct byListenings;
	friend struct byDate;

	friend class TListenings;

	ListeningItem(UnicodeString ar, int lis, UnicodeString d, unsigned short day); // just constructor

	void sortArtists(short choice);
};



extern PACKAGE TListenings *Listenings;
//---------------------------------------------------------------------------
#endif
