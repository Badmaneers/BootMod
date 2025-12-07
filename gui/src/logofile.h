#ifndef LOGOFILE_H
#define LOGOFILE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QImage>
#include "../../include/bootmod.h"
#include "../../include/splash.h"

class ThumbnailProvider;

struct LogoEntry {
    int index;
    int width;
    int height;
    QString format;
    int size;
    QImage thumbnail;
    std::vector<uint8_t> rawData;
};

class LogoFile : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)
    Q_PROPERTY(int logoCount READ logoCount NOTIFY logoCountChanged)
    Q_PROPERTY(QString headerInfo READ headerInfo NOTIFY headerInfoChanged)
    Q_PROPERTY(bool isProjectMode READ isProjectMode NOTIFY isProjectModeChanged)
    Q_PROPERTY(QString formatType READ formatType NOTIFY formatTypeChanged)
    Q_PROPERTY(QString projectPath READ projectPath NOTIFY isProjectModeChanged)
    Q_PROPERTY(ThumbnailProvider* thumbnailProvider READ thumbnailProvider WRITE setThumbnailProvider)

public:
    explicit LogoFile(QObject *parent = nullptr);
    
    QString filePath() const { return m_filePath; }
    bool isLoaded() const { return m_isLoaded; }
    int logoCount() const { return m_logos.size(); }
    QString headerInfo() const { return m_headerInfo; }
    bool isProjectMode() const { return !m_projectDir.isEmpty(); }
    QString formatType() const { return m_formatType; }
    QString projectPath() const { return m_projectDir; }
    
    ThumbnailProvider* thumbnailProvider() const { return m_thumbnailProvider; }
    void setThumbnailProvider(ThumbnailProvider* provider) { m_thumbnailProvider = provider; }
    
    Q_INVOKABLE bool loadFile(const QString &path);
    Q_INVOKABLE void clearFile();
    Q_INVOKABLE bool extractLogo(int index, const QString &outputPath);
    Q_INVOKABLE bool extractAll(const QString &outputDir);
    Q_INVOKABLE bool replaceLogo(int index, const QString &imagePath);
    Q_INVOKABLE bool saveFile(const QString &outputPath);
    
    // Native file dialogs
    Q_INVOKABLE QString browseForFile();
    Q_INVOKABLE QString browseForSaveFile();
    Q_INVOKABLE QString browseForFolder();
    Q_INVOKABLE QString browseForImage();
    
    // Combined browse + action methods
    Q_INVOKABLE void browseAndExtractLogo(int index);
    Q_INVOKABLE void browseAndReplaceLogo(int index);
    
    // Project-based workflow
    Q_INVOKABLE bool unpackToProject(const QString &logoPath, const QString &projectDir);
    Q_INVOKABLE bool openProject(const QString &projectDir);
    Q_INVOKABLE bool saveProject();
    Q_INVOKABLE bool exportProject(const QString &outputPath);
    Q_INVOKABLE bool isProjectFolder(const QString &path);
    Q_INVOKABLE void rescanProjectImages();  // Rescan images folder to update logo count
    Q_INVOKABLE void refreshSingleLogo(int index);  // Refresh just one logo's thumbnail
    
    const QList<LogoEntry>& logos() const { return m_logos; }

signals:
    void filePathChanged();
    void isLoadedChanged();
    void logoCountChanged();
    void headerInfoChanged();
    void isProjectModeChanged();
    void formatTypeChanged();
    void errorOccurred(const QString &message);
    void operationCompleted(const QString &message);

private:
    QString m_filePath;
    QString m_projectDir;  // Current project directory
    QString m_formatType;  // "MediaTek" or "Snapdragon"
    bootmod::FormatType m_currentFormat;
    bool m_isLoaded = false;
    QString m_headerInfo;
    QList<LogoEntry> m_logos;
    std::vector<std::vector<uint8_t>> m_logoImages;
    
    // Format-specific storage
    std::unique_ptr<bootmod::splash::SplashImage> m_splashImage;
    
    ThumbnailProvider* m_thumbnailProvider = nullptr;
    
    QImage createThumbnail(const QImage &source, int maxSize = 128);
    QString formatToString(mtklogo::ColorMode format);
    
    // Format-specific loaders
    bool loadMtkFile(const QString &path);
    bool loadSplashFile(const QString &path);
    bool createProjectIdentifier(const QString &projectDir);
    bool loadProjectMetadata(const QString &projectDir);
};

#endif // LOGOFILE_H
