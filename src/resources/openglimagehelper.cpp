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

#include "resources/openglimagehelper.h"

#ifdef USE_OPENGL

#include "client.h"
#include "game.h"
#include "graphicsmanager.h"
#include "logger.h"
#include "mgl.h"
#include "mobileopenglgraphics.h"
#include "normalopenglgraphics.h"
#include "safeopenglgraphics.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include "debug.h"

int OpenGLImageHelper::mTextureType = 0;
int OpenGLImageHelper::mInternalTextureType = GL_RGBA8;
int OpenGLImageHelper::mTextureSize = 0;
bool OpenGLImageHelper::mBlur = true;
int OpenGLImageHelper::mUseOpenGL = 0;
bool OpenGLImageHelper::mUseTextureSampler = false;

Image *OpenGLImageHelper::load(SDL_RWops *const rw, Dye const &dye) const
{
    SDL_Surface *const tmpImage = loadPng(rw);
    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return nullptr;
    }

    SDL_Surface *const surf = convertTo32Bit(tmpImage);
    SDL_FreeSurface(tmpImage);

    uint32_t *pixels = static_cast<uint32_t *>(surf->pixels);
    const int type = dye.getType();

    switch (type)
    {
        case 1:
        {
            DyePalette *const pal = dye.getSPalete();

            if (pal)
            {
                for (uint32_t *p_end = pixels + surf->w * surf->h;
                    pixels != p_end; ++pixels)
                {
                    uint8_t *p = reinterpret_cast<uint8_t *>(pixels);
                    const int alpha = *p & 255;
                    if (!alpha)
                        continue;
                    pal->replaceSOGLColor(p);
                }
            }
            break;
        }
        case 2:
        {
            DyePalette *const pal = dye.getAPalete();
            if (pal)
            {
                for (uint32_t *p_end = pixels + surf->w * surf->h;
                    pixels != p_end; ++pixels)
                {
                    pal->replaceAOGLColor(reinterpret_cast<uint8_t *>(pixels));
                }
            }
            break;
        }
        case 0:
        default:
        {
            for (uint32_t *p_end = pixels + surf->w * surf->h;
                 pixels != p_end; ++pixels)
            {
                const uint32_t p = *pixels;
                const int alpha = (p >> 24) & 255;
                if (!alpha)
                    continue;
                int v[3];
                v[0] = (p) & 255;
                v[1] = (p >> 8) & 255;
                v[2] = (p >> 16) & 255;
                dye.update(v);
                *pixels = (v[0]) | (v[1] << 8) | (v[2] << 16) | (alpha << 24);
            }
            break;
        }
    }

    Image *const image = load(surf);
    SDL_FreeSurface(surf);
    return image;
}

Image *OpenGLImageHelper::load(SDL_Surface *const tmpImage) const
{
    return glLoad(tmpImage);
}

Image *OpenGLImageHelper::createTextSurface(SDL_Surface *const tmpImage,
                                            const int width, const int height,
                                            const float alpha) const
{
    if (!tmpImage)
        return nullptr;

    Image *const img = glLoad(tmpImage, width, height);
    if (img)
        img->setAlpha(alpha);
    return img;
}

int OpenGLImageHelper::powerOfTwo(const int input) const
{
    int value;
    if (mTextureType == GL_TEXTURE_2D)
    {
        value = 1;
        while (value < input && value < mTextureSize)
            value <<= 1;
    }
    else
    {
        value = input;
    }
    return value >= mTextureSize ? mTextureSize : value;
}

Image *OpenGLImageHelper::glLoad(SDL_Surface *tmpImage,
                                 int width, int height) const
{
    if (!tmpImage)
        return nullptr;

    // Flush current error flag.
    glGetError();

    if (!width)
        width = tmpImage->w;
    if (!height)
        height = tmpImage->h;
    int realWidth = powerOfTwo(width);
    int realHeight = powerOfTwo(height);

    if (realWidth < width || realHeight < height)
    {
        logger->log("Warning: image too large, cropping to %dx%d texture!",
                    tmpImage->w, tmpImage->h);
    }

    // Make sure the alpha channel is not used, but copied to destination
    SDL_SetAlpha(tmpImage, 0, SDL_ALPHA_OPAQUE);

    // Determine 32-bit masks based on byte order
    uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_Surface *oldImage = nullptr;
    if (tmpImage->format->BitsPerPixel != 32
        || realWidth != width || realHeight != height
        || rmask != tmpImage->format->Rmask
        || gmask != tmpImage->format->Gmask
        || amask != tmpImage->format->Amask)
    {
        oldImage = tmpImage;
        tmpImage = SDL_CreateRGBSurface(SDL_SWSURFACE, realWidth, realHeight,
            32, rmask, gmask, bmask, amask);

        if (!tmpImage)
        {
            logger->log("Error, image convert failed: out of memory");
            return nullptr;
        }
        SDL_BlitSurface(oldImage, nullptr, tmpImage, nullptr);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    switch (mUseOpenGL)
    {
#ifndef ANDROID
        case 1:
            NormalOpenGLGraphics::bindTexture(mTextureType, texture);
            break;
        case 2:
            SafeOpenGLGraphics::bindTexture(mTextureType, texture);
            break;
#else
        case 1:
        case 2:
#endif
        case 3:
            MobileOpenGLGraphics::bindTexture(mTextureType, texture);
            break;
        default:
            logger->log("Unknown OpenGL backend: %d", mUseOpenGL);
            break;
    }

    if (SDL_MUSTLOCK(tmpImage))
        SDL_LockSurface(tmpImage);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (!mUseTextureSampler)
    {
        if (mBlur)
        {
            glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }
#ifndef ANDROID
    glTexParameteri(mTextureType, GL_TEXTURE_MAX_LEVEL, 0);
#endif

    glTexImage2D(mTextureType, 0, mInternalTextureType,
        tmpImage->w, tmpImage->h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, tmpImage->pixels);

/*
    GLint compressed;
    glGetTexLevelParameteriv(mTextureType, 0,
        GL_TEXTURE_COMPRESSED_ARB, &compressed);
    if (compressed)
        logger->log("image compressed");
    else
        logger->log("image not compressed");
*/

#ifdef DEBUG_OPENGL_LEAKS
    textures_count ++;
#endif

    if (SDL_MUSTLOCK(tmpImage))
        SDL_UnlockSurface(tmpImage);

    if (oldImage)
        SDL_FreeSurface(tmpImage);

    GLenum error = glGetError();
    if (error)
    {
        std::string errmsg = GraphicsManager::errorToString(error);
        logger->log("Error: Image GL import failed: %s (%d)",
            errmsg.c_str(), error);
        return nullptr;
    }

    return new Image(texture, width, height, realWidth, realHeight);
}

void OpenGLImageHelper::setLoadAsOpenGL(const int useOpenGL)
{
    OpenGLImageHelper::mUseOpenGL = useOpenGL;
}

int OpenGLImageHelper::useOpenGL() const
{
    return mUseOpenGL;
}

void OpenGLImageHelper::initTextureSampler(const GLint id)
{
    if (mBlur)
    {
        mglSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        mglSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        mglSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        mglSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

SDL_Surface *OpenGLImageHelper::create32BitSurface(int width, int height) const
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const int rmask = 0xff000000;
    const int gmask = 0x00ff0000;
    const int bmask = 0x0000ff00;
    const int amask = 0x000000ff;
#else
    const int rmask = 0x000000ff;
    const int gmask = 0x0000ff00;
    const int bmask = 0x00ff0000;
    const int amask = 0xff000000;
#endif

    width = powerOfTwo(width);
    height = powerOfTwo(height);

    return SDL_CreateRGBSurface(SDL_SWSURFACE,
        width, height, 32, rmask, gmask, bmask, amask);
}

#endif
