/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "playerinfo.h"

#include "client.h"
#include "depricatedevent.h"
#include "inventory.h"
#include "listener.h"
#include "logger.h"

#include "gui/inventorywindow.h"
#include "gui/npcdialog.h"
#include "gui/npcpostdialog.h"

#include "resources/iteminfo.h"

#include "net/playerhandler.h"

#include "debug.h"

namespace PlayerInfo
{

PlayerInfoBackend mData;
int mCharId = 0;

Inventory *mInventory = nullptr;
Equipment *mEquipment = nullptr;

#ifdef MANASERV_SUPPORT
std::map<int, Special> mSpecials;
signed char mSpecialRechargeUpdateNeeded = 0;
#endif

bool mTrading = false;
int mLevelProgress = 0;

// --- Triggers ---------------------------------------------------------------

void triggerAttr(const int id, const int old)
{
    DepricatedEvent event(EVENT_UPDATEATTRIBUTE);
    event.setInt("id", id);
    event.setInt("oldValue", old);
    event.setInt("newValue", mData.mAttributes.find(id)->second);
    DepricatedEvent::trigger(CHANNEL_ATTRIBUTES, event);
}

void triggerStat(const int id, const std::string &changed,
                 const int old1, const int old2)
{
    const StatMap::const_iterator it = mData.mStats.find(id);
    if (it == mData.mStats.end())
        return;

    DepricatedEvent event(EVENT_UPDATESTAT);
    event.setInt("id", id);
    const Stat &stat = it->second;
    event.setInt("base", stat.base);
    event.setInt("mod", stat.mod);
    event.setInt("exp", stat.exp);
    event.setInt("expNeeded", stat.expNeed);
    event.setString("changed", changed);
    event.setInt("oldValue1", old1);
    event.setInt("oldValue2", old2);
    DepricatedEvent::trigger(CHANNEL_ATTRIBUTES, event);
}

// --- Attributes -------------------------------------------------------------

int getAttribute(const int id)
{
    const IntMap::const_iterator it = mData.mAttributes.find(id);
    if (it != mData.mAttributes.end())
        return it->second;
    else
        return 0;
}

void setAttribute(const int id, const int value, const bool notify)
{
    const int old = mData.mAttributes[id];
    mData.mAttributes[id] = value;
    if (notify)
        triggerAttr(id, old);
}

int getSkillLevel(const int id)
{
    const IntMap::const_iterator it = mData.mSkills.find(id);
    if (it != mData.mSkills.end())
        return it->second;
    else
        return 0;
}

void setSkillLevel(const int id, const int value)
{
    mData.mSkills[id] = value;
}

// --- Stats ------------------------------------------------------------------

int getStatBase(const int id)
{
    const StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base;
    else
        return 0;
}

void setStatBase(const int id, const int value, const bool notify)
{
    const int old = mData.mStats[id].base;
    mData.mStats[id].base = value;
    if (notify)
        triggerStat(id, "base", old);
}

int getStatMod(const int id)
{
    const StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.mod;
    else
        return 0;
}

void setStatMod(const int id, const int value, const bool notify)
{
    const int old = mData.mStats[id].mod;
    mData.mStats[id].mod = value;
    if (notify)
        triggerStat(id, "mod", old);
}

int getStatEffective(const int id)
{
    const StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base + it->second.mod;
    else
        return 0;
}

const std::pair<int, int> getStatExperience(const int id)
{
    const StatMap::const_iterator it = mData.mStats.find(id);
    int a, b;
    if (it != mData.mStats.end())
    {
        a = it->second.exp;
        b = it->second.expNeed;
    }
    else
    {
        a = 0;
        b = 0;
    }
    return std::pair<int, int>(a, b);
}

void setStatExperience(const int id, const int have,
                       const int need, const bool notify)
{
    Stat &stat = mData.mStats[id];

    const int oldExp = stat.exp;
    const int oldExpNeed = stat.expNeed;
    stat.exp = have;
    stat.expNeed = need;
    if (notify)
        triggerStat(id, "exp", oldExp, oldExpNeed);
}

// --- Inventory / Equipment --------------------------------------------------

Inventory *getInventory()
{
    return mInventory;
}

void clearInventory()
{
    if (mEquipment)
        mEquipment->clear();
    if (mInventory)
        mInventory->clear();
}

void setInventoryItem(const int index, const int id,
                      const int amount, const int refine)
{
    bool equipment = false;
    const int itemType = ItemDB::get(id).getType();
    if (itemType != ITEM_UNUSABLE && itemType != ITEM_USABLE)
        equipment = true;
    if (mInventory)
        mInventory->setItem(index, id, amount, refine, equipment);
}

Equipment *getEquipment()
{
    return mEquipment;
}

Item *getEquipment(const unsigned int slot)
{
    if (mEquipment)
        return mEquipment->getEquipment(slot);
    else
        return nullptr;
}

void setEquipmentBackend(Equipment::Backend *const backend)
{
    if (mEquipment)
        mEquipment->setBackend(backend);
}

// --- Misc -------------------------------------------------------------------

void setBackend(const PlayerInfoBackend &backend)
{
    mData = backend;
}

void setCharId(const int charId)
{
    mCharId = charId;
}

int getCharId()
{
    return mCharId;
}

void logic()
{
#ifdef MANASERV_SUPPORT
    if ((mSpecialRechargeUpdateNeeded % 11) == 0)
    {
        mSpecialRechargeUpdateNeeded = 0;
        FOR_EACH (SpecialsMap::iterator, it, mSpecials)
        {
            Special &special = it->second;
            special.currentMana += special.recharge;
            if (special.currentMana > special.neededMana)
                special.currentMana = special.neededMana;
        }
    }
    mSpecialRechargeUpdateNeeded++;
#endif
}

bool isTrading()
{
    return mTrading;
}

void setTrading(const bool trading)
{
    mTrading = trading;
}

void updateAttrs()
{
    const Net::PlayerHandler *const handler = Net::getPlayerHandler();
    if (!handler)
        return;
    const int attr = handler->getAttackLocation();
    const int attackDelay = getStatBase(ATTACK_DELAY);
    if (attr != -1 && attackDelay)
    {
        setStatBase(ATTACK_SPEED, getStatBase(attr) * 1000
            / attackDelay, false);
        setStatMod(ATTACK_SPEED, getStatMod(attr) * 1000
            / attackDelay, true);
    }
    else
    {
        setStatBase(ATTACK_SPEED, 0, false);
        setStatMod(ATTACK_SPEED, 0, true);
    }
}

void init()
{
}

void deinit()
{
    clearInventory();
}

void clear()
{
    mData.mSkills.clear();
}

bool isTalking()
{
    return NpcDialog::isActive() || NpcPostDialog::isActive()
        || InventoryWindow::isStorageActive();
}

void gameDestroyed()
{
    delete mInventory;
    mInventory = nullptr;
    delete mEquipment;
    mEquipment = nullptr;
}

void stateChange(const int state)
{
    if (state == STATE_GAME)
    {
        if (!mInventory)
        {
            mInventory = new Inventory(Inventory::INVENTORY);
            mEquipment = new Equipment();
        }
    }
}

}  // namespace PlayerInfo
