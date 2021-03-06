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

#ifndef QUITDIALOG_H
#define QUITDIALOG_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include <vector>

class Button;
class RadioButton;

/**
 * The quit dialog.
 *
 * \ingroup Interface
 */
class QuitDialog final : public Window, public gcn::ActionListener,
                         public gcn::KeyListener
{
    public:
        /**
         * Constructor
         *
         * @pointerToMe  will be set to NULL when the QuitDialog is destroyed
         */
        explicit QuitDialog(QuitDialog **const pointerToMe);

        A_DELETE_COPY(QuitDialog)

        /**
         * Destructor
         */
        ~QuitDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event) override;

        void keyPressed(gcn::KeyEvent &keyEvent) override;

    private:
        void placeOption(ContainerPlacer &placer,
                         RadioButton *const option);
        std::vector<RadioButton*> mOptions;

        RadioButton *mLogoutQuit;
        RadioButton *mForceQuit;
        RadioButton *mSwitchAccountServer;
        RadioButton *mSwitchCharacter;
        RadioButton *mRate;
        Button *mOkButton;
        Button *mCancelButton;

        QuitDialog **mMyPointer;
        bool mNeedForceQuit;
};

#endif
