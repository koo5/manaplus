/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2013  The ManaPlus Developers
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

#include "logger.h"

#include "client.h"

#include "gui/sdlfont.h"
#include "gui/theme.h"

#include "gui/widgets/browserbox.h"

#include "gtest/gtest.h"

#include <physfs.h>

#include <list>
#include <string>
#include <vector>

#include "debug.h"

extern const char *dirSeparator;

TEST(browserbox, test1)
{
    PHYSFS_init("manaplus");
    dirSeparator = "/";
    Client::Options options;
    Client client(options);

    logger = new Logger();
    Theme *theme = Theme::instance();
    gcn::Widget::setGlobalFont(new SDLFont("/usr/share/fonts/truetype/"
        "ttf-dejavu/DejaVuSans-Oblique.ttf", 18));
    BrowserBox *box = new BrowserBox(nullptr, BrowserBox::AUTO_WRAP);
    box->setWidth(100);
    std::string row = "test";
    box->addRow(row);
    EXPECT_TRUE(box->hasRows());
    box->clearRows();
    row = "@@";
    box->addRow(row);
    row = "@@|";
    box->addRow(row);
    row = "|@@";
    box->addRow(row);
    row = "@@|@@";
    box->addRow(row);
    row = "|@@@@";
    box->addRow(row);
    row = "@@11|22@@";
    box->addRow(row);
    row = "##@@11|22@@";
    box->addRow(row);
    row = "@@##|22@@";
    box->addRow(row);
    row = "@@11|##22@@";
    box->addRow(row);
    row = "@@11|22##@@";
    box->addRow(row);
    row = "@@11|22@@##";
    box->addRow(row);
    row = "<##@@11|22@@";
    box->addRow(row);
    row = "@@<##|22@@";
    box->addRow(row);
    row = "@@11|<##22@@";
    box->addRow(row);
    row = "@@11|22<##@@";
    box->addRow(row);
    row = "@@11|22@@<##";
    box->addRow(row);
    row = "<##11|22@@";
    box->addRow(row);
    row = "<##|22@@";
    box->addRow(row);
    row = "11|<##22@@";
    box->addRow(row);
    row = "11|22<##@@";
    box->addRow(row);
    row = "11|22@@<##";
    box->addRow(row);
    row = "##>@@11|22@@";
    box->addRow(row);
    row = "@@##>|22@@";
    box->addRow(row);
    row = "@@11|##>22@@";
    box->addRow(row);
    row = "@@11|22##>@@";
    box->addRow(row);
    row = "@@11|22@@##>";
    box->addRow(row);
    row = "<##11|22##>";
    box->addRow(row);
    row = "<##|22##>";
    box->addRow(row);
    row = "11|<##22##>";
    box->addRow(row);
    row = "11|22<####>";
    box->addRow(row);
    row = "11|22##><##";
}
