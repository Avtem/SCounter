object formArtists: TformArtists
  AlignWithMargins = True
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsToolWindow
  Caption = 'Copying artists'
  ClientHeight = 383
  ClientWidth = 299
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poDefault
  OnKeyDown = FormKeyDown
  OnShow = FormShow
  DesignSize = (
    299
    383)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 28
    Height = 13
    Caption = 'From:'
  end
  object Label2: TLabel
    Left = 8
    Top = 31
    Width = 16
    Height = 13
    Caption = 'To:'
  end
  object lv: TListView
    AlignWithMargins = True
    Left = 3
    Top = 60
    Width = 293
    Height = 293
    Margins.Top = 6
    Align = alClient
    BorderStyle = bsNone
    Checkboxes = True
    Color = 2235673
    Columns = <
      item
        Caption = 'plName'
        Width = 265
      end>
    DoubleBuffered = True
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clHighlightText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    HideSelection = False
    MultiSelect = True
    StyleElements = [seFont]
    ReadOnly = True
    RowSelect = True
    ParentDoubleBuffered = False
    ParentFont = False
    ShowColumnHeaders = False
    TabOrder = 0
    ViewStyle = vsReport
    OnChange = lvChange
    OnDblClick = lvDblClick
    OnKeyDown = lvKeyDown
    OnKeyPress = lvKeyPress
    OnItemChecked = lvItemChecked
  end
  object editQSearch: TEdit
    AlignWithMargins = True
    Left = 3
    Top = 359
    Width = 216
    Height = 21
    Hint = 'Type at least 2 characters to find song.'
    Margins.Right = 80
    TabStop = False
    Align = alBottom
    Color = 2367001
    DoubleBuffered = True
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clInactiveCaption
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentDoubleBuffered = False
    ParentFont = False
    TabOrder = 1
    Text = 'Quick search [F3]'
    StyleElements = [seBorder]
    OnChange = editQSearchChange
    OnEnter = editQSearchEnter
    OnExit = editQSearchExit
    OnKeyDown = editQSearchKeyDown
  end
  object btnAdd: TButton
    Left = 229
    Top = 361
    Width = 61
    Height = 19
    Anchors = [akRight, akBottom]
    Caption = '&Add'
    TabOrder = 2
    OnClick = btnAddClick
  end
  object comboTo: TComboBox
    AlignWithMargins = True
    Left = 55
    Top = 30
    Width = 241
    Height = 21
    Margins.Left = 55
    Align = alTop
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 3
    Text = 'currPlaylist'
    OnSelect = comboToSelect
    Items.Strings = (
      'currPlaylist')
  end
  object comboFrom: TComboBox
    AlignWithMargins = True
    Left = 55
    Top = 3
    Width = 241
    Height = 21
    Margins.Left = 55
    Align = alTop
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 4
    Text = 'Choose playlist...'
    OnEnter = comboFromEnter
    OnSelect = comboFromSelect
    Items.Strings = (
      'Choose playlist...')
  end
  object prbarIndexation: TProgressBar
    Left = 3
    Top = 57
    Width = 290
    Height = 5
    Anchors = [akLeft, akTop, akRight]
    DoubleBuffered = False
    ParentDoubleBuffered = False
    BarColor = clAqua
    BackgroundColor = 2302755
    Step = 1
    TabOrder = 5
  end
  object panelProgress: TPanel
    Left = 81
    Top = 185
    Width = 133
    Height = 32
    Alignment = taLeftJustify
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 6
    Visible = False
    object labelProgress: TLabel
      Left = 12
      Top = 8
      Width = 121
      Height = 16
      AutoSize = False
      Caption = 'Label3'
    end
  end
  object timerShowDelay: TTimer
    Enabled = False
    Interval = 20
    OnTimer = timerShowDelayTimer
    Left = 40
    Top = 104
  end
end
