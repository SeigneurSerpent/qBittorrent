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

#ifndef STACKTRACE_WIN_DLG_H
#define STACKTRACE_WIN_DLG_H

#include <QString>
#include <QProgressDialog>
#include <memory>
#include "Windows.h"
#include "ui_stacktrace_win_dlg.h"

namespace straceWin
{
    namespace dialog
    {
        class DumpThread;
        class PathHelper;

        class StraceDlg : public QDialog, private Ui::errorDialog
        {
            Q_OBJECT

        private:
            HANDLE m_hProcess;
            QProgressDialog* m_progress;
            DumpThread* m_thread;
            std::unique_ptr<PathHelper> m_path;

            QString chooseFilePath();
            void saveChosenDirectoryBetweenCalls(const QString& path);
            
        public:
            StraceDlg( HANDLE hProcess, QWidget* parent = 0);
            void setStacktraceString(const QString& trace);

            ~StraceDlg();
        public slots:
            void saveDump();

        private slots:
            void compress_changed(int state);

            void cancelTask();
            void closeProgress();
            void reportError(const QString& errMsg);
        };
    }
}
#endif // STACKTRACE_WIN_DLG_H

