#ifndef BITMAPEDITORWRAPPER_H
#define BITMAPEDITORWRAPPER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QProcess>

class BitmapEditorWrapper : public QObject {
    Q_OBJECT
    
public:
    explicit BitmapEditorWrapper(QObject* parent = nullptr);
    ~BitmapEditorWrapper();
    
    // Open bitmap editor with a specific image (launches external process)
    Q_INVOKABLE bool openEditor(const QString& imagePath);
    
    // Check if editor is currently open
    Q_INVOKABLE bool isEditorOpen() const;
    
signals:
    void editorClosed();
    void editorError(const QString& error);
    
private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    
private:
    QProcess* m_editorProcess;
    QString m_editorPath;
    QString m_currentImagePath;
};

#endif // BITMAPEDITORWRAPPER_H
