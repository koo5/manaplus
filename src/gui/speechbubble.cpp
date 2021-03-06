/*
 *  Speech bubbles
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/speechbubble.h"

#include "gui/gui.h"
#include "gui/sdlfont.h"

#include "gui/widgets/label.h"
#include "gui/widgets/textbox.h"

#include <guichan/font.hpp>

#include "debug.h"

SpeechBubble::SpeechBubble() :
    Popup("Speech", "speechbubble.xml"),
    mText(),
    mCaption(new Label(this)),
    mSpeechBox(new TextBox(this))
{
    setContentSize(140, 46);
    setMinWidth(29);
    setMinHeight(29);

    mCaption->setFont(boldFont);
    mSpeechBox->setEditable(false);
    mSpeechBox->setOpaque(false);
    mSpeechBox->setForegroundColorAll(getThemeColor(Theme::BUBBLE_TEXT),
        getThemeColor(Theme::BUBBLE_TEXT_OUTLINE));

    add(mCaption);
    add(mSpeechBox);
}

void SpeechBubble::setCaption(const std::string &name,
                              const gcn::Color *const color1,
                              const gcn::Color *const color2)
{
    mCaption->setCaption(name);
    mCaption->adjustSize();
    mCaption->setForegroundColorAll(*color1, *color2);
}

void SpeechBubble::setText(const std::string &text, const bool showName)
{
    if (text == mText && (mCaption->getWidth() <= mSpeechBox->getMinWidth()))
        return;

    mSpeechBox->setForegroundColorAll(getThemeColor(Theme::BUBBLE_TEXT),
        getThemeColor(Theme::BUBBLE_TEXT_OUTLINE));

    int width = mCaption->getWidth() + 2 * getPadding();
    mSpeechBox->setTextWrapped(text, 130 > width ? 130 : width);
    const int speechWidth = mSpeechBox->getMinWidth() + 2 * getPadding();

    const int fontHeight = getFont()->getHeight();
    const int nameHeight = showName ? mCaption->getHeight() +
                           (getPadding() / 2) : 0;
    const int numRows = mSpeechBox->getNumberOfRows();
    const int height = (numRows * fontHeight) + nameHeight + getPadding();

    if (width < speechWidth)
        width = speechWidth;

    width += 2 * getPadding();

    setContentSize(width, height);

    const int xPos = ((getWidth() - width) / 2);
    const int yPos = ((getHeight() - height) / 2) + nameHeight;

    mCaption->setPosition(xPos, getPadding());
    mSpeechBox->setPosition(xPos, yPos);
}
