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

#include "gui/textdialog.h"

#include "keyboardconfig.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/passwordfield.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>

#include "debug.h"

int TextDialog::instances = 0;

TextDialog::TextDialog(const std::string &title, const std::string &msg,
                       Window *const parent, const bool isPassword):
    Window(title, true, parent, "textdialog.xml"),
    gcn::ActionListener(),
    mTextField(nullptr),
    mPasswordField(nullptr),
    // TRANSLATORS: text dialog button
    mOkButton(new Button(this, _("OK"), "OK", this)),
    mEnabledKeyboard(keyboard.isEnabled())
{
    keyboard.setEnabled(false);

    Label *const textLabel = new Label(this, msg);
    // TRANSLATORS: text dialog button
    Button *const cancelButton = new Button(this, _("Cancel"), "CANCEL", this);

    place(0, 0, textLabel, 4);
    if (isPassword)
    {
        mPasswordField = new PasswordField(this);
        place(0, 1, mPasswordField, 4);
    }
    else
    {
        mTextField = new TextField(this);
        place(0, 1, mTextField, 4);
    }
    place(2, 2, mOkButton);
    place(3, 2, cancelButton);

    const gcn::Font *const font = getFont();
    if (font)
    {
        int width = font->getWidth(title);
        if (width < textLabel->getWidth())
            width = textLabel->getWidth();
        reflowLayout(static_cast<int>(width + 20));
    }
    else
    {
        reflowLayout(static_cast<int>(textLabel->getWidth() + 20));
    }

    if (getParent())
    {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }
    setVisible(true);
    requestModalFocus();
    if (isPassword)
        mPasswordField->requestFocus();
    else
        mTextField->requestFocus();

    instances++;
}

TextDialog::~TextDialog()
{
    instances--;
}

void TextDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "CANCEL")
        setActionEventId("~" + getActionEventId());

    distributeActionEvent();
    close();
}

const std::string &TextDialog::getText() const
{
    if (mTextField)
        return mTextField->getText();
    else
        return mPasswordField->getText();
}

void TextDialog::setText(std::string text)
{
    if (mTextField)
        mTextField->setText(text);
    else
        mPasswordField->setText(text);
}

void TextDialog::close()
{
    keyboard.setEnabled(mEnabledKeyboard);
    scheduleDelete();
}
