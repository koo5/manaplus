/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2012  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POPUP_LIST_H
#define POPUP_LIST_H

#include "gui/widgets/linkhandler.h"
#include "gui/widgets/popup.h"

#include <guichan/actionlistener.hpp>
#include "guichan/focuslistener.hpp"
#include <guichan/listmodel.hpp>
#include "guichan/selectionlistener.hpp"

#include "localconsts.h"

class DropDown;
class ListBox;
class ScrollArea;

class PopupList final : public Popup,
                        public gcn::FocusListener,
                        public gcn::SelectionListener
{
    public:
        PopupList(DropDown *const widget,
                  gcn::ListModel *const listModel);

        A_DELETE_COPY(PopupList)

        void show(int x, int y);

        void widgetResized(const gcn::Event &event) override;

        void setSelected(int selected);

        int getSelected() const;

        void setListModel(gcn::ListModel *model);

        gcn::ListModel *getListModel() const
        { return mListModel; }

        void adjustSize();

        void valueChanged(const gcn::SelectionEvent& event);

        void focusLost(const gcn::Event& event A_UNUSED);

    private:
        gcn::ListModel *mListModel;
        ListBox *mListBox;
        ScrollArea *mScrollArea;
        DropDown *mDropDown;
};

#endif