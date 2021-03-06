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

#ifndef ACTION_H
#define ACTION_H

#include <map>

#include "localconsts.h"

class Animation;

/**
 * An action consists of several animations, one for each direction.
 */
class Action final
{
    public:
        Action();

        A_DELETE_COPY(Action)

        ~Action();

        void setAnimation(const int direction, Animation *const animation);

        Animation *getAnimation(int direction) const A_WARN_UNUSED;

        unsigned getNumber() const A_WARN_UNUSED
        { return mNumber; }

        void setNumber(const unsigned n)
        { mNumber = n; }

        void setLastFrameDelay(const int delay);

    protected:
        typedef std::map<int, Animation*> Animations;
        typedef Animations::iterator AnimationIter;

        Animations mAnimations;
        unsigned mNumber;
};

#endif
