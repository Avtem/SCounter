#ifndef SCOUNTER - DEBUG_CPP
#define SCOUNTER - DEBUG_CPP

//---------------------------------------------------------------------------
#pragma hdrstop

#include <vcl.h>
#include <tchar.h>
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>

		// MY INCLUDES

//---------------------------------------------------------------------------

USEFORM("PlaylistWindow.cpp", formPlaylists);
USEFORM("llistenings.cpp", Listenings);
USEFORM("message.cpp", messageForm);
USEFORM("ssetings.cpp", setForm);
USEFORM("Unit1.cpp", mform);
USEFORM("sameFileExists.cpp", Form2);
USEFORM("artists.cpp", formArtists);
USEFORM("badSongs.cpp", formBadSongs);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		UnicodeString mutexName = L"SCounterTV220 MUTEXX";
		#ifdef _DEBUG
			mutexName += " DEBUG";
		#endif

			// Try to open the mutex.
		HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, mutexName.c_str());

		if (!hMutex)     // Mutex doesnt exist. This is the first instance so create the mutex.
		   hMutex = CreateMutex(0, 0, mutexName.c_str());
		else     // The mutex exists so this is the the second instance so return.
		{
//		  HWND hWnd = FindWindow(0,  L"SCounter");
//					SetWindowTextA(hWnd, "text"); // to check whether right handle received

		  ShowMessage("SCounter is already running");

		  return 0;
		}

		Application->Initialize();
		#ifdef _DEBUG
			TStyleManager::TrySetStyle("Golden Graphite");
		#else
			TStyleManager::TrySetStyle("Avtem");
		#endif

		Application->CreateForm(__classid(Tmform), &mform);
		Application->CreateForm(__classid(TForm2), &Form2);
		Application->CreateForm(__classid(TListenings), &Listenings);
		Application->CreateForm(__classid(TsetForm), &setForm);
		Application->CreateForm(__classid(TmessageForm), &messageForm);
		Application->CreateForm(__classid(TformPlaylists), &formPlaylists);
		Application->CreateForm(__classid(TformBadSongs), &formBadSongs);
		Application->CreateForm(__classid(TformArtists), &formArtists);
		Application->Run();
			// The app is closing so release the mutex.
		ReleaseMutex(hMutex);
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}

	return 0;
}
//---------------------------------------------------------------------------

#endif
