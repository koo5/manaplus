/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2013  The ManaPlus Developers
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

#ifndef GUI_WIDGETS_SLIDERLIST_H
#define GUI_WIDGETS_SLIDERLIST_H

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>
#include <guichan/mouselistener.hpp>

#include "gui/widgets/container.h"

#include "localconsts.h"

class Button;
class Label;

class SliderList final : public Container,
                         public gcn::ActionListener,
                         public gcn::MouseListener
{
    public:
        SliderList(const Widget2 *const widget,
                   gcn::ListModel *const listModel = nullptr,
                   gcn::ActionListener *const listener = nullptr,
                   std::string eventId = "");

        A_DELETE_COPY(SliderList)

        ~SliderList();

        void updateAlpha();

        void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent) override;

        void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent) override;

        void resize();

        void draw(gcn::Graphics *graphics) override;

        void action(const gcn::ActionEvent &event) override;

        void setSelectedString(std::string str);

        std::string getSelectedString() const A_WARN_UNUSED;

        void setSelected(int idx);

        void adjustSize();

        int getSelected() A_WARN_UNUSED
        { return mSelectedIndex; }

    protected:
        void updateLabel();

        int getMaxLabelWidth() A_WARN_UNUSED;

        Button *mButtons[2];
        Label *mLabel;
        gcn::ListModel *mListModel;
        std::string mPrevEventId;
        std::string mNextEventId;
        int mOldWidth;
        int mSelectedIndex;
};

#endif  // end GUI_WIDGETS_SLIDERLIST_H
