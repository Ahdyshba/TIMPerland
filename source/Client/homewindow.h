#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include "qtablewidget.h"

#include <QMainWindow>
#include <vector>

namespace Ui {
class HomeWindow;
}

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();

private slots:
    void onImportXLSXClicked();
    void on_inputTable_itemChanged(QTableWidgetItem *item);
    void onClearTableClicked();
    void onInterpolateClicked();
    void onSaveGraphClicked();
    void onSaveXLSXClicked();

private:
    Ui::HomeWindow *ui;

    std::vector<double> x_points;
    std::vector<double> y_points;
    std::vector<double> lastDenseX = {0};
    std::vector<double> lastDenseY = {0};
    std::vector<double> lastWarningX = {1};
    std::vector<double> lastWarningY = {1};

    std::vector<double> getXValues();
    std::vector<double> getYValues();
    void plotGraph(const std::vector<double> &x_points, const std::vector<double> &y_points);
    void checkForOutliers(const std::vector<double> &x_points, const std::vector<double> &y_points);
};

#endif //HOMEWINDOW_H
