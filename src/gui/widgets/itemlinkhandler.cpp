/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/widgets/itemlinkhandler.h"

#include "item.h"

#include "gui/confirmdialog.h"
#include "gui/helpwindow.h"
#include "gui/itempopup.h"
#include "gui/viewport.h"

#include "utils/gettext.h"
#include "utils/process.h"

#include <sstream>
#include <string>

#include <guichan/actionlistener.hpp>
#include <guichan/mouseinput.hpp>

#include "debug.h"

struct OpenUrlListener : public gcn::ActionListener
{
    OpenUrlListener() :
        url()
    {
    }

    A_DELETE_COPY(OpenUrlListener)

    void action(const gcn::ActionEvent &event)
    {
        if (event.getId() == "yes")
            openBrowser(url);
    }

    std::string url;
} listener;

ItemLinkHandler::ItemLinkHandler() :
    mItemPopup(new ItemPopup)
{
}

ItemLinkHandler::~ItemLinkHandler()
{
    delete mItemPopup;
    mItemPopup = nullptr;
}

void ItemLinkHandler::handleLink(const std::string &link,
                                 gcn::MouseEvent *event)
{
    if (strStartWith(link, "http://") || strStartWith(link, "https://"))
    {
        if (!event)
            return;
        std::string url = link;
        replaceAll(url, " ", "");
        listener.url = url;
        const int button = event->getButton();
        if (button == gcn::MouseInput::LEFT)
        {
            ConfirmDialog *const confirmDlg = new ConfirmDialog(
                // TRANSLATORS: dialog message
                _("Open url"), url, false, true);
            confirmDlg->addActionListener(&listener);
        }
        else if (button == gcn::MouseInput::RIGHT)
        {
            if (viewport)
                viewport->showLinkPopup(url);
        }
    }
    else if (!link.empty() && link[0] == '?')
    {
        if (helpWindow)
        {
            helpWindow->search(link.substr(1));
            helpWindow->requestMoveToTop();
        }
    }
    else if (strStartWith(link, "help://"))
    {
        if (helpWindow)
        {
            helpWindow->loadHelp(link.substr(7));
            helpWindow->requestMoveToTop();
        }
    }
    else
    {
        if (!mItemPopup)
            return;

        int id = 0;
        std::stringstream stream;
        stream << link;
        stream >> id;

        if (id > 0)
        {
            const ItemInfo &itemInfo = ItemDB::get(id);
            // +++ need add color to links?
            mItemPopup->setItem(itemInfo, 1, true);

            if (mItemPopup->isPopupVisible())
            {
                mItemPopup->setVisible(false);
            }
            else if (viewport)
            {
                mItemPopup->position(viewport->getMouseX(),
                    viewport->getMouseY());
            }
        }
    }
}
