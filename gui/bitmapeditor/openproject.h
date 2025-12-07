#ifndef OPENPROJECT_H
#define OPENPROJECT_H

#include <QWidget>

#include "base/bitmapappcontext.h"


namespace Ui {
class OpenProject;
}

/**
 * @brief The NewProject class
 */
class OpenProject : public QWidget
{
    Q_OBJECT

public:
    explicit OpenProject(BitmapAppContext *context, QWidget *parent = nullptr);
    ~OpenProject();

private slots:
    /**
     * @brief on_pushButton_clicked
     */
    void on_pushButton_clicked();

    /**
     * @brief on_pushButton_path_clicked
     */
    void on_pushButton_path_clicked();

private:
    Ui::OpenProject *ui;

    // kontext aplikace
    BitmapAppContext *context;

signals:
    void projectOpened();
};

#endif // NEWPROJECT_H
