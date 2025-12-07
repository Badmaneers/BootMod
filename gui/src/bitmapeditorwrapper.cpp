#include "bitmapeditorwrapper.h"
#include "mainwindow.h"
#include "base/bitmapappcontext.h"
#include "base/project.h"
#include "layer/bitmaplayer.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImageReader>
#include <QImage>
#include <QPainter>
#include <QEvent>

BitmapEditorWrapper::BitmapEditorWrapper(QObject *parent)
    : QObject(parent)
    , m_editorWindow(nullptr)
    , m_appContext(nullptr)
{
}

BitmapEditorWrapper::~BitmapEditorWrapper() {
    if (m_editorWindow) {
        m_editorWindow->close();
        delete m_editorWindow;
    }
    if (m_appContext) {
        delete m_appContext;
    }
}

void BitmapEditorWrapper::openEditor(const QString &imagePath) {
    qDebug() << "BitmapEditorWrapper::openEditor -" << imagePath;
    
    // Check if file exists
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        qWarning() << "Image file does not exist:" << imagePath;
        qWarning() << "Directory:" << fileInfo.absolutePath();
        qWarning() << "Directory exists:" << QDir(fileInfo.absolutePath()).exists();
        
        // List files in the directory to help debug
        QDir dir(fileInfo.absolutePath());
        if (dir.exists()) {
            qDebug() << "Files in directory:";
            QStringList files = dir.entryList(QDir::Files);
            for (const QString &file : files) {
                qDebug() << "  -" << file;
            }
        }
        
        // Show error to user
        emit editorError("Image file not found: " + imagePath + "\n\nMake sure the logo is extracted to project mode.");
        return;
    }
    
    // Create app context if it doesn't exist
    if (!m_appContext) {
        m_appContext = new BitmapAppContext();
    }
    
    // Create editor window if it doesn't exist
    if (!m_editorWindow) {
        m_editorWindow = new MainWindow(m_appContext);
        
        // Connect close event to auto-export as PNG
        m_editorWindow->installEventFilter(this);
        
        // Connect destroyed signal
        connect(m_editorWindow, &QObject::destroyed, this, [this]() {
            emit editorClosed();
        });
    }
    
    // Create a new project from the image
    QImageReader imgReader(imagePath);
    if (!imgReader.canRead()) {
        qWarning() << "Cannot read image:" << imagePath;
        emit editorError("Cannot read image file: " + imagePath);
        return;
    }
    
    QSize imageSize = imgReader.size();
    qDebug() << "Loading image of size:" << imageSize;
    
    // Create project with image name and a temporary path
    QString projectName = fileInfo.baseName();
    QString projectPath = fileInfo.absolutePath() + "/" + projectName + ".qtbe";
    
    Project *project = new Project(nullptr, projectName, projectPath, imageSize);
    
    // Add the image as a background layer
    BitmapLayer *layer = new BitmapLayer(project, "Background", imagePath);
    project->addLayerAtTop(layer);
    project->setSelectedLayer(layer);
    
    // Set the project in the context
    m_appContext->setProject(project);
    
    // Store the original image path for auto-save
    m_originalImagePath = imagePath;
    
    qDebug() << "Project created and image loaded";
    
    // Show the editor window
    m_editorWindow->show();
    m_editorWindow->raise();
    m_editorWindow->activateWindow();
    
    qDebug() << "Bitmap editor opened with image loaded";
}

void BitmapEditorWrapper::openEditorWithNewProject(int width, int height) {
    qDebug() << "BitmapEditorWrapper::openEditorWithNewProject -" << width << "x" << height;
    
    // Create app context if it doesn't exist
    if (!m_appContext) {
        m_appContext = new BitmapAppContext();
    }
    
    // Create editor window if it doesn't exist
    if (!m_editorWindow) {
        m_editorWindow = new MainWindow(m_appContext);
        
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

bool BitmapEditorWrapper::saveCurrentProjectToPath(const QString &outputPath) {
    if (!m_appContext) {
        qWarning() << "No app context available";
        return false;
    }
    
    Project *project = m_appContext->getProject();
    if (!project) {
        qWarning() << "No project loaded in bitmap editor";
        return false;
    }
    
    // Render the project to an image
    QImage image(project->getSize(), QImage::Format_ARGB32);
    QPainter painter;
    painter.begin(&image);
    project->paintEvent(painter, true);  // true = export mode
    painter.end();
    
    // Save the image
    bool success = image.save(outputPath);
    
    if (success) {
        qDebug() << "Successfully saved image to:" << outputPath;
        emit imageSaved(outputPath);
    } else {
        qWarning() << "Failed to save image to:" << outputPath;
    }
    
    return success;
}

bool BitmapEditorWrapper::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_editorWindow && event->type() == QEvent::Close) {
        // Export the image as PNG before the window closes
        if (!m_originalImagePath.isEmpty()) {
            qDebug() << "Window closing - exporting image to:" << m_originalImagePath;
            saveCurrentProjectToPath(m_originalImagePath);
        }
    }
    
    // Pass the event on
    return QObject::eventFilter(obj, event);
}
