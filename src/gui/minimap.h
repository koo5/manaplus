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

#ifndef MINIMAP_H
#define MINIMAP_H

#include "gui/widgets/window.h"

class Image;
class Map;
class TextPopup;

/**
 * Minimap window. Shows a minimap image and the name of the current map.
 *
 * The name of the map is defined by the map property "name". The minimap image
 * is defined by the map property "minimap". The path to the image should be
 * given relative to the root of the client data.
 *
 * \ingroup Interface
 */
class Minimap final : public Window, public ConfigListener
{
    public:
        Minimap();

        A_DELETE_COPY(Minimap)

        ~Minimap();

        /**
         * Sets the map image that should be displayed.
         */
        void setMap(const Map *const map);

        /**
         * Toggles the displaying of the minimap.
         */
        void toggle();

        /**
         * Draws the minimap.
         */
        void draw(gcn::Graphics *graphics) override;

        void mouseMoved(gcn::MouseEvent &event) override;

        void mouseReleased(gcn::MouseEvent &event) override;

        void mouseExited(gcn::MouseEvent &event) override;

        void screenToMap(int &x, int &y);

        void optionChanged(const std::string &name);

    private:
        Image *mMapImage;
        float mWidthProportion;
        float mHeightProportion;
        static bool mShow;
        bool mCustomMapImage;
        int mMapOriginX;
        int mMapOriginY;
        TextPopup *mTextPopup;
        bool mAutoResize;
};

extern Minimap *minimap;

#endif
