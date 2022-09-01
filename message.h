//---------------------------------------------------------------------------

#ifndef messageH
#define messageH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
// ============== my includes ===================== //
#include "libraries/bass.h"

//---------------------------------------------------------------------------
class TmessageForm : public TForm
{
__published:	// IDE-managed Components
	TLabel *text;
	TButton *button;
	TTimer *timer;
	TCheckBox *dontAnPPR;
	TCheckBox *neverAnPPR;
	TButton *btnAbort;
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall buttonClick(TObject *Sender);
	void __fastcall timerTimer(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall dontAnPPRClick(TObject *Sender);
	void __fastcall neverAnPPRClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnAbortClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TmessageForm(TComponent* Owner);

	HSTREAM beepStream;                                  		         // bass stream

	void showMessage(UnicodeString inputText, char beep, int rows);      // main function
	void playBeep(char choice);                          		         // play beep
};
//---------------------------------------------------------------------------
extern PACKAGE TmessageForm *messageForm;
//---------------------------------------------------------------------------
#endif
