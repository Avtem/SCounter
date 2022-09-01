object formPlaylists: TformPlaylists
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  BorderWidth = 1
  Caption = 'Rename?'
  ClientHeight = 91
  ClientWidth = 210
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poMainFormCenter
  OnClose = FormClose
  OnKeyDown = FormKeyDown
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object textWarning: TLabel
    Left = 8
    Top = 41
    Width = 194
    Height = 17
    AutoSize = False
    Caption = 'Too short name'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 236
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    Visible = False
    WordWrap = True
    StyleElements = [seClient, seBorder]
  end
  object editName: TEdit
    Left = 8
    Top = 13
    Width = 193
    Height = 21
    TabOrder = 0
    OnChange = editNameChange
  end
  object btnOk: TButton
    Left = 13
    Top = 63
    Width = 75
    Height = 25
    Caption = 'Ok'
    TabOrder = 1
    OnClick = btnOkClick
  end
  object btnCancel: TButton
    Left = 120
    Top = 63
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 2
    OnClick = btnCancelClick
  end
end
