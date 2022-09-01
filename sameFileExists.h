//---------------------------------------------------------------------------

#ifndef sameFileExistsH
#define sameFileExistsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>

//---------------------------------------------------------------------------
class TForm2 : public TForm
{
__published:	// IDE-managed Components
	TEdit *path1;
	TEdit *path2;
	TLabel *Label1;
	TLabel *Label2;
	TButton *fixed;
	TButton *skipAll;
	TLabel *tagsOfSong;
	TSpeedButton *openf1;
	TSpeedButton *openfboth;
	TSpeedButton *openf2;
	TPanel *dur1;
	TPanel *dur2;

	void __fastcall skipAllClick(TObject *Sender);
	void __fastcall fixedClick(TObject *Sender);
	void __fastcall openf1Click(TObject *Sender);
	void __fastcall openf2Click(TObject *Sender);
	void __fastcall openfbothClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm2(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
