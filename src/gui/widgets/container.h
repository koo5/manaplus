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

#ifndef GUI_CONTAINER_H
#define GUI_CONTAINER_H

#include "gui/widgets/widget2.h"

#include <guichan/widget.hpp>
#include <guichan/widgets/container.hpp>

#if !defined USE_INTERNALGUICHAN
typedef std::list<gcn::Widget *>::const_iterator WidgetListConstIterator;
#endif

/**
 * A widget container.
 *
 * The main difference between the standard Guichan container and this one is
 * that childs added to this container are automatically deleted when the
 * container is deleted.
 *
 * This container is also non-opaque by default.
 */
class Container : public gcn::Container,
                  public Widget2
{
    public:
        explicit Container(const Widget2 *const widget);

        ~Container();

        bool safeRemove(gcn::Widget *const widget);
};

#endif
