/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011  The ManaPlus Developers
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

#ifndef NET_EA_INVENTORYHANDLER_H
#define NET_EA_INVENTORYHANDLER_H

#include "equipment.h"
#include "inventory.h"
#include "logger.h"
#include "playerinfo.h"

#include "gui/inventorywindow.h"

#include "net/inventoryhandler.h"
#include "net/messagein.h"
#include "net/net.h"

#include <vector>
#include <queue>

#ifdef __GNUC__
#define A_UNUSED  __attribute__ ((unused))
#else
#define A_UNUSED
#endif

namespace Ea
{

class EquipBackend : public Equipment::Backend
{
    public:
        EquipBackend()
        {
            memset(mEquipment, -1, sizeof(mEquipment));
        }

        Item *getEquipment(int index) const
        {
            int invyIndex = mEquipment[index];
            if (invyIndex == -1)
                return NULL;

            return PlayerInfo::getInventory()->getItem(invyIndex);
        }

        void clear()
        {
            for (int i = 0; i < EQUIPMENT_SIZE; i++)
            {
                if (mEquipment[i] != -1)
                {
                    Item* item = PlayerInfo::getInventory()->getItem(i);
                    if (item)
                        item->setEquipped(false);
                }

                mEquipment[i] = -1;
            }
        }

        void setEquipment(int index, int inventoryIndex)
        {
            // Unequip existing item
            Item* item = PlayerInfo::getInventory()
                ->getItem(mEquipment[index]);

            if (item)
                item->setEquipped(false);

            mEquipment[index] = inventoryIndex;

            item = PlayerInfo::getInventory()->getItem(inventoryIndex);
            if (item)
                item->setEquipped(true);

            if (inventoryWindow)
                inventoryWindow->updateButtons();
        }

    private:
        int mEquipment[EQUIPMENT_SIZE];
};

/**
 * Used to cache storage data until we get size data for it.
 */
class InventoryItem
{
    public:
        int slot;
        int id;
        int quantity;
        unsigned char color;
        int refine;
        bool equip;

        InventoryItem(int slot, int id, int quantity, int refine,
                      unsigned char color, bool equip)
        {
            this->slot = slot;
            this->id = id;
            this->quantity = quantity;
            this->refine = refine;
            this->color = color;
            this->equip = equip;
        }
};

typedef std::vector<InventoryItem> InventoryItems;

class InventoryHandler : public Net::InventoryHandler
{
    public:
        enum
        {
            GUILD_STORAGE = Inventory::TYPE_END,
            CART
        };

        InventoryHandler();

        ~InventoryHandler();

        bool canSplit(const Item *item) const;

        void splitItem(const Item *item, int amount);

        void moveItem(int oldIndex, int newIndex);

        void openStorage(int type);

        size_t getSize(int type) const;

        int convertFromServerSlot(int serverSlot) const;

        void pushPickup(int floorId)
        { mSentPickups.push(floorId); }

        int getSlot(int eAthenaSlot);

        void processPlayerInventory(Net::MessageIn &msg, bool playerInvintory);

        void processPlayerStorageEquip(Net::MessageIn &msg);

        void processPlayerInventoryAdd(Net::MessageIn &msg);

        void processPlayerInventoryRemove(Net::MessageIn &msg);

        void processPlayerInventoryUse(Net::MessageIn &msg);

        void processItemUseResponse(Net::MessageIn &msg);

        void processPlayerStorageStatus(Net::MessageIn &msg);

        void processPlayerStorageAdd(Net::MessageIn &msg);

        void processPlayerStorageRemove(Net::MessageIn &msg);

        void processPlayerStorageClose(Net::MessageIn &msg);

        void processPlayerEquipment(Net::MessageIn &msg);

        void processPlayerEquip(Net::MessageIn &msg);

        void processPlayerUnEquip(Net::MessageIn &msg);

        void processPlayerAttackRange(Net::MessageIn &msg);

        void processPlayerArrowEquip(Net::MessageIn &msg);

    protected:
        EquipBackend mEquips;
        InventoryItems mInventoryItems;
        Inventory *mStorage;
        InventoryWindow *mStorageWindow;
        bool mDebugInventory;

        typedef std::queue<int> PickupQueue;
        PickupQueue mSentPickups;
};

} // namespace Ea

#endif // NET_EA_INVENTORYHANDLER_H