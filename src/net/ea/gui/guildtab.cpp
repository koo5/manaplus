/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "net/ea/gui/guildtab.h"

#include "chatlogger.h"
#include "configuration.h"
#include "guild.h"
#include "localplayer.h"
#include "soundmanager.h"

#include "net/net.h"

#include "net/ea/guildhandler.h"

#include "resources/iteminfo.h"
#include "resources/itemdb.h"

#include "utils/gettext.h"

#include "debug.h"

namespace Ea
{
extern Guild *taGuild;

GuildTab::GuildTab(const Widget2 *const widget) :
    // TRANSLATORS: guild chat tab name
    ChatTab(widget, _("Guild"), "")
{
    setTabColor(&getThemeColor(Theme::GUILD_CHAT_TAB),
        &getThemeColor(Theme::GUILD_CHAT_TAB_OUTLINE));
    setHighlightedTabColor(&getThemeColor(Theme::GUILD_CHAT_TAB_HIGHLIGHTED),
        &getThemeColor(Theme::GUILD_CHAT_TAB_HIGHLIGHTED_OUTLINE));
    setSelectedTabColor(&getThemeColor(Theme::GUILD_CHAT_TAB_SELECTED),
        &getThemeColor(Theme::GUILD_CHAT_TAB_SELECTED_OUTLINE));
    mShowOnline = config.getBoolValue("showGuildOnline");
    config.addListener("showGuildOnline", this);
}

GuildTab::~GuildTab()
{
    config.removeListeners(this);
}

bool GuildTab::handleCommand(const std::string &type, const std::string &args)
{
    if (type == "help")
    {
        if (args == "invite")
        {
            // TRANSLATORS: guild chat help
            chatLog(_("Command: /invite <nick>"));
            // TRANSLATORS: guild chat help
            chatLog(_("This command invites <nick> to the guild you're in."));
            // TRANSLATORS: guild chat help
            chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
        }
        else if (args == "leave")
        {
            // TRANSLATORS: guild chat help
            chatLog(_("Command: /leave"));
            // TRANSLATORS: guild chat help
            chatLog(_("This command causes the player to leave the guild."));
        }
        else
            return false;
    }
/*
    else if (type == "create" || type == "new")
    {
        if (args.empty())
            chatLog(_("Guild name is missing."), BY_SERVER);
        else
            Net::getGuildHandler()->create(args);
    }
*/
    else if (type == "invite" && taGuild)
    {
        Net::getGuildHandler()->invite(taGuild->getId(), args);
    }
    else if (type == "leave" && taGuild)
    {
        Net::getGuildHandler()->leave(taGuild->getId());
    }
    else if (type == "kick" && taGuild)
    {
        Net::getGuildHandler()->kick(taGuild->getMember(args), "");
    }
    else if (type == "notice" && taGuild)
    {
        std::string str1 = args.substr(0, 60);
        std::string str2("");
        if (args.size() > 60)
            str2 = args.substr(60);
        Net::getGuildHandler()->changeNotice(taGuild->getId(), str1, str2);
    }
    else
    {
        return false;
    }

    return true;
}

void GuildTab::handleInput(const std::string &msg)
{
    if (!taGuild)
        return;

    if (chatWindow)
    {
        Net::getGuildHandler()->chat(taGuild->getId(),
            chatWindow->doReplace(msg));
    }
    else
    {
        Net::getGuildHandler()->chat(taGuild->getId(), msg);
    }
}

void GuildTab::showHelp()
{
    // TRANSLATORS: guild chat help
    chatLog(_("/help > Display this help."));
    // TRANSLATORS: guild chat help
    chatLog(_("/invite > Invite a player to your guild"));
    // TRANSLATORS: guild chat help
    chatLog(_("/leave > Leave the guild you are in"));
    // TRANSLATORS: guild chat help
    chatLog(_("/kick > Kick some one from the guild you are in"));
}

void GuildTab::getAutoCompleteList(StringVect &names) const
{
    if (taGuild)
        taGuild->getNames(names);
    names.push_back("/notice ");
}

void GuildTab::saveToLogFile(std::string &msg)
{
    if (chatLogger)
        chatLogger->log("#Guild", msg);
}

void GuildTab::playNewMessageSound()
{
    soundManager.playGuiSound(SOUND_GUILD);
}

void GuildTab::optionChanged(const std::string &value)
{
    if (value == "showGuildOnline")
        mShowOnline = config.getBoolValue("showGuildOnline");
}

}  // namespace Ea
