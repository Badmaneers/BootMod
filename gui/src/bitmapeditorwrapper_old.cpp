#include "bitmapeditorwrapper.h"
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

BitmapEditorWrapper::BitmapEditorWrapper(QObject* parent)
    : QObject(parent)
    , m_editorProcess(nullptr)
    , m_currentImagePath("")
{
    // Find the bitmap editor executable
    // First check in the same directory as our binary
    QFileInfo ourBinary(QCoreApplication::applicationFilePath());
    QString editorInSameDir = ourBinary.absolutePath() + "/QtBitmapEditor";
    
    if (QFileInfo::exists(editorInSameDir)) {
        m_editorPath = editorInSameDir;
    } else {
        // Try the build directory
        m_editorPath = ourBinary.absolutePath() + "/../bitmapeditor/build/QtBitmapEditor";
    }
    
    qDebug() << "BitmapEditorWrapper: Looking for editor at:" << m_editorPath;
}

BitmapEditorWrapper::~BitmapEditorWrapper() {
    if (m_editorProcess && m_editorProcess->state() != QProcess::NotRunning) {
        m_editorProcess->terminate();
        m_editorProcess->waitForFinished(3000);
        m_editorProcess->deleteLater();
    }
}

bool BitmapEditorWrapper::openEditor(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        qWarning() << "BitmapEditorWrapper::openEditor - Empty image path";
        return false;
    }
    
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        qWarning() << "BitmapEditorWrapper::openEditor - File does not exist:" << imagePath;
        emit editorError("Image file does not exist: " + imagePath);
        return false;
    }
    
    if (!QFileInfo::exists(m_editorPath)) {
        qWarning() << "BitmapEditorWrapper::openEditor - Editor executable not found:" << m_editorPath;
        emit editorError("Bitmap editor not found. Please build it first.");
        return false;
    }
    
    try {
        // If editor is already running, just activate it
        if (m_editorProcess && m_editorProcess->state() != QProcess::NotRunning) {
            qDebug() << "Editor already running";
            return true;
        }
        
        // Create new process
        if (m_editorProcess) {
            m_editorProcess->deleteLater();
        }
        
        m_editorProcess = new QProcess(this);
        connect(m_editorProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &BitmapEditorWrapper::onProcessFinished);
        connect(m_editorProcess, &QProcess::errorOccurred,
                this, &BitmapEditorWrapper::onProcessError);
        
        m_currentImagePath = imagePath;
        
        // Launch the bitmap editor with the image as argument
        QStringList args;
        args << fileInfo.absoluteFilePath();
        
        qDebug() << "Launching bitmap editor:" << m_editorPath << "with args:" << args;
        m_editorProcess->start(m_editorPath, args);
        
        if (!m_editorProcess->waitForStarted(5000)) {
            qWarning() << "Failed to start bitmap editor";
            emit editorError("Failed to start bitmap editor");
            return false;
        }
        
        qDebug() << "Bitmap editor started successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "BitmapEditorWrapper::openEditor - Exception:" << e.what();
        emit editorError(QString("Exception: ") + e.what());
        return false;
    }
}

bool BitmapEditorWrapper::isEditorOpen() const {
    return m_editorProcess != nullptr && m_editorProcess->state() != QProcess::NotRunning;
}

void BitmapEditorWrapper::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "Bitmap editor closed with exit code:" << exitCode;
    emit editorClosed();
    
    // Clean up
    m_editorProcess->deleteLater();
    m_editorProcess = nullptr;
}

void BitmapEditorWrapper::onProcessError(QProcess::ProcessError error) {
    QString errorString;
    switch (error) {
        case QProcess::FailedToStart:
            errorString = "Failed to start bitmap editor";
            break;
        case QProcess::Crashed:
            errorString = "Bitmap editor crashed";
            break;
        case QProcess::Timedout:
            errorString = "Bitmap editor timed out";
            break;
        default:
            errorString = "Bitmap editor error: " + QString::number(error);
            break;
    }
    
    qWarning() << "Process error:" << errorString;
    emit editorError(errorString);
}
