#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>
#include <QString>
#include <QPointF>
#include "logolayer.h"

// Base command interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual QString description() const = 0;
};

// Command for moving a layer
class MoveLayerCommand : public Command {
public:
    MoveLayerCommand(LayerManager* manager, int layerIndex, QPointF oldPos, QPointF newPos);
    void execute() override;
    void undo() override;
    QString description() const override;

private:
    LayerManager* m_manager;
    int m_layerIndex;
    QPointF m_oldPos;
    QPointF m_newPos;
};

// Command for changing layer opacity
class OpacityCommand : public Command {
public:
    OpacityCommand(LayerManager* manager, int layerIndex, qreal oldOpacity, qreal newOpacity);
    void execute() override;
    void undo() override;
    QString description() const override;

private:
    LayerManager* m_manager;
    int m_layerIndex;
    qreal m_oldOpacity;
    qreal m_newOpacity;
};

// Command for adding a layer
class AddLayerCommand : public Command {
public:
    AddLayerCommand(LayerManager* manager, LogoLayer* layer);
    ~AddLayerCommand() override;
    void execute() override;
    void undo() override;
    QString description() const override;

private:
    LayerManager* m_manager;
    LogoLayer* m_layer;
    bool m_ownsLayer;
    int m_insertedIndex;
};

// Command for removing a layer
class RemoveLayerCommand : public Command {
public:
    RemoveLayerCommand(LayerManager* manager, int layerIndex);
    ~RemoveLayerCommand() override;
    void execute() override;
    void undo() override;
    QString description() const override;

private:
    LayerManager* m_manager;
    int m_layerIndex;
    LogoLayer* m_layer;
    bool m_ownsLayer;
};

// Command Manager - manages undo/redo stacks
class CommandManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY stackChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY stackChanged)
    Q_PROPERTY(QString undoText READ undoText NOTIFY stackChanged)
    Q_PROPERTY(QString redoText READ redoText NOTIFY stackChanged)

public:
    explicit CommandManager(QObject* parent = nullptr);

    Q_INVOKABLE void executeCommand(QSharedPointer<Command> cmd);
    Q_INVOKABLE bool canUndo() const;
    Q_INVOKABLE bool canRedo() const;
    Q_INVOKABLE QString undoText() const;
    Q_INVOKABLE QString redoText() const;
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clear();
    
    // Helper methods for QML to create and execute commands
    Q_INVOKABLE void executeMoveCommand(LayerManager* manager, int layerIndex, QPointF oldPos, QPointF newPos);
    Q_INVOKABLE void executeOpacityCommand(LayerManager* manager, int layerIndex, qreal oldOpacity, qreal newOpacity);
    Q_INVOKABLE void executeAddCommand(LayerManager* manager, int layerIndex, LogoLayer* layer);
    Q_INVOKABLE void executeRemoveCommand(LayerManager* manager, int layerIndex, LogoLayer* layer);

    void setMaxStackSize(int size) { m_maxStackSize = size; }
    int maxStackSize() const { return m_maxStackSize; }

signals:
    void stackChanged();

private:
    QVector<QSharedPointer<Command>> m_undoStack;
    QVector<QSharedPointer<Command>> m_redoStack;
    int m_maxStackSize;
};

#endif // COMMANDMANAGER_H
