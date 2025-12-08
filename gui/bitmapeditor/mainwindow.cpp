#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTimer>

#include "tool/pencil.h"
#include "tool/eraser.h"
#include "tool/brush.h"
#include "tool/fillcolor.h"
#include "tool/eyedropper.h"
#include "tool/text.h"
#include "layer/bitmaplayer.h"
#include "layer/imagelayer.h"


#define DIALOG_CLOSE_MSG tr("Are you sure you want to quit the app? If you haven't saved the project, you will lose your last unsaved work.")


MainWindow::MainWindow(BitmapAppContext *context, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->context = context;

    /*****************************************************************************/
    // Basic UI initialization
    this->ui->setupUi(this);
    
    // Set window title for BootMod integration
    this->setWindowTitle(tr("Logo Frame Editor - BootMod"));
    
    // Setup toolbar
    this->colorPicker = new ColorPicker(this->ui->toolBar);
    this->ui->toolBar->addWidget(this->colorPicker);
    this->ui->toolBar->addAction(this->ui->actionZoom_in);
    this->ui->toolBar->addAction(this->ui->actionZoom_out);
    this->ui->toolBar->addAction(this->ui->actionEye_Dropper);
    this->ui->toolBar->addAction(this->ui->actionPen);
    this->ui->toolBar->addAction(this->ui->actionEraser);
    this->ui->toolBar->addAction(this->ui->actionBrush);
    this->ui->toolBar->addAction(this->ui->actionFill);
    this->ui->toolBar->addAction(this->ui->actionText);
    
    // Hide/disable features not needed for logo editing
    this->ui->actionNew_project->setVisible(false);
    this->ui->actionOpen_project->setVisible(false);
    this->ui->actionSave_project->setVisible(false);
    this->ui->actionPrint->setVisible(false);
    this->ui->actionAbout->setVisible(false);
    
    // Rename menu items for BootMod context
    this->ui->actionImport_image->setText(tr("Import Image..."));
    this->ui->actionExport_image->setText(tr("Export as PNG..."));
    
    // Label for status bar
    this->statusLabel = new QLabel(this->ui->statusbar);
    this->statusLabel->setAlignment(Qt::AlignLeft);
    this->statusLabel->setText(tr("Logo Frame Editor"));
    this->statusBar()->addPermanentWidget(this->statusLabel, 1);
    /*****************************************************************************/


    /*****************************************************************************/
    // inicializace kontextu
    this->context->setWorkspace(new Workspace(Workspace_defaultConfig(), this));
    this->context->setToolController(new ToolController(this));
    this->context->setLayerManager(new BitmapLayerManager(this));
    this->context->addTool(new PencilTool(this, this->colorPicker));
    this->context->addTool(new EraserTool(this));
    this->context->addTool(new BrushTool(this, this->colorPicker));
    this->context->addTool(new FillColorTool(this, this->colorPicker));
    this->context->addTool(new EyeDropperTool(this, this->colorPicker));
    this->context->addTool(new TextTool(this));
    this->context->selectToolFromList(TOOL_PENCIL);
    /*****************************************************************************/


    /*****************************************************************************/
    // sestaveni celkove pracovni plochy s vyuzitim splitteru
    this->splitter_horizontal = new QSplitter(Qt::Horizontal);
    this->splitter_horizontal->setObjectName("bg-widget");
    // leva strana (pracovani plocha pro upravu bitmapove grafiky)
    this->splitter_horizontal->addWidget(this->context->getWorkspace());

    // prava strany (manazer vrstev + ovladani aktualne vybraneho nastroje)
    this->splitter_vertical = new QSplitter(Qt::Vertical);
    this->splitter_vertical->setObjectName("bg-widget");
    // horni strana (ovladani nastroje)
    this->splitter_vertical->addWidget(this->context->getToolController());
    // dolni strana (manazer vrstev)
    this->splitter_vertical->addWidget(this->context->getLayerManager());

    this->splitter_horizontal->addWidget(this->splitter_vertical);
    this->setCentralWidget(this->splitter_horizontal);
    this->splitter_horizontal->setStretchFactor(0, 3);
    this->splitter_horizontal->setStretchFactor(1, 1);
    /*****************************************************************************/


    /*****************************************************************************/
    // inicializace oken
    this->window_importImage = new ImportImage(this->context);
    this->window_exportProject = new ExportProject(this->context);
    /*****************************************************************************/


    /*****************************************************************************/
    // vlastni styly
    this->splitter_horizontal->setStyleSheet("QWidget#bg-widget {background: rgb(49, 49, 49);}");
    /*****************************************************************************/


    // finish
    this->updateStatusBar();
    this->highlightToolbar(this->ui->actionPen);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->context;

    if(this->colorPicker) delete this->colorPicker;
    if(this->splitter_horizontal) delete this->splitter_horizontal;
    if(this->statusLabel) delete this->statusLabel;

    // Delete dialogs
    if(this->window_importImage) delete this->window_importImage;
    if(this->window_exportProject) delete this->window_exportProject;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QTimer::singleShot(200, this, SLOT(workspace_setDefaultScale()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Simplified close - auto-saves via BitmapEditorWrapper event filter
    // No confirmation needed since changes are saved automatically
    event->accept();
}

void MainWindow::updateStatusBar()
{
    Project *p = this->context->getProject();
    if(p == NULL) {
        this->statusLabel->setText(tr("Logo Frame Editor"));
    } else {
        this->statusLabel->setText(QString(tr("Editing: %1x%2 Logo Frame")).arg(p->getSize().width()).arg(p->getSize().height()));
    }
}

void MainWindow::on_actionExport_image_triggered()
{
    this->window_exportProject->show();
}

void MainWindow::on_actionImport_image_triggered()
{
    this->window_importImage->show();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}


void MainWindow::on_actionEye_Dropper_triggered()
{
    if(this->context == NULL) return;
    this->context->selectToolFromList(TOOL_EYEDROPPER);
    this->highlightToolbar(this->ui->actionEye_Dropper);
}

void MainWindow::on_actionPen_triggered()
{
    if(this->context == NULL) return;
    this->context->selectToolFromList(TOOL_PENCIL);
    this->highlightToolbar(this->ui->actionPen);
}


void MainWindow::on_actionBrush_triggered()
{
    if(this->context == NULL) return;
    this->context->selectToolFromList(TOOL_BRUSH);
    this->highlightToolbar(this->ui->actionBrush);
}


void MainWindow::on_actionFill_triggered()
{
    if(this->context == NULL) return;
    this->context->selectToolFromList(TOOL_FILLCOLOR);
    this->highlightToolbar(this->ui->actionFill);
}


void MainWindow::on_actionText_triggered()
{
    if(this->context == NULL) return;
    this->context->selectToolFromList(TOOL_TEXT);
    this->highlightToolbar(this->ui->actionText);
}


void MainWindow::on_actionEraser_triggered()
{
    if(this->context == NULL) return;
    this->context->selectToolFromList(TOOL_ERASER);
    this->highlightToolbar(this->ui->actionEraser);
}


void MainWindow::on_actionAdd_layer_triggered()
{
    if(this->context == NULL) return;
    BitmapLayerManager *lm = this->context->getLayerManager();
    if(lm) {
        lm->addLayer();
    }
}


void MainWindow::on_actionRemove_layer_triggered()
{
    if(this->context == NULL) return;
    BitmapLayerManager *lm = this->context->getLayerManager();
    if(lm) {
        lm->removeLayer();
    }
}

void MainWindow::on_actionConvert_to_Bitmap_triggered()
{
    if(this->context == NULL) return;
    
    Project *project = this->context->getProject();
    if(project == NULL) {
        QMessageBox::warning(
            this,
            tr("Convert to Bitmap"),
            tr("No project is open."));
        return;
    }
    
    // Get current layer
    Layer *currentLayer = project->getSelectedLayer();
    if(currentLayer == NULL) {
        QMessageBox::warning(
            this,
            tr("Convert to Bitmap"),
            tr("No layer selected."));
        return;
    }
    
    // Check if it's an ImageLayer
    if(currentLayer->getType() != IMAGE_LAYER_TYPE) {
        QMessageBox::information(
            this,
            tr("Convert to Bitmap"),
            tr("Selected layer is not an image layer. Only image layers can be converted to bitmap layers."));
        return;
    }
    
    ImageLayer *imageLayer = (ImageLayer*)currentLayer;
    
    // Create a new BitmapLayer with the same properties
    BitmapLayer *bitmapLayer = new BitmapLayer(
        project,
        imageLayer->getName(),
        imageLayer->image.size()
    );
    
    // Copy the image content
    QPainter painter(&bitmapLayer->image);
    painter.drawImage(0, 0, imageLayer->image);
    painter.end();
    
    // Copy layer properties (using public getters/setters)
    bitmapLayer->setOpacity(imageLayer->getOpacity());
    bitmapLayer->setBlendMode(imageLayer->getBlendMode());
    bitmapLayer->enableAntialiasing(imageLayer->isAntialiasingEnabled());
    
    // Find the index of the current layer
    Layers_t *layers = project->getLayers();
    int layerIndex = -1;
    for(int i = 0; i < layers->size(); i++) {
        if(layers->at(i) == currentLayer) {
            layerIndex = i;
            break;
        }
    }
    
    if(layerIndex < 0) {
        delete bitmapLayer;
        QMessageBox::warning(
            this,
            tr("Convert to Bitmap"),
            tr("Could not find layer in project."));
        return;
    }
    
    // Remove the old layer and insert the new one at the same position
    project->removeLayer(imageLayer);
    project->insertLayer(layerIndex, bitmapLayer);
    project->setSelectedLayer(bitmapLayer);
    
    // Repaint
    project->requestRepaint();
    
    QMessageBox::information(
        this,
        tr("Convert to Bitmap"),
        tr("Image layer successfully converted to bitmap layer. You can now edit it with drawing tools."));
}


void MainWindow::highlightToolbar(QAction *action)
{
    QWidget *w;
    for(QAction *a :this->ui->toolBar->actions()) {
        if(a != NULL) {
            w = this->ui->toolBar->widgetForAction(a);
            if(w != NULL && w != this->colorPicker) {
                w->setStyleSheet("");
            }
        }
    }
    w = this->ui->toolBar->widgetForAction(action);
    if(w != NULL) {
        w->setStyleSheet("background: rgba(183, 134, 32, 20%); border: 1px solid #b78620;");
    }
}

void MainWindow::on_actionZoom_in_triggered()
{
    if(this->context == NULL) return;
    if(this->context->getWorkspace() == NULL) return;
    this->context->getWorkspace()->zoomIN();
    this->context->getWorkspace()->repaint();
}


void MainWindow::on_actionZoom_out_triggered()
{
    if(this->context == NULL) return;
    if(this->context->getWorkspace() == NULL) return;
    this->context->getWorkspace()->zoomOUT();
    this->context->getWorkspace()->repaint();
}



void MainWindow::on_actionSwitch_Image_Mask_triggered()
{
    if(this->context == NULL) return;

    // zmeni mod (PROJECT_EDIT <-> MASK_EDIT)
    if(this->context->getLayerManager() != NULL &&
            this->context->getProject() != NULL) {
        if(this->context->getProject()->getMode() == PROJECT_EDIT) {
            this->context->getLayerManager()->selectMode(MASK_EDIT);
        } else {
            this->context->getLayerManager()->selectMode(PROJECT_EDIT);
        }
    }
}

void MainWindow::workspace_setDefaultScale()
{
    if(this->context == NULL) return;
    Workspace *w = this->context->getWorkspace();
    if(w == NULL) return;
    w->setDefaultScale();
}

