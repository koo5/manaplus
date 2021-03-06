/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef LISTBOX_H
#define LISTBOX_H

#include "gui/widgets/widget2.h"

#include <guichan/widgets/listbox.hpp>

#include "localconsts.h"

class SelectionListener;
class Skin;

/**
 * A list box, meant to be used inside a scroll area. Same as the Guichan list
 * box except this one doesn't have a background, instead completely relying
 * on the scroll area. It also adds selection listener functionality.
 *
 * \ingroup GUI
 */
class ListBox : public gcn::ListBox,
                public Widget2
{
    public:
        /**
         * Constructor.
         */
        ListBox(const Widget2 *const widget,
                gcn::ListModel *const listModel,
                const std::string &skin);

        A_DELETE_COPY(ListBox)

        ~ListBox();

        /**
         * Draws the list box.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Update the alpha value to the graphic components.
         */
        void updateAlpha();

        // Inherited from KeyListener

        void keyPressed(gcn::KeyEvent& keyEvent) override;

        // Inherited from MouseListener

        void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent) override;

        void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent) override;

        void mousePressed(gcn::MouseEvent &event) override;

        void mousePressed1(gcn::MouseEvent &event);

        void mouseDragged(gcn::MouseEvent &event) override;

        void refocus();

        void setDistributeMousePressed(bool b)
        { mDistributeMousePressed = b; }

        void adjustSize();

        void logic() override;

    protected:
        gcn::Color mHighlightColor;
        gcn::Color mForegroundColor2;
        gcn::Color mForegroundSelectedColor;
        gcn::Color mForegroundSelectedColor2;
        bool mDistributeMousePressed;
        int mOldSelected;
        int mPadding;
        static float mAlpha;
        Skin *mSkin;
};

#endif
