#ifndef IMPORTIMAGE_H
#define IMPORTIMAGE_H

#include <QWidget>

#include "base/bitmapappcontext.h"


namespace Ui {
class ImportImage;
}

/**
 * @brief The ImportImage class
 */
class ImportImage : public QWidget
{
    Q_OBJECT
public:
    explicit ImportImage(BitmapAppContext *context, QWidget *parent = nullptr);
    ~ImportImage();

private slots:
    /**
     * @brief on_pushButton_import_clicked
     */
    void on_pushButton_import_clicked();

    /**
     * @brief on_pushButton_path_clicked
     */
    void on_pushButton_path_clicked();

private:
    Ui::ImportImage *ui;

    // kontext aplikace
    BitmapAppContext *context;
};

#endif // IMPORTIMAGE_H
