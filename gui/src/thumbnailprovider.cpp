#include "thumbnailprovider.h"

ThumbnailProvider::ThumbnailProvider() 
    : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap ThumbnailProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    int index = id.toInt();
    
    if (m_thumbnails.contains(index)) {
        QPixmap pixmap = m_thumbnails[index];
        
        if (size) {
            *size = pixmap.size();
        }
        
        if (requestedSize.width() > 0 && requestedSize.height() > 0) {
            return pixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        
        return pixmap;
    }
    
    // Return empty pixmap if not found
    return QPixmap();
}

void ThumbnailProvider::addThumbnail(int index, const QPixmap &pixmap) {
    m_thumbnails[index] = pixmap;
}

void ThumbnailProvider::clear() {
    m_thumbnails.clear();
}
