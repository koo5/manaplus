/*
 *  The ManaPlus Client
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


#ifndef NET_DOWNLOAD_H
#define NET_DOWNLOAD_H

#include <SDL_types.h>

#include <stdio.h>
#include <string>

#include "localconsts.h"

enum DownloadStatus
{
    DOWNLOAD_STATUS_CANCELLED = -3,
    DOWNLOAD_STATUS_THREAD_ERROR = -2,
    DOWNLOAD_STATUS_ERROR = -1,
    DOWNLOAD_STATUS_STARTING = 0,
    DOWNLOAD_STATUS_IDLE,
    DOWNLOAD_STATUS_COMPLETE
};

typedef int (*DownloadUpdate)(void *ptr, DownloadStatus status,
                              size_t total, size_t remaining);

// Matches what CURL expects
typedef size_t (*WriteFunction)( void *ptr, size_t size, size_t nmemb,
                                 void *stream);

struct SDL_Thread;
typedef void CURL;
struct curl_slist;

namespace Net
{
class Download final
{
    public:
        Download(void *const ptr, const std::string &url,
                 const DownloadUpdate updateFunction,
                 const bool ignoreError = false);

        A_DELETE_COPY(Download)

        ~Download();

        void addHeader(const std::string &header);

        /**
         * Convience method for adding no-cache headers.
         */
        void noCache();

        void setFile(const std::string &filename, const int64_t adler32 = -1);

        void setWriteFunction(WriteFunction write);

        /**
         * Starts the download thread.
         * @returns true  if thread was created
         *          false if the thread could not be made or download wasn't
         *                properly setup
         */
        bool start();

        /**
         * Cancels the download. Returns immediately, the cancelled status will
         * be noted in the next avialable update call.
         */
        void cancel();

        char *getError() const A_WARN_UNUSED;

        void setIgnoreError(const bool n)
        { mIgnoreError = n; }

        static unsigned long fadler32(FILE *const file) A_WARN_UNUSED;

        static void addProxy(CURL *const curl);

        static void secureCurl(CURL *const curl);

    private:
        static int downloadThread(void *ptr);
        static int downloadProgress(void *clientp, double dltotal,
                                    double dlnow, double ultotal,
                                    double ulnow);
        void *mPtr;
        std::string mUrl;
        struct
        {
            unsigned cancel : 1;
            unsigned memoryWrite: 1;
            unsigned checkAdler: 1;
        } mOptions;
        std::string mFileName;
        WriteFunction mWriteFunction;
        unsigned long mAdler;
        DownloadUpdate mUpdateFunction;
        SDL_Thread *mThread;
        CURL *mCurl;
        curl_slist *mHeaders;
        char *mError;
        bool mIgnoreError;
};

}  // namespace Net

#endif  // NET_DOWNLOAD_H
