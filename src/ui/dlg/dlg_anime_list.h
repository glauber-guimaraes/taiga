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

#ifndef TAIGA_UI_DLG_ANIME_LIST_H
#define TAIGA_UI_DLG_ANIME_LIST_H

#include "win/ctrl/win_ctrl.h"
#include "win/win_dialog.h"
#include "win/win_gdi.h"

namespace anime {
class Item;
}

namespace ui {

class AnimeListDialog : public win::Dialog {
public:
  AnimeListDialog();
  virtual ~AnimeListDialog() {}

  INT_PTR DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  BOOL OnInitDialog();
  LRESULT OnListNotify(LPARAM lParam);
  LRESULT OnListCustomDraw(LPARAM lParam);
  LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);
  void OnSize(UINT uMsg, UINT nType, SIZE size);
  LRESULT OnTabNotify(LPARAM lParam);

  int GetCurrentId();
  anime::Item* GetCurrentItem();
  void SetCurrentId(int anime_id);
  int GetListIndex(int anime_id);
  void RefreshList(int index = -1);
  void RefreshListItem(int anime_id);
  void RefreshTabs(int index = -1);

  void GoToPreviousTab();
  void GoToNextTab();

public:
  // List-view control
  class ListView : public win::ListView {
  public:
    ListView();

    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void DrawProgressBar(HDC hdc, RECT* rc, int index, UINT uItemState, anime::Item& anime_item);
    void DrawScoreBox(HDC hdc, RECT* rc, int index, UINT uItemState, anime::Item& anime_item);

    int GetSortType(int column);
    void RefreshItem(int index);

    win::Rect button_rect[3];
    bool button_visible[3];
    bool dragging;
    win::ImageList drag_image;
    int hot_item;
    win::Tooltip tooltips;
    AnimeListDialog* parent;
  } listview;

  // Other controls
  win::Tab tab;

private:
  int current_id_;
  int current_status_;
};

extern AnimeListDialog DlgAnimeList;

}  // namespace ui

#endif  // TAIGA_UI_DLG_ANIME_LIST_H