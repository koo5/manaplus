/*
 *  The ManaPlus Client
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

#include "gui/widgets/icon.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "debug.h"

Icon::Icon(const Widget2 *const widget, const std::string &file) :
    gcn::Widget(),
    Widget2(widget),
    mImage(ResourceManager::getInstance()->getImage(file))
{
    if (mImage)
        setSize(mImage->mBounds.w, mImage->mBounds.h);
}

Icon::Icon(const Widget2 *const widget, Image *const image) :
    gcn::Widget(),
    Widget2(widget),
    mImage(image)
{
    if (mImage)
        setSize(mImage->mBounds.w, mImage->mBounds.h);
}

Icon::~Icon()
{
    if (gui)
        gui->removeDragged(this);
}

void Icon::setImage(Image *const image)
{
    mImage = image;
    if (mImage)
        setSize(mImage->mBounds.w, mImage->mBounds.h);
}

void Icon::draw(gcn::Graphics *g)
{
    BLOCK_START("Icon::draw")
    if (mImage)
    {
        Graphics *const graphics = static_cast<Graphics*>(g);
        graphics->drawImage(mImage, (getWidth() - mImage->mBounds.w) / 2,
            (getHeight() - mImage->mBounds.h) / 2);
    }
    BLOCK_END("Icon::draw")
}
