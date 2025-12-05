#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QQuickImageProvider>
#include <QPixmap>
#include <QHash>

class ThumbnailProvider : public QQuickImageProvider {
public:
    ThumbnailProvider();
    
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    
    void addThumbnail(int index, const QPixmap &pixmap);
    void clear();

private:
    QHash<int, QPixmap> m_thumbnails;
};

#endif // THUMBNAILPROVIDER_H
