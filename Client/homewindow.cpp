#include "homewindow.h"
#include "ui_homewindow.h"
#include "interpolator.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include "xlsxdocument.h"
using namespace QXlsx;


HomeWindow::HomeWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::HomeWindow) {
    ui->setupUi(this);
    connect(ui->interpolateButton, &QPushButton::clicked, this, &HomeWindow::onInterpolateClicked);
    connect(ui->loadXLSXButton, &QPushButton::clicked, this, &HomeWindow::onImportXLSXClicked);
    connect(ui->saveXLSXButton, &QPushButton::clicked, this, &HomeWindow::onSaveXLSXClicked);
    connect(ui->saveGraphButton, &QPushButton::clicked, this, &HomeWindow::onSaveGraphClicked);
    connect(ui->depthSlider, &QSlider::valueChanged, this, &HomeWindow::onDepthChanged);
    connect(ui->depthSlider, &QSlider::valueChanged, this, [=](int value) {
        ui->depthLabel->setText(QString("Depth: %1").arg(value));
    });
    connect(ui->inputTable, &QTableWidget::itemChanged, this, &HomeWindow::on_inputTable_itemChanged);
}

HomeWindow::~HomeWindow() {
    delete ui;
}

std::vector<double> HomeWindow::getXValues() {
    std::vector<double> x;
    int rows = ui->inputTable->rowCount();
    for (int i = 0; i < rows; ++i) {
        auto item = ui->inputTable->item(i, 0);
        if (item && !item->text().isEmpty()) {
            bool ok;
            double val = item->text().toDouble(&ok);
            if (ok) x.push_back(val);
        }
    }
    return x;
}

std::vector<double> HomeWindow::getYValues() {
    std::vector<double> y;
    int rows = ui->inputTable->rowCount();
    for (int i = 0; i < rows; ++i) {
        auto item = ui->inputTable->item(i, 1);
        if (item && !item->text().isEmpty()) {
            bool ok;
            double val = item->text().toDouble(&ok);
            if (ok) y.push_back(val);
        }
    }
    return y;
}

void HomeWindow::onInterpolateClicked() {
    std::vector<double> x_points = getXValues();
    std::vector<double> y_points = getYValues();
    int depth = ui->depthSlider->value();

    try {
        interpolator interp;
        interp.setData(x_points, y_points);

        auto coefficients = interp.interpolate();

        plotGraph(x_points, y_points, depth);

        ui->saveGraphButton->setEnabled(true);
        ui->saveXLSXButton->setEnabled(true);

    } catch (const std::exception& ex) {
        QMessageBox::warning(this, "Interpolation Error", ex.what());
    }
}

void HomeWindow::onImportXLSXClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open XLSX File", "", "Excel Files (*.xlsx)");
    if (fileName.isEmpty()) return;

    Document xlsx(fileName);
    if (!xlsx.load()) {
        QMessageBox::warning(this, "Error", "Failed to open XLSX file.");
        return;
    }

    int row = 1;

    ui->inputTable->clearContents();
    ui->inputTable->setRowCount(0);

    while (!xlsx.read(row, 1).toString().isEmpty()) {
        ui->inputTable->insertRow(ui->inputTable->rowCount());

        QString x = xlsx.read(row, 1).toString();
        QString y = xlsx.read(row, 2).toString();

        ui->inputTable->setItem(row - 1, 0, new QTableWidgetItem(x));
        ui->inputTable->setItem(row - 1, 1, new QTableWidgetItem(y));

        ++row;
    }

    if (row == 1) { ui->inputTable->insertRow(ui->inputTable->rowCount()); }

    QMessageBox::information(this, "Import", "Excel data loaded successfully.");
}

void HomeWindow::onSaveXLSXClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save XLSX", "output.xlsx", "Excel Files (*.xlsx)");
    if (fileName.isEmpty()) return;

    // Gather input points from table
    std::vector<double> x_points, y_points;
    int rowCount = ui->inputTable->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QTableWidgetItem *xItem = ui->inputTable->item(i, 0);
        QTableWidgetItem *yItem = ui->inputTable->item(i, 1);
        if (xItem && yItem) {
            x_points.push_back(xItem->text().toDouble());
            y_points.push_back(yItem->text().toDouble());
        }
    }

    if (x_points.size() < 2) {
        QMessageBox::warning(this, "Error", "Not enough points to interpolate.");
        return;
    }

    // Create interpolator
    interpolator interp;
    interp.setData(x_points, y_points);

    // Get interpolation depth from slider (assume you have it in UI)
    int depth = ui->depthSlider->value();

    // Generate dense x values for interpolation
    std::vector<double> dense_x;
    for (size_t i = 0; i < x_points.size() - 1; ++i) {
        double start = x_points[i];
        double end = x_points[i + 1];
        int segments = depth + 1;
        for (int j = 0; j < segments; ++j) {
            double val = start + (end - start) * j / segments;
            dense_x.push_back(val);
        }
    }
    dense_x.push_back(x_points.back());

    // Evaluate interpolated y values
    std::vector<double> dense_y;
    for (double x : dense_x) {
        dense_y.push_back(interp.evaluateLagrange(x_points, y_points, x));
    }

    Document xlsx;

    for (size_t i = 0; i < dense_x.size(); ++i) {
        xlsx.write(i + 1, 1, dense_x[i]);
        xlsx.write(i + 1, 2, dense_y[i]);
    }

    if (!xlsx.saveAs(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to save XLSX file.");
        return;
    }

    QMessageBox::information(this, "Export", "Interpolated data exported to XLSX successfully.");
}

void HomeWindow::onSaveGraphClicked() {
    QPixmap pixmap = ui->chartView->grab();
    QString fileName = QFileDialog::getSaveFileName(this, "Save Graph", "graph.png", "PNG Files (*.png)");
    if (!fileName.isEmpty()) {
        pixmap.save(fileName);
    }
}

void HomeWindow::onDepthChanged(int value) {
    currentDepth = value;
    if (!x_points.empty() && !y_points.empty()) {
        plotGraph(x_points, y_points, currentDepth);
    }
}

void HomeWindow::plotGraph(const std::vector<double>& x_points_in, const std::vector<double>& y_points_in, int depth) {
    // Copy input vectors so we can sort and manipulate them
    std::vector<std::pair<double, double>> points;
    for (size_t i = 0; i < x_points_in.size(); ++i) {
        points.emplace_back(x_points_in[i], y_points_in[i]);
    }

    std::sort(points.begin(), points.end(), [](auto &a, auto &b) {
        return a.first < b.first;
    });

    std::vector<double> x_points, y_points;
    for (auto &p : points) {
        x_points.push_back(p.first);
        y_points.push_back(p.second);
    }

    // Now use these mutable local vectors

    ui->chartView->chart()->removeAllSeries();

    interpolator interp;
    interp.setData(x_points, y_points);

    QLineSeries* series = new QLineSeries();

    // Generate dense x values
    std::vector<double> dense_x;
    for (size_t i = 0; i < x_points.size() - 1; ++i) {
        double start = x_points[i];
        double end = x_points[i + 1];
        int segments = depth + 1;
        for (int j = 0; j < segments; ++j) {
            double val = start + (end - start) * j / segments;
            dense_x.push_back(val);
        }
    }
    dense_x.push_back(x_points.back());

    // Evaluate interpolated y values
    for (double x : dense_x) {
        double y = interp.evaluateLagrange(x_points, y_points, x);
        series->append(x, y);
    }

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Interpolated Graph");

    QValueAxis *axisX = new QValueAxis;
    QValueAxis *axisY = new QValueAxis;

    axisX->setTitleText("X");
    axisY->setTitleText("Y");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    ui->chartView->setChart(chart);
    ui->chartView->repaint();
}


void HomeWindow::on_inputTable_itemChanged() {
    int rowCount = ui->inputTable->rowCount();
    int columnCount = ui->inputTable->columnCount();

    int lastRow = rowCount - 1;
    bool isLastRowFilled = true;

    for (int col = 0; col < columnCount; ++col) {
        QTableWidgetItem* cell = ui->inputTable->item(lastRow, col);
        if (!cell || cell->text().trimmed().isEmpty()) {
            isLastRowFilled = false;
            break;
        }
    }

    if (isLastRowFilled) {
        ui->inputTable->insertRow(rowCount);
    }

    int emptyRows = 0;

    for (int row = rowCount - 1; row >= 0; --row) {
        bool isEmpty = true;
        for (int col = 0; col < ui->inputTable->columnCount(); ++col) {
            auto item = ui->inputTable->item(row, col);
            if (item && !item->text().trimmed().isEmpty()) {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty) {
            ++emptyRows;
            if (emptyRows > 1) {
                ui->inputTable->removeRow(row);
            }
        }
    }
}
