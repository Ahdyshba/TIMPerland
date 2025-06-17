#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <vector>

class Interpolator
{
public:
    double evaluateLagrange(const std::vector<double> &x_points, const std::vector<double> &y_points, double x);
    struct InterpolatedData
    {
        std::vector<double> dense_x;
        std::vector<double> dense_y;
    };
    InterpolatedData computeInterpolatedData(const std::vector<double> &x_points, const std::vector<double> &y_points, int depth);

private:
    std::vector<double> xi, yi;
    void setData(const std::vector<double> &x, const std::vector<double> &y);
    void sortPoints();
};

#endif //INTERPOLATOR_H
