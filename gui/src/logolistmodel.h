#ifndef LOGOLISTMODEL_H
#define LOGOLISTMODEL_H

#include <QAbstractListModel>
#include "logofile.h"

class LogoListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(LogoFile* logoFile READ logoFile WRITE setLogoFile NOTIFY logoFileChanged)

public:
    enum LogoRoles {
        IndexRole = Qt::UserRole + 1,
        WidthRole,
        HeightRole,
        FormatRole,
        SizeRole,
        ThumbnailRole
    };

    explicit LogoListModel(QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    LogoFile* logoFile() const { return m_logoFile; }
    void setLogoFile(LogoFile* file);

signals:
    void logoFileChanged();

private slots:
    void handleFileLoaded();
    void handleFileCleared();

private:
    LogoFile* m_logoFile = nullptr;
};

#endif // LOGOLISTMODEL_H
