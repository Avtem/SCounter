//---------------------------------------------------------------------------

#pragma hdrstop
#pragma package(smart_init)

#include "Folders.h"



      // ============ MY INCLUDES ============
#include <dirent.h> // for windows folder functions

	  // ============ MY FUNCTIONS ============


	  // ============ MY VARIABLES ============


SrcFolder::SrcFolder(UnicodeString pathToSourceDir)		// CONSTRUCTOR
{
	pathToSourceDir = findRepChar(pathToSourceDir, '/', '\\');   // replace slashes


	if(!isFolder(pathToSourceDir) || !DirectoryExists(pathToSourceDir))   // user passed FILE
	{
	   objectIsValid = false;
//       ShowMessage("Passed file to the folder function! Or file doesn't exist!");
	   return;
    }

	objectIsValid = true;
	folderName = ExtractFileName(pathToSourceDir);
	pathToTheFolder = pathToSourceDir.SubString0(0, pathToSourceDir.Length() - folderName.Length());

	getAllFilesFromDir(pathToSourceDir);        // add all paths
	deletePathFromNames();                      // and remove beginning of every path ("C:/my/programs");
}

void SrcFolder::getAllFilesFromDir(const UnicodeString &dirPath)
{                      // finds all files and folders and adds to vectors
	wDIR *wdir = wopendir(dirPath.c_str()); 		// well, what did we receive here?
	struct wdirent *ent;
	UnicodeString currDir;    	// create main dir

 //=============================================== if we have a folder
	if(wdir)  // it's a folder!
	{
		folders.push_back(dirPath);

		while ((ent=wreaddir(wdir)) != NULL)  	// reading all files in the folder
		{
			if ( wcscmp(ent->d_name, L".") && wcscmp(ent->d_name, L"..") )
			{
				currDir = dirPath + "\\" + ent->d_name;

				if(isFolder(currDir) == true) // were folders in dropped folder?  // RECURSION
					getAllFilesFromDir(currDir);
				else                          // we had no folders in  the folder
				   files.push_back(currDir);
			}
		}

		wclosedir(wdir);  // be nice guy and close files after you.
	}
	else
	{
		files.push_back(dirPath);    // we had no folders in  the folder
	}
}

bool SrcFolder::isFolder(const UnicodeString &filePath)               // is that folder?
{
	wDIR *openedDir = wopendir(filePath.c_str());
	if(openedDir) {
		wclosedir(openedDir);
		return true;      // it's folder
    }

	return false;
}



std::vector <UnicodeString> SrcFolder::getFileNames(const UnicodeString &folderPath)
		// returns all filenames in given folder
{
	wDIR *wdir = wopendir(folderPath.c_str());                 // well, what did we receive here?
	struct wdirent *ent;
	UnicodeString currDir;    	// create main dir
	std::vector <UnicodeString> names;

 //=============================================== if we have a folder
	if(wdir)  // it's a folder!
	{
		while ((ent=wreaddir(wdir)) != NULL)  	// reading all files in the folder
		{
			if( wcscmp(ent->d_name, L".") && wcscmp(ent->d_name, L"..") )
				names.push_back(folderPath + '\\' + ent->d_name);
		}
     	wclosedir(wdir);
	}

	return names;
}

UnicodeString findRepChar(UnicodeString str, wchar_t what, wchar_t with)
	// replaces all "what" chars with "with" char in string "str"
{
	UINT pos = str.Pos(what);

	while(pos)
	{
		str = str.Delete(pos, 1);             // find & replace
		str = str.Insert(with, pos);

		pos = str.Pos(what);                  // find next "what"
	}

	return str;
}

void SrcFolder::deletePathFromNames()              // deletes "C:/my/programs/" from all paths
{
	for(UINT i=0; i < folders.size(); i++)
		folders[i].Delete0(0, pathToTheFolder.Length());

	for(UINT i=0; i < files.size(); i++)
		files[i].Delete0(0, pathToTheFolder.Length());
}


void SrcFolder::copyToAnotherFolder(UnicodeString destPath)   // copy that folder to another "E:/newFolder"
{                                                             // where 'newFolder' is name of new folder
    destPath.Insert0('\\', destPath.Length());
	destPath = findRepChar(destPath, '/', '\\');
	if(!DirectoryExists(destPath)) {                              // GUARD
		ShowMessage(destPath + "\nNot found! Can't copy there.");
        return;
    }

	for(UINT i=0; i < folders.size(); i++) {             // copy every folder
		newFolders.push_back(destPath + folders[i]);
        CreateDir(newFolders[i]);
    }
	for(UINT i=0; i < files.size(); i++) {               // copy every file
        Application->ProcessMessages();
		newFiles.push_back(destPath + files[i]);
		CopyFile((pathToTheFolder + files[i]).c_str(), newFiles[i].c_str(), false);
	}
}

