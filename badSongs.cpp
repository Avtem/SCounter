//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "badSongs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TformBadSongs *formBadSongs;



	  // ============ MY INCLUDES ============
#include "Unit1.h"  		// mform
#include "Playlists.h"   // playlists class
#include "AddingFiles.h" // adding files class
#include "Songs.h"       // songs class
#include <vector>        // vectors
#include "enums.h"       // enums

      // ============ MY FUNCTIONS ============
extern void openFolderSelect(UnicodeString path); // opens and selects given file
extern UnicodeString intToUS(const int &integer);  	// conver int to US string

	  // ============ MY VARIABLES ============
std::vector <Songs*> songsCopy;
ReasonToDelete currReason;
bool userWantsAbort = false;		// if user pressed "Cancel" while songs are checking
UINT step1ItemsCount =0;            // how many songs we have in 1st step?
bool badSongsCalling = false;       // using for "findDoubles"

extern std::vector <Songs> songs;                 // main info about songs in playlist
extern Playlists pl;
extern AddingFiles af;



//---------------------------------------------------------------------------
__fastcall TformBadSongs::TformBadSongs(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------


void __fastcall TformBadSongs::btnCancelClick(TObject *Sender)
{
	userWantsAbort = true;
	pl.restoreSelection();    // user didn't delete anything, thus restore their selection
	this->Close();
}
//---------------------------------------------------------------------------



void __fastcall TformBadSongs::FormShow(TObject *Sender)
{
   timerShowDelay->Enabled = true;   // all things happen in timer()
}
//---------------------------------------------------------------------------

void TformBadSongs::receiveFiles(std::vector <Songs*> vec)     // get that files and show in LV
{
    lv->Clear();
	songsCopy = vec;
	lv->Items->Count = vec.size();
}

void __fastcall TformBadSongs::lvData(TObject *Sender, TListItem *Item)
{                                                           // here are only columns. Item->Caption is in Draw
   Item->SubItems->Add((*songsCopy[Item->Index]).name);
   Item->SubItems->Add((*songsCopy[Item->Index]).path);
}
//---------------------------------------------------------------------------
void __fastcall TformBadSongs::btnDeleteAllClick(TObject *Sender)
{
	if (btnDeleteAll->Caption != "Close")
	{
		af.deleteFiles();               // delete songs from mfrom playlist

		mform->playlist->Selected = false;          // some songs were deleted. Select first song.
		mform->playlist->ItemIndex = 0;
		mform->playlist->Items->Item[0]->MakeVisible(true);
	}
	else
        pl.restoreSelection();		// there were no bad songs. thus - restore user selection

    this->Close();                  // close this form
}
//---------------------------------------------------------------------------

void __fastcall TformBadSongs::FormClose(TObject *Sender, TCloseAction &Action)
{
	userWantsAbort = false; // reset this value
}
//---------------------------------------------------------------------------

void __fastcall TformBadSongs::btnShowInExplorerClick(TObject *Sender)
{
	UnicodeString path = lv->Selected->SubItems->Strings[1];
	if(FileExists(path))
		openFolderSelect(path);             // try to open folder. if failst, show message
	else if((UINT)ShellExecute(NULL, NULL, ExtractFileDir(path).c_str(), NULL, NULL, SW_SHOWNORMAL) <= 32 )
		messageForm->showMessage("Can not find folder too.", 'e', 2);
}
//---------------------------------------------------------------------------

void __fastcall TformBadSongs::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
   if(Key == vkF4) Key = 0;         // prevent Alt+f4
}
//---------------------------------------------------------------------------

void __fastcall TformBadSongs::timerShowDelayTimer(TObject *Sender)
{
	timerShowDelay->Enabled = false;         // disable this timer.
	TListView *lvMform = mform->playlist;	 // MAKING STUFF IN MFORM
	prepareForm(currReason);    			 // prepare the form.
	UINT badSongsCount = 0;


 lv->Items->BeginUpdate();

	switch (currReason)
	{
		case Twin:
		{
            lv->Clear();      		    // clear lv
            pl.saveSelection();         // save user's selection
			lvMform->Selected = false;
              af.sameAdded.clear();           // cccclear!

			for(UINT i=0; i < songs.size(); i++)        // (STEP 1/2) select unexisting songs
			{
				  Application->ProcessMessages();
				  if(userWantsAbort)       // allow user to escape from this window
					  break;
                                                            // percent stuff
				  labelProgress->Caption = "Progress " + UnicodeString((i*100) / songs.size()) + '%';
				  prbar->Position = (i*1000) / songs.size();

				  if(!FileExists(songs[i].path))
				  {
					 lvMform->Items->Item[i]->Selected = true;
																	// count selected songs
					 badSongsCount ++;
                     step1ItemsCount ++;
					 labelFound->Caption = "Found bad songs: " + intToUS(badSongsCount);

					 songsCopy.push_back(&songs[i]);      // show that song in lv
					 lv->Items->Count ++;

					 af.sameAdded.push_back(songs[i].path);  // add unexisting song to the vector,
					 										 // so findDoubles won't check it.
				  }
			}

															  // (STEP 2/2) select twins
			label2->Caption = "Step 2/2:      Checking for 'Twins'";

			badSongsCalling = true;
			pl.findDoubles(false);
			badSongsCalling = false;

			for(UINT i=0; i < af.samePlaylist2.size(); i++)     // select twins
			{
				Application->ProcessMessages();
                if(userWantsAbort)       // allow user to escape from this window
					  break;
                                                                         // percent stuff
				labelProgress->Caption = "Progress " + intToUS((i *100) / af.samePlaylist2.size()) + '%';
				prbar->Position = (i *1000) / af.samePlaylist2.size();

				lvMform->Items->Item[af.samePlaylist2[i]]->Selected = true;

				badSongsCount ++;
				labelFound->Caption = "Found bad songs: " + intToUS(badSongsCount);

				songsCopy.push_back(&songs[af.samePlaylist2[i]]);      // show that song in lv
				lv->Items->Count ++;
			}
		}
		break;

       	  // ======================

		case NotFoundInAllSongs:             // ALL SONGS CASE
		{
			badSongsCount = songsCopy.size();
            labelFound->Caption = "Found bad songs: " + intToUS(badSongsCount);
		}
		break;
	}

 lv->Items->EndUpdate();

	if (badSongsCount == 0)                       // no bad songs found. restore selection
		btnDeleteAll->Caption = "Close";
	else
		btnDeleteAll->Caption = "Delete all";

	label2->Caption = "Checking done.";      // when checking done
	btnDeleteAll->Enabled = true;
	labelProgress->Visible = false;
    prbar->Position = 1000;
}
//---------------------------------------------------------------------------

void __fastcall TformBadSongs::lvDrawItem(TCustomListView *Sender, TListItem *Item, TRect &Rect,
          TOwnerDrawState State)
{
    TListView *lv = dynamic_cast<TListView*>(Sender);
	TCanvas *canvas = Sender->Canvas;
	int index = Item->Index;

	canvas->Brush->Color = RGB(25,29,34);               // LV usual color
	canvas->Font->Color = RGB(210,210,210);             // font (WHITE) for usual songs

	if(lv->SelCount && lv->Items->Item[index]->Selected)
	   canvas->Brush->Color = RGB(75,75,75);    // selectedSong (GRAY)

	canvas->FillRect(Rect);
	canvas->Brush->Style = bsClear;
	Rect.Right = lv->Columns->Items[0]->Width;


	UnicodeString captionText;              // set text of caption
	if(currReason == NotFoundInAllSongs)
        captionText = "Not found in AllSongs data";
	else if(index < step1ItemsCount)            // how many songs we have in 1st step?
		captionText = "Not found";
	else
        captionText = "Twin";

	canvas->TextRect(Rect, Rect.Left+7, Rect.Top+1, captionText);

	for(int col = 0;  col < 2; col++)
	{
						  // draw a column
		Rect.Left = Rect.Left + lv->Columns->Items[col]->Width + 1;
		Rect.Right = Rect.Left + lv->Columns->Items[col +1]->Width;

		UnicodeString text;									   // set text of 2,3 column (name)
		if(!col) text = (*songsCopy[index]).name;
		else     text = (*songsCopy[index]).path;

//		short center = Rect.Left + (Rect.Right - Rect.Left - canvas->TextWidth(text))/2;
		canvas->TextRect(Rect, Rect.Left+7, Rect.Top+1, text);
	}
}
//---------------------------------------------------------------------------

void TformBadSongs::prepareForm(ReasonToDelete reason)    // sets proper btn, labels accordingly to reason
{
	btnShowInExplorer->Enabled = false;      // applies to both cases
	labelFound->Caption = "Found bad songs: 0";
    labelProgress->Visible = true;
   	btnDeleteAll->Enabled = false;         // clear everything to start searching with 0 progress
	btnShowInExplorer->Enabled = false;

    af.sameAdded.clear();           // cccclear!
	af.samePlaylist.clear();
	af.samePlaylist2.clear();


	switch (reason)
	{
		case Twin:
		{
			 if(lv->Column[0]->Width > 60) {
				 lv->Column[0]->Width = 60;          // column width
				 lv->Column[1]->Width += 80;
			 }

			 label2->Caption = "Step 1/2:      Checking for 'removed/deleted/renamed' songs";

             btnCancel->Enabled = true;
			 songsCopy.clear();     // clear this lv
			 lv->Clear();
             step1ItemsCount = 0;
		}
		break;

		case NotFoundInAllSongs:
		{
			 if(lv->Column[0]->Width < 140) {
				 lv->Column[0]->Width = 140;          // column width
				 lv->Column[1]->Width -= 80;
			 }

//             btnCancel->Enabled = false;
        }
        break;
    }
}
//---------------------------------------------------------------------------


void __fastcall TformBadSongs::FormResize(TObject *Sender)
{
   if(this->Height < 200)           // don't resize to very small size
		this->Height = 200;

   if(this->Width < 600)
	   this->Width = 600;
}
//---------------------------------------------------------------------------

void __fastcall TformBadSongs::lvChange(TObject *Sender, TListItem *Item, TItemChange Change)

{
	if(lv->SelCount == 1)
		btnShowInExplorer->Enabled = true;
	else
        btnShowInExplorer->Enabled = false;
}
//---------------------------------------------------------------------------

