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

#ifndef STATUS_H
#define STATUS_H

#include "listener.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <map>

class AttrDisplay;
class Button;
class Label;
class ProgressBar;
class ScrollArea;
class VertContainer;

/**
 * The player status dialog.
 *
 * \ingroup Interface
 */
class StatusWindow final : public Window,
                           public gcn::ActionListener,
                           public Listener
{
    public:
        /**
         * Constructor.
         */
        StatusWindow();

        A_DELETE_COPY(StatusWindow)

        void processEvent(Channels channel,
                          const DepricatedEvent &event) override;

        void setPointsNeeded(const int id, const int needed);

        void addAttribute(const int id, const std::string &name,
                          const std::string &shortName = "",
                          const bool modifiable = false,
                          const std::string &description = "");

        static void updateHPBar(ProgressBar *const bar,
                                const bool showMax = false);
        static void updateMPBar(ProgressBar *bar, bool showMax = false);
        static void updateJobBar(ProgressBar *const bar,
                                 const bool percent = true);
        static void updateXPBar(ProgressBar *const bar,
                                const bool percent = true);
        static void updateWeightBar(ProgressBar *const bar);
        static void updateInvSlotsBar(ProgressBar *const bar);
        static void updateMoneyBar(ProgressBar *const bar);
        static void updateArrowsBar(ProgressBar *const bar);
        static void updateStatusBar(ProgressBar *const bar,
                                    const bool percent = true);
        static void updateProgressBar(ProgressBar *const bar, const int value,
                                      const int max, const bool percent);
        void updateProgressBar(ProgressBar *const bar, const int id,
                               const bool percent = true) const;

        void action(const gcn::ActionEvent &event) override;

        void clearAttributes();

    private:
        static std::string translateLetter(const char *const letters);

        static std::string translateLetter2(std::string letters);

        /**
         * Status Part
         */
        Label *mLvlLabel;
        Label *mMoneyLabel;
        Label *mHpLabel;
        Label *mMpLabel;
        Label *mXpLabel;
        ProgressBar *mHpBar;
        ProgressBar *mMpBar;
        ProgressBar *mXpBar;

        Label *mJobLvlLabel;
        Label *mJobLabel;
        ProgressBar *mJobBar;

        VertContainer *mAttrCont;
        ScrollArea *mAttrScroll;
        VertContainer *mDAttrCont;
        ScrollArea *mDAttrScroll;

        Label *mCharacterPointsLabel;
        Label *mCorrectionPointsLabel;
        Button *mCopyButton;

        typedef std::map<int, AttrDisplay*> Attrs;
        Attrs mAttrs;
};

extern StatusWindow *statusWindow;

#endif
