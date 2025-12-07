#ifndef EXPORTPROJECT_H
#define EXPORTPROJECT_H

#include <QWidget>

#include "base/bitmapappcontext.h"

namespace Ui {
class ExportProject;
}

class ExportProject : public QWidget
{
    Q_OBJECT

public:
    explicit ExportProject(BitmapAppContext *context, QWidget *parent = nullptr);
    ~ExportProject();

protected:
    void paintPreview();

private slots:
    void on_pushButton_export_clicked();

    void on_pushButton_path_clicked();

private:
    Ui::ExportProject *ui;

    BitmapAppContext *context;

    float scale;
};

#endif // EXPORTPROJECT_H
