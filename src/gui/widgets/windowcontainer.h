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

#ifndef WINDOWCONTAINER_H
#define WINDOWCONTAINER_H

#include "gui/widgets/container.h"

#include <vector>

/**
 * A window container. This container adds functionality for more convenient
 * widget (windows in particular) destruction.
 *
 * \ingroup GUI
 */
class WindowContainer : public Container
{
    public:
        explicit WindowContainer(const Widget2 *const widget);

        void slowLogic();

        /**
         * Schedule a widget for deletion. It will be deleted at the start of
         * the next logic update.
         */
        void scheduleDelete(gcn::Widget *const widget);

        /**
         * Ensures that all visible windows are on the screen after the screen
         * has been resized.
         */
        void adjustAfterResize(const int oldScreenWidth,
                               const int oldScreenHeight);

#ifdef USE_PROFILER
        void draw(gcn::Graphics* graphics);
#endif

    private:
        /**
         * List of widgets that are scheduled to be deleted.
         */
        typedef std::vector<gcn::Widget*> Widgets;
        typedef Widgets::iterator WidgetIterator;
        Widgets mDeathList;
};

extern WindowContainer *windowContainer;

#endif
