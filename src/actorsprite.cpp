/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "actorsprite.h"

#include "actorspritelistener.h"
#include "client.h"
#include "configuration.h"
#include "effectmanager.h"
#include "imagesprite.h"
#include "localplayer.h"
#include "simpleanimation.h"
#include "soundmanager.h"
#include "statuseffect.h"

#include "gui/theme.h"

#include "net/net.h"

#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include "utils/checkutils.h"

#include "debug.h"

ImageSet *ActorSprite::targetCursorImages[2][NUM_TC];
SimpleAnimation *ActorSprite::targetCursor[2][NUM_TC];
bool ActorSprite::loaded = false;

ActorSprite::ActorSprite(const int id) :
    CompoundSprite(),
    Actor(),
    mId(id),
    mStunMode(0),
    mStatusEffects(),
    mStunParticleEffects(),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false),
    mMustResetParticles(false),
    mUsedTargetCursor(nullptr),
    mActorSpriteListeners()
{
}

ActorSprite::~ActorSprite()
{
    setMap(nullptr);

    mUsedTargetCursor = nullptr;

    if (player_node && player_node->getTarget() == this)
        player_node->setTarget(nullptr);

    // Notify listeners of the destruction.
    FOR_EACH (ActorSpriteListenerIterator, iter, mActorSpriteListeners)
    {
        if (reportFalse(*iter))
            (*iter)->actorSpriteDestroyed(*this);
    }
}

bool ActorSprite::draw(Graphics *const graphics,
                       const int offsetX, const int offsetY) const
{
    FUNC_BLOCK("ActorSprite::draw", 1)
    const int px = getPixelX() + offsetX - 16;
    // Temporary fix to the Y offset.
#ifdef MANASERV_SUPPORT
    const int py = getPixelY() + offsetY -
        ((Net::getNetworkType() == ServerInfo::MANASERV) ? 15 : 32);
#else
    const int py = getPixelY() + offsetY - 32;
#endif

    if (mUsedTargetCursor)
    {
        mUsedTargetCursor->reset();
        mUsedTargetCursor->update(tick_time * MILLISECONDS_IN_A_TICK);
        mUsedTargetCursor->draw(graphics, px + getTargetOffsetX(),
            py + getTargetOffsetY());
    }

    return drawSpriteAt(graphics, px, py);
}

bool ActorSprite::drawSpriteAt(Graphics *const graphics,
                               const int x, const int y) const
{
    return CompoundSprite::draw(graphics, x, y);
}

void ActorSprite::logic()
{
    BLOCK_START("ActorSprite::logic")
    // Update sprite animations
    update(tick_time * MILLISECONDS_IN_A_TICK);

    // Restart status/particle effects, if needed
    if (mMustResetParticles)
    {
        mMustResetParticles = false;
        FOR_EACH (std::set<int>::const_iterator, it, mStatusEffects)
        {
            const StatusEffect *const effect
                = StatusEffect::getStatusEffect(*it, true);
            if (effect && effect->particleEffectIsPersistent())
                updateStatusEffect(*it, true);
        }
    }

    // Update particle effects
    mChildParticleEffects.moveTo(mPos.x, mPos.y);
    BLOCK_END("ActorSprite::logic")
}

void ActorSprite::actorLogic()
{
}

void ActorSprite::setMap(Map *const map)
{
    Actor::setMap(map);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
    mMustResetParticles = true;  // Reset status particles on next redraw
}

void ActorSprite::controlParticle(Particle *const particle)
{
    mChildParticleEffects.addLocally(particle);
}

void ActorSprite::setTargetType(const TargetCursorType type)
{
    if (type == TCT_NONE)
        untarget();
    else
        mUsedTargetCursor = targetCursor[type][getTargetCursorSize()];
}

struct EffectDescription final
{
    std::string mGFXEffect;
    std::string mSFXEffect;
};

void ActorSprite::setStatusEffect(const int index, const bool active)
{
    const bool wasActive = mStatusEffects.find(index) != mStatusEffects.end();

    if (active != wasActive)
    {
        updateStatusEffect(index, active);
        if (active)
            mStatusEffects.insert(index);
        else
            mStatusEffects.erase(index);
    }
}

void ActorSprite::setStatusEffectBlock(const int offset,
                                       const uint16_t newEffects)
{
    for (unsigned i = 0; i < STATUS_EFFECTS; i++)
    {
        const int index = StatusEffect::blockEffectIndexToEffectIndex(
            offset + i);

        if (index != -1)
            setStatusEffect(index, (newEffects & (1 << i)) > 0);
    }
}

void ActorSprite::updateStunMode(const int oldMode, const int newMode)
{
    handleStatusEffect(StatusEffect::getStatusEffect(oldMode, false), -1);
    handleStatusEffect(StatusEffect::getStatusEffect(newMode, true), -1);
}

void ActorSprite::updateStatusEffect(const int index, const bool newStatus)
{
    handleStatusEffect(StatusEffect::getStatusEffect(index, newStatus), index);
}

void ActorSprite::handleStatusEffect(StatusEffect *const effect,
                                     const int effectId)
{
    if (!effect)
        return;

    Particle *const particle = effect->getParticle();

    if (effectId >= 0)
    {
        mStatusParticleEffects.setLocally(effectId, particle);
    }
    else
    {
        mStunParticleEffects.clearLocally();
        if (particle)
            mStunParticleEffects.addLocally(particle);
    }
}

void ActorSprite::setupSpriteDisplay(const SpriteDisplay &display,
                                     const bool forceDisplay,
                                     const int imageType,
                                     const std::string &color)
{
    clear();

    FOR_EACH (SpriteRefs, it, display.sprites)
    {
        if (!*it)
            continue;
        const std::string file = paths.getStringValue("sprites").append(
            combineDye2((*it)->sprite, color));

        const int variant = (*it)->variant;
        addSprite(AnimatedSprite::delayedLoad(file, variant));
    }

    // Ensure that something is shown, if desired
    if (empty() && forceDisplay)
    {
        if (display.image.empty())
        {
            addSprite(AnimatedSprite::delayedLoad(
                paths.getStringValue("sprites").append(
                paths.getStringValue("spriteErrorFile"))));
        }
        else
        {
            ResourceManager *const resman = ResourceManager::getInstance();
            std::string imagePath;
            switch (imageType)
            {
                case 0:
                default:
                    imagePath = paths.getStringValue("itemIcons").append(
                        display.image);
                    break;
                case 1:
                    imagePath = paths.getStringValue("itemIcons").append(
                        display.floor);
                    break;
            }
            imagePath = combineDye2(imagePath, color);

            Image *img = resman->getImage(imagePath);

            if (!img)
                img = Theme::getImageFromTheme("unknown-item.png");

            addSprite(new ImageSprite(img));
            if (img)
                img->decRef();
        }
    }

    mChildParticleEffects.clear();

    // setup particle effects
    if (Particle::enabled && particleEngine)
    {
        FOR_EACH (StringVectCIter, itr, display.particles)
        {
            Particle *const p = particleEngine->addEffect(*itr, 0, 0);
            controlParticle(p);
        }
    }

    mMustResetParticles = true;
}

void ActorSprite::load()
{
    if (loaded)
        unload();

    initTargetCursor();

    loaded = true;
}

void ActorSprite::unload()
{
    if (reportTrue(!loaded))
        return;

    cleanupTargetCursors();
    loaded = false;
}

void ActorSprite::addActorSpriteListener(ActorSpriteListener *const listener)
{
    mActorSpriteListeners.push_front(listener);
}

void ActorSprite::removeActorSpriteListener(ActorSpriteListener *const
                                            listener)
{
    mActorSpriteListeners.remove(listener);
}

static const char *cursorType(const int type)
{
    switch (type)
    {
        case ActorSprite::TCT_IN_RANGE:
            return "in-range";
        default:
        case ActorSprite::TCT_NORMAL:
            return "normal";
    }
}

static const char *cursorSize(const int size)
{
    switch (size)
    {
        case ActorSprite::TC_LARGE:
            return "l";
        case ActorSprite::TC_MEDIUM:
            return "m";
        default:
        case ActorSprite::TC_SMALL:
            return "s";
    }
}

void ActorSprite::initTargetCursor()
{
    static const std::string targetCursorFile = "target-cursor-%s-%s.png";
    static const int targetWidths[NUM_TC] = {44, 62, 82};
    static const int targetHeights[NUM_TC] = {35, 44, 60};

    // Load target cursors
    for (int size = TC_SMALL; size < NUM_TC; size++)
    {
        for (int type = TCT_NORMAL; type < NUM_TCT; type++)
        {
            loadTargetCursor(strprintf(targetCursorFile.c_str(),
                cursorType(type), cursorSize(size)), targetWidths[size],
                targetHeights[size], type, size);
        }
    }
}

void ActorSprite::cleanupTargetCursors()
{
    for (int size = TC_SMALL; size < NUM_TC; size++)
    {
        for (int type = TCT_NORMAL; type < NUM_TCT; type++)
        {
            if (targetCursor[type][size])
            {
                delete targetCursor[type][size];
                targetCursor[type][size] = nullptr;
            }
            if (targetCursorImages[type][size])
            {
                targetCursorImages[type][size]->decRef();
                targetCursorImages[type][size] = nullptr;
            }
        }
    }
}

void ActorSprite::loadTargetCursor(const std::string &filename,
                                   const int width, const int height,
                                   const int type, const int size)
{
    if (reportTrue(size < TC_SMALL || size >= NUM_TC))
        return;

    ImageSet *const currentImageSet = Theme::getImageSetFromTheme(
        filename, width, height);

    if (!currentImageSet)
    {
        logger->log("Error loading target cursor: %s", filename.c_str());
        return;
    }

    Animation *const anim = new Animation;

    for (size_t i = 0; i < currentImageSet->size(); ++i)
    {
        anim->addFrame(currentImageSet->get(i), 75,
            (16 - (currentImageSet->getWidth() / 2)),
            (16 - (currentImageSet->getHeight() / 2)),
            100);
    }

    SimpleAnimation *const currentCursor = new SimpleAnimation(anim);

    if (targetCursor[type][size])
    {
        delete targetCursor[type][size];
        targetCursor[type][size] = nullptr;
        if (targetCursorImages[type][size])
            targetCursorImages[type][size]->decRef();
    }

    targetCursorImages[type][size] = currentImageSet;
    targetCursor[type][size] = currentCursor;
}
