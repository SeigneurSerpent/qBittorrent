/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015 The qBittorrent project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 */

#ifndef STACKTRACE_WIN_DLG_MISC_H
#define STACKTRACE_WIN_DLG_MISC_H

#include "stacktrace_win_dlg.h"
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include "DbgHelp.h"

namespace straceWin
{
    namespace dialog
    {
        class DumpThread : public QThread
        {
            Q_OBJECT

        private:
            bool m_saveMiniDump{ false };
            bool m_compress{ true };
            HANDLE m_hProcess{ nullptr };
            QString m_path{};

            bool saveDump(const QString& dumpPath);
            void compressDump(const QString& dumpPath, const QString& fileName); 

        protected:
            void run() override;

        public:
            DumpThread(QObject* parent = nullptr);
            void setTask(HANDLE hProcess, bool saveMiniDump, bool compress, QString&& path);

        signals:
            void error(const QString& errMsg);
        };


        class PathHelper
        {
        private:
            bool m_compress;
            QString m_directory{};

            static const QString DUMP_EXTENSION;
            static const QString COMPRESSED_EXTENSION;

            void saveDirectory(const QString& filepath);

            QString getTimeStampedBaseName() const;
            QString getExtension() const;

        public:
            PathHelper(bool compress);

            void setDirectory(QString&& directory);
            void setCompress(bool compress);

            QString getTimeStampedFilePath() const;
            QString getFilter() const;
            QString addExtensionIfNecessary(const QString& filepath);

            // changes compressed extension to dump extension
            static QString getCompressedFileName(const QString& filePath);
            static QString getChunkFilePath(const QString& filePath, int chunkNum);
        };
    }
}

#endif // STACKTRACE_WIN_DLG_MISC_H
