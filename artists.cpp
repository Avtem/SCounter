//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "artists.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TformArtists *formArtists;

struct LvItem
{
	UnicodeString name;
	bool checked;

	LvItem (const UnicodeString &Name)   // constructor
    {  name = Name; checked = false; }
};


      // ============ MY INCLUDES ============
#include "Unit1.h"		// mform
#include "enums.h"      // enums
#include <vector>       // vectors
#include "Playlists.h"  // loadplaylists and etc.
#include "AddingFiles.h"// adding files
#include "Songs.h"      // songs class

	  // ============ MY FUNCTIONS ============
extern UnicodeString intToUS(const int &integer);  	// conver int to US string
extern bool containsOnly (const TShiftState &state, ModifierKey key, ModifierKey key2 = ModifierKey::None);
													 // returns true if only Ctrl was pressed e.g.
std::vector <UINT> getSelectedSongs(); 		// gets indexes of all selected songs in pl by user

	  // ============ MY VARIABLES ============
UnicodeString prevQSearchText = "";
bool dontEnterOnChange = false;                         // should function trigger F3Search onChange function?
extern std::vector <TButton*> plButtons;                // SCouner playlists
extern Playlists pl;
extern AddingFiles af;

std::vector <LvItem> lvItems;      // ALL items for lv
std::vector <LvItem> foundMatches; // FOUND items for lv


//---------------------------------------------------------------------------
__fastcall TformArtists::TformArtists(TComponent* Owner)
	: TForm(Owner)
{
   	SHAutoComplete(editQSearch->Handle, SHACF_AUTOAPPEND_FORCE_OFF); // allow ctrl+backspace  F3 Search
}
//---------------------------------------------------------------------------
void __fastcall TformArtists::editQSearchEnter(TObject *Sender)
{
   if(editQSearch->Text == "Quick search [F3]")         // clear edit
		editQSearch->Text = "";

   editQSearch->Font->Color = clWhite;                  // white "active" font
}
//---------------------------------------------------------------------------
void __fastcall TformArtists::editQSearchExit(TObject *Sender)
{
	 dontEnterOnChange = true;

	 if(editQSearch->Text == "")
		editQSearch->Text = "Quick search [F3]";
	 editQSearch->Font->Color = clInactiveCaption; // gray font

     dontEnterOnChange = false;	// reset this bool
}
//---------------------------------------------------------------------------
void __fastcall TformArtists::editQSearchKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(Key == vkEscape || Key == vkReturn)                      // exit QSearch
	   lv->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TformArtists::editQSearchChange(TObject *Sender)
{
	editQSearch->ReadOnly = true;               // don't allow user to seek too fast

	if(editQSearch->Text.Length() < 2 && editQSearch->Text != "" || dontEnterOnChange) {
		editQSearch->ReadOnly = false;
		return;
	}

	if(editQSearch->Text == "" || editQSearch->Text == "Quick search [F3]")
	{                                                                     		//GUARD
		if(!lv->SelCount && prevQSearchText != "" && prevQSearchText != "Quick search [F3]")
			showItems(lvItems, true);        // refresh lv

		prevQSearchText = editQSearch->Text;
		editQSearch->ReadOnly = false;
		return;
	}


	foundMatches.clear();
	for(UINT i=0; i < lvItems.size(); i++)
	{
	   if(lvItems[i].name.LowerCase().Pos1(editQSearch->Text.LowerCase()))    // there is such song!
		  foundMatches.push_back(lvItems[i]);    // add to vector
	}

	showItems(foundMatches, true);
	prevQSearchText = editQSearch->Text;
	editQSearch->ReadOnly = false;
}
//---------------------------------------------------------------------------
void __fastcall TformArtists::btnAddClick(TObject *Sender)
{
	pl.savePlaylist(pl.activePlaylist);         // is this important??S

	TStringList *list = new TStringList;
	list->LoadFromFile(pl.getSettingsPath() + "playlists\\" + pl.delNonPathChars(comboFrom->Text)
		 + "\\playlistData");

	std::vector <UnicodeString> artists;         // indexation of artists in the file
	for(UINT i=0; i < list->Count; i+= 7)
		artists.push_back(list->Strings[i +1]);
	af.Indexation(artists, af.artBef, af.artIndexesBef);

	std::vector <UINT> checkedArtists;
	std::vector <Songs> songsToCopy;
	for(UINT i=0; i < lvItems.size(); i++)               // find all checked artists
	{
		if(lvItems[i].checked) {                         // this lvItem is checked, find it in index. vector
			for(UINT j=0; j < af.artBef.size(); j++) {
				if (af.artBef[j].LowerCase() == lvItems[i].name.LowerCase()) {   // we found this art in indexation
					for(UINT k=0; k < af.artIndexesBef[j].size(); k++) {

						UINT index = af.artIndexesBef[j][k] *7;  // index in 'playlistData' file
//                        ShowMessage(index);

						Songs newSong(list->Strings[index]         	       // title
										,list->Strings[index+1]            // artist
										,list->Strings[index+2]            // path
										,list->Strings[index+3]            // duration
										,list->Strings[index+4]            // date
										,list->Strings[index+5].ToInt()    // listenings
										,list->Strings[index+6].ToInt() ); // indexNorm

						songsToCopy.push_back(newSong); // push song
					}

					break;
				}
			}

		}
	}
	delete list;

	af.copyToPlaylist(pl.delNonPathChars(comboTo->Text), songsToCopy);		// copy to playlist


	this->Close();
}
//---------------------------------------------------------------------------
void __fastcall TformArtists::FormShow(TObject *Sender)
{
   	UINT space = 75;                             // position
	Left = mform->Left +space;
	Top  = mform->Top +space;

	comboFrom->Clear();
	comboTo->Clear();
    lv->SetFocus();

	for(UINT i=0; i < plButtons.size(); i++)            // set comboTo   items
	{
		comboTo->Items->Add(plButtons[i]->Caption);
		if(plButtons[i]->Font->Color == clLime)
            comboTo->ItemIndex = i;
	}

	for(UINT i=0; i < plButtons.size(); i++)            // set comboFrom   items
	{
		if(plButtons[i]->Font->Color != clLime)
			comboFrom->Items->Add(plButtons[i]->Caption);
	}
	comboFrom->ItemIndex = 0;  // this is All songs playlist usually

    timerShowDelay->Enabled = true;     // load all indexed artists
}
//---------------------------------------------------------------------------
void __fastcall TformArtists::comboFromEnter(TObject *Sender)
{
	UnicodeString currText = comboFrom->Text;
	comboFrom->Clear();                             // comboFrom (has all W\OUT COMBOTO-item playlists)

	for(UINT i=0; i < plButtons.size(); i++)
	{
		if(plButtons[i]->Caption != comboTo->Text)
			comboFrom->Items->Add(plButtons[i]->Caption);
		if(currText == plButtons[i]->Caption)
			comboFrom->ItemIndex = comboFrom->Items->Count -1;
	}
}
//---------------------------------------------------------------------------

void __fastcall TformArtists::comboToSelect(TObject *Sender)
{
	if(comboTo->Text == comboFrom->Text)            // do not let combo boxes show the same playlist!
	{
		if(comboFrom->ItemIndex +1 < comboFrom->Items->Count)
			comboFrom->ItemIndex ++;
		else if(comboFrom->ItemIndex -1 >= 0)
			comboFrom->ItemIndex --;

		loadArtists(pl.delNonPathChars(comboFrom->Text)); // refresh lv
	}
}
//---------------------------------------------------------------------------


void TformArtists::loadArtists(UnicodeString plName)     // loads all artists to lv
{
	labelProgress->Caption = "   Indexation...  0%";
	prbarIndexation->Position = 0;
	lv->Clear();
	lvItems.clear();
    foundMatches.clear();

	TStringList *list = new TStringList;
	list->LoadFromFile(pl.getSettingsPath() + "playlists\\" + pl.delNonPathChars(comboFrom->Text)
		 + "\\playlistData");

	if(list->Count > 1500)
        panelProgress->Visible = true;   // reset progress panel stuff

	std::vector <UnicodeString> artists;         // indexation of artists in the file
	for(UINT i=0; i < list->Count; i+= 7)
		artists.push_back(list->Strings[i +1]);
	af.Indexation(artists, af.artBef, af.artIndexesBef);

	for(UINT i=0; i < af.artBef.size(); i++)           // add artists to the lv vector
		lvItems.push_back(LvItem(af.artBef[i]));
	showItems(lvItems, true);

	delete list;
	panelProgress->Visible = false;  // hide progress panel
    editQSearch->Enabled = true;     // allow quick search
}

void __fastcall TformArtists::comboFromSelect(TObject *Sender)
{
	loadArtists(comboFrom->Text);
}
//---------------------------------------------------------------------------


std::vector <UINT> getSelectedSongs() 		// gets indexes of all selected songs in pl by user
{
   std::vector<UINT> tempVec;
   if(!formArtists->lv->SelCount) return tempVec;    // 0 songs selected
														 // get first selected item
   int lastSelSong = ListView_GetNextItem(formArtists->lv->Handle, -1, LVNI_SELECTED);

   while(true)
   {
	  tempVec.push_back(lastSelSong);
														 // get next selected item
	  if(ListView_GetNextItem(formArtists->lv->Handle, lastSelSong, LVNI_SELECTED) != -1)
		 lastSelSong = ListView_GetNextItem(formArtists->lv->Handle, lastSelSong, LVNI_SELECTED);
	  else break;
   }

   return tempVec;
}

void __fastcall TformArtists::lvKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
   	if(Key == vkSpace)                                  // check / uncheck items
	{
		std::vector <UINT> selSongs = getSelectedSongs();
		for(UINT i=0; i < selSongs.size(); i++) {
			 lv->Items->Item[selSongs[i]]->Checked = !lv->Items->Item[selSongs[i]]->Checked;
		}

		Key = 0;    // don't mess checking
	}
}
//---------------------------------------------------------------------------



void __fastcall TformArtists::lvChange(TObject *Sender, TListItem *Item, TItemChange Change)
{
    bool itemsChecked = false;
	for(UINT i=0; i < lv->Items->Count; i++)
		if(lv->Items->Item[i]->Checked)
            itemsChecked = true;

	if(btnAdd->Enabled && !itemsChecked)               // disable / enable btnAdd + minimize blinking
		btnAdd->Enabled = false;
	else if(!btnAdd->Enabled && itemsChecked)
		btnAdd->Enabled = true;
}
//---------------------------------------------------------------------------

void TformArtists::showItems(const std::vector <LvItem> &LvItems, bool showProgress)  // shows that pushed items in lv
{
	lv->Clear();
    bool comboFromWasFocused = comboFrom->Focused();
	comboFrom->Enabled = false;     // don't let user annoy combobox too fast

	UINT percentage;

	lv->Items->BeginUpdate();
	if(showProgress)
	{
		for(UINT i=0; i < LvItems.size(); i++)
		{
			Application->ProcessMessages();
			percentage = i*100 / (LvItems.size() +1);          // show user progress
			prbarIndexation->Position = percentage;
			labelProgress->Caption = "Indexation...  ";
			labelProgress->Caption = labelProgress->Caption + percentage;
			labelProgress->Caption = labelProgress->Caption + '%';

			lv->AddItem(LvItems[i].name, NULL);
			lv->Items->Item[i]->Checked = LvItems[i].checked;
		}
	}
	else
	{
        for(UINT i=0; i < LvItems.size(); i++)
		{
			lv->AddItem(LvItems[i].name, NULL);
			lv->Items->Item[i]->Checked = LvItems[i].checked;
		}
    }
	lv->Items->EndUpdate();

   	prbarIndexation->Position = 100;
   	comboFrom->Enabled = true;
	if(comboFromWasFocused)
		comboFrom->SetFocus();
}

void __fastcall TformArtists::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
   if(Key == vkF3 && prbarIndexation->Position == prbarIndexation->Max)
	  editQSearch->SetFocus();
   if(Key == vkEscape && !editQSearch->Focused())
	  this->Close();

   if(Tmform::containsOnly(Shift, ModifierKey::Ctrl) && Key == vkA)
      lv->SelectAll();

   if(Key == vkReturn && !editQSearch->Focused() && btnAdd->Enabled) // user pressed ENTER, so finish the task
      btnAddClick(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TformArtists::lvItemChecked(TObject *Sender, TListItem *Item)
{
	 for(UINT i=0; i < lvItems.size(); i++)               // check / uncheck this item in vector too
		 if(Item->Caption == lvItems[i].name)
			lvItems[i].checked = Item->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TformArtists::timerShowDelayTimer(TObject *Sender)
{
	timerShowDelay->Enabled = false; /// turn off the timer

	loadArtists(comboFrom->Text);
}
//---------------------------------------------------------------------------



void __fastcall TformArtists::lvDblClick(TObject *Sender)
{
	 if(lv->SelCount == 1)
        lv->Selected->Checked = !lv->Selected->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TformArtists::lvKeyPress(TObject *Sender, System::WideChar &Key)
{
    Key = 0;		// don't allow user to use default listview search, ha-ha =D
}
//---------------------------------------------------------------------------

