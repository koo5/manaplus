/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2013  The ManaPlus Developers
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

#include "gui/widgets/popuplist.h"

#include "gui/gui.h"

#include "gui/widgets/dropdown.h"
#include "gui/widgets/extendedlistbox.h"
#include "gui/widgets/scrollarea.h"

#include "utils/gettext.h"

#include "debug.h"

PopupList::PopupList(DropDown *const widget,
                     gcn::ListModel *const listModel,
                     bool extended, bool modal):
    Popup("PopupList", "popuplist.xml"),
    gcn::FocusListener(),
    mListModel(listModel),
    mListBox(extended ? new ExtendedListBox(
        widget, listModel, "extendedlistbox.xml", 0) :
        new ListBox(widget, listModel, "popuplistbox.xml")),
    mScrollArea(new ScrollArea(mListBox, false)),
    mDropDown(widget),
    mModal(modal)
{
    setFocusable(true);

    mListBox->setDistributeMousePressed(true);
    mScrollArea->setPosition(mPadding, mPadding);
    add(mScrollArea);

//    if (getParent())
//        getParent()->addFocusListener(this);
    if (gui)
        gui->addGlobalFocusListener(this);

    addKeyListener(mDropDown);
    addMouseListener(this);
    adjustSize();
}

PopupList::~PopupList()
{
    if (getParent())
        getParent()->removeFocusListener(this);
    if (gui)
        gui->removeGlobalFocusListener(this);
    removeKeyListener(mDropDown);
}

void PopupList::show(int x, int y)
{
    int len = mListBox->getHeight() + 8;
    if (len > 250)
        len = 250;
    setContentSize(mListBox->getWidth() + 8, len);
    if (mainGraphics->mWidth < (x + getWidth() + 5))
        x = mainGraphics->mWidth - getWidth();
    if (mainGraphics->mHeight < (y + getHeight() + 5))
        y = mainGraphics->mHeight - getHeight();
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
    if (mModal)
        requestModalFocus();
}

void PopupList::widgetResized(const gcn::Event &event)
{
    Popup::widgetResized(event);
    adjustSize();
}

void PopupList::setSelected(int selected)
{
    if (!mListBox)
        return;

    mListBox->setSelected(selected);
}

int PopupList::getSelected() const
{
    if (!mListBox)
        return -1;

    return mListBox->getSelected();
}

void PopupList::setListModel(gcn::ListModel *model)
{
    if (mListBox)
        mListBox->setListModel(model);
    mListModel = model;
}

void PopupList::adjustSize()
{
    const int pad2 = 2 * mPadding;
    mScrollArea->setWidth(getWidth() - pad2);
    mScrollArea->setHeight(getHeight() - pad2);
    mListBox->adjustSize();
    mListBox->setWidth(getWidth() - pad2);
}

void PopupList::mousePressed(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.getSource() == mScrollArea)
        return;
    if (mDropDown)
        mDropDown->updateSelection();
    setVisible(false);
    if (mModal)
        releaseModalFocus();
}

void PopupList::focusGained(const gcn::Event& event A_UNUSED)
{
    const gcn::Widget *const source = event.getSource();
    if (!mVisible || source == this || source == mListBox
        || source == mScrollArea || source == mDropDown)
    {
        return;
    }

    if (mDropDown)
        mDropDown->updateSelection();
    setVisible(false);
    if (mModal)
        releaseModalFocus();
}

void PopupList::focusLost(const gcn::Event& event A_UNUSED)
{
    if (mDropDown)
        mDropDown->updateSelection();
}
