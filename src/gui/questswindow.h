/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2013  The ManaPlus developers
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

#ifndef QUESTS_WINDOW_H
#define QUESTS_WINDOW_H

#include "localconsts.h"

#include "gui/widgets/window.h"

#include "utils/xml.h"

#include <guichan/actionlistener.hpp>

#include <map>
#include <vector>

class Being;
class Button;
class BrowserBox;
class ExtendedListBox;
class ItemLinkHandler;
class Map;
class ScrollArea;
class QuestEffect;
class QuestsModel;

struct QuestItem;

typedef std::map<int, const QuestEffect*> NpcQuestEffectMap;
typedef NpcQuestEffectMap::const_iterator NpcQuestEffectMapCIter;

class QuestsWindow final : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        QuestsWindow();

        A_DELETE_COPY(QuestsWindow)

        ~QuestsWindow();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event) override;

        void updateQuest(const int var, const int val);

        void rebuild(const bool playSound);

        void showQuest(const QuestItem *const quest);

        void setMap(const Map *const map);

        void updateEffects();

        void addEffect(Being *const being);

    private:
        void loadXml();

        void loadQuest(const int var, const XmlNodePtr node);

        void loadEffect(const int var, const XmlNodePtr node);

        QuestsModel *mQuestsModel;
        ExtendedListBox *mQuestsListBox;
        ScrollArea *mQuestScrollArea;
        ItemLinkHandler *mItemLinkHandler;
        BrowserBox *mText;
        ScrollArea *mTextScrollArea;
        Button *mCloseButton;
        // quest variables: var, value
        std::map<int, int> mVars;
        // quests: var, quests
        std::map<int, std::vector<QuestItem*> > mQuests;
        std::vector<QuestEffect*> mAllEffects;
        std::vector<const QuestEffect*> mMapEffects;
        // npc effects for current map and values: npc, effect
        NpcQuestEffectMap mNpcEffects;
        std::vector<QuestItem*> mQuestLinks;
        Image *mCompleteIcon;
        Image *mIncompleteIcon;
        int mNewQuestEffectId;
        int mCompleteQuestEffectId;
        const Map *mMap;
};

extern QuestsWindow *questsWindow;

#endif
