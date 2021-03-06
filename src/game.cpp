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

#include "game.h"

#include "main.h"

#include "actorspritemanager.h"
#include "auctionmanager.h"
#include "animatedsprite.h"
#include "client.h"
#include "commandhandler.h"
#include "effectmanager.h"
#include "emoteshortcut.h"
#include "guildmanager.h"
#include "inputmanager.h"
#include "itemshortcut.h"
#include "joystick.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "particle.h"
#include "playerinfo.h"
#include "soundmanager.h"
#include "spellshortcut.h"
#include "touchmanager.h"

#include "gui/botcheckerwindow.h"
#include "gui/debugwindow.h"
#include "gui/didyouknowwindow.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/helpwindow.h"
#include "gui/inventorywindow.h"
#include "gui/killstats.h"
#include "gui/minimap.h"
#include "gui/ministatuswindow.h"
#include "gui/okdialog.h"
#include "gui/outfitwindow.h"
#include "gui/setup.h"
#include "gui/shopwindow.h"
#include "gui/shortcutwindow.h"
#include "gui/socialwindow.h"
#ifdef MANASERV_SUPPORT
#include "gui/specialswindow.h"
#endif
#include "gui/skilldialog.h"
#include "gui/statuswindow.h"
#include "gui/textdialog.h"
#include "gui/tradewindow.h"
#include "gui/viewport.h"
#include "gui/questswindow.h"
#include "gui/windowmenu.h"
#include "gui/whoisonline.h"

#include "gui/widgets/battletab.h"
#include "gui/widgets/dropshortcutcontainer.h"
#include "gui/widgets/emoteshortcutcontainer.h"
#include "gui/widgets/gmtab.h"
#include "gui/widgets/itemshortcutcontainer.h"
#include "gui/widgets/langtab.h"
#include "gui/widgets/spellshortcutcontainer.h"
#include "gui/widgets/tradetab.h"

#include "net/generalhandler.h"
#include "net/gamehandler.h"
#include "net/packetcounters.h"
#include "net/playerhandler.h"

#include "resources/imagewriter.h"
#include "resources/mapdb.h"
#include "resources/mapreader.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/langs.h"
#include "utils/mkdir.h"
#include "utils/physfstools.h"
#include "utils/process.h"

#include <guichan/exception.hpp>
#include <guichan/focushandler.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include "mumblemanager.h"

#include "debug.h"

Joystick *joystick = nullptr;

OkDialog *weightNotice = nullptr;
int weightNoticeTime = 0;
OkDialog *deathNotice = nullptr;
QuitDialog *quitDialog = nullptr;
OkDialog *disconnectedDialog = nullptr;

ChatWindow *chatWindow = nullptr;
StatusWindow *statusWindow = nullptr;
MiniStatusWindow *miniStatusWindow = nullptr;
InventoryWindow *inventoryWindow = nullptr;
ShopWindow *shopWindow = nullptr;
SkillDialog *skillDialog = nullptr;
Minimap *minimap = nullptr;
EquipmentWindow *equipmentWindow = nullptr;
EquipmentWindow *beingEquipmentWindow = nullptr;
TradeWindow *tradeWindow = nullptr;
HelpWindow *helpWindow = nullptr;
DebugWindow *debugWindow = nullptr;
ShortcutWindow *itemShortcutWindow = nullptr;
ShortcutWindow *emoteShortcutWindow = nullptr;
OutfitWindow *outfitWindow = nullptr;
#ifdef MANASERV_SUPPORT
SpecialsWindow *specialsWindow = nullptr;
#endif
ShortcutWindow *dropShortcutWindow = nullptr;
ShortcutWindow *spellShortcutWindow = nullptr;
WhoIsOnline *whoIsOnline = nullptr;
DidYouKnowWindow *didYouKnowWindow = nullptr;
KillStats *killStats = nullptr;
BotCheckerWindow *botCheckerWindow = nullptr;
SocialWindow *socialWindow = nullptr;
QuestsWindow *questsWindow = nullptr;
WindowMenu *windowMenu = nullptr;

ActorSpriteManager *actorSpriteManager = nullptr;
CommandHandler *commandHandler = nullptr;
#ifdef USE_MUMBLE
MumbleManager *mumbleManager = nullptr;
#endif
Particle *particleEngine = nullptr;
EffectManager *effectManager = nullptr;
SpellManager *spellManager = nullptr;
Viewport *viewport = nullptr;                    /**< Viewport on the map. */
GuildManager *guildManager = nullptr;
AuctionManager *auctionManager = nullptr;

ChatTab *localChatTab = nullptr;
ChatTab *debugChatTab = nullptr;
TradeTab *tradeChatTab = nullptr;
BattleTab *battleChatTab = nullptr;
GmTab *gmChatTab = nullptr;
LangTab *langChatTab = nullptr;

const unsigned adjustDelay = 10;

/**
 * Initialize every game sub-engines in the right order
 */
static void initEngines()
{
    actorSpriteManager = new ActorSpriteManager;
    commandHandler = new CommandHandler;
    effectManager = new EffectManager;
    AuctionManager::init();
    GuildManager::init();

    particleEngine = new Particle(nullptr);
    particleEngine->setupEngine();
    BeingInfo::init();

    Net::getGameHandler()->initEngines();

    keyboard.update();
    if (joystick)
        joystick->update();
}

/**
 * Create all the various globally accessible gui windows
 */
static void createGuiWindows()
{
    if (setupWindow)
        setupWindow->clearWindowsForReset();

    if (emoteShortcut)
        emoteShortcut->load();

    // Create dialogs
    chatWindow = new ChatWindow;
    chatWindow->updateVisibility();
    tradeWindow = new TradeWindow;
    equipmentWindow = new EquipmentWindow(PlayerInfo::getEquipment(),
        player_node);
    beingEquipmentWindow = new EquipmentWindow(nullptr, nullptr, true);
    beingEquipmentWindow->setVisible(false);
    statusWindow = new StatusWindow;
    miniStatusWindow = new MiniStatusWindow;
    inventoryWindow = new InventoryWindow(PlayerInfo::getInventory());
    shopWindow = new ShopWindow;
    skillDialog = new SkillDialog;
    minimap = new Minimap;
    helpWindow = new HelpWindow;
    debugWindow = new DebugWindow;
    itemShortcutWindow = new ShortcutWindow(
        "ItemShortcut", "items.xml", 83, 460);

    for (unsigned f = 0; f < SHORTCUT_TABS; f ++)
    {
        itemShortcutWindow->addTab(toString(f + 1),
            new ItemShortcutContainer(f));
    }
    didYouKnowWindow = new DidYouKnowWindow;
    if (config.getBoolValue("showDidYouKnow"))
    {
        didYouKnowWindow->setVisible(true);
        didYouKnowWindow->loadData();
    }

    emoteShortcutWindow = new ShortcutWindow("EmoteShortcut",
        new EmoteShortcutContainer, "emotes.xml", 130, 480);
    outfitWindow = new OutfitWindow();
#ifdef MANASERV_SUPPORT
    specialsWindow = new SpecialsWindow();
#endif
    dropShortcutWindow = new ShortcutWindow("DropShortcut",
        new DropShortcutContainer, "drops.xml");

    spellShortcutWindow = new ShortcutWindow("SpellShortcut", "spells.xml",
                                             265, 328);
    for (unsigned f = 0; f < SPELL_SHORTCUT_TABS; f ++)
    {
        spellShortcutWindow->addTab(toString(f + 1),
            new SpellShortcutContainer(f));
    }

    botCheckerWindow = new BotCheckerWindow;
    whoIsOnline = new WhoIsOnline;
    killStats = new KillStats;
    socialWindow = new SocialWindow;
    if (serverVersion >= 6)
        questsWindow = new QuestsWindow;

    // TRANSLATORS: chat tab header
    localChatTab = new ChatTab(chatWindow, _("General"), GENERAL_CHANNEL);
    localChatTab->setAllowHighlight(false);
    if (config.getBoolValue("showChatHistory"))
        localChatTab->loadFromLogFile("#General");

    if (serverVersion >= 8 && serverConfig.getValue("enableLangTab", 1))
    {
        const std::string lang = getLangShort();
        if (lang.size() == 2)
        {
            langChatTab = new LangTab(chatWindow, lang);
            langChatTab->setAllowHighlight(false);
        }
    }

    // TRANSLATORS: chat tab header
    debugChatTab = new ChatTab(chatWindow, _("Debug"), "");
    debugChatTab->setAllowHighlight(false);

    if (config.getBoolValue("enableTradeTab"))
    {
        tradeChatTab = new TradeTab(chatWindow);
        tradeChatTab->setAllowHighlight(false);
    }
    else
    {
        tradeChatTab = nullptr;
    }

    if (config.getBoolValue("enableBattleTab"))
    {
        battleChatTab = new BattleTab(chatWindow);
        battleChatTab->setAllowHighlight(false);
    }
    else
    {
        battleChatTab = nullptr;
    }

    if (player_node && !gmChatTab && config.getBoolValue("enableGmTab")
        && player_node->getGMLevel() > 0)
    {
        gmChatTab = new GmTab(chatWindow);
    }

    if (config.getBoolValue("logToChat"))
        logger->setChatWindow(chatWindow);

    if (!isSafeMode && chatWindow)
        chatWindow->loadState();

    if (setupWindow)
        setupWindow->externalUpdate();

    if (player_node)
        player_node->updateStatus();

    Net::getGeneralHandler()->gameStarted();
}

#define del_0(X) { delete X; X = nullptr; }

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    Net::getGeneralHandler()->gameEnded();

    logger->setChatWindow(nullptr);
    if (whoIsOnline)
        whoIsOnline->setAllowUpdate(false);

    if (auctionManager)
        auctionManager->clear();

    if (guildManager)
        guildManager->clear();

    del_0(windowMenu);
    del_0(localChatTab)  // Need to do this first, so it can remove itself
    del_0(debugChatTab)
    del_0(tradeChatTab)
    del_0(battleChatTab)
    del_0(gmChatTab);
    logger->log("start deleting");
    del_0(chatWindow)
    logger->log("end deleting");
    del_0(statusWindow)
    del_0(miniStatusWindow)
    del_0(inventoryWindow)
    del_0(shopWindow)
    del_0(skillDialog)
    del_0(minimap)
    del_0(equipmentWindow)
    del_0(beingEquipmentWindow)
    del_0(tradeWindow)
    del_0(helpWindow)
    del_0(debugWindow)
    del_0(itemShortcutWindow)
    del_0(emoteShortcutWindow)
    del_0(outfitWindow)
#ifdef MANASERV_SUPPORT
    del_0(specialsWindow)
#endif
    del_0(socialWindow)
    del_0(dropShortcutWindow);
    del_0(spellShortcutWindow);
    del_0(botCheckerWindow);
    del_0(questsWindow);
    del_0(whoIsOnline);
    del_0(killStats);
    del_0(didYouKnowWindow);

    if (auctionManager && AuctionManager::getEnableAuctionBot())
        auctionManager->reload();

    if (guildManager && GuildManager::getEnableGuildBot())
        guildManager->reload();
}

Game *Game::mInstance = nullptr;

Game::Game():
    mLastTarget(ActorSprite::UNKNOWN),
    mCurrentMap(nullptr),
    mMapName(""),
    mValidSpeed(true),
    mLastAction(0),
    mNextAdjustTime(cur_time + adjustDelay),
    mAdjustLevel(0),
    mAdjustPerfomance(config.getBoolValue("adjustPerfomance")),
    mLowerCounter(0),
    mPing(0),
    mLogInput(config.getBoolValue("logInput")),
    mTime(cur_time + 1)
{
    touchManager.setInGame(true);
    spellManager = new SpellManager;
    spellShortcut = new SpellShortcut;

    assert(!mInstance);
    mInstance = this;

    config.incValue("gamecount");

    disconnectedDialog = nullptr;

    // Create the viewport
    viewport = new Viewport;
    viewport->setSize(mainGraphics->mWidth, mainGraphics->mHeight);
    PlayerInfo::clear();

    gcn::Container *const top = static_cast<gcn::Container*>(gui->getTop());
    if (top)
        top->add(viewport);
    viewport->requestMoveToBottom();

    AnimatedSprite::setEnableCache(mainGraphics->getOpenGL()
        && config.getBoolValue("enableDelayedAnimations"));

    CompoundSprite::setEnableDelay(
        config.getBoolValue("enableCompoundSpriteDelay"));

    createGuiWindows();
    windowMenu = new WindowMenu(nullptr);

    if (windowContainer)
        windowContainer->add(windowMenu);

    initEngines();

    // Initialize beings
    if (actorSpriteManager)
        actorSpriteManager->setPlayer(player_node);

    Net::getGameHandler()->ping(tick_time);

    if (setupWindow)
        setupWindow->setInGame(true);
    clearKeysArray();

    if (guildManager && GuildManager::getEnableGuildBot())
        guildManager->requestGuildInfo();

    if (player_node)
        player_node->updatePets();
}

Game::~Game()
{
    touchManager.setInGame(false);
    config.write();
    serverConfig.write();
    resetAdjustLevel();
    destroyGuiWindows();

    AnimatedSprite::setEnableCache(false);

    del_0(actorSpriteManager)
    if (Client::getState() != STATE_CHANGE_MAP)
        del_0(player_node)
    del_0(commandHandler)
    del_0(effectManager)
    del_0(particleEngine)
    del_0(viewport)
    del_0(mCurrentMap)
    del_0(spellManager)
    del_0(spellShortcut)
    del_0(auctionManager)
    del_0(guildManager)
#ifdef USE_MUMBLE
    del_0(mumbleManager)
#endif

    Being::clearCache();
    mInstance = nullptr;
    PlayerInfo::gameDestroyed();
}

bool Game::createScreenshot()
{
    SDL_Surface *screenshot = nullptr;

    if (!config.getBoolValue("showip"))
    {
        mainGraphics->setSecure(true);
        mainGraphics->prepareScreenshot();
        gui->draw();
        screenshot = mainGraphics->getScreenshot();
        mainGraphics->setSecure(false);
    }
    else
    {
        screenshot = mainGraphics->getScreenshot();
    }

    if (!screenshot)
        return false;

    return saveScreenshot(screenshot);
}

bool Game::saveScreenshot(SDL_Surface *const screenshot)
{
    std::string screenshotDirectory = Client::getScreenshotDirectory();

    if (mkdir_r(screenshotDirectory.c_str()) != 0)
    {
        logger->log("Directory %s doesn't exist and can't be created! "
                    "Setting screenshot directory to home.",
                    screenshotDirectory.c_str());
        screenshotDirectory = std::string(PhysFs::getUserDir());
    }

    // Search for an unused screenshot name
    std::stringstream filenameSuffix;
    std::stringstream filename;
    std::fstream testExists;
    bool found = false;
    static unsigned int screenshotCount = 0;
    do
    {
        screenshotCount++;
        filenameSuffix.str("");
        filename.str("");
        filename << screenshotDirectory << "/";
        filenameSuffix << branding.getValue("appName", "ManaPlus")
                       << "_Screenshot_" << screenshotCount << ".png";
        filename << filenameSuffix.str();
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    }
    while (!found);

    const bool success = ImageWriter::writePNG(screenshot, filename.str());

    if (success)
    {
        std::stringstream chatlogentry;
        // TRANSLATORS: save file message
        chatlogentry << strprintf(_("Screenshot saved as %s"),
            filenameSuffix.str().c_str());
        if (localChatTab)
            localChatTab->chatLog(chatlogentry.str(), BY_SERVER);
    }
    else
    {
        if (localChatTab)
        {
            // TRANSLATORS: save file message
            localChatTab->chatLog(_("Saving screenshot failed!"),
                                  BY_SERVER);
        }
        logger->log1("Error: could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);

    return success;
}

void Game::logic()
{
    BLOCK_START("Game::logic")
    handleInput();

    // Handle all necessary game logic
    ActorSprite::actorLogic();
    if (actorSpriteManager)
        actorSpriteManager->logic();
    if (particleEngine)
        particleEngine->update();
    if (mCurrentMap)
        mCurrentMap->update();

    cur_time = static_cast<int>(time(nullptr));
    BLOCK_END("Game::logic")
}

void Game::slowLogic()
{
    BLOCK_START("Game::slowLogic")
    if (player_node)
        player_node->slowLogic();
    const int time = cur_time;
    if (mTime <= time)
    {
        mTime = time + 1;
        if (botCheckerWindow)
            botCheckerWindow->slowLogic();
        if (debugWindow)
            debugWindow->slowLogic();
        if (killStats)
            killStats->update();
        if (socialWindow)
            socialWindow->slowLogic();
        if (whoIsOnline)
            whoIsOnline->slowLogic();
        Being::reReadConfig();
        if (killStats)
            killStats->recalcStats();
    }

    if (shopWindow)
        shopWindow->updateTimes();
    if (mainGraphics->getOpenGL())
        ResourceManager::delayedLoad();
    if (guildManager)
        guildManager->slowLogic();
    PacketCounters::update();

    // Handle network stuff
    if (!Net::getGameHandler()->isConnected())
    {
        if (Client::getState() == STATE_CHANGE_MAP)
            return;  // Not a problem here

        if (Client::getState() != STATE_ERROR)
        {
            if (!disconnectedDialog)
            {
                // TRANSLATORS: error message text
                errorMessage = _("The connection to the server was lost.");
                // TRANSLATORS: error message header
                disconnectedDialog = new OkDialog(_("Network Error"),
                    errorMessage, DIALOG_ERROR, false);
                disconnectedDialog->addActionListener(&errorListener);
                disconnectedDialog->requestMoveToTop();
            }
        }

        if (viewport && !errorMessage.empty())
        {
            const Map *const map = viewport->getCurrentMap();
            if (map)
            {
                logger->log("state: %d", Client::getState());
                map->saveExtraLayer();
            }
        }
        closeDialogs();
        Client::setFramerate(config.getIntValue("fpslimit"));
        mNextAdjustTime = cur_time + adjustDelay;
        if (Client::getState() != STATE_ERROR)
            errorMessage.clear();
    }
    else
    {
        if (Net::getGameHandler()->mustPing()
            && get_elapsed_time1(mPing) > 3000)
        {
            mPing = tick_time;
            Net::getGameHandler()->ping(tick_time);
        }

        if (mAdjustPerfomance)
            adjustPerfomance();
        if (disconnectedDialog)
        {
            disconnectedDialog->scheduleDelete();
            disconnectedDialog = nullptr;
        }
    }
    BLOCK_END("Game::slowLogic")
}

void Game::adjustPerfomance()
{
    FUNC_BLOCK("Game::adjustPerfomance", 1)
    const int time = cur_time;
    if (mNextAdjustTime <= adjustDelay)
    {
        mNextAdjustTime = time + adjustDelay;
    }
    else if (mNextAdjustTime < static_cast<unsigned>(time))
    {
        mNextAdjustTime = time + adjustDelay;

        if (mAdjustLevel > 3 || !player_node || player_node->getHalfAway()
            || player_node->getAway())
        {
            return;
        }

        int maxFps = Client::getFramerate();
        if (maxFps != config.getIntValue("fpslimit"))
            return;

        if (!maxFps)
            maxFps = 30;
        else if (maxFps < 10)
            return;

        if (fps < maxFps - 10)
        {
            if (mLowerCounter < 2)
            {
                mLowerCounter ++;
                mNextAdjustTime = cur_time + 1;
                return;
            }
            mLowerCounter = 0;
            mAdjustLevel ++;
            switch (mAdjustLevel)
            {
                case 1:
                {
                    if (config.getBoolValue("beingopacity"))
                    {
                        config.setValue("beingopacity", false);
                        config.setSilent("beingopacity", true);
                        if (localChatTab)
                        {
                            localChatTab->chatLog("Auto disable Show "
                                "beings transparency", BY_SERVER);
                        }
                    }
                    else
                    {
                        mNextAdjustTime = time + 1;
                        mLowerCounter = 2;
                    }
                    break;
                }
                case 2:
                    if (Particle::emitterSkip < 4)
                    {
                        Particle::emitterSkip = 4;
                        if (localChatTab)
                        {
                            localChatTab->chatLog("Auto lower Particle "
                                "effects", BY_SERVER);
                        }
                    }
                    else
                    {
                        mNextAdjustTime = time + 1;
                        mLowerCounter = 2;
                    }
                    break;
                case 3:
                    if (!config.getBoolValue("alphaCache"))
                    {
                        config.setValue("alphaCache", true);
                        config.setSilent("alphaCache", false);
                        if (localChatTab)
                        {
                            localChatTab->chatLog("Auto enable opacity cache",
                                BY_SERVER);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void Game::resetAdjustLevel()
{
    if (!mAdjustPerfomance)
        return;

    mNextAdjustTime = cur_time + adjustDelay;
    switch (mAdjustLevel)
    {
        case 1:
            config.setValue("beingopacity",
                config.getBoolValue("beingopacity"));
            break;
        case 2:
            config.setValue("beingopacity",
                config.getBoolValue("beingopacity"));
            Particle::emitterSkip = config.getIntValue(
                "particleEmitterSkip") + 1;
            break;
        default:
        case 3:
            config.setValue("beingopacity",
                config.getBoolValue("beingopacity"));
            Particle::emitterSkip = config.getIntValue(
                "particleEmitterSkip") + 1;
            config.setValue("alphaCache",
                config.getBoolValue("alphaCache"));
            break;
    }
    mAdjustLevel = 0;
}

void Game::handleMove()
{
    if (!player_node)
        return;

    // Moving player around
    if (player_node->isAlive() && !PlayerInfo::isTalking()
        && chatWindow && !chatWindow->isInputFocused()
        && !InventoryWindow::isAnyInputFocused() && !quitDialog)
    {
        // Get the state of the keyboard keys
        keyboard.refreshActiveKeys();

        // Ignore input if either "ignore" key is pressed
        // Stops the character moving about if the user's window manager
        // uses "ignore+arrow key" to switch virtual desktops.
        if (inputManager.isActionActive(Input::KEY_IGNORE_INPUT_1) ||
            inputManager.isActionActive(Input::KEY_IGNORE_INPUT_2))
        {
            return;
        }

        unsigned char direction = 0;

        // Translate pressed keys to movement and direction
        if (inputManager.isActionActive(Input::KEY_MOVE_UP) ||
            (joystick && joystick->isUp()))
        {
            direction |= Being::UP;
            setValidSpeed();
            player_node->cancelFollow();
        }
        else if (inputManager.isActionActive(Input::KEY_MOVE_DOWN) ||
                 (joystick && joystick->isDown()))
        {
            direction |= Being::DOWN;
            setValidSpeed();
            player_node->cancelFollow();
        }

        if (inputManager.isActionActive(Input::KEY_MOVE_LEFT) ||
            (joystick && joystick->isLeft()))
        {
            direction |= Being::LEFT;
            setValidSpeed();
            player_node->cancelFollow();
        }
        else if (inputManager.isActionActive(Input::KEY_MOVE_RIGHT) ||
                 (joystick && joystick->isRight()))
        {
            direction |= Being::RIGHT;
            setValidSpeed();
            player_node->cancelFollow();
        }

        if (!inputManager.isActionActive(Input::KEY_EMOTE) || direction == 0)
            moveInDirection(direction);
    }
}

void Game::moveInDirection(const unsigned char direction)
{
    if (!viewport)
        return;

    if (!viewport->getCameraMode())
    {
        if (player_node)
            player_node->specialMove(direction);
    }
    else
    {
        int dx = 0;
        int dy = 0;
        if (direction & Being::LEFT)
            dx = -5;
        else if (direction & Being::RIGHT)
            dx = 5;

        if (direction & Being::UP)
            dy = -5;
        else if (direction & Being::DOWN)
            dy = 5;
        viewport->moveCamera(dx, dy);
    }
}

void Game::handleActive(const SDL_Event &event)
{
//    logger->log("SDL_ACTIVEEVENT");
//    logger->log("state: %d", (int)event.active.state);
//    logger->log("gain: %d", (int)event.active.gain);

    int fpsLimit = 0;
    if (event.active.state & SDL_APPACTIVE)
    {
        if (event.active.gain)
        {   // window restore
            Client::setIsMinimized(false);
            if (player_node)
            {
                if (!player_node->getAway())
                    fpsLimit = config.getIntValue("fpslimit");
                player_node->setHalfAway(false);
            }
            setPriority(true);
        }
        else
        {   // window minimization
#ifdef ANDROID
            Client::setState(STATE_EXIT);
#else
            Client::setIsMinimized(true);
            if (player_node && !player_node->getAway())
            {
                fpsLimit = config.getIntValue("altfpslimit");
                player_node->setHalfAway(true);
            }
            setPriority(false);
#endif
        }
        if (player_node)
            player_node->updateStatus();
    }
    if (player_node)
        player_node->updateName();

    if (event.active.state & SDL_APPINPUTFOCUS)
        Client::setInputFocused(event.active.gain);
    if (event.active.state & SDL_APPMOUSEFOCUS)
        Client::setMouseFocused(event.active.gain);

    if (!fpsLimit)
    {
        if (player_node && player_node->getAway())
        {
            if (Client::getInputFocused() || Client::getMouseFocused())
                fpsLimit = config.getIntValue("fpslimit");
            else
                fpsLimit = config.getIntValue("altfpslimit");
        }
        else
        {
            fpsLimit = config.getIntValue("fpslimit");
        }
    }
    Client::setFramerate(fpsLimit);
    mNextAdjustTime = cur_time + adjustDelay;
}

/**
 * The huge input handling method.
 */
void Game::handleInput()
{
    BLOCK_START("Game::handleInput 1")
    if (joystick)
        joystick->logic();

    // Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        BLOCK_START("Game::handleInput 2")
        if (mLogInput)
            Client::logEvent(event);
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            updateHistory(event);
        checkKeys();

        if (inputManager.handleEvent(event))
        {
            BLOCK_END("Game::handleInput 2")
            BLOCK_END("Game::handleInput 1")
            return;
        }

        switch (event.type)
        {
            case SDL_VIDEORESIZE:
                // Let the client deal with this one (it'll
                // pass down from there)
                Client::resize(event.resize.w, event.resize.h);
                break;
            // Active event
            case SDL_ACTIVEEVENT:
                handleActive(event);
                break;
            // Quit event
            case SDL_QUIT:
                Client::setState(STATE_EXIT);
                break;
#ifdef ANDROID
            case SDL_KEYBOARDSHOW:
                Client::updateScreenKeyboard(event.user.code);
                break;
#endif
            default:
                break;
        }
        BLOCK_END("Game::handleInput 2")
    }  // End while

    // If the user is configuring the keys then don't respond.
    if (!player_node || !keyboard.isEnabled() || player_node->getAway())
    {
        BLOCK_END("Game::handleInput 1")
        return;
    }

    // If pressed outfits keys, stop processing keys.
    if (inputManager.isActionActive(Input::KEY_WEAR_OUTFIT)
        || inputManager.isActionActive(Input::KEY_COPY_OUTFIT)
        || (setupWindow && setupWindow->isWindowVisible()))
    {
        BLOCK_END("Game::handleInput 1")
        return;
    }

    handleMove();
    inputManager.handleRepeat();
    BLOCK_END("Game::handleInput 1")
}

/**
 * Changes the currently active map. Should only be called while the game is
 * running.
 */
void Game::changeMap(const std::string &mapPath)
{
    resetAdjustLevel();

    ResourceManager *const resman = ResourceManager::getInstance();
    resman->cleanProtected();

    if (viewport)
        viewport->clearPopup();

    // Clean up floor items, beings and particles
    if (actorSpriteManager)
        actorSpriteManager->clear();

    // Close the popup menu on map change so that invalid options can't be
    // executed.
    if (viewport)
    {
        viewport->closePopupMenu();
        viewport->cleanHoverItems();
    }

    // Unset the map of the player so that its particles are cleared before
    // being deleted in the next step
    if (player_node)
        player_node->setMap(nullptr);

    if (particleEngine)
        particleEngine->clear();

    mMapName = mapPath;

    std::string fullMap = paths.getValue("maps", "maps/").append(
        mMapName).append(".tmx");
    std::string realFullMap = paths.getValue("maps", "maps/").append(
        MapDB::getMapName(mMapName)).append(".tmx");

    if (!resman->exists(realFullMap))
        realFullMap.append(".gz");

    // Attempt to load the new map
    Map *const newMap = MapReader::readMap(fullMap, realFullMap);

    if (mCurrentMap)
        mCurrentMap->saveExtraLayer();

    if (newMap)
        newMap->addExtraLayer();

    if (socialWindow)
        socialWindow->setMap(newMap);

    // Notify the minimap and actorSpriteManager about the map change
    if (minimap)
        minimap->setMap(newMap);
    if (actorSpriteManager)
        actorSpriteManager->setMap(newMap);
    if (particleEngine)
        particleEngine->setMap(newMap);
    if (viewport)
        viewport->setMap(newMap);

    // Initialize map-based particle effects
    if (newMap)
        newMap->initializeParticleEffects(particleEngine);

    // Start playing new music file when necessary
    const std::string oldMusic = mCurrentMap
        ? mCurrentMap->getMusicFile() : "";
    const std::string newMusic = newMap ? newMap->getMusicFile() : "";
    if (newMusic != oldMusic)
    {
        if (newMusic.empty())
            soundManager.fadeOutMusic();
        else
            soundManager.fadeOutAndPlayMusic(newMusic);
    }

    if (mCurrentMap)
        mCurrentMap->saveExtraLayer();

    delete mCurrentMap;
    mCurrentMap = newMap;

    if (questsWindow)
        questsWindow->setMap(mCurrentMap);

#ifdef USE_MUMBLE
    if (mumbleManager)
        mumbleManager->setMap(mapPath);
#endif
    Net::getGameHandler()->mapLoadedEvent();
}

void Game::updateHistory(const SDL_Event &event)
{
    if (!player_node || !player_node->getAttackType())
        return;

    if (event.key.keysym.sym != -1)
    {
        bool old = false;

        const int key = keyboard.getKeyIndex(event);
        const int time = cur_time;
        int idx = -1;
        for (int f = 0; f < MAX_LASTKEYS; f ++)
        {
            LastKey &lastKey = mLastKeys[f];
            if (lastKey.key == key)
            {
                idx = f;
                old = true;
                break;
            }
            else if (idx >= 0 && lastKey.time < mLastKeys[idx].time)
            {
                idx = f;
            }
        }
        if (idx < 0)
        {
            idx = 0;
            for (int f = 0; f < MAX_LASTKEYS; f ++)
            {
                LastKey &lastKey = mLastKeys[f];
                if (lastKey.key == -1 ||  lastKey.time < mLastKeys[idx].time)
                    idx = f;
            }
        }

        if (idx < 0)
            idx = 0;

        LastKey &keyIdx = mLastKeys[idx];
        if (!old)
        {
            keyIdx.time = time;
            keyIdx.key = key;
            keyIdx.cnt = 0;
        }
        else
        {
            keyIdx.cnt++;
        }
    }
}

void Game::checkKeys()
{
    const int timeRange = 120;
    const int cntInTime = 130;

    if (!player_node || !player_node->getAttackType())
        return;

    const int time = cur_time;
    for (int f = 0; f < MAX_LASTKEYS; f ++)
    {
        LastKey &lastKey = mLastKeys[f];
        if (lastKey.key != -1)
        {
            if (lastKey.time + timeRange < time)
            {
                if (lastKey.cnt > cntInTime)
                    mValidSpeed = false;
                lastKey.key = -1;
            }
        }
    }
}

void Game::setValidSpeed()
{
    clearKeysArray();
    mValidSpeed = true;
}

void Game::clearKeysArray()
{
    for (int f = 0; f < MAX_LASTKEYS; f ++)
    {
        mLastKeys[f].time = 0;
        mLastKeys[f].key = -1;
        mLastKeys[f].cnt = 0;
    }
}

void Game::closeDialogs()
{
    Client::closeDialogs();
    if (deathNotice)
    {
        deathNotice->scheduleDelete();
        deathNotice = nullptr;
    }
}

void Game::videoResized(const int width, const int height) const
{
    if (viewport)
        viewport->setSize(width, height);
    if (windowMenu)
        windowMenu->setPosition(width - windowMenu->getWidth(), 0);
}
