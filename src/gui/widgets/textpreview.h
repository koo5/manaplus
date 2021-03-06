/*
 *  The ManaPlus Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#ifndef TEXTPREVIEW_H
#define TEXTPREVIEW_H

#include "gui/widgets/widget2.h"

#include <guichan/color.hpp>
#include <guichan/font.hpp>
#include <guichan/widget.hpp>

#include "localconsts.h"

/**
 * Preview widget for particle colors, etc.
 */
class TextPreview final : public gcn::Widget,
                          public Widget2
{
    public:
        TextPreview(const Widget2 *const widget, const std::string &text);

        A_DELETE_COPY(TextPreview)

        ~TextPreview();

        inline void setTextColor(const gcn::Color *color)
        { mTextColor = color; adjustSize(); }

        inline void setTextColor2(const gcn::Color *color)
        { mTextColor2 = color; adjustSize(); }

        /**
         * Sets the text to use the set alpha value.
         *
         * @param alpha whether to use alpha values for the text or not
         */
        inline void useTextAlpha(bool alpha)
        { mTextAlpha = alpha; }

        /**
         * Sets the color the text background is drawn in. This is only the
         * rectangle directly behind the text, not to full widget.
         *
         * @param color the color to set
         */
        inline void setTextBGColor(const gcn::Color *color)
        { mTextBGColor = color; }

        /**
         * Sets the background color of the widget.
         *
         * @param color the color to set
         */
        inline void setBGColor(const gcn::Color *color)
        { mBGColor = color; }

        /**
         * Sets the font to render the text in.
         *
         * @param font the font to use.
         */
        inline void setFont(gcn::Font *font)
        { mFont = font; }

        /**
         * Sets whether to use a shadow while rendering.
         *
         * @param shadow true, if a shadow is wanted, false else
         */
        inline void setShadow(bool shadow)
        { mShadow = shadow; }

        /**
         * Sets whether to use an outline while rendering.
         *
         * @param outline true, if an outline is wanted, false else
         */
        inline void setOutline(bool outline)
        { mOutline = outline; }

        /**
         * Widget's draw method. Does the actual job.
         *
         * @param graphics graphics to draw into
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Set opacity for this widget (whether or not to show the background
         * color)
         *
         * @param opaque Whether the widget should be opaque or not
         */
        void setOpaque(bool opaque)
        { mOpaque = opaque; }

        /**
         * Gets opacity for this widget (whether or not the background color
         * is shown below the widget)
         */
        bool isOpaque() const A_WARN_UNUSED
        { return mOpaque; }

        void adjustSize();

    private:
        gcn::Font *mFont;
        std::string mText;
        const gcn::Color *mTextColor;
        const gcn::Color *mTextColor2;
        const gcn::Color *mBGColor;
        const gcn::Color *mTextBGColor;
        bool mTextAlpha;
        bool mOpaque;
        bool mShadow;
        bool mOutline;
        int mPadding;
        static int instances;
        static float mAlpha;
        static Skin *mSkin;
};

#endif
