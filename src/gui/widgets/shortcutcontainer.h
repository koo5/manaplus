/*
 *  The ManaPlus Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#ifndef SHORTCUTCONTAINER_H
#define SHORTCUTCONTAINER_H

#include "gui/widgets/tab.h"

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

class Image;
class ImageCollection;

/**
 * A generic shortcut container.
 *
 * \ingroup GUI
 */
class ShortcutContainer : public gcn::Widget,
                          public Widget2,
                          public gcn::WidgetListener,
                          public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the shortcut container.
         */
        ShortcutContainer();

        A_DELETE_COPY(ShortcutContainer)

        /**
         * Destructor.
         */
        ~ShortcutContainer();

        /**
         * Draws the shortcuts
         */
        virtual void draw(gcn::Graphics *graphics) override = 0;

        /**
         * Invoked when a widget changes its size. This is used to determine
         * the new height of the container.
         */
        virtual void widgetResized(const gcn::Event &event) override;

        virtual void widgetMoved(const gcn::Event& event) override;

        /**
         * Handles mouse when dragged.
         */
        virtual void mouseDragged(gcn::MouseEvent &event) override = 0;

        /**
         * Handles mouse when pressed.
         */
        virtual void mousePressed(gcn::MouseEvent &event) override = 0;

        /**
         * Handles mouse release.
         */
        virtual void mouseReleased(gcn::MouseEvent &event) override = 0;

        int getMaxItems() const A_WARN_UNUSED
        { return mMaxItems; }

        int getBoxWidth() const A_WARN_UNUSED
        { return mBoxWidth; }

        int getBoxHeight() const A_WARN_UNUSED
        { return mBoxHeight; }

        void drawBackground(Graphics *g);

        void setRedraw(bool b)
        { mRedraw = b; }

    protected:
        /**
         * Gets the index from the grid provided the point is in an item box.
         *
         * @param pointX X coordinate of the point.
         * @param pointY Y coordinate of the point.
         * @return index on success, -1 on failure.
         */
        int getIndexFromGrid(const int pointX,
                             const int pointY) const A_WARN_UNUSED;

        Image *mBackgroundImg;
        static float mAlpha;

        unsigned mMaxItems;
        int mBoxWidth;
        int mBoxHeight;
        int mCursorPosX, mCursorPosY;
        int mGridWidth, mGridHeight;
        ImageCollection *mVertexes;
        bool mRedraw;
};

#endif
