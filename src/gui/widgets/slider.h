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

#ifndef SLIDER_H
#define SLIDER_H

#include "gui/widgets/widget2.h"

#include <guichan/widgets/slider.hpp>

#include "localconsts.h"

class Image;
class ImageRect;

/**
 * Slider widget. Same as the Guichan slider but with custom look.
 *
 * \ingroup GUI
 */
class Slider final : public gcn::Slider,
                     public Widget2
{
    public:
        /**
         * Constructor with scale start equal to 0.
         */
        explicit Slider(const double scaleEnd = 1.0);

        /**
         * Constructor.
         */
        Slider(const double scaleStart, const double scaleEnd);

        A_DELETE_COPY(Slider)

        /**
         * Destructor.
         */
        ~Slider();

        /**
         * Update the alpha value to the graphic components.
         */
        void updateAlpha();

        /**
         * Draws the slider.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Draws the marker.
         */
        void drawMarker(gcn::Graphics *const graphics) const;

        /**
         * Called when the mouse enteres the widget area.
         */
        void mouseEntered(gcn::MouseEvent& event) override;

        /**
         * Called when the mouse leaves the widget area.
         */
        void mouseExited(gcn::MouseEvent& event) override;

        void keyPressed(gcn::KeyEvent& keyEvent) override;

        enum SLIDER_ENUM
        {
            HSTART = 0,
            HMID,
            HEND,
            HGRIP,
            VSTART,
            VMID,
            VEND,
            VGRIP,
            SLIDER_MAX
        };

    private:
        /**
         * Used to initialize instances.
         */
        void init();

        static ImageRect buttons[2];
        bool mHasMouse;
        static float mAlpha;
        static int mInstances;
};

#endif
