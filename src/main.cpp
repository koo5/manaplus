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

#include "main.h"

#include "client.h"
#include "logger.h"

#include <getopt.h>
#include <iostream>
#include <unistd.h>

#include "utils/gettext.h"
#ifdef ANDROID
#include "utils/mkdir.h"
#endif
#include "utils/paths.h"
#include "utils/process.h"
#include "utils/physfstools.h"
#include "utils/xml.h"

#ifdef UNITTESTS
#include <gtest/gtest.h>
#endif

#ifdef __MINGW32__
#include <windows.h>
#endif

#include "debug.h"

char *selfName = nullptr;

static void printHelp()
{
    std::cout
        // TRANSLATORS: command line help
        << _("manaplus [options] [manaplus-file]") << std::endl << std::endl
        // TRANSLATORS: command line help
        << _("[manaplus-file] : The manaplus file is an XML file (.manaplus)")
        << std::endl
        // TRANSLATORS: command line help
        << _("                  used to set custom parameters") << std::endl
        // TRANSLATORS: command line help
        << _("                  to the manaplus client.")
        << std::endl << std::endl
        // TRANSLATORS: command line help
        << _("Options:") << std::endl
        // TRANSLATORS: command line help
        << _("  -l --log-file       : Log file to use") << std::endl
        // TRANSLATORS: command line help
        << _("  -a --chat-log-dir   : Chat log dir to use") << std::endl
        // TRANSLATORS: command line help
        << _("  -v --version        : Display the version") << std::endl
        // TRANSLATORS: command line help
        << _("  -h --help           : Display this help") << std::endl
        // TRANSLATORS: command line help
        << _("  -C --config-dir     : Configuration directory to use")
        << std::endl
        // TRANSLATORS: command line help
        << _("  -U --username       : Login with this username") << std::endl
        // TRANSLATORS: command line help
        << _("  -P --password       : Login with this password") << std::endl
        // TRANSLATORS: command line help
        << _("  -c --character      : Login with this character") << std::endl
        // TRANSLATORS: command line help
        << _("  -s --server         : Login server name or IP") << std::endl
        // TRANSLATORS: command line help
        << _("  -p --port           : Login server port") << std::endl
        // TRANSLATORS: command line help
        << _("  -H --update-host    : Use this update host") << std::endl
        // TRANSLATORS: command line help
        << _("  -D --default        : Choose default character server and "
                                     "character") << std::endl
        // TRANSLATORS: command line help
        << _("  -u --skip-update    : Skip the update downloads") << std::endl
        // TRANSLATORS: command line help
        << _("  -d --data           : Directory to load game "
             "data from") << std::endl
        // TRANSLATORS: command line help
        << _("  -L --localdata-dir  : Directory to use as local data"
             " directory") << std::endl
        // TRANSLATORS: command line help
        << _("     --screenshot-dir : Directory to store screenshots")
        << std::endl
        // TRANSLATORS: command line help
        << _("     --safemode       : Start game in safe mode") << std::endl
        // TRANSLATORS: command line help
        << _("  -T --tests          : Start testing drivers and "
                                     "auto configuring") << std::endl
#ifdef USE_OPENGL
        // TRANSLATORS: command line help
        << _("  -O --no-opengl      : Disable OpenGL for this session")
        << std::endl
#endif
        ;
}

static void printVersion()
{
    std::cout << strprintf("ManaPlus client %s", FULL_VERSION) << std::endl;
}

static void parseOptions(const int argc, char *const argv[],
                         Client::Options &options)
{
    const char *const optstring = "hvud:U:P:Dc:p:l:L:C:s:t:T:a";

    const struct option long_options[] =
    {
        { "config-dir",     required_argument, nullptr, 'C' },
        { "data",           required_argument, nullptr, 'd' },
        { "default",        no_argument,       nullptr, 'D' },
        { "password",       required_argument, nullptr, 'P' },
        { "character",      required_argument, nullptr, 'c' },
        { "help",           no_argument,       nullptr, 'h' },
        { "localdata-dir",  required_argument, nullptr, 'L' },
        { "update-host",    required_argument, nullptr, 'H' },
        { "port",           required_argument, nullptr, 'p' },
        { "server",         required_argument, nullptr, 's' },
        { "skip-update",    no_argument,       nullptr, 'u' },
        { "username",       required_argument, nullptr, 'U' },
        { "no-opengl",      no_argument,       nullptr, 'O' },
        { "chat-log-dir",   required_argument, nullptr, 'a' },
        { "version",        no_argument,       nullptr, 'v' },
        { "log-file",       required_argument, nullptr, 'l' },
        { "screenshot-dir", required_argument, nullptr, 'i' },
        { "safemode",       no_argument,       nullptr, 'm' },
        { "tests",          no_argument,       nullptr, 'T' },
        { "test",           required_argument, nullptr, 't' },
        { nullptr,          0,                 nullptr, 0 }
    };

    while (optind < argc)
    {
        const int result = getopt_long(argc, argv,
            optstring, long_options, nullptr);

        if (result == -1)
            break;

        switch (result)
        {
            case 'C':
                options.configDir = optarg;
                break;
            case 'd':
                options.dataPath = optarg;
                break;
            case 'D':
                options.chooseDefault = true;
                break;
            case '?':  // Unknown option
            case ':':  // Missing argument
            case 'h':
                options.printHelp = true;
                break;
            case 'H':
                if (checkPath(optarg))
                    options.updateHost = optarg;
                else
                    options.updateHost.clear();
                break;
            case 'c':
                options.character = optarg;
                break;
            case 'P':
                options.password = optarg;
                break;
            case 's':
                options.serverName = optarg;
                break;
            case 'p':
                options.serverPort = static_cast<uint16_t>(atoi(optarg));
                break;
            case 'u':
                options.skipUpdate = true;
                break;
            case 'U':
                options.username = optarg;
                break;
            case 'v':
                options.printVersion = true;
                break;
            case 'L':
                options.localDataDir = optarg;
                break;
            case 'O':
                options.noOpenGL = true;
                break;
            case 'l':
                options.logFileName = std::string(optarg);
                break;
            case 'a':
                options.chatLogDir = std::string(optarg);
                break;
            case 'i':
                options.screenshotDir = optarg;
                break;
            case 'm':
                options.safeMode = true;
                break;
            case 'T':
                options.testMode = true;
                options.test.clear();
                break;
            case 't':
                options.testMode = true;
                options.test = std::string(optarg);
                break;
            default:
                break;
        }
    }

    // when there are still options left use the last
    // one as branding file
    if (optind < argc)
    {
        options.brandingPath = argv[optind];
    }
}

#ifdef WIN32
extern "C" char const *_nl_locale_name_default(void);
#endif

#ifndef UNITTESTS
// main for normal game usage
int main(int argc, char *argv[])
{
#if defined(__MINGW32__)
    // load mingw crash handler. Won't fail if dll is not present.
    // may load libray from current dir, it may not same as program dir
    LoadLibrary("exchndl.dll");
#endif

    selfName = argv[0];

    // Parse command line options
    Client::Options options;
    parseOptions(argc, argv, options);

    if (options.printHelp)
    {
        printHelp();
        _exit(0);
    }
    else if (options.printVersion)
    {
        printVersion();
        _exit(0);
    }

    // Initialize PhysicsFS
#ifdef ANDROID
    mkdir_r(getenv("DATADIR2"));

    if (!PHYSFS_init((getRealPath(".").append("/fakebinary")).c_str()))
#else
    if (!PHYSFS_init(argv[0]))
#endif
    {
        std::cout << "Error while initializing PhysFS: "
                  << PHYSFS_getLastError() << std::endl;
        return 1;
    }

    PhysFs::updateDirSeparator();

    atexit((void(*)()) PHYSFS_deinit);

    XML::initXML();

#ifdef WIN32
    SetCurrentDirectory(PhysFs::getBaseDir());
#endif
    setPriority(true);
    Client client(options);
    if (!options.testMode)
    {
        client.gameInit();
        return client.gameExec();
    }
    else
    {
        client.testsInit();
        return client.testsExec();
    }
}

#else

// main for unit testing
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
