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

#include "gui/socialwindow.h"

#include "actorspritemanager.h"
#include "guild.h"
#include "guildmanager.h"
#include "inputmanager.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "maplayer.h"
#include "party.h"
#include "playerrelations.h"
#include "gui/whoisonline.h"

#include "gui/confirmdialog.h"
#include "gui/okdialog.h"
#include "gui/outfitwindow.h"
#include "gui/setup.h"
#include "gui/textdialog.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/container.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollarea.h"

#include "net/net.h"
#include "net/guildhandler.h"
#include "net/partyhandler.h"

#include "utils/gettext.h"

#include "debug.h"

static class SortFriendsFunctor final
{
    public:
        bool operator() (const Avatar *const m1, const Avatar *const m2) const
        {
            if (!m1 || !m2)
                return false;

            if (m1->getOnline() != m2->getOnline())
                return m1->getOnline() > m2->getOnline();

            if (m1->getName() != m2->getName())
            {
                std::string s1 = m1->getName();
                std::string s2 = m2->getName();
                toLower(s1);
                toLower(s2);
                return s1 < s2;
            }
            return false;
        }
} friendSorter;


class SocialTab : public Tab
{
public:
    A_DELETE_COPY(SocialTab)

    virtual void invite() = 0;

    virtual void leave() = 0;

    virtual void updateList() = 0;

    virtual void updateAvatar(std::string name) = 0;

    virtual void resetDamage(std::string name) = 0;

    virtual void selectIndex(const unsigned num A_UNUSED)
    { }

protected:
    friend class SocialWindow;

    explicit SocialTab(const Widget2 *const widget):
        Tab(widget),
        mInviteDialog(nullptr),
        mConfirmDialog(nullptr),
        mScroll(nullptr),
        mList(nullptr)
    {
    }

    virtual ~SocialTab()
    {
        // Cleanup dialogs
        if (mInviteDialog)
        {
            mInviteDialog->close();
            mInviteDialog->scheduleDelete();
            mInviteDialog = nullptr;
        }

        if (mConfirmDialog)
        {
            mConfirmDialog->close();
            mConfirmDialog->scheduleDelete();
            mConfirmDialog = nullptr;
        }
    }

    TextDialog *mInviteDialog;
    ConfirmDialog *mConfirmDialog;
    ScrollArea *mScroll;
    AvatarListBox *mList;
};

class SocialGuildTab final : public SocialTab, public gcn::ActionListener
{
public:
    SocialGuildTab(const Widget2 *const widget,
                   Guild *const guild, const bool showBackground) :
        SocialTab(widget),
        gcn::ActionListener(),
        mGuild(guild)
    {
        // TRANSLATORS: tab in social window
        setCaption(_("Guild"));

        setTabColor(&getThemeColor(Theme::GUILD_SOCIAL_TAB),
            &getThemeColor(Theme::GUILD_SOCIAL_TAB_OUTLINE));
        setHighlightedTabColor(&getThemeColor(
            Theme::GUILD_SOCIAL_TAB_HIGHLIGHTED), &getThemeColor(
            Theme::GUILD_SOCIAL_TAB_HIGHLIGHTED_OUTLINE));
        setSelectedTabColor(&getThemeColor(Theme::GUILD_SOCIAL_TAB_SELECTED),
            &getThemeColor(Theme::GUILD_SOCIAL_TAB_SELECTED_OUTLINE));

        mList = new AvatarListBox(this, guild);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
    }

    A_DELETE_COPY(SocialGuildTab)

    ~SocialGuildTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
    }

    void action(const gcn::ActionEvent &event)
    {
        const std::string &eventId = event.getId();
        if (eventId == "do invite")
        {
            std::string name = mInviteDialog->getText();
            Net::getGuildHandler()->invite(mGuild->getId(), name);

            if (localChatTab)
            {
                localChatTab->chatLog(strprintf(
                    // TRANSLATORS: chat message
                    _("Invited user %s to guild %s."),
                    name.c_str(), mGuild->getName().c_str()), BY_SERVER);
            }
            mInviteDialog = nullptr;
        }
        else if (eventId == "~do invite")
        {
            mInviteDialog = nullptr;
        }
        else if (eventId == "yes")
        {
            Net::getGuildHandler()->leave(mGuild->getId());
            if (localChatTab)
            {
                // TRANSLATORS: chat message
                localChatTab->chatLog(strprintf(_("Guild %s quit requested."),
                    mGuild->getName().c_str()), BY_SERVER);
            }
            mConfirmDialog = nullptr;
        }
        else if (eventId == "~yes")
        {
            mConfirmDialog = nullptr;
        }
    }

    void updateList()
    {
    }

    void updateAvatar(std::string name A_UNUSED)
    {
    }

    void resetDamage(std::string name A_UNUSED)
    {
    }

    void invite()
    {
        // TRANSLATORS: guild invite message
        mInviteDialog = new TextDialog(_("Member Invite to Guild"),
            // TRANSLATORS: guild invite message
            strprintf(_("Who would you like to invite to guild %s?"),
            mGuild->getName().c_str()), socialWindow);
        mInviteDialog->setActionEventId("do invite");
        mInviteDialog->addActionListener(this);
    }

    void leave()
    {
        // TRANSLATORS: guild leave message
        mConfirmDialog = new ConfirmDialog(_("Leave Guild?"),
            // TRANSLATORS: guild leave message
            strprintf(_("Are you sure you want to leave guild %s?"),
            mGuild->getName().c_str()), socialWindow);

        mConfirmDialog->addActionListener(this);
    }

private:
    Guild *mGuild;
};

class SocialGuildTab2 final : public SocialTab, public gcn::ActionListener
{
public:
    SocialGuildTab2(const Widget2 *const widget,
                    Guild *const guild, const bool showBackground) :
        SocialTab(widget),
        gcn::ActionListener(),
        mGuild(guild)
    {
        // TRANSLATORS: tab in social window
        setCaption(_("Guild"));

        setTabColor(&getThemeColor(Theme::GUILD_SOCIAL_TAB),
            &getThemeColor(Theme::GUILD_SOCIAL_TAB_OUTLINE));
        setHighlightedTabColor(&getThemeColor(
            Theme::GUILD_SOCIAL_TAB_HIGHLIGHTED), &getThemeColor(
            Theme::GUILD_SOCIAL_TAB_HIGHLIGHTED_OUTLINE));
        setSelectedTabColor(&getThemeColor(Theme::GUILD_SOCIAL_TAB_SELECTED),
            &getThemeColor(Theme::GUILD_SOCIAL_TAB_SELECTED_OUTLINE));

        mList = new AvatarListBox(this, guild);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
    }

    A_DELETE_COPY(SocialGuildTab2)

    ~SocialGuildTab2()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
    }

    void action(const gcn::ActionEvent &event A_UNUSED)
    {
    }

    void updateList()
    {
    }

    void updateAvatar(std::string name A_UNUSED)
    {
    }

    void resetDamage(std::string name A_UNUSED)
    {
    }

    void invite()
    {
    }

    void leave()
    {
    }

private:
    Guild *mGuild;
};

class SocialPartyTab final : public SocialTab, public gcn::ActionListener
{
public:
    SocialPartyTab(const Widget2 *const widget,
                   Party *const party, const bool showBackground) :
        SocialTab(widget),
        gcn::ActionListener(),
        mParty(party)
    {
        // TRANSLATORS: tab in social window
        setCaption(_("Party"));

        setTabColor(&getThemeColor(Theme::PARTY_SOCIAL_TAB),
            &getThemeColor(Theme::PARTY_SOCIAL_TAB_OUTLINE));
        setHighlightedTabColor(&getThemeColor(
            Theme::PARTY_SOCIAL_TAB_HIGHLIGHTED), &getThemeColor(
            Theme::PARTY_SOCIAL_TAB_HIGHLIGHTED_OUTLINE));
        setSelectedTabColor(&getThemeColor(Theme::PARTY_SOCIAL_TAB_SELECTED),
            &getThemeColor(Theme::PARTY_SOCIAL_TAB_SELECTED_OUTLINE));

        mList = new AvatarListBox(this, party);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
    }

    A_DELETE_COPY(SocialPartyTab)

    ~SocialPartyTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
    }

    void action(const gcn::ActionEvent &event)
    {
        const std::string &eventId = event.getId();
        if (eventId == "do invite")
        {
            std::string name = mInviteDialog->getText();
            Net::getPartyHandler()->invite(name);

            if (localChatTab)
            {
                // TRANSLATORS: chat message
                localChatTab->chatLog(strprintf(_("Invited user %s to party."),
                    name.c_str()), BY_SERVER);
            }
            mInviteDialog = nullptr;
        }
        else if (eventId == "~do invite")
        {
            mInviteDialog = nullptr;
        }
        else if (eventId == "yes")
        {
            Net::getPartyHandler()->leave();
            if (localChatTab)
            {
                // TRANSLATORS: tab in social window
                localChatTab->chatLog(strprintf(_("Party %s quit requested."),
                    mParty->getName().c_str()), BY_SERVER);
            }
            mConfirmDialog = nullptr;
        }
        else if (eventId == "~yes")
        {
            mConfirmDialog = nullptr;
        }
    }

    void updateList()
    {
    }

    void updateAvatar(std::string name A_UNUSED)
    {
    }

    void resetDamage(std::string name A_UNUSED)
    {
    }

    void invite()
    {
        // TRANSLATORS: party invite message
        mInviteDialog = new TextDialog(_("Member Invite to Party"),
            // TRANSLATORS: party invite message
            strprintf(_("Who would you like to invite to party %s?"),
            mParty->getName().c_str()), socialWindow);
        mInviteDialog->setActionEventId("do invite");
        mInviteDialog->addActionListener(this);
    }

    void leave()
    {
        // TRANSLATORS: party leave message
        mConfirmDialog = new ConfirmDialog(_("Leave Party?"),
            // TRANSLATORS: party leave message
            strprintf(_("Are you sure you want to leave party %s?"),
            mParty->getName().c_str()), socialWindow);

        mConfirmDialog->addActionListener(this);
    }

private:
    Party *mParty;
};

class BeingsListModal final : public AvatarListModel
{
public:
    BeingsListModal() :
        AvatarListModel(),
        mMembers()
    {
    }

    A_DELETE_COPY(BeingsListModal)

    ~BeingsListModal()
    {
        delete_all(mMembers);
        mMembers.clear();
    }

    std::vector<Avatar*> *getMembers()
    {
        return &mMembers;
    }

    virtual Avatar *getAvatarAt(int index)
    {
        return mMembers[index];
    }

    int getNumberOfElements()
    {
        return static_cast<int>(mMembers.size());
    }

    std::vector<Avatar*> mMembers;
};

class SocialPlayersTab final : public SocialTab
{
public:
    SocialPlayersTab(const Widget2 *const widget,
                     std::string name, const bool showBackground) :
        SocialTab(widget),
        mBeings(new BeingsListModal)
    {
        mList = new AvatarListBox(this, mBeings);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

        updateList();
        setCaption(name);
    }

    A_DELETE_COPY(SocialPlayersTab)

    ~SocialPlayersTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
        delete mBeings;
        mBeings = nullptr;
    }

    void updateList()
    {
        getPlayersAvatars();
    }

    void updateAvatar(std::string name)
    {
        if (!actorSpriteManager)
            return;

        Avatar *const avatar = findAvatarbyName(name);
        if (!avatar)
            return;
        if (Party::getParty(1))
        {
            const PartyMember *const pm = Party::getParty(1)->getMember(name);
            if (pm && pm->getMaxHp() > 0)
            {
                avatar->setMaxHp(pm->getMaxHp());
                avatar->setHp(pm->getHp());
            }
        }
        const Being *const being = actorSpriteManager->findBeingByName(
            name, Being::PLAYER);
        if (being)
        {
            avatar->setDamageHp(being->getDamageTaken());
            avatar->setLevel(being->getLevel());
            avatar->setGender(being->getGender());
            avatar->setIp(being->getIp());
        }
    }

    void resetDamage(std::string name)
    {
        if (!actorSpriteManager)
            return;

        Avatar *const avatar = findAvatarbyName(name);
        if (!avatar)
            return;
        avatar->setDamageHp(0);
        Being *const being = actorSpriteManager->findBeingByName(
            name, Being::PLAYER);

        if (being)
            being->setDamageTaken(0);
    }

    Avatar* findAvatarbyName(std::string name)
    {
        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        if (!avatars)
            return nullptr;

        Avatar *ava = nullptr;
        std::vector<Avatar*>::const_iterator i = avatars->begin();
        const std::vector<Avatar*>::const_iterator i_end = avatars->end();
        while (i != i_end)
        {
            ava = (*i);
            if (ava && ava->getName() == name)
                return ava;
            ++i;
        }
        ava = new Avatar(name);
        ava->setOnline(true);
        avatars->push_back(ava);
        return ava;
    }

    void getPlayersAvatars()
    {
        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        if (!avatars)
            return;

        if (actorSpriteManager)
        {
            StringVect names;
            actorSpriteManager->getPlayerNames(names, false);

            std::vector<Avatar*>::iterator ai = avatars->begin();
            while (ai != avatars->end())
            {
                bool finded = false;
                const Avatar *const ava = (*ai);
                if (!ava)
                    break;

                StringVectCIter i = names.begin();
                const StringVectCIter i_end = names.end();
                while (i != i_end)
                {
                    if (ava->getName() == (*i) && (*i) != "")
                    {
                        finded = true;
                        break;
                    }
                    ++i;
                }

                if (!finded)
                {
                    delete *ai;
                    ai = avatars->erase(ai);
                }
                else
                {
                    ++ai;
                }
            }

            StringVectCIter i = names.begin();
            const StringVectCIter i_end = names.end();

            while (i != i_end)
            {
                if ((*i) != "")
                    updateAvatar(*i);
                ++i;
            }
        }
    }

    void invite()
    {
    }

    void leave()
    {
    }

private:
    BeingsListModal *mBeings;
};


class SocialNavigationTab final : public SocialTab
{
public:
    SocialNavigationTab(const Widget2 *const widget,
                        const bool showBackground) :
        SocialTab(widget),
        mBeings(new BeingsListModal)
    {
        mList = new AvatarListBox(this, mBeings);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

        // TRANSLATORS: Navigation tab name in social window. Should be small
        setCaption(_("Nav"));
    }

    A_DELETE_COPY(SocialNavigationTab)

    ~SocialNavigationTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
        delete mBeings;
        mBeings = nullptr;
    }

    void invite()
    {
    }

    void leave()
    {
    }

    void updateList()
    {
        if (!socialWindow || !player_node)
            return;

        const Map *const map = socialWindow->getMap();
        if (!map || map->empty())
            return;

        if (socialWindow->getProcessedPortals())
            return;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        std::vector<MapItem*> portals = map->getPortals();

        std::vector<MapItem*>::const_iterator i = portals.begin();
        const SpecialLayer *const specialLayer = map->getSpecialLayer();

        std::vector<Avatar*>::iterator ia = avatars->begin();

        while (ia != avatars->end())
        {
            delete *ia;
            ++ ia;
        }

        avatars->clear();

        int idx = 0;
        while (i != portals.end())
        {
            MapItem *portal = *i;
            if (!portal)
                continue;

            const int x = portal->getX();
            const int y = portal->getY();

            std::string name = strprintf("%s [%d %d]",
                portal->getComment().c_str(), x, y);

            Avatar *const ava = new Avatar(name);
            if (player_node)
                ava->setOnline(player_node->isReachable(x, y, true));
            else
                ava->setOnline(false);
            ava->setLevel(-1);
            ava->setType(portal->getType());
            ava->setX(x);
            ava->setY(y);
            avatars->push_back(ava);

            if (config.getBoolValue("drawHotKeys") && idx < 80 && outfitWindow)
            {
                Being *const being = actorSpriteManager
                    ->findPortalByTile(x, y);
                if (being)
                {
                    being->setName(keyboard.getKeyShortString(
                        outfitWindow->keyName(idx)));
                }

                if (specialLayer)
                {
                    portal = specialLayer->getTile(ava->getX(), ava->getY());
                    if (portal)
                    {
                        portal->setName(keyboard.getKeyShortString(
                            outfitWindow->keyName(idx)));
                    }
                }
            }

            ++i;
            idx ++;
        }
        if (socialWindow)
            socialWindow->setProcessedPortals(true);
    }


    virtual void selectIndex(const unsigned num)
    {
        if (!player_node)
            return;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        if (!avatars || avatars->size() <= num)
            return;

        const Avatar *const ava = avatars->at(num);
        if (ava && player_node)
            player_node->navigateTo(ava->getX(), ava->getY());
    }

    void updateNames()
    {
        if (!socialWindow)
            return;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        if (!avatars)
            return;

        const Map *const map = socialWindow->getMap();
        if (!map)
            return;

        std::vector<Avatar*>::const_iterator i = avatars->begin();
        const std::vector<Avatar*>::const_iterator i_end = avatars->end();
        while (i != i_end)
        {
            Avatar *const ava = *i;
            if (!ava)
                break;

            const  MapItem *const item = map->findPortalXY(
                ava->getX(), ava->getY());
            if (item)
            {
                std::string name = strprintf("%s [%d %d]",
                    item->getComment().c_str(), item->getX(), item->getY());
                ava->setName(name);
                ava->setOriginalName(name);
            }

            ++i;
        }
    }

    int getPortalIndex(const int x, const int y)
    {
        if (!socialWindow)
            return -1;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        if (!avatars)
            return -1;

        const Map *const map = socialWindow->getMap();
        if (!map)
            return 01;

        std::vector<Avatar*>::const_iterator i = avatars->begin();
        const std::vector<Avatar*>::const_iterator i_end = avatars->end();
        unsigned num = 0;
        while (i != i_end)
        {
            const Avatar *const ava = *i;
            if (!ava)
                break;

            if (ava->getX() == x && ava->getY() == y)
                return num;

            ++i;
            num ++;
        }
        return -1;
    }

    void addPortal(const int x, const int y)
    {
        if (!socialWindow || !player_node)
            return;

        const Map *const map = socialWindow->getMap();
        if (!map)
            return;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();

        if (!avatars)
            return;

        const MapItem *const portal = map->findPortalXY(x, y);
        if (!portal)
            return;

        std::string name = strprintf("%s [%d %d]",
            portal->getComment().c_str(), x, y);

        Avatar *const ava = new Avatar(name);
        if (player_node)
            ava->setOnline(player_node->isReachable(x, y, true));
        else
            ava->setOnline(false);
        ava->setLevel(-1);
        ava->setType(portal->getType());
        ava->setX(x);
        ava->setY(y);
        avatars->push_back(ava);
    }

    void removePortal(const int x, const int y)
    {
        if (!socialWindow || !player_node)
            return;

        const Map *const map = socialWindow->getMap();
        if (!map)
            return;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();

        if (!avatars)
            return;

        std::vector<Avatar*>::iterator i = avatars->begin();
        const std::vector<Avatar*>::iterator i_end = avatars->end();

        while (i != i_end)
        {
            Avatar *ava = (*i);

            if (!ava)
                break;

            if (ava->getX() == x && ava->getY() == y)
            {
                delete ava;
                avatars->erase(i);
                return;
            }

            ++ i;
        }
    }

    void updateAvatar(std::string)
    {
    }

    void resetDamage(std::string)
    {
    }

private:
    BeingsListModal *mBeings;

protected:
//    friend class SocialWindow;
};


#define addAvatars(mob, str, type) \
{\
    ava = new Avatar(str);\
    ava->setOnline(false);\
    ava->setLevel(-1);\
    ava->setType(MapItem::SEPARATOR);\
    ava->setX(0);\
    ava->setY(0);\
    avatars->push_back(ava);\
    mobs = actorSpriteManager->get##mob##s();\
    i = mobs.begin();\
    i_end = mobs.end();\
    while (i != i_end)\
    {\
        std::string name;\
        int level = -1;\
        if (*i == "")\
        {\
            name = _("(default)");\
            level = 0;\
        }\
        else\
        {\
            name = *i;\
        }\
        ava = new Avatar(name);\
        ava->setOnline(true);\
        ava->setLevel(level);\
        ava->setType(MapItem::type);\
        ava->setX(0);\
        ava->setY(0);\
        avatars->push_back(ava);\
        ++ i;\
    }\
}

#define updateAtkListStart() \
    if (!socialWindow || !player_node || !actorSpriteManager)\
        return;\
    std::vector<Avatar*> *const avatars = mBeings->getMembers();\
    std::vector<Avatar*>::iterator ia = avatars->begin();\
    while (ia != avatars->end())\
    {\
        delete *ia;\
        ++ ia;\
    }\
    avatars->clear();\
    Avatar *ava;\
    std::list<std::string> mobs;\
    std::list<std::string>::const_iterator i;\
    std::list<std::string>::const_iterator i_end;

class SocialAttackTab final : public SocialTab
{
public:
    SocialAttackTab(const Widget2 *const widget,
                    const bool showBackground) :
        SocialTab(widget),
        mBeings(new BeingsListModal)
    {
        mList = new AvatarListBox(this, mBeings);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

        // TRANSLATORS: Attack filter tab name in social window. Should be small
        setCaption(_("Atk"));
    }

    A_DELETE_COPY(SocialAttackTab)

    ~SocialAttackTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
        delete mBeings;
        mBeings = nullptr;
    }

    void invite()
    {
    }

    void leave()
    {
    }

    void updateList()
    {
        updateAtkListStart();
        // TRANSLATORS: mobs group name in social window
        addAvatars(PriorityAttackMob, _("Priority mobs"), PRIORITY);
        // TRANSLATORS: mobs group name in social window
        addAvatars(AttackMob, _("Attack mobs"), ATTACK);
        // TRANSLATORS: mobs group name in social window
        addAvatars(IgnoreAttackMob, _("Ignore mobs"), IGNORE_);
    }

    void updateAvatar(std::string name A_UNUSED)
    {
    }

    void resetDamage(std::string name A_UNUSED)
    {
    }

private:
    BeingsListModal *mBeings;
};

class SocialPickupTab final : public SocialTab
{
public:
    SocialPickupTab(const Widget2 *const widget,
                    const bool showBackground) :
        SocialTab(widget),
        mBeings(new BeingsListModal)
    {
        mList = new AvatarListBox(this, mBeings);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

        // TRANSLATORS: Pickup filter tab name in social window. Should be small
        setCaption(_("Pik"));
    }

    A_DELETE_COPY(SocialPickupTab)

    ~SocialPickupTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
        delete mBeings;
        mBeings = nullptr;
    }

    void invite()
    {
    }

    void leave()
    {
    }

    void updateList()
    {
        updateAtkListStart();
        // TRANSLATORS: items group name in social window
        addAvatars(PickupItem, _("Pickup items"), PICKUP);
        // TRANSLATORS: items group name in social window
        addAvatars(IgnorePickupItem, _("Ignore items"), NOPICKUP);
    }

    void updateAvatar(std::string name A_UNUSED)
    {
    }

    void resetDamage(std::string name A_UNUSED)
    {
    }

private:
    BeingsListModal *mBeings;
};


class SocialFriendsTab final : public SocialTab
{
public:
    SocialFriendsTab(const Widget2 *const widget,
                     std::string name, const bool showBackground) :
        SocialTab(widget),
        mBeings(new BeingsListModal)
    {
        mList = new AvatarListBox(this, mBeings);
        mScroll = new ScrollArea(mList, showBackground,
            "social_background.xml");

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);

        updateList();
        setCaption(name);
    }

    A_DELETE_COPY(SocialFriendsTab)

    ~SocialFriendsTab()
    {
        delete mList;
        mList = nullptr;
        delete mScroll;
        mScroll = nullptr;
        delete mBeings;
        mBeings = nullptr;
    }

    void updateList()
    {
        getPlayersAvatars();
    }

    void updateAvatar(std::string name A_UNUSED)
    {
    }

    void resetDamage(std::string name A_UNUSED)
    {
    }

    void getPlayersAvatars()
    {
        if (!actorSpriteManager)
            return;

        std::vector<Avatar*> *const avatars = mBeings->getMembers();
        if (!avatars)
            return;

        std::vector<Avatar*>::iterator ia = avatars->begin();
        while (ia != avatars->end())
        {
            delete *ia;
            ++ ia;
        }
        avatars->clear();

        StringVect *players
            = player_relations.getPlayersByRelation(PlayerRelation::FRIEND);

        const std::set<std::string> &players2 = whoIsOnline->getOnlineNicks();

        if (!players)
            return;

        FOR_EACHP (StringVectCIter, it, players)
        {
            Avatar *const ava = new Avatar(*it);
            if (actorSpriteManager->findBeingByName(*it, Being::PLAYER)
                || players2.find(*it) != players2.end())
            {
                ava->setOnline(true);
            }
            avatars->push_back(ava);
        }
        std::sort(avatars->begin(), avatars->end(), friendSorter);
        delete players;
    }

    void invite()
    {
    }

    void leave()
    {
    }

private:
    BeingsListModal *mBeings;
};


class CreatePopup final : public Popup, public LinkHandler
{
public:
    CreatePopup() :
        Popup("SocialCreatePopup"),
        LinkHandler(),
        mBrowserBox(new BrowserBox(this))
    {
        mBrowserBox->setPosition(4, 4);
        mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
        mBrowserBox->setOpaque(false);
        mBrowserBox->setLinkHandler(this);

        // TRANSLATORS: party popup item
        mBrowserBox->addRow(strprintf("@@party|%s@@", _("Create Party")));
        mBrowserBox->addRow("##3---");
        // TRANSLATORS: party popup item
        mBrowserBox->addRow(strprintf("@@cancel|%s@@", _("Cancel")));

        add(mBrowserBox);

        setContentSize(mBrowserBox->getWidth() + 8,
                       mBrowserBox->getHeight() + 8);
    }

    A_DELETE_COPY(CreatePopup)

    void handleLink(const std::string &link, gcn::MouseEvent *event A_UNUSED)
    {
        if (link == "guild" && socialWindow)
        {
            socialWindow->showGuildCreate();
        }
        else if (link == "party" && socialWindow)
        {
            socialWindow->showPartyCreate();
        }

        setVisible(false);
    }

    void show(gcn::Widget *parent)
    {
        if (!parent)
            return;

        int x, y;
        parent->getAbsolutePosition(x, y);
        y += parent->getHeight();
        setPosition(x, y);
        setVisible(true);
        requestMoveToTop();
    }

private:
    BrowserBox* mBrowserBox;
};

SocialWindow::SocialWindow() :
    // TRANSLATORS: social window name
    Window(_("Social"), false, nullptr, "social.xml"),
    gcn::ActionListener(),
    mGuildInvited(0),
    mGuildAcceptDialog(nullptr),
    mGuildCreateDialog(nullptr),
    mPartyInviter(),
    mPartyAcceptDialog(nullptr),
    mPartyCreateDialog(nullptr),
    mGuilds(),
    mParties(),
    mAttackFilter(nullptr),
    mPickupFilter(nullptr),
    // TRANSLATORS: here P is title for visible players tab in social window
    mPlayers(new SocialPlayersTab(this, _("P"),
        getOptionBool("showtabbackground"))),
    mNavigation(new SocialNavigationTab(this,
        getOptionBool("showtabbackground"))),
    // TRANSLATORS: here F is title for friends tab in social window
    mFriends(new SocialFriendsTab(this, _("F"),
        getOptionBool("showtabbackground"))),
    mCreatePopup(new CreatePopup),
    // TRANSLATORS: social window button
    mCreateButton(new Button(this, _("Create"), "create", this)),
    // TRANSLATORS: social window button
    mInviteButton(new Button(this, _("Invite"), "invite", this)),
    // TRANSLATORS: social window button
    mLeaveButton(new Button(this, _("Leave"), "leave", this)),
    mTabs(new TabbedArea(this)),
    mMap(nullptr),
    mLastUpdateTime(0),
    mNeedUpdate(false),
    mProcessedPortals(false)
{
    setWindowName("Social");
    setVisible(false);
    setSaveVisible(true);
    setResizable(true);
    setSaveVisible(true);
    setCloseButton(true);
    setStickyButtonLock(true);

    setMinWidth(120);
    setMinHeight(55);
    setDefaultSize(590, 200, 180, 300);
    setupWindow->registerWindowForReset(this);


    place(0, 0, mCreateButton);
    place(1, 0, mInviteButton);
    place(2, 0, mLeaveButton);
    place(0, 1, mTabs, 4, 4);

    widgetResized(gcn::Event(nullptr));

    loadWindowState();

    mTabs->addTab(mPlayers, mPlayers->mScroll);

    mTabs->addTab(mFriends, mFriends->mScroll);

    mTabs->addTab(mNavigation, mNavigation->mScroll);

    if (config.getBoolValue("enableAttackFilter"))
    {
        mAttackFilter = new SocialAttackTab(this,
            getOptionBool("showtabbackground"));
        mTabs->addTab(mAttackFilter, mAttackFilter->mScroll);
    }
    else
    {
        mAttackFilter = nullptr;
    }

    if (config.getBoolValue("enablePickupFilter"))
    {
        mPickupFilter = new SocialPickupTab(this,
            getOptionBool("showtabbackground"));
        mTabs->addTab(mPickupFilter, mPickupFilter->mScroll);
    }
    else
    {
        mPickupFilter = nullptr;
    }

    if (player_node && player_node->getParty())
        addTab(player_node->getParty());

    if (player_node && player_node->getGuild())
        addTab(player_node->getGuild());

    enableVisibleSound(true);
    updateButtons();
}

SocialWindow::~SocialWindow()
{
    // Cleanup invites
    if (mGuildAcceptDialog)
    {
        mGuildAcceptDialog->close();
        mGuildAcceptDialog->scheduleDelete();
        mGuildAcceptDialog = nullptr;

        mGuildInvited = 0;
    }

    if (mPartyAcceptDialog)
    {
        mPartyAcceptDialog->close();
        mPartyAcceptDialog->scheduleDelete();
        mPartyAcceptDialog = nullptr;

        mPartyInviter.clear();
    }
    delete mCreatePopup;
    mCreatePopup = nullptr;
    delete mPlayers;
    mPlayers = nullptr;
    delete mNavigation;
    mNavigation = nullptr;
    delete mAttackFilter;
    mAttackFilter = nullptr;
    delete mPickupFilter;
    mPickupFilter = nullptr;
    delete mFriends;
    mFriends = nullptr;
}

bool SocialWindow::addTab(Guild *const guild)
{
    if (mGuilds.find(guild) != mGuilds.end())
        return false;

    SocialTab *tab = nullptr;
    if (guild->getServerGuild())
    {
        tab = new SocialGuildTab(this, guild,
            getOptionBool("showtabbackground"));
    }
    else
    {
        tab = new SocialGuildTab2(this, guild,
            getOptionBool("showtabbackground"));
    }

    mGuilds[guild] = tab;
    mTabs->addTab(tab, tab->mScroll);

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Guild *const guild)
{
    const GuildMap::iterator it = mGuilds.find(guild);
    if (it == mGuilds.end())
        return false;

    mTabs->removeTab(it->second);
    delete it->second;
    mGuilds.erase(it);

    updateButtons();

    return true;
}

bool SocialWindow::addTab(Party *const party)
{
    if (mParties.find(party) != mParties.end())
        return false;

    SocialPartyTab *const tab = new SocialPartyTab(this, party,
        getOptionBool("showtabbackground"));
    mParties[party] = tab;

    mTabs->addTab(tab, tab->mScroll);

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Party *const party)
{
    const PartyMap::iterator it = mParties.find(party);
    if (it == mParties.end())
        return false;

    mTabs->removeTab(it->second);
    delete it->second;
    mParties.erase(it);

    updateButtons();

    return true;
}

void SocialWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    if (event.getSource() == mPartyAcceptDialog)
    {
        // check if they accepted the invite
        if (eventId == "yes")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Accepted party invite from %s."),
                    mPartyInviter.c_str()));
            }
            Net::getPartyHandler()->inviteResponse(mPartyInviter, true);
        }
        else if (eventId == "no")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Rejected party invite from %s."),
                    mPartyInviter.c_str()));
            }
            Net::getPartyHandler()->inviteResponse(mPartyInviter, false);
        }

        mPartyInviter.clear();
        mPartyAcceptDialog = nullptr;
    }
    else if (event.getSource() == mGuildAcceptDialog)
    {
        // check if they accepted the invite
        if (eventId == "yes")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Accepted guild invite from %s."),
                    mPartyInviter.c_str()));
            }
            if (!guildManager || !GuildManager::getEnableGuildBot())
                Net::getGuildHandler()->inviteResponse(mGuildInvited, true);
            else
                guildManager->inviteResponse(true);
        }
        else if (eventId == "no")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Rejected guild invite from %s."),
                    mPartyInviter.c_str()));
            }
            if (!guildManager || !GuildManager::getEnableGuildBot())
                Net::getGuildHandler()->inviteResponse(mGuildInvited, false);
            else
                guildManager->inviteResponse(false);
        }

        mGuildInvited = 0;
        mGuildAcceptDialog = nullptr;
    }
    else if (eventId == "create")
    {
        showPartyCreate();
    }
    else if (eventId == "invite" && mTabs->getSelectedTabIndex() > -1)
    {
        if (mTabs->getSelectedTab())
            static_cast<SocialTab*>(mTabs->getSelectedTab())->invite();
    }
    else if (eventId == "leave" && mTabs->getSelectedTabIndex() > -1)
    {
        if (mTabs->getSelectedTab())
            static_cast<SocialTab*>(mTabs->getSelectedTab())->leave();
    }
    else if (eventId == "create guild")
    {
        std::string name = mGuildCreateDialog->getText();

        if (name.size() > 16)
            return;

        Net::getGuildHandler()->create(name);
        if (localChatTab)
        {
            // TRANSLATORS: chat message
            localChatTab->chatLog(strprintf(_("Creating guild called %s."),
                name.c_str()), BY_SERVER);
        }

        mGuildCreateDialog = nullptr;
    }
    else if (eventId == "~create guild")
    {
        mGuildCreateDialog = nullptr;
    }
    else if (eventId == "create party")
    {
        std::string name = mPartyCreateDialog->getText();

        if (name.size() > 16)
            return;

        Net::getPartyHandler()->create(name);
        if (localChatTab)
        {
            // TRANSLATORS: chat message
            localChatTab->chatLog(strprintf(_("Creating party called %s."),
                name.c_str()), BY_SERVER);
        }

        mPartyCreateDialog = nullptr;
    }
    else if (eventId == "~create party")
    {
        mPartyCreateDialog = nullptr;
    }
}

void SocialWindow::showGuildCreate()
{
    // TRANSLATORS: guild creation message
    mGuildCreateDialog = new TextDialog(_("Guild Name"),
        // TRANSLATORS: guild creation message
        _("Choose your guild's name."), this);
    mGuildCreateDialog->setActionEventId("create guild");
    mGuildCreateDialog->addActionListener(this);
}

void SocialWindow::showGuildInvite(const std::string &guildName,
                                   const int guildId,
                                   const std::string &inviterName)
{
    // check there isnt already an invite showing
    if (mGuildInvited != 0)
    {
        if (localChatTab)
        {
            // TRANSLATORS: chat message
            localChatTab->chatLog(_("Received guild request, but one already "
                "exists."), BY_SERVER);
        }
        return;
    }

    // TRANSLATORS: chat message
    std::string msg = strprintf(_("%s has invited you to join the guild %s."),
                                inviterName.c_str(), guildName.c_str());
    if (localChatTab)
        localChatTab->chatLog(msg, BY_SERVER);

    // show invite
    // TRANSLATORS: guild invite message
    mGuildAcceptDialog = new ConfirmDialog(_("Accept Guild Invite"),
                                           msg, false, false, this);
    mGuildAcceptDialog->addActionListener(this);

    mGuildInvited = guildId;
}

void SocialWindow::showPartyInvite(const std::string &partyName,
                                   const std::string &inviter)
{
    // check there isnt already an invite showing
    if (mPartyInviter != "")
    {
        if (localChatTab)
        {
            // TRANSLATORS: chat message
            localChatTab->chatLog(_("Received party request, but one already "
                "exists."), BY_SERVER);
        }
        return;
    }

    std::string msg;
    if (inviter.empty())
    {
        if (partyName.empty())
        {
            // TRANSLATORS: party invite message
            msg = _("You have been invited you to join a party.");
        }
        else
        {
            // TRANSLATORS: party invite message
            msg = strprintf(_("You have been invited to join the %s party."),
                            partyName.c_str());
        }
    }
    else
    {
        if (partyName.empty())
        {
            // TRANSLATORS: party invite message
            msg = strprintf(_("%s has invited you to join their party."),
                            inviter.c_str());
        }
        else
        {
            // TRANSLATORS: party invite message
            msg = strprintf(_("%s has invited you to join the %s party."),
                            inviter.c_str(), partyName.c_str());
        }
    }

    if (localChatTab)
        localChatTab->chatLog(msg, BY_SERVER);

    // show invite
    // TRANSLATORS: party invite message
    mPartyAcceptDialog = new ConfirmDialog(_("Accept Party Invite"),
                                           msg, false, false, this);
    mPartyAcceptDialog->addActionListener(this);

    mPartyInviter = inviter;
}

void SocialWindow::showPartyCreate()
{
    if (!player_node)
        return;

    if (player_node->getParty())
    {
        // TRANSLATORS: party creation message
        new OkDialog(_("Create Party"),
            _("Cannot create party. You are already in a party"),
            DIALOG_ERROR, true, true, this);
        return;
    }

    // TRANSLATORS: party creation message
    mPartyCreateDialog = new TextDialog(_("Party Name"),
        // TRANSLATORS: party creation message
        _("Choose your party's name."), this);
    mPartyCreateDialog->setActionEventId("create party");
    mPartyCreateDialog->addActionListener(this);
}

void SocialWindow::updateActiveList()
{
    mNeedUpdate = true;
}

void SocialWindow::slowLogic()
{
    BLOCK_START("SocialWindow::slowLogic")
    const unsigned int nowTime = cur_time;
    if (mNeedUpdate && nowTime - mLastUpdateTime > 1)
    {
        mPlayers->updateList();
        mFriends->updateList();
        mNeedUpdate = false;
        mLastUpdateTime = nowTime;
    }
    else if (nowTime - mLastUpdateTime > 5)
    {
        mPlayers->updateList();
        mNeedUpdate = false;
        mLastUpdateTime = nowTime;
    }
    BLOCK_END("SocialWindow::slowLogic")
}

void SocialWindow::updateAvatar(std::string name)
{
    mPlayers->updateAvatar(name);
}

void SocialWindow::resetDamage(std::string name)
{
    mPlayers->resetDamage(name);
}

void SocialWindow::updateButtons()
{
    if (!mTabs)
        return;

    const bool hasTabs = mTabs->getNumberOfTabs() > 0;
    mInviteButton->setEnabled(hasTabs);
    mLeaveButton->setEnabled(hasTabs);
}

void SocialWindow::updatePortals()
{
    if (mNavigation)
        mNavigation->updateList();
}

void SocialWindow::updatePortalNames()
{
    if (mNavigation)
        static_cast<SocialNavigationTab*>(mNavigation)->updateNames();
}

void SocialWindow::selectPortal(const unsigned num)
{
    if (mNavigation)
        mNavigation->selectIndex(num);
}

int SocialWindow::getPortalIndex(const int x, const int y)
{
    if (mNavigation)
    {
        return static_cast<SocialNavigationTab*>(
            mNavigation)->getPortalIndex(x, y);
    }
    else
    {
        return -1;
    }
}

void SocialWindow::addPortal(const int x, const int y)
{
    if (mNavigation)
        static_cast<SocialNavigationTab*>(mNavigation)->addPortal(x, y);
}

void SocialWindow::removePortal(const int x, const int y)
{
    if (mNavigation)
        static_cast<SocialNavigationTab*>(mNavigation)->removePortal(x, y);
}

void SocialWindow::nextTab()
{
    if (!mTabs)
        return;

    int tab = mTabs->getSelectedTabIndex();

    tab++;
    if (tab == mTabs->getNumberOfTabs())
        tab = 0;

    mTabs->setSelectedTabByPos(tab);
}

void SocialWindow::prevTab()
{
    if (!mTabs)
        return;

    int tab = mTabs->getSelectedTabIndex();

    if (tab == 0)
        tab = mTabs->getNumberOfTabs();
    tab--;

    mTabs->setSelectedTabByPos(tab);
}

void SocialWindow::updateAttackFilter()
{
    if (mAttackFilter)
        mAttackFilter->updateList();
}

void SocialWindow::updatePickupFilter()
{
    if (mPickupFilter)
        mPickupFilter->updateList();
}

void SocialWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    if (mTabs)
        mTabs->fixSize();
}
