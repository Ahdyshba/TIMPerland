#include "interpolator.h"

#include <algorithm>
#include <stdexcept>


// Computes interpolated data using Lagrange polynomial interpolation
Interpolator::InterpolatedData Interpolator::computeInterpolatedData(const std::vector<double> &x_points,
                                                                     const std::vector<double> &y_points,
                                                                     int depth)
{
    // Ensure x and y data are of equal size
    if (x_points.size() != y_points.size())
        throw std::invalid_argument("Incomplete point input");

    // Store and sort the input data
    setData(x_points, y_points);

    // Generate a denser set of x-values between each input interval
    std::vector<double> dense_x;
    for (size_t i = 0; i < xi.size() - 1; ++i) {
        double start = xi[i];
        double end = xi[i + 1];
        int segments = depth + 1;           // Number of subdivisions per interval
        for (int j = 0; j < segments; ++j) {
            // Linearly interpolate x-values between start and end
            double val = start + (end - start) * j / segments;
            dense_x.push_back(val);
        }
    }

    dense_x.push_back(xi.back());           // Add the last x point to complete the range

    // Compute the interpolated y-values using Lagrange polynomial for each dense x
    std::vector<double> dense_y;
    for (double x : dense_x) {
        dense_y.push_back(evaluateLagrange(xi, yi, x));
    }

    return {dense_x, dense_y};          // Return the new, dense set of x and y values
}

// Stores and sorts the data points by x-coordinate
void Interpolator::setData(const std::vector<double> &x,
                           const std::vector<double> &y)
{
    xi = x;
    yi = y;
    sortPoints();           // Ensure the points are ordered
}

// Sorts the input points in ascending order of x
void Interpolator::sortPoints()
{
    // Combine x and y into pairs
    std::vector<std::pair<double, double>> points;
    for (size_t i = 0; i < xi.size(); ++i)
        points.emplace_back(xi[i], yi[i]);
    std::sort(points.begin(), points.end(), [](auto &a, auto &b) { return a.first < b.first; });            // Sort pairs based on x value

    // Separate the sorted pairs back into xi and yi
    xi.clear();
    yi.clear();
    for (auto &p : points) {
        xi.push_back(p.first);
        yi.push_back(p.second);
    }
}

// Evaluates the Lagrange interpolating polynomial at a given x
double Interpolator::evaluateLagrange(const std::vector<double> &x_points,
                                      const std::vector<double> &y_points,
                                      double x)
{
    double result = 0.0;
    size_t n = x_points.size();

    // Compute Lagrange basis polynomials and combine them
    for (size_t i = 0; i < n; ++i) {
        double term = y_points[i];          // Start with y_i
        // Multiply by product of (x - x_j) / (x_i - x_j) for all j ≠ i
        for (size_t j = 0; j < n; ++j)
            if (j != i)
                term *= (x - x_points[j]) / (x_points[i] - x_points[j]);

        result += term;         // Add the term to the result
    }

    return result;          // Final interpolated y-value at x
}
