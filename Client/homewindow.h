#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H


#include "qtablewidget.h"
#include <QMainWindow>
#include <vector>


namespace Ui { class HomeWindow; }

class HomeWindow : public QMainWindow {
    Q_OBJECT

public:
    HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();

private slots:
    void onInterpolateClicked();
    void onImportXLSXClicked();
    void onSaveXLSXClicked();
    void onSaveGraphClicked();
    void onDepthChanged(int value);
    void on_inputTable_itemChanged();

private:
    Ui::HomeWindow *ui;
    std::vector<double> getXValues();
    std::vector<double> getYValues();
    void plotGraph(const std::vector<double>& x_points, const std::vector<double>& y_points, int depth);
    std::vector<double> x_points;
    std::vector<double> y_points;
    int currentDepth = 1; // default value
};


#endif //HOMEWINDOW_H
