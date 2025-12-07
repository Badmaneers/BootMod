#include "logolayer.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QDir>
#include <QRegularExpression>

// LogoLayer implementation
LogoLayer::LogoLayer(QObject *parent)
    : QObject(parent)
{
}

void LogoLayer::setLogoIndex(int index) {
    if (m_logoIndex != index) {
        m_logoIndex = index;
        emit logoIndexChanged();
    }
}

void LogoLayer::setXPosition(qreal x) {
    if (m_xPosition != x) {
        m_xPosition = x;
        emit xPositionChanged();
    }
}

void LogoLayer::setYPosition(qreal y) {
    if (m_yPosition != y) {
        m_yPosition = y;
        emit yPositionChanged();
    }
}

void LogoLayer::setZIndex(int z) {
    if (m_zIndex != z) {
        m_zIndex = z;
        emit zIndexChanged();
    }
}

void LogoLayer::setOpacity(qreal opacity) {
    if (m_opacity != opacity) {
        m_opacity = qBound(0.0, opacity, 1.0);
        emit opacityChanged();
    }
}

void LogoLayer::setStartFrame(int frame) {
    if (m_startFrame != frame) {
        m_startFrame = frame;
        emit startFrameChanged();
    }
}

void LogoLayer::setEndFrame(int frame) {
    if (m_endFrame != frame) {
        m_endFrame = frame;
        emit endFrameChanged();
    }
}

void LogoLayer::setVisible(bool visible) {
    if (m_visible != visible) {
        m_visible = visible;
        emit visibleChanged();
    }
}

void LogoLayer::setName(const QString &name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void LogoLayer::setCustomImagePath(const QString &path) {
    if (m_customImagePath != path) {
        m_customImagePath = path;
        emit customImagePathChanged();
    }
}

QVariantList LogoLayer::linkedLayers() const {
    QVariantList list;
    for (QObject* obj : m_linkedLayers) {
        list.append(QVariant::fromValue(obj));
    }
    return list;
}

void LogoLayer::addLinkedLayer(LogoLayer* layer) {
    if (layer && !m_linkedLayers.contains(layer)) {
        m_linkedLayers.append(layer);
        emit linkedLayersChanged();
    }
}

void LogoLayer::removeLinkedLayer(LogoLayer* layer) {
    if (m_linkedLayers.removeAll(layer) > 0) {
        emit linkedLayersChanged();
    }
}

bool LogoLayer::isVisibleAtFrame(int frameIndex) const {
    if (!m_visible) return false;
    if (m_endFrame == -1) return frameIndex >= m_startFrame;
    return frameIndex >= m_startFrame && frameIndex <= m_endFrame;
}

QVariantMap LogoLayer::toVariantMap() const {
    QVariantMap map;
    map["logoIndex"] = m_logoIndex;
    map["xPosition"] = m_xPosition;
    map["yPosition"] = m_yPosition;
    map["zIndex"] = m_zIndex;
    map["opacity"] = m_opacity;
    map["startFrame"] = m_startFrame;
    map["endFrame"] = m_endFrame;
    map["visible"] = m_visible;
    map["name"] = m_name;
    return map;
}

void LogoLayer::fromVariantMap(const QVariantMap &map) {
    setLogoIndex(map.value("logoIndex", 0).toInt());
    setXPosition(map.value("xPosition", 0.0).toReal());
    setYPosition(map.value("yPosition", 0.0).toReal());
    setZIndex(map.value("zIndex", 0).toInt());
    setOpacity(map.value("opacity", 1.0).toReal());
    setStartFrame(map.value("startFrame", 0).toInt());
    setEndFrame(map.value("endFrame", -1).toInt());
    setVisible(map.value("visible", true).toBool());
    setName(map.value("name", "").toString());
}

QJsonObject LogoLayer::toJson() const {
    QJsonObject obj;
    obj["logoIndex"] = m_logoIndex;
    obj["xPosition"] = m_xPosition;
    obj["yPosition"] = m_yPosition;
    obj["zIndex"] = m_zIndex;
    obj["opacity"] = m_opacity;
    obj["startFrame"] = m_startFrame;
    obj["endFrame"] = m_endFrame;
    obj["visible"] = m_visible;
    obj["name"] = m_name;
    return obj;
}

void LogoLayer::fromJson(const QJsonObject &json) {
    setLogoIndex(json.value("logoIndex").toInt(0));
    setXPosition(json.value("xPosition").toDouble(0.0));
    setYPosition(json.value("yPosition").toDouble(0.0));
    setZIndex(json.value("zIndex").toInt(0));
    setOpacity(json.value("opacity").toDouble(1.0));
    setStartFrame(json.value("startFrame").toInt(0));
    setEndFrame(json.value("endFrame").toInt(-1));
    setVisible(json.value("visible").toBool(true));
    setName(json.value("name").toString(""));
}

// LayerManager implementation
LayerManager::LayerManager(QObject *parent)
    : QObject(parent)
{
}

LayerManager::~LayerManager() {
    qDeleteAll(m_layers);
}

LogoLayer* LayerManager::createLayer() {
    LogoLayer *layer = new LogoLayer(this);
    layer->setName(QString("Layer %1").arg(m_layers.size() + 1));
    layer->setZIndex(m_layers.size());
    m_layers.append(layer);
    
    emit layerAdded(layer);
    emit layerCountChanged();
    // DON'T emit layersChanged() - let the Repeater handle it via layerCountChanged
    
    return layer;
}

LogoLayer* LayerManager::getLayer(int index) {
    if (index >= 0 && index < m_layers.size()) {
        return m_layers[index];
    }
    return nullptr;
}

void LayerManager::addLayer(LogoLayer* layer) {
    if (!layer) return;
    
    layer->setParent(this);
    m_layers.append(layer);
    
    emit layerAdded(layer);
    emit layerCountChanged();
}

void LayerManager::insertLayer(int index, LogoLayer* layer) {
    if (!layer) return;
    
    layer->setParent(this);
    
    if (index < 0 || index >= m_layers.size()) {
        m_layers.append(layer);
    } else {
        m_layers.insert(index, layer);
    }
    
    emit layerAdded(layer);
    emit layerCountChanged();
}

void LayerManager::setSelectedLayerIndex(int index) {
    if (m_selectedLayerIndex != index) {
        m_selectedLayerIndex = index;
        emit selectedLayerIndexChanged();
    }
}

void LayerManager::removeLayer(int index) {
    if (index < 0 || index >= m_layers.size()) {
        qDebug() << "LayerManager::removeLayer - Invalid index:" << index;
        return;
    }
    
    LogoLayer *layer = m_layers.takeAt(index);
    
    // Check if this layer has linked layers - they should be children
    // so they'll be deleted automatically, but we need to clear our list first
    if (!layer->linkedLayers().isEmpty()) {
        qDebug() << "LayerManager::removeLayer - Layer has" << layer->linkedLayers().size() << "linked layers";
    }
    
    // Check if this layer is linked to any other layer
    // If so, we need to remove it from those linked lists before deleting
    for (LogoLayer *otherLayer : m_layers) {
        if (otherLayer) {
            otherLayer->removeLinkedLayer(layer);
        }
    }
    
    // Update selectedLayerIndex if needed
    if (m_selectedLayerIndex == index) {
        // If we deleted the selected layer, deselect
        setSelectedLayerIndex(-1);
    } else if (m_selectedLayerIndex > index) {
        // If selected layer is after the deleted one, shift index down
        setSelectedLayerIndex(m_selectedLayerIndex - 1);
    }
    
    delete layer;
    
    emit layerRemoved(index);
    emit layerCountChanged();
    // DON'T emit layersChanged() - let Repeater handle via layerCountChanged
}

LogoLayer* LayerManager::takeLayer(int index) {
    if (index < 0 || index >= m_layers.size()) {
        qDebug() << "LayerManager::takeLayer - Invalid index:" << index;
        return nullptr;
    }
    
    LogoLayer *layer = m_layers.takeAt(index);
    
    // Check if this layer is linked to any other layer
    // If so, we need to remove it from those linked lists
    for (LogoLayer *otherLayer : m_layers) {
        if (otherLayer) {
            otherLayer->removeLinkedLayer(layer);
        }
    }
    
    // Update selectedLayerIndex if needed
    if (m_selectedLayerIndex == index) {
        // If we removed the selected layer, deselect
        setSelectedLayerIndex(-1);
    } else if (m_selectedLayerIndex > index) {
        // If selected layer is after the removed one, shift index down
        setSelectedLayerIndex(m_selectedLayerIndex - 1);
    }
    
    // Return the layer WITHOUT deleting it - caller takes ownership
    emit layerRemoved(index);
    emit layerCountChanged();
    return layer;
}


void LayerManager::moveLayer(int fromIndex, int toIndex) {
    if (fromIndex >= 0 && fromIndex < m_layers.size() &&
        toIndex >= 0 && toIndex < m_layers.size() &&
        fromIndex != toIndex) {
        
        m_layers.move(fromIndex, toIndex);
        
        // Update z-indices
        for (int i = 0; i < m_layers.size(); ++i) {
            m_layers[i]->setZIndex(i);
        }
        
        emit layerMoved(fromIndex, toIndex);
        emit layersChanged();
    }
}

void LayerManager::clearLayers() {
    qDeleteAll(m_layers);
    m_layers.clear();
    emit layerCountChanged();
    emit layersChanged();
}

QList<LogoLayer*> LayerManager::getLayersForFrame(int frameIndex) const {
    QList<LogoLayer*> visibleLayers;
    for (LogoLayer *layer : m_layers) {
        if (layer->isVisibleAtFrame(frameIndex)) {
            visibleLayers.append(layer);
        }
    }
    
    // Sort by z-index
    std::sort(visibleLayers.begin(), visibleLayers.end(),
              [](const LogoLayer *a, const LogoLayer *b) {
                  return a->zIndex() < b->zIndex();
              });
    
    return visibleLayers;
}

QVariantList LayerManager::getLayerListForQml() const {
    QVariantList list;
    for (const LogoLayer *layer : m_layers) {
        list.append(layer->toVariantMap());
    }
    return list;
}

bool LayerManager::saveToFile(const QString &filePath) {
    QJsonArray layersArray;
    for (const LogoLayer *layer : m_layers) {
        layersArray.append(layer->toJson());
    }
    
    QJsonObject root;
    root["version"] = "1.0";
    root["layers"] = layersArray;
    
    QJsonDocument doc(root);
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "Saved" << m_layers.size() << "layers to" << filePath;
    return true;
}

bool LayerManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in file:" << filePath;
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray layersArray = root.value("layers").toArray();
    
    clearLayers();
    
    for (const QJsonValue &value : layersArray) {
        if (value.isObject()) {
            LogoLayer *layer = createLayer();
            layer->fromJson(value.toObject());
        }
    }
    
    qDebug() << "Loaded" << m_layers.size() << "layers from" << filePath;
    emit layersChanged();
    
    return true;
}

QString LayerManager::getLayerPreviewPath(int layerIndex, const QString &projectPath) {
    if (layerIndex < 0 || layerIndex >= m_layers.size()) {
        return QString();
    }
    
    LogoLayer *layer = m_layers[layerIndex];
    int logoIndex = layer->logoIndex();
    
    // This should match the format used by LogoFile
    return QString("%1/images/logo_%2.png").arg(projectPath).arg(logoIndex);
}

bool LayerManager::mergeLayers(int upperIndex, int lowerIndex, const QString &projectPath, int canvasWidth, int canvasHeight) {
    if (upperIndex < 0 || upperIndex >= m_layers.size() ||
        lowerIndex < 0 || lowerIndex >= m_layers.size()) {
        qWarning() << "Invalid layer indices for merge:" << upperIndex << lowerIndex;
        return false;
    }
    
    LogoLayer *upperLayer = m_layers[upperIndex];
    LogoLayer *lowerLayer = m_layers[lowerIndex];
    
    if (!upperLayer || !lowerLayer) {
        qWarning() << "Null layers in merge";
        return false;
    }
    
    // Can't merge auto-animation layer
    if (upperLayer->endFrame() == -1 || lowerLayer->endFrame() == -1) {
        qWarning() << "Cannot merge auto-animation layer";
        return false;
    }
    
    qDebug() << "Layer linking merge: Linking" << upperLayer->name() << "to" << lowerLayer->name();

    // IMPORTANT: Change the parent of upperLayer to lowerLayer before linking
    // This prevents it from being deleted when removed from the main list
    upperLayer->setParent(lowerLayer);
    
    // Link the upper layer to the lower layer
    lowerLayer->addLinkedLayer(upperLayer);
    lowerLayer->setName(lowerLayer->name() + " + " + upperLayer->name());
    // Optionally, update frame range
    lowerLayer->setStartFrame(qMin(lowerLayer->startFrame(), upperLayer->startFrame()));
    lowerLayer->setEndFrame(qMax(lowerLayer->endFrame(), upperLayer->endFrame()));

    // Remove upper layer from main list (but don't delete it - it's now owned by lowerLayer)
    m_layers.removeAt(upperIndex);
    emit layerRemoved(upperIndex);
    emit layerCountChanged();
    // DON'T emit layersChanged() here - it causes Repeater to recreate all delegates
    
    qDebug() << "Layer linking merge complete!";
    qDebug() << "  Linked layer:" << lowerLayer->name();
    qDebug() << "  Frame range:" << lowerLayer->startFrame() << "-" << lowerLayer->endFrame();

    return true;
}

