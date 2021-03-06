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

#ifndef NET_EA_CHATHANDLER_H
#define NET_EA_CHATHANDLER_H

#include "net/chathandler.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"

#include <queue>

namespace Ea
{

class ChatHandler : public Net::ChatHandler
{
    public:
        ChatHandler();

        A_DELETE_COPY(ChatHandler)

        void me(const std::string &text,
                const std::string &channel) const override;

        virtual void processWhisperResponse(Net::MessageIn &msg);

        virtual void processWhisper(Net::MessageIn &msg) const;

        virtual void processBeingChat(Net::MessageIn &msg,
                                      const bool channels) const;

        virtual void processChat(Net::MessageIn &msg, bool normalChat,
                                 const bool channels) const;

        virtual void processMVP(Net::MessageIn &msg) const;

        virtual void processIgnoreAllResponse(Net::MessageIn &msg) const;

    protected:
        typedef std::queue<std::string> WhisperQueue;
        WhisperQueue mSentWhispers;
        bool mShowAllLang;
};

}  // namespace Ea

#endif  // NET_EA_CHATHANDLER_H
