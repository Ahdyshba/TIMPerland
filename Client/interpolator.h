#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H


#include <vector>


class interpolator {
public:
    void setData(const std::vector<double>& x, const std::vector<double>& y);
    std::vector<double> interpolate();
    double evaluateLagrange(const std::vector<double>& x_points, const std::vector<double>& y_points, double x);

private:
    std::vector<double> xi;
    std::vector<double> yi;

    std::vector<std::vector<double>> getCoefficients(int pl);
    std::vector<std::vector<double>> getPolynomialL();

    void sortPoints();
};


#endif //INTERPOLATOR_H
