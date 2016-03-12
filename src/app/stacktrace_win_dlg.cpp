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

#include "stacktrace_win.h"
#include "stacktrace_win_dlg.h"
#include "stacktrace_win_dlg_misc.h"
#include <QString>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <atomic>
#include "boost/version.hpp"
#include "libtorrent/version.hpp"
#include "base/utils/misc.h"


namespace straceWin { namespace dialog
{
    std::atomic<bool> cancelMemoryDump { false };

    StraceDlg::~StraceDlg() = default;

    StraceDlg::StraceDlg( HANDLE hProcess, QWidget* parent)
        : QDialog(parent), 
        m_hProcess(hProcess)
    {
        setupUi(this);

        m_path.reset(new PathHelper(compress_checkbox->isChecked()));
        connect(compress_checkbox, SIGNAL(stateChanged(int)), this, SLOT(compress_changed(int)));

        connect(saveDump_button, SIGNAL(clicked(bool)), this, SLOT(saveDump()));

        m_progress = new QProgressDialog(this);
        m_progress->setLabelText("Saving Memory Dump...");
        m_progress->setMaximum(0);
        m_progress->setMinimum(0);
        m_progress->reset();

        m_thread = new DumpThread(this);
        connect(m_thread, SIGNAL(finished()), this, SLOT(closeProgress()), Qt::QueuedConnection);
        connect(m_thread, SIGNAL(error(QString)), this, SLOT(reportError(QString)), Qt::QueuedConnection);
    }

    void StraceDlg::setStacktraceString(const QString& trace)
    {
        QString htmlStr = QString(
            "<p align=center><b><font size=7 color=red>"
            "qBittorrent has crashed"
            "</font></b></p>"
            "<font size=4><p>"
            "Please file a bug report at "
            "<a href=\"http://bugs.qbittorrent.org\">http://bugs.qbittorrent.org</a> "
            "and provide the information given below.<br/><br/>"
            "It would be very helpful if you could also upload a compressed memory dump by dragging and dropping it along with the bug report."
            "</p></font>"
            "<br/><hr><br/>"
            "<p align=center><font size=4>"
            "qBittorrent version: " VERSION "<br/>"
            "Libtorrent version: %1<br/>"
            "Qt version: " QT_VERSION_STR "<br/>"
            "Boost version: %2<br/>"
            "OS version: %3"
            "</font></p><br/>"
            "<pre><code>%4</code></pre>"
            "<br/><hr><br/><br/>")
            .arg(Utils::Misc::libtorrentVersionString())
            .arg(Utils::Misc::boostVersionString())
            .arg(Utils::Misc::osName())
            .arg(trace);

        errorText->setHtml(htmlStr);
    }

    void StraceDlg::saveDump()
    {
        QString chosenPath = QFileDialog::getSaveFileName(this, "", m_path->getTimeStampedFilePath(), m_path->getFilter());

        if(chosenPath.isEmpty())
            return;

        QString path = m_path->addExtensionIfNecessary(chosenPath);;

        // show progress dialog
        m_progress->setLabelText(QString("Saving ") + path);
        cancelMemoryDump = false;
        m_progress->open(this, SLOT(cancelTask()));

        m_thread->setTask(m_hProcess, minidump_checkbox->isChecked(), compress_checkbox->isChecked(), std::move(path));
        m_thread->start();
    }

    void StraceDlg::cancelTask()
    {
        m_progress->setLabelText("Stopping...");

        // prevent progress dialog from disappearing
        m_progress->setVisible(true);
        // wait until thread finishes
        m_progress->blockSignals(true);

        cancelMemoryDump = true;
    }

    void StraceDlg::closeProgress()
    {
        m_progress->blockSignals(false);

        // close progress dialog
        m_progress->accept();
    }

    void StraceDlg::reportError(const QString& errMsg)
    {
        m_progress->blockSignals(false);

        QMessageBox::warning(this, "Error", errMsg, QMessageBox::Ok);
    }

    void StraceDlg::compress_changed(int state)
    {
        m_path->setCompress(state == Qt::Checked);
    }
}}
