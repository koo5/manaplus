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

#include "gui/ministatuswindow.h"

#include "animatedsprite.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "gui/chatwindow.h"
#include "gui/gui.h"
#include "gui/statuswindow.h"
#include "gui/statuspopup.h"
#include "gui/textpopup.h"
#include "gui/viewport.h"

#include "gui/widgets/chattab.h"
#include "gui/widgets/label.h"
#include "gui/widgets/progressbar.h"

#include "net/net.h"
#include "net/playerhandler.h"
#include "net/gamehandler.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include "debug.h"

extern volatile int tick_time;

typedef std::vector <ProgressBar*>::const_iterator ProgressBarVectorCIter;

MiniStatusWindow::MiniStatusWindow() :
    Popup("MiniStatus", "ministatus.xml"),
    InventoryListener(),
    mBars(),
    mBarNames(),
    mIcons(),
    mSpacing(mSkin ? mSkin->getOption("spacing", 3) : 3),
    mIconPadding(mSkin ? mSkin->getOption("iconPadding", 3) : 3),
    mIconSpacing(mSkin ? mSkin->getOption("iconSpacing", 2) : 2),
    mMaxX(0),
    // TRANSLATORS: status bar name
    mHpBar(createBar(0, 100, 0, Theme::PROG_HP, "hp bar", _("health bar"))),
    mMpBar(Net::getGameHandler()->canUseMagicBar()
        ? createBar(0, 100, 0, Net::getPlayerHandler()->canUseMagic()
        // TRANSLATORS: status bar name
        ? Theme::PROG_MP : Theme::PROG_NO_MP, "mp bar", _("mana bar"))
        : nullptr),
    mXpBar(createBar(0, 100, 0, Theme::PROG_EXP,
        // TRANSLATORS: status bar name
        "xp bar", _("experience bar"))),
    mJobBar(nullptr),
    mWeightBar(createBar(0, 140, 0, Theme::PROG_WEIGHT,
        // TRANSLATORS: status bar name
        "weight bar", _("weight bar"))),
    mInvSlotsBar(createBar(0, 45, 0, Theme::PROG_INVY_SLOTS,
        // TRANSLATORS: status bar name
        "inventory slots bar", _("inventory slots bar"))),
    mMoneyBar(createBar(0, 130, 0, Theme::PROG_INVY_SLOTS,
        // TRANSLATORS: status bar name
        "money bar", _("money bar"))),
    mArrowsBar(createBar(0, 50, 0, Theme::PROG_INVY_SLOTS,
        // TRANSLATORS: status bar name
        "arrows bar", _("arrows bar"))),
    mStatusBar(createBar(100, (config.getIntValue("fontSize") > 16
        ? 250 : 165), 0, Theme::PROG_EXP,
        // TRANSLATORS: status bar name
        "status bar", _("status bar"))),
    mTextPopup(new TextPopup),
    mStatusPopup(new StatusPopup)
{
    listen(CHANNEL_ATTRIBUTES);

    StatusWindow::updateHPBar(mHpBar);

    if (Net::getGameHandler()->canUseMagicBar())
        StatusWindow::updateMPBar(mMpBar);

    const int job = Net::getPlayerHandler()->getJobLocation()
        && serverConfig.getValueBool("showJob", true);

    StatusWindow::updateXPBar(mXpBar);

    if (job)
    {
        mJobBar = createBar(0, 100, 0, Theme::PROG_JOB, "job bar",
            // TRANSLATORS: status bar name
            _("job bar"));
        StatusWindow::updateJobBar(mJobBar);
    }

    loadBars();
    updateBars();

    setVisible(config.getValueBool(getPopupName() + "Visible", true));
    addMouseListener(this);
    Inventory *const inv = PlayerInfo::getInventory();
    if (inv)
        inv->addInventoyListener(this);

    StatusWindow::updateMoneyBar(mMoneyBar);
    StatusWindow::updateArrowsBar(mArrowsBar);
    updateStatus();
}

MiniStatusWindow::~MiniStatusWindow()
{
    delete mTextPopup;
    mTextPopup = nullptr;
    delete mStatusPopup;
    mStatusPopup = nullptr;
    delete_all(mIcons);
    mIcons.clear();

    Inventory *const inv = PlayerInfo::getInventory();
    if (inv)
        inv->removeInventoyListener(this);

    FOR_EACH (ProgressBarVectorCIter, it, mBars)
    {
        ProgressBar *bar = *it;
        if (!bar)
            continue;
        if (!bar->isVisible())
            delete bar;
    }
}

ProgressBar *MiniStatusWindow::createBar(const float progress,
                                         const int width, const int height,
                                         const int color,
                                         const std::string &name,
                                         const std::string &description)
{
    ProgressBar *const bar = new ProgressBar(this,
        progress, width, height, color);
    bar->setActionEventId(name);
    bar->setId(description);
    mBars.push_back(bar);
    mBarNames[name] = bar;
    return bar;
}

void MiniStatusWindow::updateBars()
{
    int x = 0;
    const ProgressBar *lastBar = nullptr;
    FOR_EACH (ProgressBarVectorCIter, it, mBars)
        safeRemove(*it);

    FOR_EACH (ProgressBarVectorCIter, it, mBars)
    {
        ProgressBar *const bar = *it;
        if (!bar)
            continue;
        if (bar->isVisible())
        {
            bar->setPosition(x, 0);
            add(bar);
            x += bar->getWidth() + mSpacing;
            lastBar = bar;
        }
    }

    if (lastBar)
    {
        setContentSize(lastBar->getX() + lastBar->getWidth(),
            lastBar->getY() + lastBar->getHeight());
    }
    mMaxX = x;
}

void MiniStatusWindow::setIcon(const int index, AnimatedSprite *const sprite)
{
    if (index >= static_cast<int>(mIcons.size()))
        mIcons.resize(index + 1, nullptr);

    delete mIcons[index];

    mIcons[index] = sprite;
}

void MiniStatusWindow::eraseIcon(const int index)
{
    if (index < static_cast<int>(mIcons.size()))
    {
        delete mIcons[index];
        mIcons.erase(mIcons.begin() + index);
    }
}

void MiniStatusWindow::drawIcons(Graphics *const graphics)
{
    // Draw icons
    int icon_x = mMaxX + mIconPadding;
    for (size_t i = 0, sz = mIcons.size(); i < sz; i ++)
    {
        if (mIcons[i])
        {
            mIcons[i]->draw(graphics, icon_x, mIconPadding);
            icon_x += mIconSpacing + mIcons[i]->getWidth();
        }
    }
}

void MiniStatusWindow::processEvent(Channels channel A_UNUSED,
                                    const DepricatedEvent &event)
{
    if (event.getName() == EVENT_UPDATEATTRIBUTE)
    {
        const int id = event.getInt("id");
        if (id == PlayerInfo::HP || id == PlayerInfo::MAX_HP)
        {
            StatusWindow::updateHPBar(mHpBar);
        }
        else if (id == PlayerInfo::MP || id == PlayerInfo::MAX_MP)
        {
            StatusWindow::updateMPBar(mMpBar);
        }
        else if (id == PlayerInfo::EXP || id == PlayerInfo::EXP_NEEDED)
        {
            StatusWindow::updateXPBar(mXpBar);
        }
        else if (id == PlayerInfo::TOTAL_WEIGHT
                 || id == PlayerInfo::MAX_WEIGHT)
        {
            StatusWindow::updateWeightBar(mWeightBar);
        }
        else if (id == PlayerInfo::MONEY)
        {
            StatusWindow::updateMoneyBar(mMoneyBar);
        }
    }
    else if (event.getName() == EVENT_UPDATESTAT)
    {
        StatusWindow::updateMPBar(mMpBar);
        StatusWindow::updateJobBar(mJobBar);
    }
}

void MiniStatusWindow::updateStatus()
{
    StatusWindow::updateStatusBar(mStatusBar);
    if (mStatusPopup && mStatusPopup->isPopupVisible())
        mStatusPopup->update();
}

void MiniStatusWindow::logic()
{
    BLOCK_START("MiniStatusWindow::logic")
    Popup::logic();

    for (size_t i = 0, sz = mIcons.size(); i < sz; i++)
    {
        if (mIcons[i])
            mIcons[i]->update(tick_time * 10);
    }
    BLOCK_END("MiniStatusWindow::logic")
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    BLOCK_START("MiniStatusWindow::draw")
    drawChildren(graphics);
    BLOCK_END("MiniStatusWindow::draw")
}

void MiniStatusWindow::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    const int x = event.getX();
    const int y = event.getY();

    if (event.getSource() == mStatusBar)
    {
        mStatusPopup->view(x + getX(), y + getY());
        mTextPopup->hide();
    }
    else if (event.getSource() == mXpBar)
    {
        std::string level;
        if (player_node && player_node->isGM())
        {
            // TRANSLATORS: status bar label
            level = strprintf(_("Level: %d (GM %d)"),
                PlayerInfo::getAttribute(PlayerInfo::LEVEL),
                player_node->getGMLevel());
        }
        else
        {
            // TRANSLATORS: status bar label
            level = strprintf(_("Level: %d"),
                PlayerInfo::getAttribute(PlayerInfo::LEVEL));
        }

        if (PlayerInfo::getAttribute(PlayerInfo::EXP)
            > PlayerInfo::getAttribute(PlayerInfo::EXP_NEEDED))
        {
            mTextPopup->show(x + getX(), y + getY(), level, strprintf("%d/%d",
                PlayerInfo::getAttribute(PlayerInfo::EXP),
                PlayerInfo::getAttribute(PlayerInfo::EXP_NEEDED)));
        }
        else
        {
            mTextPopup->show(x + getX(), y + getY(), level, strprintf("%d/%d",
                PlayerInfo::getAttribute(PlayerInfo::EXP),
                PlayerInfo::getAttribute(PlayerInfo::EXP_NEEDED)),
                // TRANSLATORS: status bar label
                strprintf("%s: %d", _("Need"),
                PlayerInfo::getAttribute(PlayerInfo::EXP_NEEDED)
                - PlayerInfo::getAttribute(PlayerInfo::EXP)));
        }
        mStatusPopup->hide();
    }
    else if (event.getSource() == mHpBar)
    {
        mTextPopup->show(x + getX(), y + getY(), event.getSource()->getId(),
            strprintf("%d/%d", PlayerInfo::getAttribute(PlayerInfo::HP),
            PlayerInfo::getAttribute(PlayerInfo::MAX_HP)));
        mStatusPopup->hide();
    }
    else if (event.getSource() == mMpBar)
    {
        mTextPopup->show(x + getX(), y + getY(), event.getSource()->getId(),
            strprintf("%d/%d", PlayerInfo::getAttribute(PlayerInfo::MP),
            PlayerInfo::getAttribute(PlayerInfo::MAX_MP)));
        mStatusPopup->hide();
    }
    else if (event.getSource() == mJobBar)
    {
        const std::pair<int, int> exp =  PlayerInfo::getStatExperience(
            Net::getPlayerHandler()->getJobLocation());

        if (exp.first > exp.second)
        {
            mTextPopup->show(x + getX(), y + getY(),
                event.getSource()->getId(),
                strprintf("%d/%d", exp.first, exp.second));
        }
        else
        {
            mTextPopup->show(x + getX(), y + getY(),
                event.getSource()->getId(),
                strprintf("%d/%d", exp.first, exp.second),
                // TRANSLATORS: status bar label
                strprintf("%s: %d", _("Need"), exp.second - exp.first));
        }
        mStatusPopup->hide();
    }
    else if (event.getSource() == mWeightBar)
    {
        mTextPopup->show(x + getX(), y + getY(), event.getSource()->getId(),
            strprintf("%d/%d", PlayerInfo::getAttribute(
            PlayerInfo::TOTAL_WEIGHT),
            PlayerInfo::getAttribute(PlayerInfo::MAX_WEIGHT)));
        mStatusPopup->hide();
    }
    else if (event.getSource() == mInvSlotsBar)
    {
        const Inventory *const inv = PlayerInfo::getInventory();
        if (inv)
        {
            const int usedSlots = inv->getNumberOfSlotsUsed();
            const int maxSlots = inv->getSize();

            mTextPopup->show(x + getX(), y + getY(),
                event.getSource()->getId(),
                strprintf("%d/%d", usedSlots, maxSlots));
        }
        mStatusPopup->hide();
    }
    else if (event.getSource() == mMoneyBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
            event.getSource()->getId(),
            toString(PlayerInfo::getAttribute(PlayerInfo::MONEY)));
    }
    else
    {
        mTextPopup->hide();
        mStatusPopup->hide();
    }
}

void MiniStatusWindow::mousePressed(gcn::MouseEvent &event)
{
    if (!viewport)
        return;

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        ProgressBar *const bar = dynamic_cast<ProgressBar*>(event.getSource());
        if (!bar)
            return;
        if (viewport)
        {
            viewport->showPopup(getX() + event.getX(),
                getY() + event.getY(), bar);
        }
    }
}

void MiniStatusWindow::mouseExited(gcn::MouseEvent &event)
{
    Popup::mouseExited(event);

    mTextPopup->hide();
    mStatusPopup->hide();
}

void MiniStatusWindow::showBar(const std::string &name, const bool visible)
{
    ProgressBar *const bar = mBarNames[name];
    if (!bar)
        return;
    bar->setVisible(visible);
    updateBars();
    saveBars();
}

void MiniStatusWindow::loadBars()
{
    if (!config.getIntValue("ministatussaved"))
    {
        if (mWeightBar)
            mWeightBar->setVisible(false);
        if (mInvSlotsBar)
            mInvSlotsBar->setVisible(false);
        if (mMoneyBar)
            mMoneyBar->setVisible(false);
        if (mArrowsBar)
            mArrowsBar->setVisible(false);
        if (mStatusBar)
            mStatusBar->setVisible(false);
        if (mJobBar)
            mJobBar->setVisible(false);
        return;
    }

    for (int f = 0; f < 10; f ++)
    {
        std::string str = config.getValue("ministatus" + toString(f), "");
        if (str == "")
            continue;
        ProgressBar *const bar = mBarNames[str];
        if (!bar)
            continue;
        bar->setVisible(false);
    }
}

void MiniStatusWindow::saveBars()
{
    int i = 0;
    FOR_EACH (ProgressBarVectorCIter, it, mBars)
    {
        const ProgressBar *const bar = *it;
        if (!bar->isVisible())
        {
            config.setValue("ministatus" + toString(i),
                bar->getActionEventId());
            i ++;
        }
    }
    for (int f = i; f < 10; f ++)
        config.deleteKey("ministatus" + toString(f));

    config.setValue("ministatussaved", true);
}

void MiniStatusWindow::slotsChanged(Inventory *const inventory)
{
    if (!inventory)
        return;

    if (inventory->getType() == Inventory::INVENTORY)
        StatusWindow::updateInvSlotsBar(mInvSlotsBar);
}

void MiniStatusWindow::updateArrows()
{
    StatusWindow::updateArrowsBar(mArrowsBar);
}

gcn::Rectangle MiniStatusWindow::getChildrenArea()
{
    return gcn::Rectangle(getPadding(), getPadding(),
        getWidth() - getPadding() * 2,
        getHeight() - getPadding() * 2);
}
