object messageForm: TmessageForm
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsSizeToolWin
  ClientHeight = 148
  ClientWidth = 180
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object text: TLabel
    Left = 5
    Top = 28
    Width = 167
    Height = 40
    Alignment = taCenter
    AutoSize = False
    Caption = 'your text'
  end
  object button: TButton
    Left = 55
    Top = 71
    Width = 75
    Height = 25
    Align = alCustom
    Caption = 'OK'
    TabOrder = 0
    OnClick = buttonClick
  end
  object dontAnPPR: TCheckBox
    Left = 8
    Top = 101
    Width = 164
    Height = 17
    Hint = 'You can turn on messages again in settings'
    Caption = 'Don'#39't annoy me in this session'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
    Visible = False
    OnClick = dontAnPPRClick
  end
  object neverAnPPR: TCheckBox
    Left = 8
    Top = 124
    Width = 117
    Height = 17
    Hint = 'You can turn on messages again in settings'
    Caption = 'Never annoy me'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    Visible = False
    OnClick = neverAnPPRClick
  end
  object btnAbort: TButton
    Left = 136
    Top = 71
    Width = 36
    Height = 25
    Align = alCustom
    Caption = 'Abort'
    TabOrder = 3
    Visible = False
    OnClick = btnAbortClick
  end
  object timer: TTimer
    Enabled = False
    Interval = 1500
    OnTimer = timerTimer
    Left = 16
    Top = 56
  end
end
