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

#include "utils/translation/poparser.h"

#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

#include "logger.h"

#include <stdlib.h>

#include "debug.h"

PoParser::PoParser() :
    mLang(),
    mFile(),
    mLine(),
    mMsgId(),
    mMsgStr(),
    mDict(nullptr),
    mReadingId(false),
    mReadingStr(false)
{
}

void PoParser::openFile(std::string name)
{
    const ResourceManager *const resman = ResourceManager::getInstance();
    if (!resman)
        return;
    int size;
    char *buf = static_cast<char*>(resman->loadFile(getFileName(name), size));

    if (buf)
    {
        mFile.str(std::string(buf, size));
        free(buf);
    }
    else
    {
        mFile.clear();
    }
}

PoDict *PoParser::load(const std::string &lang, const std::string &fileName,
                       PoDict *const dict)
{
    logger->log("loading lang: %s, file: %s", lang.c_str(), fileName.c_str());

    setLang(lang);
    if (!dict)
        mDict = getDict();
    else
        mDict = dict;

    if (fileName.empty())
        openFile(mLang);
    else
        openFile(fileName);

    mMsgId.clear();
    mMsgStr.clear();

    // cycle by msgid+msgstr
    while (readLine())
    {
        // reading msgid
        while (readMsgId())
        {
            if (!readLine())
                break;
        }

        if (!mMsgId.empty())
        {
            // if we got msgid then reading msgstr
            while (readMsgStr())
            {
                if (!readLine())
                    break;
            }
        }

        if (!mMsgId.empty() && !mMsgStr.empty())
        {
//            logger->log("add key: " + mMsgId);
//            logger->log("add value: " + mMsgStr);

            convertStr(mMsgId);
            convertStr(mMsgStr);
            // store key and value
            mDict->set(mMsgId, mMsgStr);
        }

        mMsgId.clear();
        mMsgStr.clear();
    }

    return mDict;
}

bool PoParser::readLine()
{
    char line[1001];
    if (!mFile.getline(line, 1000))
        return false;
    mLine = line;
    return true;
}

bool PoParser::readMsgId()
{
    // if we reading msgstr then stop here
    if (mReadingStr)
        return false;

    const std::string msgId1 = "msgid \"";

    // check if in reading process
    if (mReadingId)
    {
        // if we get empty line in file then stop reading
        if (mLine.empty())
        {
            mReadingId = false;
            return false;
        }
        else if (checkLine())
        {
            // reading text from: "text"
            mMsgId.append(mLine.substr(1, mLine.size() - 2));
            mLine.clear();
            return true;
        }
        // stop reading in other case
        mReadingId = false;
        return false;
    }
    else
    {
        // check line start from msgid "
        if (strStartWith(mLine, msgId1))
        {
            mReadingId = true;
            // reading text from: msgid "text"
            mMsgId.append(mLine.substr(msgId1.size(),
                mLine.size() - 1 - msgId1.size()));
            mLine.clear();
            return true;
        }
        // stop reading if we don't read msgid before
        return mMsgId.empty();
    }
}

bool PoParser::readMsgStr()
{
    const std::string msgStr1 = "msgstr \"";

    // check if in reading process
    if (mReadingStr)
    {
        // if we get empty line in file then stop reading
        if (mLine.empty())
        {
            mReadingStr = false;
            return false;
        }
        if (checkLine())
        {
            // reading text from: "text"
            mMsgStr.append(mLine.substr(1, mLine.size() - 2));
            mLine.clear();
            return true;
        }
        // stop reading in other case
        mReadingStr = false;
    }
    else
    {
        // check line start from msgstr "
        if (strStartWith(mLine, msgStr1))
        {
            mReadingStr = true;
            // reading text from: msgid "text"
            mMsgStr.append(mLine.substr(msgStr1.size(),
                mLine.size() - 1 - msgStr1.size()));
            mLine.clear();
            return true;
        }
    }

    // stop reading in other case
    return false;
}

bool PoParser::checkLine()
{
    // check is line in format: "text"
    return mLine.size() > 2 && mLine[0] == '\"'
        && mLine[mLine.size() - 1] == '\"';
}

PoDict *PoParser::getEmptyDict()
{
    return new PoDict("");
}

bool PoParser::checkLang(std::string lang) const
{
    // check is po file exists
    const ResourceManager *const resman = ResourceManager::getInstance();
    return resman->exists(getFileName(lang));
}

std::string PoParser::getFileName(std::string lang) const
{
    // get po file name from lang name
//    logger->log("getFileName: translations/%s.po", lang.c_str());
    return strprintf("translations/%s.po", lang.c_str());
}

PoDict *PoParser::getDict() const
{
    return new PoDict(mLang);
}

void PoParser::convertStr(std::string &str) const
{
    if (str.empty())
        return;

    replaceAll(str, "\\n", "\n");
    replaceAll(str, "\\\"", "\"");
    replaceAll(str, "\\\\", "\\");
}
