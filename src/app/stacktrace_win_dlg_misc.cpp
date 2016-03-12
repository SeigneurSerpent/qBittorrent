#include "stacktrace_win_dlg_misc.h"
#include "stacktrace_win.h"

#include <QTemporaryFile>
#include <atomic>
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/device/file_descriptor.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/stream.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/filesystem.hpp"

namespace straceWin { namespace dialog
{
    extern std::atomic<bool> cancelMemoryDump;

    BOOL CALLBACK checkIfCanceled( PVOID, const PMINIDUMP_CALLBACK_INPUT input, PMINIDUMP_CALLBACK_OUTPUT output)
    {
        if (input->CallbackType == MINIDUMP_CALLBACK_TYPE::CancelCallback) {

            output->CheckCancel = true;
            output->Cancel = cancelMemoryDump;
        }

        return true;
    }

    bool DumpThread::saveDump(const QString& dumpPath)
    {
        MINIDUMP_TYPE dumpType = m_saveMiniDump ? MINIDUMP_TYPE::MiniDumpNormal
            : MINIDUMP_TYPE::MiniDumpWithFullMemory;

        HANDLE hFile = CreateFile(dumpPath.toStdWString().c_str(), GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            QString errMsg = straceWin::GetLastErrorString();
            emit error(errMsg);
            return false;
        }

        auto deleteFileIfCanceled_guard = straceWin::make_resource_guard(
            [&] { if (cancelMemoryDump) DeleteFile(dumpPath.toStdWString().c_str()); });
        auto closeFileHandle_guard = straceWin::make_resource_guard([&] { CloseHandle(hFile); });

        MINIDUMP_CALLBACK_INFORMATION mci;
        mci.CallbackParam = nullptr;
        mci.CallbackRoutine = checkIfCanceled;

        if (!MiniDumpWriteDump(m_hProcess, GetProcessId(m_hProcess), hFile, dumpType, NULL, NULL, &mci)) {
            DWORD errNum = GetLastError();
            if (errNum != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
                QString errMsg = straceWin::GetErrorString(errNum);
                emit error(errMsg);
                return false;
            }
        }

        return true;
    }

    void DumpThread::compressDump(const QString& dumpPath, const QString& fileName)
    {
        using namespace boost::iostreams;
        using namespace boost::filesystem;

        try
        {
            filtering_istream dump;
            gzip_params p = gzip::best_compression;
            // gzip does not support Unicode file names
            // each Unicode symbol will be replaced by an underscore
            p.file_name = fileName.toLocal8Bit();
            dump.push(gzip_compressor(p));
            dump.push(file_descriptor_source(path(dumpPath.toStdWString().c_str())));

            stream<file_descriptor_sink> gz(file_descriptor_sink(path(m_path.toStdWString().c_str()), std::ios_base::binary | std::ios_base::out));
            copy(dump, gz);
        }
        catch (std::ios_base::failure&)
        {
            QString errMsg = straceWin::GetLastErrorString();
            emit error(errMsg);
        }
    }

    void DumpThread::run()
    {
        if (m_compress)
        {
            QTemporaryFile tmpFile;
            if (!tmpFile.open())
            {
                emit error(tmpFile.errorString());
                return;
            }

            if(!cancelMemoryDump && saveDump(tmpFile.fileName()))
            {
                if(!cancelMemoryDump)
                    compressDump(tmpFile.fileName(), PathHelper::getCompressedFileName(m_path));
            }
        }
        else
            saveDump(m_path);
    }

    DumpThread::DumpThread(QObject* parent)
        : QThread(parent)
    {}

    void DumpThread::setTask(HANDLE hProcess, bool saveMiniDump, bool compress, QString&& path)
    {
        m_hProcess = hProcess;
        m_saveMiniDump = saveMiniDump;
        m_compress = compress;
        m_path = std::move(path);
    }

    const QString PathHelper::DUMP_EXTENSION = QString(".dmp");
    const QString PathHelper::COMPRESSED_EXTENSION = QString(".gz");

    void PathHelper::saveDirectory(const QString& filepath)
    {
        m_directory.swap(QDir::toNativeSeparators(QFileInfo(filepath).absoluteDir().absolutePath()));

        //append directory separator if necessary
        if (!m_directory.endsWith(QDir::separator()))
            m_directory += QDir::separator();
    }

    QString PathHelper::getTimeStampedBaseName() const
    {
        return QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss");
    }

    QString PathHelper::getExtension() const
    {
        return QString(m_compress ? COMPRESSED_EXTENSION : DUMP_EXTENSION);
    }

    PathHelper::PathHelper(bool compress)
        :m_compress(compress)
    {}

    void PathHelper::setDirectory(QString&& directory)
    {
        std::swap(m_directory, directory);
    }

    void PathHelper::setCompress(bool compress)
    {
        m_compress = compress;
    }

    QString PathHelper::getTimeStampedFilePath() const
    {
        QString name;
        if (!m_directory.isEmpty())
            name = m_directory;

        return name + getTimeStampedBaseName() + getExtension();
    }

    QString PathHelper::getFilter() const
    {
        return QString(m_compress ? "Gzip *.gz" : "Memory dumps *.dmp");
    }

    QString PathHelper::addExtensionIfNecessary(const QString& filepath)
    {
        if (filepath.isEmpty())
            return QString();

        QString optionalExtension = QFileInfo(filepath).suffix().isEmpty() ? getExtension() : "";
        QString newPath = QDir::toNativeSeparators(filepath + optionalExtension);

        saveDirectory(newPath);

        return newPath;
    }

    QString PathHelper::getCompressedFileName(const QString& filePath)
    {
        return QFileInfo(filePath).completeBaseName() + DUMP_EXTENSION;
    }

    QString PathHelper::getChunkFilePath(const QString& filePath, int chunkNum)
    {
        QFileInfo fp(filePath);
        return QString("%1/%2.%3%4")
                .arg(fp.dir().canonicalPath())
                .arg(fp.completeBaseName())
                .arg(chunkNum)
                .arg(COMPRESSED_EXTENSION);
    }
}}
