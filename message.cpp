//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "message.h"
// ============== my includes ===================== //
#include "libraries/bass.h"
#include "ssetings.h"
#include "Playlists.h"


extern void myShowModal(TForm *form);                     // shows form modal
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TmessageForm *messageForm;


//---------------------------------------------------------------------------
__fastcall TmessageForm::TmessageForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TmessageForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)    // keyDown
{
   if(Key == vkF4 && pl.terminatingSong) Key = 0;     	// prevent Alt+F4
   if(Key == vkEscape && !pl.terminatingSong) messageForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TmessageForm::buttonClick(TObject *Sender)                                 // buttonClick
{
   if(pl.terminatingSong)                       // minimize, if termSong
   {
	   messageForm->Visible = false;
	   mform->minimize(); 			// hide to tray / minimize
	   return;
   }

   messageForm->Close();                              // close message
}
//---------------------------------------------------------------------------

void TmessageForm::showMessage(UnicodeString inputText, char beep, int rows)   // main function
{                                                                              // showing text, changing form
	short  defTextTop = 28;

	if(rows == 3 || rows == 33 || rows == 666) text->Top = 15;        // text TOP
	else	 text->Top = defTextTop;

	if(rows == 33)            // don't annoy me again
	{
	   dontAnPPR->Visible = true;
	   neverAnPPR->Visible = true;
	   messageForm->Height = 170;
	}
	else
	{
	   dontAnPPR->Visible = false;
	   neverAnPPR->Visible = false;
	   messageForm->Height = 125;
	}

	size_t pos = inputText.Pos("\n");            // position of "\n" character
	messageForm->Width =
	text->Canvas->TextWidth(inputText.SubString(pos, inputText.Length()-pos)) + 50;     // form WIDTH
    if(rows == 666) messageForm->Width = 200;    // count us

	text->Width  = messageForm->Width - 30;                           // set text WIDTH
	text->Left   = (messageForm->Width - text->Width)/2 - 8;          // text LEFT
	text->Caption = inputText;                                        // set message text
	button->Left = (messageForm->Width - button->Width)/2 - 7;        // button LEFT
	button->Caption = L"OK";                                          // button Caption
	if(rows == 666)   button->Caption = L"Wow!";
	if(beep == 't') {
	   button->Left -= 40;  					// if termination Song
	   dontAnPPR->Caption = "Minimize immediately";
	   dontAnPPR->Visible = true;
       messageForm->Height = 148;
	}
	else  dontAnPPR->Caption = "Don't annoy me in this session";
	messageForm->Position = poScreenCenter;                           // set form POS

	playBeep(beep);                                                   // play beep
					 // NEXTBTN tag '2' means, that SCounter hides immediately)
	if(mform->nextbtn->Tag != 2)
		myShowModal(messageForm);       		  // SHOW FORM
}
	// ---------------------------------------------------------------------------
void TmessageForm::playBeep(char choice)                                            // play beep
{
	UnicodeString path;
	switch(choice)
	{
		case 'i': path = L"res/sounds/_info.mp3";     break;
		case 'w': path = L"res/sounds/_warning.mp3";  break;
		case 'q': path = L"res/sounds/_question.mp3"; break;
		case 'e': path = L"res/sounds/_error.mp3";	break;
		case 'E': path = L"res/sounds/endOfPl.mp3";
				  timer->Enabled = true;                break;
	}
	beepStream = BASS_StreamCreateFile(false, path.c_str(), 0,0,0);
	BASS_ChannelPlay(beepStream, false);
}

// =====================================================================================

void __fastcall TmessageForm::timerTimer(TObject *Sender)                              // timer
{                                                        // for theEnd()
    timer->Enabled = false;
	messageForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TmessageForm::FormClose(TObject *Sender, TCloseAction &Action)       // onClose
{
//	BASS_StreamFree(beepStream);      //DINT?
}
//---------------------------------------------------------------------------



void __fastcall TmessageForm::dontAnPPRClick(TObject *Sender)                // [DONTANNOY CHECKBOX]
{   if(pl.terminatingSong) pl.showTermSongMes = !dontAnPPR->Checked;    // term song
	else 	      setForm->dontAnPPR->Checked = !dontAnPPR->Checked; }  // usual case
//---------------------------------------------------------------------------

void __fastcall TmessageForm::neverAnPPRClick(TObject *Sender)           // [NEVERANNOY CHECKBOX]
{   setForm->neverAnPPR->Checked = !neverAnPPR->Checked;
	setForm->dontAnPPR->Checked = setForm->neverAnPPR->Checked;    }
//---------------------------------------------------------------------------



void __fastcall TmessageForm::FormShow(TObject *Sender)         // onShow
{
   button->SetFocus();

   if(pl.terminatingSong) dontAnPPR->Checked = !pl.showTermSongMes;
   else dontAnPPR->Checked = !setForm->dontAnPPR->Checked;
   neverAnPPR->Checked = !setForm->neverAnPPR->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TmessageForm::btnAbortClick(TObject *Sender)                 // TERM.SONG [ABORT]
{
	 pl.terminatingSong = false;                 // user aborted terminating song
	 mform->nextbtn->Font->Color = clBlack;
	 btnAbort->Visible = false;
	 mform->nextbtn->Tag = 1; 			// to default tag
	 messageForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TmessageForm::FormResize(TObject *Sender)
{
	 btnAbort->Left = button->Left + button->Width + 20;
     btnAbort->Width = button->Width;
}
//---------------------------------------------------------------------------

void __fastcall TmessageForm::FormDestroy(TObject *Sender)
{
//	 BASS_StreamFree(beepStream);
//	 BASS_Free();
}
//---------------------------------------------------------------------------

