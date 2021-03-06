/*
 *  Color database
 *  Copyright (C) 2008  Aethyra Development Team
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

#include "resources/mapdb.h"

#include "configuration.h"
#include "client.h"
#include "logger.h"

#include "debug.h"

namespace
{
    bool mLoaded = false;
    MapDB::Maps mMaps;
    MapDB::MapInfos mInfos;
    MapDB::Atlases mAtlases;
}

namespace MapDB
{
    void readMap(XmlNodePtr node);
    void readAtlas(XmlNodePtr node);
}

void MapDB::load()
{
    if (mLoaded)
        unload();

    loadRemap();
    loadInfo();
    mLoaded = true;
}

void MapDB::loadRemap()
{
    XML::Document *const doc = new XML::Document(
        paths.getStringValue("maps").append("remap.xml"));

    const XmlNodePtr root = doc->rootNode();
    if (!root)
    {
        delete doc;
        return;
    }

    for_each_xml_child_node(node, root)
    {
        if (xmlNameEqual(node, "map"))
        {
            const std::string name = XML::getProperty(node, "name", "");
            if (name.empty())
                continue;

            const std::string value = XML::getProperty(node, "value", "");
            if (value.empty())
                continue;

            mMaps[name] = value;
        }
    }

    delete doc;
}

void MapDB::readMap(XmlNodePtr node)
{
    const std::string map = XML::getProperty(node, "name", "");
    if (map.empty())
        return;

    for_each_xml_child_node(childNode, node)
    {
        if (xmlNameEqual(childNode, "atlas"))
        {
            const std::string atlas = XML::getProperty(childNode, "name", "");
            if (atlas.empty())
                continue;
            mInfos[map].atlas = atlas;
        }
    }
}

void MapDB::readAtlas(XmlNodePtr node)
{
    const std::string atlas = XML::getProperty(node, "name", "");
    if (atlas.empty())
        return;
    for_each_xml_child_node(childNode, node)
    {
        if (xmlNameEqual(childNode, "file"))
        {
            const std::string file = XML::getProperty(childNode, "name", "");
            if (file.empty())
                continue;
            mAtlases[atlas].push_back(file);
        }
    }
}

void MapDB::loadInfo()
{
    XML::Document *doc = new XML::Document("maps.xml");
    const XmlNodePtr root = doc->rootNode();
    if (!root)
    {
        delete doc;
        return;
    }

    for_each_xml_child_node(node, root)
    {
        if (xmlNameEqual(node, "map"))
            readMap(node);
        else if (xmlNameEqual(node, "atlas"))
            readAtlas(node);
    }
    delete doc;
}

void MapDB::unload()
{
    logger->log1("Unloading map database...");

    mMaps.clear();
    mLoaded = false;
}

const std::string MapDB::getMapName(const std::string &name)
{
    const MapIterator it = mMaps.find(name);

    if (it != mMaps.end())
        return it->second;
    return name;
}

const MapDB::MapInfo *MapDB::getMapAtlas(const std::string &name)
{
    const MapInfoIter it = mInfos.find(name);
    if (it == mInfos.end())
        return nullptr;
    MapInfo *const info = &(*it).second;
    const AtlasCIter it2 = mAtlases.find(info->atlas);
    if (it2 == mAtlases.end())
        return nullptr;
    info->files = &((*it2).second);
    return info;
}
