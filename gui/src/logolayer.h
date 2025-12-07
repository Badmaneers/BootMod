#ifndef LOGOLAYER_H
#define LOGOLAYER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>

class LogoLayer : public QObject {
    Q_OBJECT
    Q_PROPERTY(int logoIndex READ logoIndex WRITE setLogoIndex NOTIFY logoIndexChanged)
    Q_PROPERTY(qreal xPosition READ xPosition WRITE setXPosition NOTIFY xPositionChanged)
    Q_PROPERTY(qreal yPosition READ yPosition WRITE setYPosition NOTIFY yPositionChanged)
    Q_PROPERTY(int zIndex READ zIndex WRITE setZIndex NOTIFY zIndexChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(int startFrame READ startFrame WRITE setStartFrame NOTIFY startFrameChanged)
    Q_PROPERTY(int endFrame READ endFrame WRITE setEndFrame NOTIFY endFrameChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString customImagePath READ customImagePath WRITE setCustomImagePath NOTIFY customImagePathChanged)
    Q_PROPERTY(QVariantList linkedLayers READ linkedLayers NOTIFY linkedLayersChanged)public:
    explicit LogoLayer(QObject *parent = nullptr);
    
    int logoIndex() const { return m_logoIndex; }
    void setLogoIndex(int index);
    
    qreal xPosition() const { return m_xPosition; }
    void setXPosition(qreal x);
    
    qreal yPosition() const { return m_yPosition; }
    void setYPosition(qreal y);
    
    int zIndex() const { return m_zIndex; }
    void setZIndex(int z);
    
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    
    int startFrame() const { return m_startFrame; }
    void setStartFrame(int frame);
    
    int endFrame() const { return m_endFrame; }
    void setEndFrame(int frame);
    
    bool visible() const { return m_visible; }
    void setVisible(bool visible);
    
    QString name() const { return m_name; }
    void setName(const QString &name);
    
    QString customImagePath() const { return m_customImagePath; }
    void setCustomImagePath(const QString &path);

    QVariantList linkedLayers() const;
    void addLinkedLayer(LogoLayer* layer);
    void removeLinkedLayer(LogoLayer* layer);
    
    Q_INVOKABLE bool isVisibleAtFrame(int frameIndex) const;
    Q_INVOKABLE QVariantMap toVariantMap() const;
    Q_INVOKABLE void fromVariantMap(const QVariantMap &map);
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void logoIndexChanged();
    void xPositionChanged();
    void yPositionChanged();
    void zIndexChanged();
    void opacityChanged();
    void startFrameChanged();
    void endFrameChanged();
    void visibleChanged();
    void nameChanged();
    void customImagePathChanged();
    void linkedLayersChanged();

private:
    int m_logoIndex = 0;
    qreal m_xPosition = 0.0;
    qreal m_yPosition = 0.0;
    int m_zIndex = 0;
    qreal m_opacity = 1.0;
    int m_startFrame = 0;
    int m_endFrame = -1;  // -1 means show in all frames
    bool m_visible = true;
    QString m_name;
    QString m_customImagePath;  // For merged layers
        QList<QObject*> m_linkedLayers;
};

class LayerManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int layerCount READ layerCount NOTIFY layerCountChanged)

public:
    explicit LayerManager(QObject *parent = nullptr);
    ~LayerManager();
    
    int layerCount() const { return m_layers.size(); }
    
    Q_INVOKABLE LogoLayer* createLayer();
    Q_INVOKABLE LogoLayer* getLayer(int index);
    Q_INVOKABLE void removeLayer(int index);
    Q_INVOKABLE void moveLayer(int fromIndex, int toIndex);
    Q_INVOKABLE void clearLayers();
    
    Q_INVOKABLE QList<LogoLayer*> getLayersForFrame(int frameIndex) const;
    Q_INVOKABLE QVariantList getLayerListForQml() const;
    
    Q_INVOKABLE bool saveToFile(const QString &filePath);
    Q_INVOKABLE bool loadFromFile(const QString &filePath);
    
    Q_INVOKABLE QString getLayerPreviewPath(int layerIndex, const QString &projectPath);
    
    // Layer merging - composite two layers into one
    Q_INVOKABLE bool mergeLayers(int upperIndex, int lowerIndex, const QString &projectPath, int canvasWidth, int canvasHeight);

signals:
    void layerCountChanged();
    void layerAdded(LogoLayer *layer);
    void layerRemoved(int index);
    void layerMoved(int fromIndex, int toIndex);
    void layersChanged();

private:
    QList<LogoLayer*> m_layers;
};

#endif // LOGOLAYER_H
