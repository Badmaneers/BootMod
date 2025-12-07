#include "commandmanager.h"
#include <QDebug>

// MoveLayerCommand implementation
MoveLayerCommand::MoveLayerCommand(LayerManager* manager, int layerIndex, QPointF oldPos, QPointF newPos)
    : m_manager(manager)
    , m_layerIndex(layerIndex)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
}

void MoveLayerCommand::execute() {
    auto layer = m_manager->getLayer(m_layerIndex);
    if (layer) {
        layer->setXPosition(m_newPos.x());
        layer->setYPosition(m_newPos.y());
    }
}

void MoveLayerCommand::undo() {
    auto layer = m_manager->getLayer(m_layerIndex);
    if (layer) {
        layer->setXPosition(m_oldPos.x());
        layer->setYPosition(m_oldPos.y());
    }
}

QString MoveLayerCommand::description() const {
    return QStringLiteral("Move Layer");
}

// OpacityCommand implementation
OpacityCommand::OpacityCommand(LayerManager* manager, int layerIndex, qreal oldOpacity, qreal newOpacity)
    : m_manager(manager)
    , m_layerIndex(layerIndex)
    , m_oldOpacity(oldOpacity)
    , m_newOpacity(newOpacity)
{
}

void OpacityCommand::execute() {
    auto layer = m_manager->getLayer(m_layerIndex);
    if (layer) {
        layer->setOpacity(m_newOpacity);
    }
}

void OpacityCommand::undo() {
    auto layer = m_manager->getLayer(m_layerIndex);
    if (layer) {
        layer->setOpacity(m_oldOpacity);
    }
}

QString OpacityCommand::description() const {
    return QStringLiteral("Change Opacity");
}

// AddLayerCommand implementation
AddLayerCommand::AddLayerCommand(LayerManager* manager, LogoLayer* layer)
    : m_manager(manager)
    , m_layer(layer)
    , m_ownsLayer(true)
    , m_insertedIndex(-1)
{
}

AddLayerCommand::~AddLayerCommand() {
    if (m_ownsLayer && m_layer) {
        delete m_layer;
    }
}

void AddLayerCommand::execute() {
    if (m_layer) {
        m_manager->addLayer(m_layer);
        m_insertedIndex = m_manager->layerCount() - 1;
        m_ownsLayer = false;
    }
}

void AddLayerCommand::undo() {
    if (m_insertedIndex >= 0 && m_insertedIndex < m_manager->layerCount()) {
        m_layer = m_manager->getLayer(m_insertedIndex);
        m_manager->removeLayer(m_insertedIndex);
        m_ownsLayer = true;
    }
}

QString AddLayerCommand::description() const {
    return QStringLiteral("Add Layer");
}

// RemoveLayerCommand implementation
RemoveLayerCommand::RemoveLayerCommand(LayerManager* manager, int layerIndex)
    : m_manager(manager)
    , m_layerIndex(layerIndex)
    , m_layer(nullptr)
    , m_ownsLayer(false)
{
}

RemoveLayerCommand::~RemoveLayerCommand() {
    if (m_ownsLayer && m_layer) {
        delete m_layer;
    }
}

void RemoveLayerCommand::execute() {
    if (m_layerIndex >= 0 && m_layerIndex < m_manager->layerCount()) {
        // Use takeLayer to get ownership without deleting
        m_layer = m_manager->takeLayer(m_layerIndex);
        m_ownsLayer = true;
        qDebug() << "RemoveLayerCommand::execute - Took layer at index" << m_layerIndex << "ptr:" << m_layer;
    }
}

void RemoveLayerCommand::undo() {
    if (m_layer && m_layerIndex >= 0) {
        // Re-insert at original index
        qDebug() << "RemoveLayerCommand::undo - Reinserting layer at index" << m_layerIndex << "ptr:" << m_layer;
        m_manager->insertLayer(m_layerIndex, m_layer);
        m_ownsLayer = false;  // Manager now owns it
    }
}

QString RemoveLayerCommand::description() const {
    return QStringLiteral("Remove Layer");
}

// CommandManager implementation
CommandManager::CommandManager(QObject* parent)
    : QObject(parent)
    , m_maxStackSize(50)
{
}

void CommandManager::executeCommand(QSharedPointer<Command> cmd) {
    if (!cmd) return;
    
    cmd->execute();
    m_undoStack.append(cmd);
    m_redoStack.clear();  // Clear redo stack on new command
    
    // Limit stack size
    if (m_undoStack.size() > m_maxStackSize) {
        m_undoStack.removeFirst();
    }
    
    emit stackChanged();
    qDebug() << "Command executed:" << cmd->description() << "| Undo stack size:" << m_undoStack.size();
}

bool CommandManager::canUndo() const {
    return !m_undoStack.isEmpty();
}

bool CommandManager::canRedo() const {
    return !m_redoStack.isEmpty();
}

QString CommandManager::undoText() const {
    if (canUndo()) {
        return QStringLiteral("Undo ") + m_undoStack.last()->description();
    }
    return QStringLiteral("Undo");
}

QString CommandManager::redoText() const {
    if (canRedo()) {
        return QStringLiteral("Redo ") + m_redoStack.last()->description();
    }
    return QStringLiteral("Redo");
}

void CommandManager::undo() {
    if (!canUndo()) return;
    
    auto cmd = m_undoStack.takeLast();
    cmd->undo();
    m_redoStack.append(cmd);
    
    emit stackChanged();
    qDebug() << "Undone:" << cmd->description();
}

void CommandManager::redo() {
    if (!canRedo()) return;
    
    auto cmd = m_redoStack.takeLast();
    cmd->execute();
    m_undoStack.append(cmd);
    
    emit stackChanged();
    qDebug() << "Redone:" << cmd->description();
}

void CommandManager::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
    emit stackChanged();
    qDebug() << "Command history cleared";
}

// Helper methods for QML
void CommandManager::executeMoveCommand(LayerManager* manager, int layerIndex, QPointF oldPos, QPointF newPos) {
    if (!manager) return;
    auto cmd = QSharedPointer<Command>(new MoveLayerCommand(manager, layerIndex, oldPos, newPos));
    executeCommand(cmd);
}

void CommandManager::executeOpacityCommand(LayerManager* manager, int layerIndex, qreal oldOpacity, qreal newOpacity) {
    if (!manager) return;
    auto cmd = QSharedPointer<Command>(new OpacityCommand(manager, layerIndex, oldOpacity, newOpacity));
    executeCommand(cmd);
}

void CommandManager::executeAddCommand(LayerManager* manager, int layerIndex, LogoLayer* layer) {
    if (!manager || !layer) return;
    // AddLayerCommand only takes manager and layer (it stores the index when executed)
    auto cmd = QSharedPointer<Command>(new AddLayerCommand(manager, layer));
    executeCommand(cmd);
}

void CommandManager::executeRemoveCommand(LayerManager* manager, int layerIndex, LogoLayer* layer) {
    if (!manager) return;
    // RemoveLayerCommand only takes manager and layerIndex (it gets the layer itself)
    auto cmd = QSharedPointer<Command>(new RemoveLayerCommand(manager, layerIndex));
    executeCommand(cmd);
}
