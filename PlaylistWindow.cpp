//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "PlaylistWindow.h"

                 // MY INCLUDES
#include "Unit1.h"
#include "Playlists.h"
#include <shlwapi.h>                    // for ctrl+backspace
#include <vector>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TformPlaylists *formPlaylists;

extern bool renaming;           // will be playlist renamed or added?
extern bool addingNewPl;        // is user adding new pl? (for rearrange plBtns)
extern std::vector <TButton*> plButtons;    // SCouner playlists
extern Playlists pl;

//---------------------------------------------------------------------------    // on Create
__fastcall TformPlaylists::TformPlaylists(TComponent* Owner)
	: TForm(Owner)
{
	SHAutoComplete(editName->Handle, SHACF_AUTOAPPEND_FORCE_OFF); // allow ctrl+backspace
}
//---------------------------------------------------------------------------

void __fastcall TformPlaylists::btnOkClick(TObject *Sender)
{
	if(!renaming) {
	   addingNewPl = true;
	   mform->addPlaylist(ampersandChanger(editName->Text, 2));     // make caption with &&
       addingNewPl = false;
	}
	else
		mform->renamePlaylist(ampersandChanger(editName->Text, 2));

	formPlaylists->Close();
}
//---------------------------------------------------------------------------

void __fastcall TformPlaylists::FormShow(TObject *Sender)                // on Show
{   editNameChange(NULL);  editName->SetFocus();
	textWarning->Visible = false;	}
//---------------------------------------------------------------------------
bool TformPlaylists::spacesOnlyString(UnicodeString str)
{
	while(str.Pos(" "))
		str = str.Delete(str.Pos(" "), 1);

	if(str.Length()) return false;
    else return true;
}

                                                                                   // on Keydown
void __fastcall TformPlaylists::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{    if(Key == vkReturn) if(btnOk->Enabled)   btnOkClick(NULL);    // [ENTER]
	 if(Key == vkEscape)    formPlaylists->Close();      		   // [ESCAPE]
}
//---------------------------------------------------------------------------

void __fastcall TformPlaylists::btnCancelClick(TObject *Sender)
{   formPlaylists->Close();                 }           // [btn CANCEL]
//---------------------------------------------------------------------------

void __fastcall TformPlaylists::FormKeyPress(TObject *Sender, System::WideChar &Key)
{     if(Key == vkReturn) Key = 0;    }                                   // prevent beeping
//---------------------------------------------------------------------------

void __fastcall TformPlaylists::FormClose(TObject *Sender, TCloseAction &Action)  // on Close
{   renaming = false;       }
//---------------------------------------------------------------------------


void __fastcall TformPlaylists::editNameChange(TObject *Sender)                  // on edit Change
{                                                             // GUARDS
	bool sameName = false, longName = false, bannedCharsOnly = false, spacesOnly = false;

	if(Canvas->TextWidth(editName->Text) +21 > mform->panelMain->Width/2) longName = 1;          // 111 LONG NAME
	for(int i=0; i < plButtons.size(); i++)        // oh no, playlist with such name exists!     // 222 SAME NAME
		if(pl.delNonPathChars(editName->Text).LowerCase() == pl.delNonPathChars(plButtons[i]->Caption).LowerCase())
			if(!renaming ||   // it's adding new playlist, thus sameName = 1
				renaming && i != mform->popRClick->Tag) // if renaming, don't check that playlist
					sameName = 1;
	UnicodeString allSongsCaption = "Allsongs";
	if(pl.delNonPathChars(editName->Text).LowerCase() == allSongsCaption.LowerCase())
        sameName = true;

	if(renaming && editName->Text == plButtons[mform->popRClick->Tag]->Caption) sameName = 0;
	if(pl.delNonPathChars(editName->Text).Length() == 0
	 ||spacesOnlyString(pl.delNonPathChars(editName->Text)))   bannedCharsOnly = 1;     // 333 BANNED CHARS
	spacesOnly = spacesOnlyString(editName->Text);                                      // 444 SHORT | SPACES

	if(spacesOnly || longName || sameName || bannedCharsOnly)
	{
	   if(pl.delNonPathChars(editName->Text).LowerCase() == allSongsCaption.LowerCase())
	   		textWarning->Caption = "This name is using by SCounter";
	   else if(longName) textWarning->Caption = "Too long name";
	   else if(sameName) textWarning->Caption = "Playlist with this name already exists";
	   else	if(spacesOnly) textWarning->Caption = "Too short name";
	   else              textWarning->Caption = "Add something except <>.?*|/\\"" 'space'";

	   if(!textWarning->Visible) textWarning->Visible = true;
	   btnOk->Enabled = false;
	   return;
	}

	btnOk->Enabled = true;           // it is okay.
	textWarning->Visible = false;
}
//---------------------------------------------------------------------------

UnicodeString TformPlaylists::ampersandChanger(UnicodeString str, short numOfAmpersands)
{                                                                       // 1 for get string with &
	if(numOfAmpersands == 1)                                        	// 2 for get string with &&
	{
	   for(int i = 1; i <= str.Length() ; i++)
		  if(str.SubString(i, 2) == "&&") str.Delete(i, 1);
    }
	else
	{
	   for(int i = str.Length(); i > 0 ; i--)                  // make && everywhere
		  if(str.SubString(i, 1) == "&") str.Insert("&", i);
    }

    return str;
}
