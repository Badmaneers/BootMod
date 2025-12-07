#include "bitmapappcontext.h"


BitmapAppContext::BitmapAppContext()
{
    this->project = NULL;
    this->tool = NULL;
    this->workspace = NULL;
    this->layerManager = NULL;
    this->toolController = NULL;

    this->copymask = NULL;
}

BitmapAppContext::~BitmapAppContext()
{
    if(this->project) delete this->project;
    if(this->tool) delete this->tool;
    if(this->workspace) delete this->workspace;
    if(this->toolController) delete this->toolController;
    if(this->layerManager) delete this->layerManager;
}

void BitmapAppContext::setProject(Project *project)
{
    if(project == NULL) return;

    if(this->project) delete this->project;
    this->project = project;

    if(this->tool) this->tool->setProject(project);
    if(this->workspace) this->workspace->setProject(project);
    if(this->toolController) this->toolController->setProject(project);
    if(this->layerManager) this->layerManager->setProject(project);
}

void BitmapAppContext::setTool(Tool *tool)
{
    this->tool = tool;
    this->tool->setProject(this->project);
    if(this->workspace) this->workspace->setTool(tool);
    if(this->toolController) this->toolController->setTool(tool);
}

Project *BitmapAppContext::getProject() const
{
    return this->project;
}

Tool *BitmapAppContext::getTool() const
{
    return this->tool;
}

Workspace *BitmapAppContext::getWorkspace() const
{
    return this->workspace;
}

void BitmapAppContext::setWorkspace(Workspace *newWorkspace)
{
    if (this->workspace == newWorkspace)
        return;
    this->workspace = newWorkspace;
    this->workspace->setContext(this);
    this->workspace->setProject(this->project);
    emit workspaceChanged();
}

ToolController *BitmapAppContext::getToolController() const
{
    return this->toolController;
}

void BitmapAppContext::setToolController(ToolController *newToolController)
{
    if (this->toolController == newToolController)
        return;
    this->toolController = newToolController;
    this->toolController->setContext(this);
    emit toolControllerChanged();
}

BitmapLayerManager *BitmapAppContext::getLayerManager() const
{
    return this->layerManager;
}

void BitmapAppContext::setLayerManager(BitmapLayerManager *newLayerManager)
{
    if (this->layerManager == newLayerManager)
        return;
    this->layerManager = newLayerManager;
    this->layerManager->setContext(this);
    this->layerManager->setProject(this->project);
    emit layerManagerChanged();
}

bool BitmapAppContext::addTool(Tool *tool)
{
    if(tool!= NULL) {
        for(Tool *t: this->tools) {
            if(t != NULL) {
                if(t->getType() == tool->getType()) {
                    return false;
                }
            }
        }
        this->tools.push_back(tool);
    }
    return true;
}

bool BitmapAppContext::selectToolFromList(int toolType)
{
    for(Tool *tool: this->tools) {
        if(tool != NULL) {
            if(tool->getType() == toolType) {
                this->setTool(tool);
                return true;
            }
        }
    }
    return false;
}

void BitmapAppContext::copyMask(QBitmap *mask)
{
    if(mask == NULL) return;
    if(this->copymask) delete this->copymask;
    this->copymask = new QBitmap(mask->size());
    *this->copymask = *mask;
}

QBitmap *BitmapAppContext::getMaskCopy() const
{
    return this->copymask;
}

