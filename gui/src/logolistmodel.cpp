#include "logolistmodel.h"
#include <QPixmap>

LogoListModel::LogoListModel(QObject *parent) : QAbstractListModel(parent) {}

int LogoListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() || !m_logoFile)
        return 0;
    return m_logoFile->logos().size();
}

QVariant LogoListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || !m_logoFile || index.row() >= m_logoFile->logos().size())
        return QVariant();
    
    const LogoEntry& logo = m_logoFile->logos().at(index.row());
    
    switch (role) {
    case IndexRole:
        return logo.index;
    case WidthRole:
        return logo.width;
    case HeightRole:
        return logo.height;
    case FormatRole:
        return logo.format;
    case SizeRole:
        return logo.size;
    case ThumbnailRole:
        return QPixmap::fromImage(logo.thumbnail);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> LogoListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IndexRole] = "logoIndex";
    roles[WidthRole] = "width";
    roles[HeightRole] = "height";
    roles[FormatRole] = "format";
    roles[SizeRole] = "size";
    roles[ThumbnailRole] = "thumbnail";
    return roles;
}

void LogoListModel::setLogoFile(LogoFile* file) {
    if (m_logoFile == file)
        return;
    
    if (m_logoFile) {
        disconnect(m_logoFile, nullptr, this, nullptr);
    }
    
    m_logoFile = file;
    
    if (m_logoFile) {
        connect(m_logoFile, &LogoFile::logoCountChanged, this, &LogoListModel::handleFileLoaded);
        connect(m_logoFile, &LogoFile::isLoadedChanged, this, [this]() {
            if (!m_logoFile->isLoaded()) {
                handleFileCleared();
            }
        });
    }
    
    emit logoFileChanged();
    handleFileLoaded();
}

void LogoListModel::handleFileLoaded() {
    beginResetModel();
    endResetModel();
}

void LogoListModel::handleFileCleared() {
    beginResetModel();
    endResetModel();
}
