#include "thumbnailprovider.h"
#include <QDebug>

ThumbnailProvider::ThumbnailProvider() 
    : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap ThumbnailProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    // Parse index from id, ignoring any query parameters (e.g., "1?t=12345" -> 1)
    QString idStr = id;
    int queryPos = idStr.indexOf('?');
    if (queryPos > 0) {
        idStr = idStr.left(queryPos);
    }
    
    int index = idStr.toInt();
    
    qDebug() << "ThumbnailProvider::requestPixmap - Requested id:" << id << "Parsed index:" << index << "Available thumbnails:" << m_thumbnails.keys();
    
    if (m_thumbnails.contains(index)) {
        QPixmap pixmap = m_thumbnails[index];
        qDebug() << "Found thumbnail for index" << index << "size:" << pixmap.size();
        
        if (size) {
            *size = pixmap.size();
        }
        
        if (requestedSize.width() > 0 && requestedSize.height() > 0) {
            return pixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        
        return pixmap;
    }
    
    qDebug() << "Thumbnail not found for index" << index;
    // Return empty pixmap if not found
    return QPixmap();
}

void ThumbnailProvider::addThumbnail(int index, const QPixmap &pixmap) {
    m_thumbnails[index] = pixmap;
}

void ThumbnailProvider::clear() {
    m_thumbnails.clear();
}
