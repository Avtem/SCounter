#pragma once

enum FocusBtnState                       // used for [FOCUSBTN]
{                                        // items in popup menu has tags associated with this enum
    FocusPlaying = 0, FocusSelected, FocusSCS
};

enum ReasonToDelete                      // for [DELETE BAD SONGS] function. Represents current state of form.
{
	Twin = 0, NotFoundInAllSongs
};

enum LetterCase
{
    Lower = 0, Sentence
};

enum ModifierKey :unsigned short
{
    Ctrl = 0, Shift, Alt, Win, None
};

enum DoublePress :short
{
    Local = 0, Global
};

enum FileDialogState :short
{
    NotShowing = 0, Showing
};

enum Filter : char
{
    All = 0, Filter1, Filter2, Filter3, Filter4
};
