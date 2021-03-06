/*
 *  The ManaPlus Client
 *  Copyright (C) 2013  The ManaPlus Developers
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

#ifndef WIDGETS_CHARACTERVIEWNORMAL_H
#define WIDGETS_CHARACTERVIEWNORMAL_H

#include "gui/widgets/characterviewbase.h"

#include "debug.h"

class CharacterViewNormal final : public CharacterViewBase
{
    public:
        CharacterViewNormal(CharSelectDialog *const widget,
                            std::vector<CharacterDisplay*> *const entries,
                            const int padding);

        A_DELETE_COPY(CharacterViewNormal)

        ~CharacterViewNormal();

        void show(const int i);

        void resize();

        void action(const gcn::ActionEvent &event A_UNUSED) override;

    private:
        CharacterDisplay *mSelectedEntry;
        std::vector<CharacterDisplay*> *mCharacterEntries;
};

#endif
