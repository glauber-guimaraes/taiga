/*
** Taiga
** Copyright (C) 2010-2014, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TAIGA_UI_DLG_FORMAT_H
#define TAIGA_UI_DLG_FORMAT_H

#include "win/ctrl/win_ctrl.h"
#include "win/win_dialog.h"

namespace ui {

enum FormatModes {
  kFormatModeHttp,
  kFormatModeMessenger,
  kFormatModeMirc,
  kFormatModeSkype,
  kFormatModeTwitter,
  kFormatModeBalloon
};

class FormatDialog : public win::Dialog {
public:
  FormatDialog();
  ~FormatDialog() {}

  BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  BOOL OnInitDialog();
  void OnOK();

  void ColorizeText();
  void RefreshPreviewText();

  int mode;

private:
  win::RichEdit rich_edit_;
};

extern FormatDialog DlgFormat;

}  // namespace ui

#endif  // TAIGA_UI_DLG_FORMAT_H