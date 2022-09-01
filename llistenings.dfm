object Listenings: TListenings
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Listenings'
  ClientHeight = 518
  ClientWidth = 488
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  OnKeyDown = FormKeyDown
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object lvListenings: TListView
    Left = 8
    Top = 68
    Width = 472
    Height = 397
    BorderStyle = bsNone
    Color = 2235673
    Columns = <
      item
        Caption = 'Artist                                                  [A-Z]'
        MaxWidth = 215
        MinWidth = 215
        Width = 215
      end
      item
        Alignment = taCenter
        Caption = 'Listenings'
        MaxWidth = 90
        MinWidth = 90
        Width = 90
      end
      item
        Alignment = taCenter
        Caption = 'Added'
        MaxWidth = 150
        MinWidth = 150
        Width = 150
      end>
    DoubleBuffered = True
    HideSelection = False
    StyleElements = [seFont]
    OwnerData = True
    ReadOnly = True
    RowSelect = True
    ParentDoubleBuffered = False
    TabOrder = 0
    ViewStyle = vsReport
    OnColumnClick = lvListeningsColumnClick
    OnData = lvListeningsData
  end
  object firstSongEver: TPanel
    Left = 322
    Top = 14
    Width = 105
    Height = 16
    Caption = '43.24.3333'
    TabOrder = 1
  end
  object totalSongsListenings: TPanel
    Left = 322
    Top = 36
    Width = 105
    Height = 16
    Caption = '234234'
    TabOrder = 2
  end
  object Panel3: TPanel
    Left = 173
    Top = 14
    Width = 140
    Height = 16
    Caption = 'Date of first added song'
    TabOrder = 3
  end
  object Panel4: TPanel
    Left = 173
    Top = 36
    Width = 140
    Height = 16
    Caption = 'Total songs listenings'
    TabOrder = 4
  end
  object Panel5: TPanel
    Left = 141
    Top = 4
    Width = 5
    Height = 60
    TabOrder = 5
  end
  object numberOfArtists: TPanel
    Left = 8
    Top = 36
    Width = 105
    Height = 16
    Caption = '234234'
    TabOrder = 6
  end
  object Panel7: TPanel
    Left = 8
    Top = 14
    Width = 104
    Height = 16
    Caption = 'Number of artists'
    TabOrder = 7
  end
  object startListenings: TPanel
    Left = 264
    Top = 474
    Width = 105
    Height = 16
    Caption = '234234'
    TabOrder = 8
  end
  object Panel6: TPanel
    Left = 104
    Top = 474
    Width = 150
    Height = 16
    Caption = 'Current session listenings'
    TabOrder = 9
  end
  object todayList: TPanel
    Left = 264
    Top = 496
    Width = 105
    Height = 16
    Caption = '234234'
    TabOrder = 10
  end
  object Panel9: TPanel
    Left = 104
    Top = 496
    Width = 150
    Height = 16
    Caption = 'Today listenings'
    TabOrder = 11
  end
end
