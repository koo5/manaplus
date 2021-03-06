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

#ifndef SETUP_H
#define SETUP_H

#include "gui/widgets/tabbedarea.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <list>

class Label;
class SetupTab;

/**
 * The setup dialog. Displays several tabs for configuring different aspects
 * of the game.
 *
 * @see Setup_Audio
 * @see Setup_Colors
 * @see Setup_Joystick
 * @see Setup_Input
 * @see Setup_Players
 * @see Setup_Video
 *
 * \ingroup GUI
 */
class Setup final : public Window, public gcn::ActionListener
{
    public:
        Setup();

        A_DELETE_COPY(Setup)

        ~Setup();

        /**
         * Event handling method.
         */
        void action(const gcn::ActionEvent &event) override;

        /**
         * Enables the reset button when in game.
         */
        void setInGame(const bool inGame);

        void externalUpdate();

        void registerWindowForReset(Window *const window);

        void clearWindowsForReset()
        { mWindowsToReset.clear(); }

        void doCancel();

        void activateTab(const std::string &name);

        void setVisible(bool visible) override;

        void widgetResized(const gcn::Event &event) override;

    private:
        std::list<SetupTab*> mTabs;
        std::list<Window*> mWindowsToReset;
        std::vector<Button*> mButtons;
        Button *mResetWindows;
        TabbedArea *mPanel;
        Label *mVersion;
};

extern Setup* setupWindow;

#endif
