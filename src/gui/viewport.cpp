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

#include "gui/viewport.h"

#include "actorspritemanager.h"
#include "client.h"
#include "configuration.h"
#include "game.h"
#include "itemshortcut.h"
#include "inputmanager.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "playerinfo.h"
#include "textmanager.h"

#include "gui/beingpopup.h"
#include "gui/chatwindow.h"
#include "gui/gui.h"
#include "gui/ministatuswindow.h"
#include "gui/popupmenu.h"
#include "gui/statuspopup.h"
#include "gui/textpopup.h"

#include "gui/widgets/button.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/textfield.h"

#include "net/net.h"

#include "resources/resourcemanager.h"

#include <guichan/font.hpp>

#include "debug.h"

extern volatile int tick_time;

Viewport::Viewport() :
    WindowContainer(nullptr),
    gcn::MouseListener(),
    mMap(nullptr),
    mScrollRadius(config.getIntValue("ScrollRadius")),
    mScrollLaziness(config.getIntValue("ScrollLaziness")),
    mShowBeingPopup(config.getBoolValue("showBeingPopup")),
    mSelfMouseHeal(config.getBoolValue("selfMouseHeal")),
    mEnableLazyScrolling(config.getBoolValue("enableLazyScrolling")),
    mScrollCenterOffsetX(config.getIntValue("ScrollCenterOffsetX")),
    mScrollCenterOffsetY(config.getIntValue("ScrollCenterOffsetY")),
    mMouseDirectionMove(config.getBoolValue("mouseDirectionMove")),
    mMouseX(0),
    mMouseY(0),
    mPixelViewX(0),
    mPixelViewY(0),
    mShowDebugPath(false),
    mCameraMode(0),
    mPlayerFollowMouse(false),
    mLocalWalkTime(-1),
    mPopupMenu(new PopupMenu),
    mHoverBeing(nullptr),
    mHoverItem(nullptr),
    mHoverSign(nullptr),
    mBeingPopup(new BeingPopup),
    mTextPopup(new TextPopup),
    mCameraRelativeX(0),
    mCameraRelativeY(0)
{
    setOpaque(false);
    addMouseListener(this);

    config.addListener("ScrollLaziness", this);
    config.addListener("ScrollRadius", this);
    config.addListener("showBeingPopup", this);
    config.addListener("selfMouseHeal", this);
    config.addListener("enableLazyScrolling", this);
    config.addListener("mouseDirectionMove", this);

    setFocusable(true);
}

Viewport::~Viewport()
{
    config.removeListeners(this);

    delete mPopupMenu;
    mPopupMenu = nullptr;
    delete mBeingPopup;
    mBeingPopup = nullptr;
    delete mTextPopup;
    mTextPopup = nullptr;
}

void Viewport::setMap(Map *const map)
{
    if (mMap && map)
        map->setDebugFlags(mMap->getDebugFlags());
    mMap = map;
}

extern MiniStatusWindow *miniStatusWindow;

void Viewport::draw(gcn::Graphics *gcnGraphics)
{
    BLOCK_START("Viewport::draw 1")
    static int lastTick = tick_time;

    if (!mMap || !player_node)
    {
        gcnGraphics->setColor(gcn::Color(64, 64, 64));
        gcnGraphics->fillRectangle(
                gcn::Rectangle(0, 0, getWidth(), getHeight()));
        BLOCK_END("Viewport::draw 1")
        return;
    }

    Graphics *const graphics = static_cast<Graphics* const>(gcnGraphics);

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
        lastTick = tick_time;

    // Calculate viewpoint
    const int midTileX = (graphics->mWidth + mScrollCenterOffsetX) / 2;
    const int midTileY = (graphics->mHeight + mScrollCenterOffsetY) / 2;

    const Vector &playerPos = player_node->getPosition();
    const int player_x = static_cast<int>(playerPos.x)
                         - midTileX + mCameraRelativeX;
    const int player_y = static_cast<int>(playerPos.y)
                         - midTileY + mCameraRelativeY;

    if (mScrollLaziness < 1)
        mScrollLaziness = 1;  // Avoids division by zero

    if (mEnableLazyScrolling)
    {
        int cnt = 0;

        // Apply lazy scrolling
        while (lastTick < tick_time && cnt < 32)
        {
            if (player_x > mPixelViewX + mScrollRadius)
            {
                mPixelViewX += static_cast<float>(player_x
                    - mPixelViewX - mScrollRadius) /
                    static_cast<float>(mScrollLaziness);
            }
            if (player_x < mPixelViewX - mScrollRadius)
            {
                mPixelViewX += static_cast<float>(player_x
                    - mPixelViewX + mScrollRadius) /
                    static_cast<float>(mScrollLaziness);
            }
            if (player_y > mPixelViewY + mScrollRadius)
            {
                mPixelViewY += static_cast<float>(player_y
                - mPixelViewY - mScrollRadius) /
                static_cast<float>(mScrollLaziness);
            }
            if (player_y < mPixelViewY - mScrollRadius)
            {
                mPixelViewY += static_cast<float>(player_y
                - mPixelViewY + mScrollRadius) /
                static_cast<float>(mScrollLaziness);
            }
            lastTick ++;
            cnt ++;
        }

        // Auto center when player is off screen
        if (cnt > 30 || player_x - mPixelViewX
            > graphics->mWidth / 2 || mPixelViewX
            - player_x > graphics->mWidth / 2 || mPixelViewY
            - player_y > graphics->getHeight() / 2 ||  player_y
            - mPixelViewY > graphics->getHeight() / 2)
        {
            if (player_x <= 0 || player_y <= 0)
            {
//                if (debugChatTab)
//                    debugChatTab->chatLog("incorrect player position!");
                logger->log("incorrect player position: %d, %d, %d, %d",
                    player_x, player_y, mPixelViewX, mPixelViewY);
                if (player_node)
                {
                    logger->log("tile position: %d, %d",
                        player_node->getTileX(), player_node->getTileY());
                }
            }
            mPixelViewX = player_x;
            mPixelViewY = player_y;
        }
    }
    else
    {
        mPixelViewX = player_x;
        mPixelViewY = player_y;
    }

    // Don't move camera so that the end of the map is on screen
    const int viewXmax =
        mMap->getWidth() * mMap->getTileWidth() - graphics->mWidth;
    const int viewYmax =
        mMap->getHeight() * mMap->getTileHeight() - graphics->mHeight;

    if (mPixelViewX < 0)
        mPixelViewX = 0;
    if (mPixelViewY < 0)
        mPixelViewY = 0;
    if (mPixelViewX > viewXmax)
        mPixelViewX = viewXmax;
    if (mPixelViewY > viewYmax)
        mPixelViewY = viewYmax;

    // Draw tiles and sprites
    mMap->draw(graphics, mPixelViewX, mPixelViewY);

    if (mShowDebugPath)
    {
        mMap->drawCollision(graphics, mPixelViewX,
            mPixelViewY, mShowDebugPath);
        if (mShowDebugPath == Map::MAP_DEBUG)
            _drawDebugPath(graphics);
    }

    if (player_node->getCheckNameSetting())
    {
        player_node->setCheckNameSetting(false);
        player_node->setName(player_node->getName());
    }

    // Draw text
    if (textManager)
    {
        textManager->draw(graphics, mPixelViewX, mPixelViewY);
    }

    // Draw player names, speech, and emotion sprite as needed
    const ActorSprites &actors = actorSpriteManager->getAll();
    FOR_EACH (ActorSpritesConstIterator, it, actors)
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;
        Being *const b = static_cast<Being*>(*it);

        b->drawSpeech(mPixelViewX, mPixelViewY);
        b->drawEmotion(graphics, mPixelViewX, mPixelViewY);
    }

    if (miniStatusWindow)
        miniStatusWindow->drawIcons(graphics);

    // Draw contained widgets
    WindowContainer::draw(gcnGraphics);
    BLOCK_END("Viewport::draw 1")
}

void Viewport::logic()
{
    BLOCK_START("Viewport::logic")
    // Make the player follow the mouse position
    // if the mouse is dragged elsewhere than in a window.
    _followMouse();
    BLOCK_END("Viewport::logic")
}

void Viewport::_followMouse()
{
    const uint8_t button = SDL_GetMouseState(&mMouseX, &mMouseY);
    // If the left button is dragged
    if (mPlayerFollowMouse && (button & SDL_BUTTON(1)))
    {
        // We create a mouse event and send it to mouseDragged.
        const uint8_t *const keys = SDL_GetKeyState(nullptr);
        gcn::MouseEvent mouseEvent(nullptr,
                      (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]),
                      false,
                      false,
                      false,
                      gcn::MouseEvent::DRAGGED,
                      gcn::MouseEvent::LEFT,
                      mMouseX,
                      mMouseY,
                      0);

        mouseDragged(mouseEvent);
    }
}

void Viewport::_drawDebugPath(Graphics *const graphics)
{
    if (!player_node || !userPalette || !actorSpriteManager || !mMap)
        return;

    // Get the current mouse position
    SDL_GetMouseState(&mMouseX, &mMouseY);

    static Path debugPath;
    static Vector lastMouseDestination = Vector(0.0f, 0.0f);
    const int mousePosX = mMouseX + mPixelViewX;
    const int mousePosY = mMouseY + mPixelViewY;
    Vector mouseDestination(mousePosX, mousePosY);

    if (mouseDestination.x != lastMouseDestination.x
        || mouseDestination.y != lastMouseDestination.y)
    {
        const Vector &playerPos = player_node->getPosition();

        debugPath = mMap->findPath(
            static_cast<int>(playerPos.x - 16) / 32,
            static_cast<int>(playerPos.y - 32) / 32,
            mousePosX / 32, mousePosY / 32, player_node->getWalkMask(), 500);

        lastMouseDestination = mouseDestination;
    }
    _drawPath(graphics, debugPath, userPalette->getColorWithAlpha(
        UserPalette::ROAD_POINT));

    // We draw the path proposed by mouse

    // Draw the path debug information for every beings.
    const ActorSprites &actors = actorSpriteManager->getAll();
    FOR_EACH (ActorSpritesConstIterator, it, actors)
    {
        const Being *const being = dynamic_cast<Being*>(*it);
        if (being && being != player_node)
        {
            Path beingPath = being->getPath();

            _drawPath(graphics, beingPath, userPalette->getColorWithAlpha(
                UserPalette::ROAD_POINT));
        }
    }
}

void Viewport::_drawPath(Graphics *const graphics, const Path &path,
                         const gcn::Color &color) const
{
    graphics->setColor(color);

    gcn::Font *const font = getFont();

#ifdef MANASERV_SUPPORT
    if (Net::getNetworkType() != ServerInfo::MANASERV)
#endif
    {
        int cnt = 1;
        FOR_EACH (Path::const_iterator, i, path)
        {
            const int squareX = i->x * 32 - mPixelViewX + 12;
            const int squareY = i->y * 32 - mPixelViewY + 12;

            graphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
            if (mMap)
            {
                const std::string str = toString(cnt);
                font->drawString(graphics, str, squareX + 4
                    - font->getWidth(str) / 2, squareY + 12);
            }
            cnt ++;
        }
    }
#ifdef MANASERV_SUPPORT
    else if (Net::getNetworkType() == ServerInfo::MANASERV)
    {
        FOR_EACH (Path::const_iterator, i, path)
        {
            int squareX = i->x - mPixelViewX;
            int squareY = i->y - mPixelViewY;

            graphics->fillRectangle(gcn::Rectangle(squareX - 4, squareY - 4,
                                                   8, 8));
            if (mMap)
            {
                const std::string str = toString(mMap->getMetaTile(
                    i->x / 32, i->y / 32)->Gcost);
                font->drawString(graphics, str,
                    squareX + 4 - font->getWidth(text) / 2, squareY + 12);
            }
        }
    }
#endif
}

void Viewport::mousePressed(gcn::MouseEvent &event)
{
    if (event.getSource() != this)
        return;

    // Check if we are alive and kickin'
    if (!mMap || !player_node)
        return;

    // Check if we are busy
    // if commented, allow context menu if npc dialog open
    if (PlayerInfo::isTalking())
        return;

    const int pixelX = event.getX() + mPixelViewX;
    const int pixelY = event.getY() + mPixelViewY;

    // Right click might open a popup
    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        mPlayerFollowMouse = false;
        if (mHoverBeing)
        {
            validateSpeed();
            if (actorSpriteManager)
            {
                std::vector<ActorSprite*> beings;
                const int x = getMouseX() + mPixelViewX;
                const int y = getMouseY() + mPixelViewY;
                actorSpriteManager->findBeingsByPixel(beings, x, y, true);
                if (beings.size() > 1)
                {
                    mPopupMenu->showPopup(event.getX(), event.getY(), beings);
                    return;
                }
                else
                {
                    mPopupMenu->showPopup(event.getX(), event.getY(),
                                          mHoverBeing);
                    return;
                }
            }
        }
        else if (mHoverItem)
        {
            validateSpeed();
            mPopupMenu->showPopup(event.getX(), event.getY(), mHoverItem);
            return;
        }
        else if (mHoverSign)
        {
            validateSpeed();
            mPopupMenu->showPopup(event.getX(), event.getY(), mHoverSign);
            return;
        }
        else if (mCameraMode)
        {
            mPopupMenu->showMapPopup(event.getX(), event.getY(),
                (getMouseX() + getCameraX()) / mMap->getTileWidth(),
                (getMouseY() + getCameraY()) / mMap->getTileHeight());
            return;
        }
    }

    // If a popup is active, just remove it
    if (mPopupMenu->isPopupVisible())
    {
        mPlayerFollowMouse = false;
        mPopupMenu->setVisible(false);
        return;
    }

    // Left click can cause different actions
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        // Interact with some being
        if (mHoverBeing)
        {
            if (!mHoverBeing->isAlive())
                return;

            if (mHoverBeing->canTalk())
            {
                validateSpeed();
                mHoverBeing->talkTo();
                return;
            }
            else
            {
                if (mHoverBeing->getType() == ActorSprite::PLAYER)
                {
                    validateSpeed();
                    if (actorSpriteManager)
                    {
                        if (player_node != mHoverBeing || mSelfMouseHeal)
                            actorSpriteManager->heal(mHoverBeing);
                        if (player_node == mHoverBeing && mHoverItem)
                            player_node->pickUp(mHoverItem);
                        return;
                    }
                }
                else if (player_node->withinAttackRange(mHoverBeing) ||
                         inputManager.isActionActive(static_cast<int>(
                         Input::KEY_ATTACK)))
                {
                    validateSpeed();
                    if (player_node != mHoverBeing)
                    {
                        player_node->attack(mHoverBeing,
                            !inputManager.isActionActive(
                            static_cast<int>(Input::KEY_STOP_ATTACK)));
                        return;
                    }
                }
                else if (!inputManager.isActionActive(static_cast<int>(
                         Input::KEY_ATTACK)))
                {
                    validateSpeed();
                    if (player_node != mHoverBeing)
                    {
                        player_node->setGotoTarget(mHoverBeing);
                        return;
                    }
                }
            }
        }
        // Picks up a item if we clicked on one
        if (mHoverItem)
        {
            validateSpeed();
            player_node->pickUp(mHoverItem);
        }
        // Just walk around
        else if (!inputManager.isActionActive(static_cast<int>(
                 Input::KEY_ATTACK)))
        {
            validateSpeed();
            player_node->stopAttack();
            player_node->cancelFollow();
            mPlayerFollowMouse = true;

            // Make the player go to the mouse position
            _followMouse();
        }
    }
    else if (event.getButton() == gcn::MouseEvent::MIDDLE)
    {
        mPlayerFollowMouse = false;
        validateSpeed();
        // Find the being nearest to the clicked position
        if (actorSpriteManager)
        {
            Being *const target = actorSpriteManager->findNearestLivingBeing(
                pixelX, pixelY, 20, ActorSprite::MONSTER);

            if (target)
                player_node->setTarget(target);
        }
    }
}

void Viewport::mouseDragged(gcn::MouseEvent &event)
{
    if (!mMap || !player_node)
        return;

    if (mPlayerFollowMouse && !event.isShiftPressed())
    {
#ifdef MANASERV_SUPPORT
        if (Net::getNetworkType() == ServerInfo::MANASERV)
        {
            if (get_elapsed_time(mLocalWalkTime) >= walkingMouseDelay)
            {
                mLocalWalkTime = tick_time;
                player_node->unSetPickUpTarget();
                player_node->setDestination(event.getX() + mPixelViewX,
                    event.getY() + mPixelViewY);
                player_node->pathSetByMouse();
            }
        }
        else
#endif
        {
            if (mLocalWalkTime != player_node->getActionTime())
            {
                mLocalWalkTime = cur_time;
                player_node->unSetPickUpTarget();
                int playerX = player_node->getTileX();
                int playerY = player_node->getTileY();
                if (mMouseDirectionMove)
                {
                    const int width = mainGraphics->mWidth / 2;
                    const int height = mainGraphics->mHeight / 2;
                    const float wh = static_cast<float>(width)
                        / static_cast<float>(height);
                    int x = event.getX() - width;
                    int y = event.getY() - height;
                    if (!x && !y)
                        return;
                    const int x2 = abs(x);
                    const int y2 = abs(y);
                    const float diff = 2;
                    int dx = 0;
                    int dy = 0;
                    if (x2 > y2)
                    {
                        if (y2 && x2 / y2 / wh > diff)
                            y = 0;
                    }
                    else
                    {
                        if (x2 && y2 * wh / x2 > diff)
                            x = 0;
                    }
                    if (x > 0)
                        dx = 1;
                    else if (x < 0)
                        dx = -1;
                    if (y > 0)
                        dy = 1;
                    else if (y < 0)
                        dy = -1;

                    if (mMap->getWalk(playerX + dx, playerY + dy))
                    {
                        player_node->navigateTo(playerX + dx, playerY + dy);
                    }
                    else
                    {
                        if (dx && dy)
                        {
                            // try avoid diagonal collision
                            if (x2 > y2)
                            {
                                if (mMap->getWalk(playerX + dx, playerY))
                                    dy = 0;
                                else
                                    dx = 0;
                            }
                            else
                            {
                                if (mMap->getWalk(playerX, playerY + dy))
                                    dx = 0;
                                else
                                    dy = 0;
                            }
                        }
                        else
                        {
                            // try avoid vertical or horisontal collision
                            if (!dx)
                            {
                                if (mMap->getWalk(playerX + 1, playerY + dy))
                                    dx = 1;
                                if (mMap->getWalk(playerX - 1, playerY + dy))
                                    dx = -1;
                            }
                            if (!dy)
                            {
                                if (mMap->getWalk(playerX + dx, playerY + 1))
                                    dy = 1;
                                if (mMap->getWalk(playerX + dx, playerY - 1))
                                    dy = -1;
                            }
                        }
                        player_node->navigateTo(playerX + dx, playerY + dy);
                    }
                }
                else
                {
                    const int destX = (event.getX() + mPixelViewX)
                        / static_cast<float>(mMap->getTileWidth());
                    const int destY = (event.getY() + mPixelViewY)
                        / static_cast<float>(mMap->getTileHeight());
                    if (playerX != destX || playerY != destY)
                    {
                        if (!player_node->navigateTo(destX, destY))
                        {
                            if (playerX > destX)
                                playerX --;
                            else if (playerX < destX)
                                playerX ++;
                            if (playerY > destY)
                                playerY --;
                            else if (playerY < destY)
                                playerY ++;
                            if (mMap->getWalk(playerX, playerY, 0))
                                player_node->navigateTo(playerX, playerY);
                        }
                    }
                }
            }
        }
    }
}

void Viewport::mouseReleased(gcn::MouseEvent &event A_UNUSED)
{
    mPlayerFollowMouse = false;

    // Only useful for eAthena but doesn't hurt under ManaServ
    mLocalWalkTime = -1;
}

void Viewport::showPopup(Window *const parent, const int x, const int y,
                         Item *const item, const bool isInventory)
{
    mPopupMenu->showPopup(parent, x, y, item, isInventory);
}

void Viewport::showPopup(MapItem *const item)
{
    mPopupMenu->showPopup(getMouseX(), getMouseY(), item);
}

void Viewport::showPopup(Window *const parent, Item *const item,
                         const bool isInventory)
{
    mPopupMenu->showPopup(parent, getMouseX(), getMouseY(), item, isInventory);
}

void Viewport::showItemPopup(Item *const item)
{
    mPopupMenu->showItemPopup(getMouseX(), getMouseY(), item);
}

void Viewport::showItemPopup(const int itemId, const unsigned char color)
{
    mPopupMenu->showItemPopup(getMouseX(), getMouseY(), itemId, color);
}

void Viewport::showDropPopup(Item *const item)
{
    mPopupMenu->showDropPopup(getMouseX(), getMouseY(), item);
}

void Viewport::showOutfitsPopup(const int x, const int y)
{
    mPopupMenu->showOutfitsPopup(x, y);
}

void Viewport::showOutfitsPopup()
{
    mPopupMenu->showOutfitsPopup(getMouseX(), getMouseY());
}

void Viewport::showSpellPopup(TextCommand *const cmd)
{
    mPopupMenu->showSpellPopup(getMouseX(), getMouseY(), cmd);
}

void Viewport::showChatPopup(const int x, const int y, ChatTab *const tab)
{
    mPopupMenu->showChatPopup(x, y, tab);
}

void Viewport::showChatPopup(ChatTab *const tab)
{
    mPopupMenu->showChatPopup(getMouseX(), getMouseY(), tab);
}

void Viewport::showPopup(const int x, const int y, const Being *const being)
{
    mPopupMenu->showPopup(x, y, being);
}

void Viewport::showPopup(const Being *const being)
{
    mPopupMenu->showPopup(getMouseX(), getMouseY(), being);
}

void Viewport::showPlayerPopup(std::string nick)
{
    mPopupMenu->showPlayerPopup(getMouseX(), getMouseY(), nick);
}

void Viewport::showPopup(const int x, const int y, Button *const button)
{
    mPopupMenu->showPopup(x, y, button);
}

void Viewport::showPopup(const int x, const int y,
                         const ProgressBar *const bar)
{
    mPopupMenu->showPopup(x, y, bar);
}

void Viewport::showAttackMonsterPopup(const std::string &name, const int type)
{
    mPopupMenu->showAttackMonsterPopup(getMouseX(), getMouseY(),
        name, type);
}

void Viewport::showPickupItemPopup(std::string name)
{
    mPopupMenu->showPickupItemPopup(getMouseX(), getMouseY(), name);
}

void Viewport::showUndressPopup(const int x, const int y,
                                const Being *const being, Item *const item)
{
    mPopupMenu->showUndressPopup(x, y, being, item);
}

void Viewport::showMapPopup(const int x, const int y)
{
    mPopupMenu->showMapPopup(getMouseX(), getMouseY(), x, y);
}

void Viewport::showTextFieldPopup(TextField *input)
{
    mPopupMenu->showTextFieldPopup(getMouseX(), getMouseY(), input);
}

void Viewport::showLinkPopup(const std::string &link)
{
    mPopupMenu->showLinkPopup(getMouseX(), getMouseY(), link);
}

void Viewport::showWindowsPopup()
{
    mPopupMenu->showWindowsPopup(getMouseX(), getMouseY());
}

void Viewport::closePopupMenu()
{
    if (mPopupMenu)
        mPopupMenu->handleLink("cancel", nullptr);
}

void Viewport::optionChanged(const std::string &name A_UNUSED)
{
    if (name == "ScrollLaziness")
        mScrollLaziness = config.getIntValue("ScrollLaziness");
    else if (name == "ScrollRadius")
        mScrollRadius = config.getIntValue("ScrollRadius");
    else if (name == "showBeingPopup")
        mShowBeingPopup = config.getBoolValue("showBeingPopup");
    else if (name == "selfMouseHeal")
        mSelfMouseHeal = config.getBoolValue("selfMouseHeal");
    else if (name == "enableLazyScrolling")
        mEnableLazyScrolling = config.getBoolValue("enableLazyScrolling");
    else if (name == "mouseDirectionMove")
        mMouseDirectionMove = config.getBoolValue("mouseDirectionMove");
}

void Viewport::mouseMoved(gcn::MouseEvent &event A_UNUSED)
{
    // Check if we are on the map
    if (!mMap || !player_node || !actorSpriteManager)
        return;

    const int x = getMouseX() + mPixelViewX;
    const int y = getMouseY() + mPixelViewY;

    mHoverBeing = actorSpriteManager->findBeingByPixel(x, y, true);
    if (mHoverBeing && (mHoverBeing->getType() == Being::PLAYER
        || mHoverBeing->getType() == Being::NPC))
    {
        mTextPopup->setVisible(false);
        if (mShowBeingPopup)
            mBeingPopup->show(getMouseX(), getMouseY(), mHoverBeing);
    }
    else
    {
        mBeingPopup->setVisible(false);
    }

    mHoverItem = actorSpriteManager->findItem(x / mMap->getTileWidth(),
        y / mMap->getTileHeight());

    if (!mHoverBeing && !mHoverItem)
    {
        const SpecialLayer *const specialLayer = mMap->getSpecialLayer();
        if (specialLayer)
        {
            const int mouseTileX = (getMouseX() + getCameraX())
                / mMap->getTileWidth();
            const int mouseTileY = (getMouseY() + getCameraY())
                / mMap->getTileHeight();

            mHoverSign = specialLayer->getTile(mouseTileX, mouseTileY);
            if (mHoverSign && mHoverSign->getType() != MapItem::EMPTY)
            {
                if (!mHoverSign->getComment().empty())
                {
                    mBeingPopup->setVisible(false);
                    mTextPopup->show(getMouseX(), getMouseY(),
                        mHoverSign->getComment());
                }
                else
                {
                    if (mTextPopup->isPopupVisible())
                        mTextPopup->setVisible(false);
                }
                gui->setCursorType(Cursor::CURSOR_UP);
                return;
            }
        }
    }
    if (mTextPopup->isPopupVisible())
        mTextPopup->setVisible(false);

    if (mHoverBeing)
    {
        switch (mHoverBeing->getType())
        {
            // NPCs
            case ActorSprite::NPC:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;

            // Monsters
            case ActorSprite::MONSTER:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;

            case ActorSprite::PORTAL:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;

            case ActorSprite::FLOOR_ITEM:
            case ActorSprite::UNKNOWN:
            case ActorSprite::PLAYER:
            case ActorSprite::PET:
            default:
                gui->setCursorType(Cursor::CURSOR_POINTER);
                break;
        }
    }
    // Item mouseover
    else if (mHoverItem)
    {
        gui->setCursorType(mHoverItem->getHoverCursor());
    }
    else
    {
        gui->setCursorType(Cursor::CURSOR_POINTER);
    }
}

void Viewport::toggleDebugPath()
{
    mShowDebugPath++;
    if (mShowDebugPath > Map::MAP_BLACKWHITE)
        mShowDebugPath = Map::MAP_NORMAL;
    if (mMap)
        mMap->setDebugFlags(mShowDebugPath);
}

void Viewport::toggleCameraMode()
{
    mCameraMode++;
    if (mCameraMode > 1)
        mCameraMode = 0;
    if (!mCameraMode)
    {
        mCameraRelativeX = 0;
        mCameraRelativeY = 0;
    }
    if (miniStatusWindow)
        miniStatusWindow->updateStatus();
}

void Viewport::hideBeingPopup()
{
    if (mBeingPopup)
        mBeingPopup->setVisible(false);
    if (mTextPopup)
        mTextPopup->setVisible(false);
}

void Viewport::clearHover(const ActorSprite *const actor)
{
    if (mHoverBeing == actor)
        mHoverBeing = nullptr;

    if (mHoverItem == actor)
        mHoverItem = nullptr;
}

void Viewport::cleanHoverItems()
{
    mHoverBeing = nullptr;
    mHoverItem = nullptr;
    mHoverSign = nullptr;
}

void Viewport::moveCamera(const int dx, const int dy)
{
    mCameraRelativeX += dx;
    mCameraRelativeY += dy;
}

bool Viewport::isPopupMenuVisible() const
{
    return mPopupMenu ? mPopupMenu->isPopupVisible() : false;
}

void Viewport::moveCameraToActor(const int actorId, const int x, const int y)
{
    if (!player_node || !actorSpriteManager)
        return;

    const Actor *const actor = actorSpriteManager->findBeing(actorId);
    if (!actor)
        return;
    const Vector &actorPos = actor->getPosition();
    const Vector &playerPos = player_node->getPosition();
    mCameraMode = 1;
    mCameraRelativeX = actorPos.x - playerPos.x + x;
    mCameraRelativeY = actorPos.y - playerPos.y + y;
}

void Viewport::moveCameraToPosition(const int x, const int y)
{
    if (!player_node)
        return;

    const Vector &playerPos = player_node->getPosition();
    mCameraMode = 1;

    mCameraRelativeX = x - playerPos.x;
    mCameraRelativeY = y - playerPos.y;
}

void Viewport::moveCameraRelative(const int x, const int y)
{
    mCameraMode = 1;
    mCameraRelativeX += x;
    mCameraRelativeY += y;
}

void Viewport::returnCamera()
{
    mCameraMode = 0;
    mCameraRelativeX = 0;
    mCameraRelativeY = 0;
}

void Viewport::validateSpeed() const
{
    if (!inputManager.isActionActive(static_cast<int>(
        Input::KEY_TARGET_ATTACK)) && !inputManager.isActionActive(
        static_cast<int>(Input::KEY_ATTACK)))
    {
        if (Game::instance())
            Game::instance()->setValidSpeed();
    }
}

void Viewport::clearPopup()
{
    if (mPopupMenu)
        mPopupMenu->clear();
}
