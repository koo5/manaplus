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

#ifndef NET_EATHENA_TRADEHANDLER_H
#define NET_EATHENA_TRADEHANDLER_H

#include "net/net.h"
#include "net/tradehandler.h"

#include "net/ea/tradehandler.h"

#include "net/eathena/messagehandler.h"

namespace EAthena
{

class TradeHandler final : public MessageHandler, public Ea::TradeHandler
{
    public:
        TradeHandler();

        A_DELETE_COPY(TradeHandler)

        void handleMessage(Net::MessageIn &msg) override;

        void request(const Being *const being) const override;

        void respond(const bool accept) const override;

        void addItem(const Item *const item, int amount) const override;

        void setMoney(const int amount) const override;

        void confirm() const override;

        void finish() const override;

        void cancel() const override;
};

}  // namespace EAthena

#endif  // NET_EATHENA_TRADEHANDLER_H
