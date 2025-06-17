#include "homewindow.h"
#include "interpolator.h"
#include "ui_homewindow.h"
#include "xlsxdocument.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

using namespace QXlsx;


// Constructor: Initializes UI and connects UI elements to their respective slots
HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeWindow)
{
    ui->setupUi(this);

    // Connect buttons and other widgets to their corresponding event handlers
    connect(ui->loadXLSXButton, &QPushButton::clicked, this, &HomeWindow::onImportXLSXClicked);
    connect(ui->inputTable, &QTableWidget::itemChanged, this, &HomeWindow::on_inputTable_itemChanged);
    connect(ui->clearButton, &QPushButton::clicked, this, &HomeWindow::onClearTableClicked);
    connect(ui->depthSlider, &QSlider::valueChanged, this, [=](int value) {
        ui->depthLabel->setText(QString("Depth: %1").arg(value));
    });
    connect(ui->interpolateButton, &QPushButton::clicked, this, &HomeWindow::onInterpolateClicked);
    connect(ui->saveGraphButton, &QPushButton::clicked, this, &HomeWindow::onSaveGraphClicked);
    connect(ui->saveXLSXButton, &QPushButton::clicked, this, &HomeWindow::onSaveXLSXClicked);
}

// Destructor: Clean up UI
HomeWindow::~HomeWindow()
{
    delete ui;
}

//                      SLOTS                       //

// Slot: Handles loading data from an XLSX file into the table
void HomeWindow::onImportXLSXClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open XLSX File", "", "Excel Files (*.xlsx)");
    if (fileName.isEmpty())
        return;

    Document xlsx(fileName);
    if (!xlsx.load()) {
        QMessageBox::warning(this, "Error", "Failed to open XLSX file.");

        return;
    }

    // Clear current table contents
    ui->inputTable->clearContents();
    ui->inputTable->setRowCount(0);

    // Read rows until the first column is empty
    int row = 1;
    while (!xlsx.read(row, 1).toString().isEmpty()) {
        ui->inputTable->insertRow(ui->inputTable->rowCount());

        QString x = xlsx.read(row, 1).toString();
        QString y = xlsx.read(row, 2).toString();

        ui->inputTable->setItem(row - 1, 0, new QTableWidgetItem(x));
        ui->inputTable->setItem(row - 1, 1, new QTableWidgetItem(y));

        ++row;
    }

    // Ensure at least one empty row is available
    if (row == 1)
        ui->inputTable->insertRow(ui->inputTable->rowCount());

    QMessageBox::information(this, "Import", "Excel data loaded successfully.");
}

// Slot: Handles changes in table cells (auto-expand and clean-up empty rows)
void HomeWindow::on_inputTable_itemChanged(QTableWidgetItem *item)
{
    int rowCount = ui->inputTable->rowCount();
    int columnCount = ui->inputTable->columnCount();
    int lastRow = rowCount - 1;

    // Check if last row is filled, if yes, add a new row
    bool isLastRowFilled = true;
    for (int col = 0; col < columnCount; ++col) {
        QTableWidgetItem *cell = ui->inputTable->item(lastRow, col);
        if (!cell || cell->text().trimmed().isEmpty()) {
            isLastRowFilled = false;
            break;
        }
    }

    if (isLastRowFilled)
        ui->inputTable->insertRow(rowCount);

    // Remove any extra empty rows (more than one)
    int emptyRows = 0;
    for (int row = rowCount - 1; row >= 0; --row) {
        bool isEmpty = true;

        for (int col = 0; col < columnCount; ++col) {
            auto cell = ui->inputTable->item(row, col);
            if (cell && !cell->text().trimmed().isEmpty()) {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty) {
            ++emptyRows;
            if (emptyRows > 1)
                ui->inputTable->removeRow(row);
        }
    }

    // Validate numeric input
    QString text = item->text().trimmed();
    bool ok = false;
    double value = text.toDouble(&ok);
    if (!ok) {
        item->setText("");

        return;
    }

    // Clamp values to a relatively safe double range
    double minValue = -999999999.0;
    double maxValue = 999999999.0;

    if (value < minValue)
        item->setText("-999999999");
    else if (value > maxValue)
        item->setText("999999999");
}

// Slot: Clears the table to one empty row
void HomeWindow::onClearTableClicked()
{
    ui->inputTable->clearContents();
    ui->inputTable->setRowCount(1);
}

// Slot: Handles interpolation and graph plotting
void HomeWindow::onInterpolateClicked()
{
    std::vector<double> x_points = getXValues();
    std::vector<double> y_points = getYValues();
    int depth = ui->depthSlider->value();

    // Require at least two data points
    if (x_points.size() < 2) {
        QMessageBox::warning(this, "Invalid Input", QString("Table has to contain 2 or more rows"));

        return;
    }

    // Check for duplicate x-values
    std::vector<double> seen_x;
    for (size_t i = 0; i < x_points.size(); ++i) {
        double x = x_points[i];

        if (std::find(seen_x.begin(), seen_x.end(), x) != seen_x.end()) {
            QMessageBox::warning(this, "Invalid Input", QString("Duplicate X value '%1' detected. Interpolation requires unique X values.").arg(x));

            return;
        }
        seen_x.push_back(x);
    }

    try {
        Interpolator interp;
        auto data = interp.computeInterpolatedData(x_points, y_points, depth);
        auto lastDenseX = data.dense_x;
        auto lastDenseY = data.dense_y;

        checkForOutliers(x_points, y_points);
        plotGraph(lastDenseX, lastDenseY);

        // Enable save buttons
        ui->saveGraphButton->setEnabled(true);
        ui->saveXLSXButton->setEnabled(true);
    } catch (const std::exception &ex) {
        QMessageBox::warning(this, "Interpolation Error", ex.what());
    }
}

// Slot: Saves chart image as PNG
void HomeWindow::onSaveGraphClicked()
{
    QPixmap pixmap = ui->chartView->grab();
    QString fileName = QFileDialog::getSaveFileName(this, "Save Graph", "graph.png", "PNG Files (*.png)");
    if (!fileName.isEmpty())
        pixmap.save(fileName);
}

// Slot: Saves interpolated data as XLSX
void HomeWindow::onSaveXLSXClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save XLSX", "output.xlsx", "Excel Files (*.xlsx>");
    if (fileName.isEmpty())
        return;

    Document xlsx;

    for (size_t i = 0; i < lastDenseX.size(); ++i) {
        xlsx.write(i + 1, 1, lastDenseX[i]);
        xlsx.write(i + 1, 2, lastDenseY[i]);
    }

    if (!xlsx.saveAs(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to save XLSX file.");

        return;
    }

    QMessageBox::information(this, "Export", "Interpolated data exported to XLSX successfully.");
}

//                      FUNCTIONS                       //

// Fetch X values from the table
std::vector<double> HomeWindow::getXValues()
{
    int rows = ui->inputTable->rowCount();

    std::vector<double> x;
    for (int i = 0; i < rows; ++i) {
        auto item = ui->inputTable->item(i, 0);
        if (item && !item->text().isEmpty()) {
            bool ok;
            double val = item->text().toDouble(&ok);
            if (ok)
                x.push_back(val);
        }
    }

    return x;
}

// Fetch Y values from the table
std::vector<double> HomeWindow::getYValues()
{
    int rows = ui->inputTable->rowCount();

    std::vector<double> y;
    for (int i = 0; i < rows; ++i) {
        auto item = ui->inputTable->item(i, 1);
        if (item && !item->text().isEmpty()) {
            bool ok;
            double val = item->text().toDouble(&ok);
            if (ok)
                y.push_back(val);
        }
    }

    return y;
}

// Plot the interpolated graph
void HomeWindow::plotGraph(const std::vector<double> &x_points,
                           const std::vector<double> &y_points)
{
    lastDenseX = x_points;
    lastDenseY = y_points;

    ui->chartView->chart()->removeAllSeries();

    QLineSeries *series = new QLineSeries();
    for (size_t i = 0; i < x_points.size(); ++i)
        series->append(x_points[i], y_points[i]);

    // Create and populate the series
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Interpolated Graph");

    // Define axes
    QValueAxis *axisX = new QValueAxis;
    QValueAxis *axisY = new QValueAxis;

    double minX = *std::min_element(x_points.begin(), x_points.end());
    double maxX = *std::max_element(x_points.begin(), x_points.end());
    double minY = *std::min_element(y_points.begin(), y_points.end());
    double maxY = *std::max_element(y_points.begin(), y_points.end());

    // Calculate padding for axes
    int maxXTicks = 10;
    int maxYTicks = 5;

    double rawXStep = (maxX - minX) / maxXTicks;
    double rawYStep = (maxY - minY) / maxYTicks;

    int tickStepX = std::max(1, static_cast<int>(std::round(rawXStep)));
    int tickStepY = std::max(1, static_cast<int>(std::round(rawYStep)));

    double paddedMinX = std::floor(minX / tickStepX) * tickStepX - tickStepX;
    double paddedMaxX = std::ceil(maxX / tickStepX) * tickStepX + tickStepX;
    double paddedMinY = std::floor(minY / tickStepY) * tickStepY - tickStepY;
    double paddedMaxY = std::ceil(maxY / tickStepY) * tickStepY + tickStepY;

    // Apply ranges and labels
    axisX->setRange(paddedMinX, paddedMaxX);
    axisX->setTickCount(static_cast<int>((paddedMaxX - paddedMinX) / tickStepX) + 1);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("X");

    axisY->setRange(paddedMinY, paddedMaxY);
    axisY->setTickCount(static_cast<int>((paddedMaxY - paddedMinY) / tickStepY) + 1);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Y");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    ui->chartView->setChart(chart);
    ui->chartView->repaint();
}

// Check for statistical outliers using IQR (Interquartile Range),
// which grabs specific data points and compares their upper and lower bounds to other data points
void HomeWindow::checkForOutliers(const std::vector<double> &x_points,
                                  const std::vector<double> &y_points)
{
    if (x_points.size() < 3 || y_points.size() < 3)
        return;

    // IQR formula function
    auto computeIQR = [](const std::vector<double> &data, double &lower, double &upper) {
        std::vector<double> sorted = data;
        std::sort(sorted.begin(), sorted.end());

        int n = sorted.size();
        double q1 = sorted[n / 4];
        double q3 = sorted[3 * n / 4];
        double iqr = q3 - q1;

        lower = q1 - 1.5 * iqr;
        upper = q3 + 1.5 * iqr;
    };

    // Searching for statistical outliers
    double x_lower, x_upper, y_lower, y_upper;

    computeIQR(x_points, x_lower, x_upper);
    computeIQR(y_points, y_lower, y_upper);

    QStringList outlierList;
    for (size_t i = 0; i < x_points.size(); ++i) {
        bool x_out = x_points[i] < x_lower || x_points[i] > x_upper;
        bool y_out = y_points[i] < y_lower || y_points[i] > y_upper;

        if (x_out || y_out) {
            outlierList.append(QString("Row %1: X=%2, Y=%3").arg(i + 1).arg(QString::number(x_points[i], 'f', 5)).arg(QString::number(y_points[i], 'f', 5)));
        }
    }

    // Warn the user only once for each unique dataset
    if (!outlierList.isEmpty() && (x_points != lastDenseX || y_points != lastDenseY)
        && (x_points != lastWarningX || y_points != lastWarningY)) {
        QMessageBox::warning(this, "Possible Outliers Detected",
                             QString("The following point(s) may be statistical outliers:\n\n%1\n\nInterpolation may be unstable.").arg(outlierList.join("\n")));

        lastWarningX = x_points;
        lastWarningY = y_points;
    }
}
