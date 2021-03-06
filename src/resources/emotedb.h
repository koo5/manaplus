/*
 *  Emote database
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef EMOTE_DB_H
#define EMOTE_DB_H

#include "utils/stringvector.h"

#include <list>
#include <map>

#include "localconsts.h"

class AnimatedSprite;

struct EmoteSprite final
{
    EmoteSprite() :
        sprite(nullptr),
        name()
    { }

    A_DELETE_COPY(EmoteSprite)

    const AnimatedSprite *sprite;
    std::string name;
};

struct EmoteInfo final
{
    EmoteInfo() :
        sprites(),
        particles(),
        time(400)
    { }

    A_DELETE_COPY(EmoteInfo)

    std::list<EmoteSprite*> sprites;
    StringVect particles;
    int time;
};

typedef std::map<int, EmoteInfo*> EmoteInfos;
typedef std::map<int, int> EmoteToEmote;

/**
 * Emote information database.
 */
namespace EmoteDB
{
    void load();

    void unload();

    const EmoteInfo *get(const int id,
                         const bool allowNull = false) A_WARN_UNUSED;

    const EmoteInfo *get2(int id, const bool allowNull = false) A_WARN_UNUSED;

    const EmoteSprite *getSprite(const int id, const bool allowNull = false)
                                 A_WARN_UNUSED;

    const int &getLast() A_WARN_UNUSED;

    typedef EmoteInfos::iterator EmoteInfosIterator;
}  // namespace EmoteDB

#endif
