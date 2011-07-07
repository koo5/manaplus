/*
 *  The Mana World
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011  The ManaPlus Developers
 *  Copyright (C) 2009-2010  Andrei Karas
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef M_CHATLOG_H
#define M_CHATLOG_H

#include <fstream>
#include <list>

class ChatLogger
{
    public:
        /**
         * Constructor.
         */
        ChatLogger();

        /**
         * Destructor, closes log file.
         */
        ~ChatLogger();

        void setLogDir(const std::string &logDir);

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log(std::string str);

        void log(std::string name, std::string str);

        void loadLast(std::string name, std::list<std::string> &list,
                      unsigned n);

        std::string getDateString() const;

        std::string secureName(std::string &str) const;

        void setServerName(const std::string &serverName);

    private:
        /**
         * Sets the file to log to and opens it
         */
        void setLogFile(const std::string &logFilename);

        void writeTo(std::ofstream &file, const std::string &str) const;

        void makeDir(const std::string &dir);

        std::ofstream mLogFile;
        std::string mLogDir;
        std::string mServerName;
        std::string mLogDate;
};

extern ChatLogger *chatLogger;

#endif
