/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef NET_TA_MAPHANDLER_H
#define NET_TA_MAPHANDLER_H

#include "net/gamehandler.h"
#include "net/net.h"

#include "net/tmwa/messagehandler.h"

#include "net/ea/gamehandler.h"

namespace TmwAthena
{

class GameHandler final : public MessageHandler, public Ea::GameHandler
{
    public:
        GameHandler();

        A_DELETE_COPY(GameHandler)

        void handleMessage(Net::MessageIn &msg) override;

        void connect() override;

        bool isConnected() const override A_WARN_UNUSED;

        void disconnect() override;

        void quit() const override;

        void ping(const int tick) const override;

        void disconnect2() const override;

        void mapLoadedEvent() const override;

        bool mustPing() const override A_WARN_UNUSED
        { return false; }
};

}  // namespace TmwAthena

#endif  // NET_TA_MAPHANDLER_H
