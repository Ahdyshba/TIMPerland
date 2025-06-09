#include "interpolator.h"
#include <stdexcept>
#include <algorithm>

void interpolator::setData(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size()) throw std::invalid_argument("x and y must be same size");
    xi = x;
    yi = y;
    sortPoints();
}

std::vector<std::vector<double>> interpolator::getCoefficients(int pl) {
    int n = xi.size();
    std::vector<std::vector<double>> filtered;
    for (int i = 0; i < n; ++i) {
        if (i == pl) continue;
        double denom = xi[pl] - xi[i];
        filtered.push_back({ -xi[i] / denom, 1.0 / denom });
    }
    return filtered;
}

std::vector<std::vector<double>> interpolator::getPolynomialL() {
    int n = xi.size();
    std::vector<std::vector<double>> pli(n, std::vector<double>(n, 0.0));

    for (int pl = 0; pl < n; ++pl) {
        auto coeffs = getCoefficients(pl);
        for (size_t i = 1; i < coeffs.size(); ++i) {
            if (i == 1) {
                pli[pl][0] = coeffs[i - 1][0] * coeffs[i][0];
                pli[pl][1] = coeffs[i - 1][1] * coeffs[i][0] + coeffs[i][1] * coeffs[i - 1][0];
                pli[pl][2] = coeffs[i - 1][1] * coeffs[i][1];
            } else {
                std::vector<double> clone = pli[pl];
                std::vector<double> zeros(n, 0.0);
                for (int j = 0; j < n - 1; ++j) {
                    zeros[j] += clone[j] * coeffs[i][0];
                    zeros[j + 1] += clone[j] * coeffs[i][1];
                }
                pli[pl] = zeros;
            }
        }
    }
    return pli;
}

std::vector<double> interpolator::interpolate() {
    int n = xi.size();
    auto polyL = getPolynomialL();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            polyL[i][j] *= yi[i];

    std::vector<double> L(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            L[i] += polyL[j][i];

    return L;
}

double interpolator::evaluateLagrange(const std::vector<double>& x_points,
                        const std::vector<double>& y_points,
                        double x) {
    double result = 0.0;
    size_t n = x_points.size();
    for (size_t i = 0; i < n; ++i) {
        double term = y_points[i];
        for (size_t j = 0; j < n; ++j) {
            if (j != i) {
                term *= (x - x_points[j]) / (x_points[i] - x_points[j]);
            }
        }
        result += term;
    }
    return result;
}

void interpolator::sortPoints() {
    std::vector<std::pair<double, double>> points;
    for (size_t i = 0; i < xi.size(); ++i) {
        points.emplace_back(xi[i], yi[i]);
    }

    std::sort(points.begin(), points.end(), [](auto& a, auto& b) {
        return a.first < b.first;
    });

    for (size_t i = 0; i < points.size(); ++i) {
        xi[i] = points[i].first;
        yi[i] = points[i].second;
    }
}
