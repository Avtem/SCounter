//---------------------------------------------------------------------------
#pragma hdrstop
#include "sameFileExists.h"

	// =============== INCLUDES ==//
#include <vector>
#include <vcl.h>
#include <Windows.h>
#include <shlobj.h>
#include "Unit1.h"
#include "message.h"
#include "AddingFiles.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm2 *Form2;

void pressKey(char arrow);    		// win+left or win+right f   NOT USED

void openFolderClick(short choice);        // onOpenFolderClick
void openFolderSelect(UnicodeString path); // opens and selects given file
extern AddingFiles af;
extern std::vector <Songs> songs;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm2::fixedClick(TObject *Sender)                                // on [FIXED] click
{
	int i = fixed->Tag;

	if(i == af.samePlaylist.size()) { Form2->Close(); return; }  // There is no doubles

	path1->Text = songs[af.samePlaylist[i]].path;
	dur1->Caption = songs[af.samePlaylist[i]].duration;
	path2->Text = af.sameAdded[i];
	HSTREAM streamTemp = BASS_StreamCreateFile(false, af.sameAdded[i].c_str(), 0, 0, 0);
	dur2->Caption = songs[0].getDuration(streamTemp, 1);
	BASS_StreamFree(streamTemp);        // free that poor guy!

	if(dur1->Caption == dur2->Caption)
	{
	   dur1->StyleElements = TStyleElements(seFont + seClient + seBorder);
	   dur2->StyleElements = TStyleElements(seFont + seClient + seBorder);
	}
	else
	{
	   dur1->StyleElements = TStyleElements(seBorder);
	   dur2->StyleElements = TStyleElements(seBorder);
	}

	tagsOfSong->Caption = songs[af.samePlaylist[i]].name;

	UnicodeString left = "Skip all (";
	left = left + (af.samePlaylist.size()-i) + ")";
	skipAll->Caption = left;
	fixed->Tag ++;  // next, please !!!
}
//---------------------------------------------------------------------------
void __fastcall TForm2::skipAllClick(TObject *Sender)                              // on [SKIPALL] click
{
	Form2->Close();
}
//---------------------------------------------------------------------------

void openFolderClick(short choice)                                                      // onOpenFolderClick
{
	switch(choice)
	{
	   case 0:
		  openFolderSelect(Form2->path1->Text);
	   break;

	   case 1:
		  openFolderSelect(Form2->path2->Text);
	   break;

	   case 2:
		  openFolderSelect(Form2->path1->Text);
		  Sleep(200);
		  openFolderSelect(Form2->path2->Text);
	   break;
	}
}
void __fastcall TForm2::openf1Click(TObject *Sender)
{  openFolderClick(0);                                                       }
//---------------------------------------------------------------------------

void __fastcall TForm2::openf2Click(TObject *Sender)
{  openFolderClick(1);                                                       }
//---------------------------------------------------------------------------

void __fastcall TForm2::openfbothClick(TObject *Sender)
{  openFolderClick(2);                                                      }
//---------------------------------------------------------------------------

void __fastcall TForm2::FormShow(TObject *Sender)                                    // FORM SHOW
{
   fixed->SetFocus();

   if(ExtractFileDir(path1->Text) == ExtractFileDir(path2->Text))  // doubles in the same folder
   {
	  openf2->Enabled = false;
	  openf2->Flat = true;
	  openfboth->Enabled = false;
   }
   else
   {
	  openf2->Enabled = true;
	  openf2->Flat = false;
	  openfboth->Enabled = true;
   }
}
//---------------------------------------------------------------------------
void pressKey(char arrow)
{
//	Form2->pressKeyTimer->Enabled = true;
//	if(arrow == 'l') Form2->pressKeyTimer->Tag = 0;
//	else             Form2->pressKeyTimer->Tag = 1;
}

void openFolderSelect(UnicodeString path)
{
	if(!PathFileExistsW(path.c_str()))
	{
	  UnicodeString warning = "File not found: \n\n";
	  warning += path;
	  messageForm->showMessage(warning, 'e', 3);
	  return;
	}

//	CoInitialize(0);                                             // this is "working"
	CoInitializeEx(0, COINIT_MULTITHREADED);
	ITEMIDLIST *pidl = ILCreateFromPath(path.c_str());
	if(pidl) {
		SHOpenFolderAndSelectItems(pidl,0,0,0);
		ILFree(pidl);
	}
}



