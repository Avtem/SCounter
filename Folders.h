#pragma once
//	This structure was written in order to copy folders to another path recursively.
//
//	Created: 04.06.2019

      // ============ MY INCLUDES ============
#include <vcl.h>    // borland stuff
#include <vector>   // vectors


	  // ============ MY FUNCTIONS ============
UnicodeString findRepChar(UnicodeString str, wchar_t what, wchar_t with);
	// replaces all "what" chars with "with" char in string "str"



	  // ============ MY VARIABLES ============




struct SrcFolder
{
	UnicodeString folderName;               // name of the folder "aegisub"
	UnicodeString pathToTheFolder;          // "C:/my/programs/"
	bool objectIsValid;                     // false if user passed FILE to "folderPath"

	std::vector <UnicodeString> folders;    // list of folder names
	std::vector <UnicodeString> files;      // list of file names
	std::vector <UnicodeString> newFolders;    // list of folder names
	std::vector <UnicodeString> newFiles;      // list of file names

  // ==========================================================================

    SrcFolder(UnicodeString pathToSourceDir);					// CONSTRUCTOR
	void deletePathFromNames();             // deletes "C:/my/programs/" from all paths
	void getAllFilesFromDir(const UnicodeString &dirPath);  // finds all files and folders and adds to vectors
	bool isFolder(const UnicodeString &filePath);               // is that folder?
	void copyToAnotherFolder(UnicodeString destPath);       // copy that folder to another "E:/newFolder"
															// where 'newFolder' is name of new folder
	static std::vector <UnicodeString> SrcFolder::getFileNames(const UnicodeString &folderPath);
		// returns all filenames in given folder
};