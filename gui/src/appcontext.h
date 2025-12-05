#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>
#include "thumbnailprovider.h"

class AppContext : public QObject {
    Q_OBJECT
    
public:
    static AppContext* instance();
    
    ThumbnailProvider* thumbnailProvider() const { return m_thumbnailProvider; }
    void setThumbnailProvider(ThumbnailProvider* provider) { m_thumbnailProvider = provider; }

private:
    explicit AppContext(QObject *parent = nullptr);
    static AppContext* s_instance;
    ThumbnailProvider* m_thumbnailProvider = nullptr;
};

#endif // APPCONTEXT_H
