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

#include "item.h"

#include "gui/theme.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"
#include "configuration.h"

#include "debug.h"

extern int serverVersion;

Item::Item(const int id, const int quantity, const int refine,
           const unsigned char color, const bool equipment,
           const bool equipped):
    mId(0),
    mColor(0),
    mQuantity(quantity),
    mImage(nullptr),
    mDrawImage(nullptr),
    mEquipment(equipment),
    mEquipped(equipped),
    mInEquipment(false),
    mRefine(refine),
    mInvIndex(0),
    mDescription(),
    mTags()
{
    setId(id, color);
}

Item::~Item()
{
    if (mImage)
    {
        mImage->decRef();
        mImage = nullptr;
    }
    if (mDrawImage)
    {
        mDrawImage->decRef();
        mDrawImage = nullptr;
    }
}

void Item::setId(const int id, const unsigned char color)
{
    mId = id;
    mColor = color;

    // Types 0 and 1 are not equippable items.
    mEquipment = id && getInfo().getType() >= 2;

    if (mImage)
        mImage->decRef();

    if (mDrawImage)
        mDrawImage->decRef();

    ResourceManager *const resman = ResourceManager::getInstance();
    const ItemInfo &info = getInfo();
    mTags = info.getTags();

    const std::string dye = combineDye2(paths.getStringValue(
        "itemIcons").append(info.getDisplay().image),
        info.getDyeColorsString(color));
    mImage = resman->getImage(dye);
    mDrawImage = resman->getImage(dye);

    if (!mImage)
    {
        mImage = Theme::getImageFromTheme(paths.getValue("unknownItemFile",
                                          "unknown-item.png"));
    }

    if (!mDrawImage)
    {
        mDrawImage = Theme::getImageFromTheme(
            paths.getValue("unknownItemFile", "unknown-item.png"));
    }
}

bool Item::isHaveTag(const int tagId)
{
    if (mTags.find(tagId) == mTags.end())
        return false;
    return mTags[tagId] > 0;
}

Image *Item::getImage(const int id, const unsigned char color)
{
    ResourceManager *const resman = ResourceManager::getInstance();
    const ItemInfo &info = ItemDB::get(id);
    Image *image = resman->getImage(combineDye2(paths.getStringValue(
        "itemIcons").append(info.getDisplay().image),
        info.getDyeColorsString(color)));

    if (!image)
        image = Theme::getImageFromTheme("unknown-item.png");
    return image;
}

std::string Item::getName()
{
    const ItemInfo &info = ItemDB::get(mId);
    if (serverVersion > 0)
        return info.getName(mColor);
    else
        return info.getName();
}
