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

#include "gui/setup_video.h"

#include "client.h"
#include "configuration.h"
#include "game.h"

#include "graphicsmanager.h"

#include "localplayer.h"

#include "gui/gui.h"
#include "gui/okdialog.h"
#include "gui/textdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/sliderlist.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/dropdown.h"

#include "resources/image.h"

#include "utils/gettext.h"

#include "test/testmain.h"

#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include <SDL.h>

#include <algorithm>

#include "debug.h"

extern Graphics *mainGraphics;

/**
 * The list model for mode list.
 *
 * \ingroup Interface
 */
class ModeListModel final : public gcn::ListModel
{
    public:
        /**
         * Constructor.
         */
        ModeListModel();

        A_DELETE_COPY(ModeListModel)

        /**
         * Destructor.
         */
        virtual ~ModeListModel()
        { }

        /**
         * Returns the number of elements in container.
         */
        int getNumberOfElements()
        { return static_cast<int>(mVideoModes.size()); }

        /**
         * Returns element from container.
         */
        std::string getElementAt(int i)
        { return mVideoModes[i]; }

        /**
         * Returns the index corresponding to the given video mode.
         * E.g.: "800x600".
         * or -1 if not found.
         */
        int getIndexOf(const std::string &widthXHeightMode);

    private:
        void addCustomMode(std::string mode);

        StringVect mVideoModes;
};

#ifndef ANDROID
static bool modeSorter(std::string mode1, std::string mode2);

static bool modeSorter(std::string mode1, std::string mode2)
{
    const int width1 = atoi(mode1.substr(0, mode1.find("x")).c_str());
    const int height1 = atoi(mode1.substr(mode1.find("x") + 1).c_str());
    if (!width1 || !height1)
        return false;

    const int width2 = atoi(mode2.substr(0, mode2.find("x")).c_str());
    const int height2 = atoi(mode2.substr(mode2.find("x") + 1).c_str());
    if (!width2 || !height2)
        return false;
    if (width1 != width2)
        return width1 < width2;

    if (height1 != height2)
        return height1 < height2;

    return false;
}
#endif

ModeListModel::ModeListModel() :
    mVideoModes()
{
    graphicsManager.getAllVideoModes(mVideoModes);
#ifndef ANDROID
    addCustomMode("640x480");
    addCustomMode("800x600");
    addCustomMode("1024x768");
    addCustomMode("1280x1024");
    addCustomMode("1400x900");
    addCustomMode("1500x990");
    addCustomMode(toString(mainGraphics->mWidth).append("x")
        .append(toString(mainGraphics->mHeight)));

    std::sort(mVideoModes.begin(), mVideoModes.end(), modeSorter);
    mVideoModes.push_back("custom");
#endif
}

void ModeListModel::addCustomMode(std::string mode)
{
    StringVectCIter it = mVideoModes.begin();
    const StringVectCIter it_end = mVideoModes.end();
    while (it != it_end)
    {
        if (*it == mode)
            return;
        ++ it;
    }
    mVideoModes.push_back(mode);
}

int ModeListModel::getIndexOf(const std::string &widthXHeightMode)
{
    std::string currentMode("");
    for (int i = 0; i < getNumberOfElements(); i++)
    {
        currentMode = getElementAt(i);
        if (currentMode == widthXHeightMode)
            return i;
    }
    return -1;
}

const char *OPENGL_NAME[4] =
{
    // TRANSLATORS: draw backend
    N_("Software"),
#ifndef ANDROID
    // TRANSLATORS: draw backend
    N_("Fast OpenGL"),
    // TRANSLATORS: draw backend
    N_("Safe OpenGL"),
#endif
    // TRANSLATORS: draw backend
    N_("Mobile OpenGL"),
};

class OpenGLListModel final : public gcn::ListModel
{
public:
    virtual ~OpenGLListModel()
    { }

    virtual int getNumberOfElements()
#ifdef ANDROID
    { return 2; }
#else
    { return 4; }
#endif

    virtual std::string getElementAt(int i)
    {
        if (i >= getNumberOfElements() || i < 0)
            return "???";
        return gettext(OPENGL_NAME[i]);
    }
};

Setup_Video::Setup_Video(const Widget2 *const widget) :
    SetupTab(widget),
    gcn::KeyListener(),
    mFullScreenEnabled(config.getBoolValue("screen")),
    mOpenGLEnabled(config.getIntValue("opengl")),
    mCustomCursorEnabled(config.getBoolValue("customcursor")),
    mFps(config.getIntValue("fpslimit")),
    mAltFps(config.getIntValue("altfpslimit")),
    mEnableResize(config.getBoolValue("enableresize")),
    mNoFrame(config.getBoolValue("noframe")),
    mModeListModel(new ModeListModel),
    mOpenGLListModel(new OpenGLListModel),
    mModeList(new ListBox(widget, mModeListModel, "")),
    // TRANSLATORS: video settings checkbox
    mFsCheckBox(new CheckBox(this, _("Full screen"), mFullScreenEnabled)),
    mOpenGLDropDown(new DropDown(widget, mOpenGLListModel)),
    mCustomCursorCheckBox(new CheckBox(this,
#ifdef ANDROID
        // TRANSLATORS: video settings checkbox
        _("Show cursor"),
#else
        // TRANSLATORS: video settings checkbox
        _("Custom cursor"),
#endif
                          mCustomCursorEnabled)),
    // TRANSLATORS: video settings checkbox
    mEnableResizeCheckBox(new CheckBox(this, _("Enable resize"),
                          mEnableResize)),
    // TRANSLATORS: video settings checkbox
    mNoFrameCheckBox(new CheckBox(this, _("No frame"), mNoFrame)),
    // TRANSLATORS: video settings checkbox
    mFpsCheckBox(new CheckBox(this, _("FPS limit:"))),
    mFpsSlider(new Slider(2, 160)),
    mFpsLabel(new Label(this)),
    mAltFpsSlider(new Slider(2, 160)),
    // TRANSLATORS: video settings label
    mAltFpsLabel(new Label(this, _("Alt FPS limit: "))),
#if !defined(ANDROID) && !defined(__APPLE__)
    // TRANSLATORS: video settings button
    mDetectButton(new Button(this, _("Detect best mode"), "detect", this)),
#endif
    mDialog(nullptr)
{
    // TRANSLATORS: video settings tab name
    setName(_("Video"));

    ScrollArea *const scrollArea = new ScrollArea(mModeList,
        true, "setup_video_background.xml");
    scrollArea->setWidth(150);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

#ifdef USE_OPENGL
#ifdef ANDROID
    if (mOpenGLEnabled == 1 || mOpenGLEnabled == 2)
        mOpenGLEnabled = 3;
#endif
    mOpenGLDropDown->setSelected(mOpenGLEnabled);
#else
    mOpenGLDropDown->setSelected(0);
#endif

    mModeList->setEnabled(true);

    // TRANSLATORS: video settings label
    mFpsLabel->setCaption(mFps > 0 ? toString(mFps) : _("None"));
    mFpsLabel->setWidth(60);
    // TRANSLATORS: video settings label
    mAltFpsLabel->setCaption(_("Alt FPS limit: ") + (mAltFps > 0
                             ? toString(mAltFps) : _("None")));
    mAltFpsLabel->setWidth(150);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mAltFpsSlider->setValue(mAltFps);
    mAltFpsSlider->setEnabled(mAltFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    // Pre-select the current video mode.
    std::string videoMode = toString(mainGraphics->mWidth).append("x")
        .append(toString(mainGraphics->mHeight));
    mModeList->setSelected(mModeListModel->getIndexOf(videoMode));

    mModeList->setActionEventId("videomode");
    mCustomCursorCheckBox->setActionEventId("customcursor");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mFpsSlider->setActionEventId("fpslimitslider");
    mAltFpsSlider->setActionEventId("altfpslimitslider");
    mOpenGLDropDown->setActionEventId("opengl");
    mEnableResizeCheckBox->setActionEventId("enableresize");
    mNoFrameCheckBox->setActionEventId("noframe");

    mModeList->addActionListener(this);
    mCustomCursorCheckBox->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mAltFpsSlider->addActionListener(this);
    mOpenGLDropDown->addActionListener(this);
    mEnableResizeCheckBox->addActionListener(this);
    mNoFrameCheckBox->addActionListener(this);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, scrollArea, 1, 5).setPadding(2);
    place(0, 5, mOpenGLDropDown, 1);

    place(1, 0, mFsCheckBox, 2);

    place(1, 1, mCustomCursorCheckBox, 3);

    place(1, 2, mEnableResizeCheckBox, 2);
    place(1, 3, mNoFrameCheckBox, 2);

    place(0, 6, mFpsSlider);
    place(1, 6, mFpsCheckBox).setPadding(3);
    place(2, 6, mFpsLabel).setPadding(1);

    place(0, 7, mAltFpsSlider);
    place(1, 7, mAltFpsLabel).setPadding(3);

#if !defined(ANDROID) && !defined(__APPLE__)
    place(0, 8, mDetectButton);
#else
    mNoFrameCheckBox->setEnabled(false);
    mEnableResizeCheckBox->setEnabled(false);
    mFsCheckBox->setEnabled(false);
#endif

    int width = 600;

    if (config.getIntValue("screenwidth") >= 730)
        width += 100;

    setDimension(gcn::Rectangle(0, 0, width, 300));
}

Setup_Video::~Setup_Video()
{
    delete mModeListModel;
    mModeListModel = nullptr;
    delete mModeList;
    mModeList = nullptr;
    delete mOpenGLListModel;
    mOpenGLListModel = nullptr;
    delete mDialog;
    mDialog = nullptr;
}

void Setup_Video::apply()
{
    // Full screen changes
    bool fullscreen = mFsCheckBox->isSelected();
    if (fullscreen != config.getBoolValue("screen"))
    {
        /* The OpenGL test is only necessary on Windows, since switching
         * to/from full screen works fine on Linux. On Windows we'd have to
         * reinitialize the OpenGL state and reload all textures.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#if defined(WIN32) || defined(__APPLE__) || defined(ANDROID)
        // checks for opengl usage
        if (!config.getIntValue("opengl"))
        {
#endif
            if (!mainGraphics->setFullscreen(fullscreen))
            {
                fullscreen = !fullscreen;
                if (!mainGraphics->setFullscreen(fullscreen))
                {
                    std::stringstream errorMsg;
                    if (fullscreen)
                    {
                        // TRANSLATORS: video error message
                        errorMsg << _("Failed to switch to windowed mode "
                            "and restoration of old mode also "
                            "failed!") << std::endl;
                    }
                    else
                    {
                        // TRANSLATORS: video error message
                        errorMsg << _("Failed to switch to fullscreen mode"
                            " and restoration of old mode also "
                            "failed!") << std::endl;
                    }
                    logger->safeError(errorMsg.str());
                }
            }
#if defined(WIN32) || defined(__APPLE__) || defined(ANDROID)
        }
        else
        {
            // TRANSLATORS: video settings warning
            new OkDialog(_("Switching to Full Screen"),
                // TRANSLATORS: video settings warning
                _("Restart needed for changes to take effect."));
        }
#endif
        config.setValue("screen", fullscreen);
    }

    int mode = mOpenGLDropDown->getSelected();
#ifdef ANDROID
    if (mode == 1 || mode == 2)
        mode = 3;
#endif

    // OpenGL change
    if (mode != mOpenGLEnabled)
    {
        config.setValue("opengl", mode);

        // OpenGL can currently only be changed by restarting, notify user.
        // TRANSLATORS: video settings warning
        new OkDialog(_("Changing to OpenGL"),
            // TRANSLATORS: video settings warning
            _("Applying change to OpenGL requires restart."));
    }

    mFps = mFpsCheckBox->isSelected() ?
        static_cast<int>(mFpsSlider->getValue()) : 0;

    mAltFps = static_cast<int>(mAltFpsSlider->getValue());

    mFpsSlider->setEnabled(mFps > 0);

    mAltFpsSlider->setEnabled(mAltFps > 0);

    // FPS change
    config.setValue("fpslimit", mFps);
    config.setValue("altfpslimit", mAltFps);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getBoolValue("screen");
    mCustomCursorEnabled = config.getBoolValue("customcursor");

    mOpenGLEnabled = config.getIntValue("opengl");
    mEnableResize = config.getBoolValue("enableresize");
    mNoFrame = config.getBoolValue("noframe");
}

void Setup_Video::cancel()
{
    mFpsCheckBox->setSelected(mFps > 0);
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLDropDown->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mAltFpsSlider->setValue(mAltFps);
    mAltFpsSlider->setEnabled(mAltFps > 0);
    mFpsLabel->setCaption(mFpsCheckBox->isSelected()
    // TRANSLATORS: video settings label
                          ? toString(mFps) : _("None"));
    // TRANSLATORS: video settings label
    mAltFpsLabel->setCaption(_("Alt FPS limit: ") + toString(mAltFps));
    mEnableResizeCheckBox->setSelected(mEnableResize);
    mNoFrameCheckBox->setSelected(mNoFrame);

    config.setValue("screen", mFullScreenEnabled);

    // Set back to the current video mode.
    std::string videoMode = toString(mainGraphics->mWidth).append("x")
        .append(toString(mainGraphics->mHeight));
    mModeList->setSelected(mModeListModel->getIndexOf(videoMode));
    config.setValue("screenwidth", mainGraphics->mWidth);
    config.setValue("screenheight", mainGraphics->mHeight);

    config.setValue("customcursor", mCustomCursorEnabled);
    config.setValue("opengl", mOpenGLEnabled);
    config.setValue("enableresize", mEnableResize);
    config.setValue("noframe", mNoFrame);
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    const std::string &id = event.getId();

    if (id == "videomode")
    {
        std::string mode = mModeListModel->getElementAt(
            mModeList->getSelected());

        if (mode == "custom")
        {
            if (mDialog)
            {
                mode = mDialog->getText();
                mDialog = nullptr;
            }
            else
            {
                mDialog = new TextDialog(
                    // TRANSLATORS: resolution question dialog
                    _("Custom resolution (example: 1024x768)"),
                    // TRANSLATORS: resolution question dialog
                    _("Enter new resolution:                "));
                mDialog->setActionEventId("videomode");
                mDialog->addActionListener(this);
                return;
            }
        }
        const int width = atoi(mode.substr(0, mode.find("x")).c_str());
        const int height = atoi(mode.substr(mode.find("x") + 1).c_str());
        if (!width || !height)
            return;

        if (width != mainGraphics->mWidth || height != mainGraphics->mHeight)
        {
#if defined(WIN32) || defined(__APPLE__) || defined(ANDROID)
            if (!config.getIntValue("opengl"))
            {
                Client::resize(width, height);
            }
            else
            {
                if (width < mainGraphics->mWidth
                    || height < mainGraphics->mHeight)
                {
                    // TRANSLATORS: video settings warning
                    new OkDialog(_("Screen Resolution Changed"),
                        // TRANSLATORS: video settings warning
                       _("Restart your client for the change to take effect.")
                       + std::string("\n") + _("Some windows may be moved to "
                        "fit the lowered resolution."));
                }
                else
                {
                    // TRANSLATORS: video settings warning
                    new OkDialog(_("Screen Resolution Changed"),
                        // TRANSLATORS: video settings warning
                        _("Restart your client for the change"
                        " to take effect."));
                }
            }
#else
            Client::resize(width, height);
#endif
        }

        config.setValue("oldscreen", config.getBoolValue("screen"));
        config.setValue("oldscreenwidth", mainGraphics->mWidth);
        config.setValue("oldscreenheight", mainGraphics->mHeight);
        config.setValue("screenwidth", width);
        config.setValue("screenheight", height);
    }
    if (id == "~videomode")
    {
        mDialog = nullptr;
    }
    else if (id == "customcursor")
    {
        config.setValue("customcursor", mCustomCursorCheckBox->isSelected());
    }
    else if (id == "fpslimitcheckbox" || id == "fpslimitslider")
    {
        int tempFps = static_cast<int>(mFpsSlider->getValue());
        if (id == "fpslimitcheckbox" && !mFpsSlider->isEnabled())
            tempFps = 60;
        else
            tempFps = tempFps > 0 ? tempFps : 60;
        mFps = mFpsCheckBox->isSelected() ? tempFps : 0;
        // TRANSLATORS: video settings label
        const std::string text = mFps > 0 ? toString(mFps) : _("None");

        mFpsLabel->setCaption(text);
        mFpsSlider->setValue(mFps);
        mFpsSlider->setEnabled(mFps > 0);
    }
    else if (id == "altfpslimitslider")
    {
        int tempFps = static_cast<int>(mAltFpsSlider->getValue());
        tempFps = tempFps > 0 ? tempFps : static_cast<int>(
            mAltFpsSlider->getScaleStart());
        mAltFps = tempFps;
        // TRANSLATORS: video settings label
        const std::string text = mAltFps > 0 ? toString(mAltFps) : _("None");

        // TRANSLATORS: video settings label
        mAltFpsLabel->setCaption(_("Alt FPS limit: ") + text);
        mAltFpsSlider->setValue(mAltFps);
        mAltFpsSlider->setEnabled(mAltFps > 0);
    }
    else if (id == "enableresize")
    {
        config.setValue("enableresize", mEnableResizeCheckBox->isSelected());
    }
    else if (id == "noframe")
    {
        config.setValue("noframe", mNoFrameCheckBox->isSelected());
    }
#if defined(USE_OPENGL) && !defined(ANDROID) && !defined(__APPLE__)
    else if (id == "detect")
    {
        TestMain *test = graphicsManager.startDetection();
        if (test)
        {
            const int val = test->getConfig().getValueInt("opengl", -1);
            if (val >= 0 && val <= 3)
                mOpenGLDropDown->setSelected(val);
            delete test;
        }
    }
#endif
}

void Setup_Video::externalUpdated()
{
}
