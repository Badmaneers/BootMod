#ifndef BITMAPEDITORWRAPPER_H
#define BITMAPEDITORWRAPPER_H

#include <QObject>
#include <QString>
#include <QPointer>

// Forward declare the MainWindow from bitmap editor
class MainWindow;

class BitmapEditorWrapper : public QObject {
    Q_OBJECT

public:
    explicit BitmapEditorWrapper(QObject *parent = nullptr);
    ~BitmapEditorWrapper();

    Q_INVOKABLE void openEditor(const QString &imagePath);
    Q_INVOKABLE void openEditorWithNewProject(int width, int height);
    Q_INVOKABLE bool isEditorOpen() const;

signals:
    void editorClosed();

private:
    QPointer<MainWindow> m_editorWindow;
};

#endif // BITMAPEDITORWRAPPER_H
