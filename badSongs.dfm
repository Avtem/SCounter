object formBadSongs: TformBadSongs
  Left = 0
  Top = 0
  Width = 560
  Height = 287
  AutoScroll = True
  BorderIcons = []
  Caption = 'Deleting bad songs'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnKeyDown = FormKeyDown
  OnResize = FormResize
  OnShow = FormShow
  DesignSize = (
    552
    260)
  PixelsPerInch = 96
  TextHeight = 13
  object labelFound: TLabel
    Left = 8
    Top = 224
    Width = 113
    Height = 16
    Anchors = [akLeft, akBottom]
    Caption = 'Found bad songs: 0'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    ExplicitTop = 273
  end
  object labelProgress: TLabel
    Left = 15
    Top = 25
    Width = 72
    Height = 13
    Caption = 'Progress: 34%'
  end
  object label2: TLabel
    Left = 8
    Top = 7
    Width = 349
    Height = 16
    Caption = 'Step 1/2:      Checking for '#39'removed/deleted/renamed'#39' songs'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object lv: TListView
    AlignWithMargins = True
    Left = 3
    Top = 44
    Width = 546
    Height = 176
    Margins.Top = 44
    Margins.Bottom = 40
    Align = alClient
    Anchors = [akLeft, akTop, akRight]
    BorderStyle = bsNone
    Color = 2235673
    Columns = <
      item
        Caption = 'Issue'
        Width = 140
      end
      item
        Caption = 'Name'
        Width = 300
      end
      item
        Caption = 'Path'
        Width = 100
      end>
    DoubleBuffered = True
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    HideSelection = False
    StyleElements = [seFont, seBorder]
    OwnerData = True
    OwnerDraw = True
    ReadOnly = True
    RowSelect = True
    ParentDoubleBuffered = False
    ParentFont = False
    TabOrder = 0
    ViewStyle = vsReport
    OnChange = lvChange
    OnData = lvData
    OnDrawItem = lvDrawItem
  end
  object btnCancel: TButton
    AlignWithMargins = True
    Left = 472
    Top = 228
    Width = 75
    Height = 25
    Margins.Bottom = 0
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = btnCancelClick
  end
  object btnDeleteAll: TButton
    AlignWithMargins = True
    Left = 391
    Top = 228
    Width = 75
    Height = 25
    Margins.Bottom = 0
    Anchors = [akRight, akBottom]
    Caption = 'Delete all'
    Default = True
    Enabled = False
    TabOrder = 2
    OnClick = btnDeleteAllClick
  end
  object prbar: TProgressBar
    Left = 3
    Top = 41
    Width = 546
    Height = 5
    Margins.Bottom = 0
    Anchors = [akLeft, akTop, akRight]
    DoubleBuffered = False
    Max = 1000
    ParentDoubleBuffered = False
    BarColor = clAqua
    BackgroundColor = 2302755
    Step = 1
    TabOrder = 3
  end
  object btnShowInExplorer: TButton
    AlignWithMargins = True
    Left = 295
    Top = 228
    Width = 90
    Height = 25
    Margins.Bottom = 0
    Anchors = [akRight, akBottom]
    Caption = 'Show in explorer'
    TabOrder = 4
    OnClick = btnShowInExplorerClick
  end
  object timerShowDelay: TTimer
    Enabled = False
    Interval = 20
    OnTimer = timerShowDelayTimer
    Left = 176
    Top = 128
  end
end
