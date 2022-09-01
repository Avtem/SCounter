//---------------------------------------------------------------------------

#ifndef PlaylistWindowH
#define PlaylistWindowH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TformPlaylists : public TForm
{
__published:	// IDE-managed Components
	TEdit *editName;
	TButton *btnOk;
	TButton *btnCancel;
	TLabel *textWarning;
	void __fastcall btnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall editNameChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TformPlaylists(TComponent* Owner);

	UnicodeString ampersandChanger(UnicodeString str, short numOfAmpersands); // 1 for &, 2 for &&
    bool spacesOnlyString(UnicodeString str);
};
//---------------------------------------------------------------------------
extern PACKAGE TformPlaylists *formPlaylists;
//---------------------------------------------------------------------------
#endif
