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

#ifndef BEINGINFO_H
#define BEINGINFO_H

#include "actorsprite.h"

#include "resources/cursor.h"
#include "resources/soundinfo.h"

#include <list>
#include <map>

struct Attack final
{
    std::string mAction;
    int mEffectId;
    int mHitEffectId;
    int mCriticalHitEffectId;
    int mMissEffectId;
    std::string mMissileParticle;

    Attack(const std::string &action, const int effectId,
           const int hitEffectId, const int criticalHitEffectId,
           const int missEffectId, const std::string &missileParticle) :
        mAction(action),
        mEffectId(effectId),
        mHitEffectId(hitEffectId),
        mCriticalHitEffectId(criticalHitEffectId),
        mMissEffectId(missEffectId),
        mMissileParticle(missileParticle)
    {
    }

    A_DELETE_COPY(Attack)
};

typedef std::map<int, Attack*> Attacks;

enum SoundEvent
{
    SOUND_EVENT_HIT = 0,
    SOUND_EVENT_MISS,
    SOUND_EVENT_HURT,
    SOUND_EVENT_DIE,
    SOUND_EVENT_MOVE,
    SOUND_EVENT_SIT,
    SOUND_EVENT_SITTOP,
    SOUND_EVENT_SPAWN
};

typedef std::map<SoundEvent, SoundInfoVect*> SoundEvents;

/**
 * Holds information about a certain type of monster. This includes the name
 * of the monster, the sprite to display and the sounds the monster makes.
 *
 * @see MonsterDB
 * @see NPCDB
 */
class BeingInfo final
{
    public:
        static BeingInfo *unknown;
        static Attack *empty;

        BeingInfo();

        A_DELETE_COPY(BeingInfo)

        ~BeingInfo();

        void setName(const std::string &name)
        { mName = name; }

        const std::string &getName() const A_WARN_UNUSED
        { return mName; }

        void setDisplay(SpriteDisplay display);

        const SpriteDisplay &getDisplay() const A_WARN_UNUSED
        { return mDisplay; }

        void setTargetCursorSize(const std::string &size);

        void setTargetCursorSize(const ActorSprite::TargetCursorSize
                                 &targetSize)
        { mTargetCursorSize = targetSize; }

        void setHoverCursor(const std::string &name)
        { return setHoverCursor(Cursor::stringToCursor(name)); }

        void setHoverCursor(const Cursor::Cursor &cursor)
        { mHoverCursor = cursor; }

        Cursor::Cursor getHoverCursor() const A_WARN_UNUSED
        { return mHoverCursor; }

        ActorSprite::TargetCursorSize getTargetCursorSize() const A_WARN_UNUSED
        { return mTargetCursorSize; }

        void addSound(const SoundEvent event, const std::string &filename,
                      const int delay);

        const SoundInfo &getSound(const SoundEvent event)
                                  const A_WARN_UNUSED;

        void addAttack(const int id, std::string action, const int effectId,
                       const int hitEffectId, const int criticalHitEffectId,
                       const int missEffectId,
                       const std::string &missileParticle);

        const Attack *getAttack(const int id) const A_WARN_UNUSED;

        void setWalkMask(const unsigned char mask)
        { mWalkMask = mask; }

        /**
         * Gets the way the being is blocked by other objects
         */
        unsigned char getWalkMask() const A_WARN_UNUSED
        { return mWalkMask; }

        void setBlockType(const Map::BlockType &blockType)
        { mBlockType = blockType; }

        Map::BlockType getBlockType() const A_WARN_UNUSED
        { return mBlockType; }

        void setTargetOffsetX(const int n)
        { mTargetOffsetX = n; }

        int getTargetOffsetX() const A_WARN_UNUSED
        { return mTargetOffsetX; }

        void setTargetOffsetY(const int n)
        { mTargetOffsetY = n; }

        int getTargetOffsetY() const A_WARN_UNUSED
        { return mTargetOffsetY; }

        void setMaxHP(const int n)
        { mMaxHP = n; }

        int getMaxHP() const A_WARN_UNUSED
        { return mMaxHP; }

        bool isStaticMaxHP() const A_WARN_UNUSED
        { return mStaticMaxHP; }

        void setStaticMaxHP(const bool n)
        { mStaticMaxHP = n; }

        void setTargetSelection(const bool n)
        { mTargetSelection = n; }

        bool isTargetSelection() const A_WARN_UNUSED
        { return mTargetSelection; }

        int getSortOffsetY() const A_WARN_UNUSED
        { return mSortOffsetY; }

        void setSortOffsetY(const int n)
        { mSortOffsetY = n; }

        int getDeadSortOffsetY() const A_WARN_UNUSED
        { return mDeadSortOffsetY; }

        void setDeadSortOffsetY(const int n)
        { mDeadSortOffsetY = n; }

        uint16_t getAvatarId() const A_WARN_UNUSED
        { return mAvatarId; }

        void setAvatarId(const uint16_t id)
        { mAvatarId = id; }

        static void init();

        static void clear();

    private:
        SpriteDisplay mDisplay;
        std::string mName;
        ActorSprite::TargetCursorSize mTargetCursorSize;
        Cursor::Cursor mHoverCursor;
        SoundEvents mSounds;
        Attacks mAttacks;
        unsigned char mWalkMask;
        Map::BlockType mBlockType;
        int mTargetOffsetX;
        int mTargetOffsetY;
        int mMaxHP;
        bool mStaticMaxHP;
        bool mTargetSelection;
        int mSortOffsetY;
        int mDeadSortOffsetY;
        uint16_t mAvatarId;
};

typedef std::map<int, BeingInfo*> BeingInfos;
typedef BeingInfos::iterator BeingInfoIterator;

#endif  // BEINGINFO_H
