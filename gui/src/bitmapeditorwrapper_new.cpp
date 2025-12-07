#include "bitmapeditorwrapper.h"
#include "mainwindow.h"
#include <QDebug>
#include <QFile>

BitmapEditorWrapper::BitmapEditorWrapper(QObject *parent)
    : QObject(parent)
    , m_editorWindow(nullptr)
{
}

BitmapEditorWrapper::~BitmapEditorWrapper() {
    if (m_editorWindow) {
        m_editorWindow->close();
        delete m_editorWindow;
    }
}

void BitmapEditorWrapper::openEditor(const QString &imagePath) {
    qDebug() << "BitmapEditorWrapper::openEditor -" << imagePath;
    
    if (!QFile::exists(imagePath)) {
        qWarning() << "Image file does not exist:" << imagePath;
        return;
    }
    
    // Create editor window if it doesn't exist
    if (!m_editorWindow) {
        m_editorWindow = new MainWindow();
        
        // Connect close signal
        connect(m_editorWindow, &QObject::destroyed, this, [this]() {
            emit editorClosed();
        });
    }
    
    // Open the image in the editor
    // TODO: Implement loading image into existing project
    // For now, just show the window
    m_editorWindow->show();
    m_editorWindow->raise();
    m_editorWindow->activateWindow();
    
    qDebug() << "Bitmap editor opened";
}

void BitmapEditorWrapper::openEditorWithNewProject(int width, int height) {
    qDebug() << "BitmapEditorWrapper::openEditorWithNewProject -" << width << "x" << height;
    
    // Create editor window if it doesn't exist
    if (!m_editorWindow) {
        m_editorWindow = new MainWindow();
        
        // Connect close signal
        connect(m_editorWindow, &QObject::destroyed, this, [this]() {
            emit editorClosed();
        });
    }
    
    // Show the window
    m_editorWindow->show();
    m_editorWindow->raise();
    m_editorWindow->activateWindow();
    
    // TODO: Create new project with specified dimensions
    
    qDebug() << "Bitmap editor opened with new project";
}

bool BitmapEditorWrapper::isEditorOpen() const {
    return m_editorWindow && m_editorWindow->isVisible();
}
