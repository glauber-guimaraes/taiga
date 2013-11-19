/*
** Taiga, a lightweight client for MyAnimeList
** Copyright (C) 2010-2012, Eren Okka
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

#ifndef DLG_ANIME_INFO_PAGE_H
#define DLG_ANIME_INFO_PAGE_H

#include "base/std.h"
#include "win32/win_dialog.h"

// =============================================================================

enum AnimeInfoPageType {
  INFOPAGE_NONE,
  INFOPAGE_SERIESINFO,
  INFOPAGE_MYINFO,
  INFOPAGE_NOTRECOGNIZED
};

class AnimeDialog;

class PageBaseInfo : public win32::Dialog {
 public:
  PageBaseInfo();
  virtual ~PageBaseInfo() {}
  
  INT_PTR DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  BOOL OnInitDialog();
  void OnPaint(HDC hdc, LPPAINTSTRUCT lpps);
  void OnSize(UINT uMsg, UINT nType, SIZE size);

  AnimeDialog* parent;

 protected:
  int anime_id_;
};

class PageSeriesInfo : public PageBaseInfo {
 public:
  void OnSize(UINT uMsg, UINT nType, SIZE size);

  void Refresh(int anime_id, bool connect);
};

class PageMyInfo : public PageBaseInfo {
 public:
  BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);
  bool Save();

  void Refresh(int anime_id);
  void RefreshFansubPreference();
};

#endif // DLG_ANIME_INFO_PAGE_H