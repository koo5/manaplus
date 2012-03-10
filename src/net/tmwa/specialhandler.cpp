/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#include "net/tmwa/specialhandler.h"

#include "logger.h"

#include "net/messagein.h"

#include "net/tmwa/protocol.h"

#include "debug.h"

extern Net::SpecialHandler *specialHandler;

namespace TmwAthena
{

SpecialHandler::SpecialHandler()
{
    static const Uint16 _messages[] =
    {
        SMSG_PLAYER_SKILLS,
        SMSG_SKILL_FAILED,
        SMSG_PLAYER_SKILL_UP,
        0
    };
    handledMessages = _messages;
    specialHandler = this;
}

void SpecialHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_PLAYER_SKILLS:
            processPlayerSkills(msg);
            break;

        case SMSG_PLAYER_SKILL_UP:
            processPlayerSkillUp(msg);
            break;

        case SMSG_SKILL_FAILED:
            processSkillFailed(msg);
            break;

        default:
            break;
    }
}

void SpecialHandler::useBeing(int id, int level, int beingId)
{
    MessageOut outMsg(CMSG_SKILL_USE_BEING);
    outMsg.writeInt16(static_cast<Sint16>(id));
    outMsg.writeInt16(static_cast<Sint16>(level));
    outMsg.writeInt32(beingId);
}

void SpecialHandler::usePos(int id, int level, int x, int y)
{
    MessageOut outMsg(CMSG_SKILL_USE_POSITION);
    outMsg.writeInt16(static_cast<Sint16>(level));
    outMsg.writeInt16(static_cast<Sint16>(id));
    outMsg.writeInt16(static_cast<Sint16>(x));
    outMsg.writeInt16(static_cast<Sint16>(y));
}

void SpecialHandler::useMap(int id, const std::string &map)
{
    MessageOut outMsg(CMSG_SKILL_USE_MAP);
    outMsg.writeInt16(static_cast<Sint16>(id));
    outMsg.writeString(map, 16);
}

} // namespace TmwAthena
