#ifndef BITMAPEDITORWRAPPER_H
#define BITMAPEDITORWRAPPER_H

#include <QObject>
#include <QString>
#include <QPointer>

// Forward declare classes from bitmap editor
class MainWindow;
class BitmapAppContext;

class BitmapEditorWrapper : public QObject {
    Q_OBJECT

public:
    explicit BitmapEditorWrapper(QObject *parent = nullptr);
    ~BitmapEditorWrapper();

    Q_INVOKABLE void openEditor(const QString &imagePath);
    Q_INVOKABLE void openEditorWithNewProject(int width, int height);
    Q_INVOKABLE bool isEditorOpen() const;
    Q_INVOKABLE bool saveCurrentProjectToPath(const QString &outputPath);

signals:
    void editorClosed();
    void editorError(const QString &message);
    void imageSaved(const QString &path);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void connectWindowSignals();
    
    QPointer<MainWindow> m_editorWindow;
    QPointer<BitmapAppContext> m_appContext;
    QString m_originalImagePath;  // Track the original image path for auto-save
};

#endif // BITMAPEDITORWRAPPER_H
