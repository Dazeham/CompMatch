#pragma once
#ifndef CM_UTILS_HPP
#define CM_UTILS_HPP
#include <ppl.h>
#include <numeric>
#include <opencv2/opencv.hpp>
#include "cm_intrinsics.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>


namespace cm {
//////*****************    三次样条插值    ******************//////
    struct CubicSpline {
        std::vector<double> a, b, c, d, x;
        void build(const std::vector<cv::Point2f>& points) {
            int n = points.size() - 1;
            x.resize(n + 1);
            a.resize(n + 1);
            b.resize(n);
            c.resize(n + 1);
            d.resize(n);

            for (int i = 0; i <= n; i++) {
                x[i] = points[i].x;
                a[i] = points[i].y;
            }

            std::vector<double> h(n);
            for (int i = 0; i < n; i++) {
                h[i] = x[i + 1] - x[i];
            }

            std::vector<double> alpha(n);
            for (int i = 1; i < n; i++) {
                alpha[i] = (3.0 / h[i]) * (a[i + 1] - a[i]) - (3.0 / h[i - 1]) * (a[i] - a[i - 1]);
            }

            std::vector<double> l(n + 1), mu(n + 1), z(n + 1);
            l[0] = 1.0;
            mu[0] = 0.0;
            z[0] = 0.0;

            for (int i = 1; i < n; i++) {
                l[i] = 2.0 * (x[i + 1] - x[i - 1]) - h[i - 1] * mu[i - 1];
                mu[i] = h[i] / l[i];
                z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
            }

            l[n] = 1.0;
            z[n] = 0.0;
            c[n] = 0.0;

            for (int j = n - 1; j >= 0; j--) {
                c[j] = z[j] - mu[j] * c[j + 1];
                b[j] = (a[j + 1] - a[j]) / h[j] - h[j] * (c[j + 1] + 2.0 * c[j]) / 3.0;
                d[j] = (c[j + 1] - c[j]) / (3.0 * h[j]);
            }
        }
        double interpolate(double X) {
            int n = x.size() - 1;
            int i = n - 1;
            for (int j = 0; j < n; j++) {
                if (X >= x[j] && X <= x[j + 1]) {
                    i = j;
                    break;
                }
            }
            double dx = X - x[i];
            return a[i] + b[i] * dx + c[i] * dx * dx + d[i] * dx * dx * dx;
        }
        cv::Point2d findSplineMaxXY()
        {
            int n = x.size() - 1;

            double xmax = x[0];
            double ymax = interpolate(x[0]);

            for (int i = 0; i < n; i++) {
                double x0 = x[i];
                double x1 = x[i + 1];

                double bi = b[i];
                double ci = c[i];
                double di = d[i];

                // ----------- 1. 检查区间两端 -----------
                double y0 = interpolate(x0);
                if (y0 > ymax) { ymax = y0; xmax = x0; }

                double y1 = interpolate(x1);
                if (y1 > ymax) { ymax = y1; xmax = x1; }

                // ----------- 2. 求 S'(x)=0 的内部点 -----------
                // S'(x) = b + 2c dx + 3d dx^2 = 0
                double A = 3.0 * di;
                double B = 2.0 * ci;
                double C = bi;

                auto try_dx = [&](double dx) {
                    double X = x0 + dx;
                    if (X <= x0 || X >= x1) return;

                    double Y = interpolate(X);
                    if (Y > ymax) { ymax = Y; xmax = X; }
                    };

                if (std::fabs(A) < 1e-12) {
                    if (std::fabs(B) > 1e-12) {
                        double dx = -C / B;
                        try_dx(dx);
                    }
                }
                else {
                    double D = B * B - 4.0 * A * C;
                    if (D >= 0.0) {
                        double sD = std::sqrt(D);
                        try_dx((-B + sD) / (2 * A));
                        try_dx((-B - sD) / (2 * A));
                    }
                }
            }

            return cv::Point2d(xmax, ymax);
        }
    };



//////*****************    非对称高斯    ******************//////
    inline double asymmetricGaussianFunction(double x, const Eigen::VectorXd& params) {
        double term = 0;
        if (x <= params(1)) {
            term = std::exp(-(x - params(1)) * (x - params(1)) / (2 * params(2) * params(2)));
        }
        else {
            term = std::exp(-(x - params(1)) * (x - params(1)) / (2 * params(3) * params(3)));
        }
        return params(0) * (2. / std::sqrt(2 * CV_PI)) * (1. / (params(2) + params(3))) * term;
    }

    inline Eigen::VectorXd asymmetricGaussianJacobian(double x, const Eigen::VectorXd& params) {
        double p0 = params(0);
        double p1 = params(1);
        double p2 = params(2);
        double p3 = params(3);

        double f = asymmetricGaussianFunction(x, params);

        Eigen::VectorXd jac(4);
        jac(0) = f / p0;  // df/dp0

        // df/dp1
        double sigma2 = (x <= p1) ? p2 : p3;
        jac(1) = f * (x - p1) / (sigma2 * sigma2);

        // df/dp2
        double denom_inv = 1.0 / (p2 + p3);
        if (x <= p1) {
            jac(2) = f * (-denom_inv + (x - p1) * (x - p1) / (p2 * p2 * p2));
        }
        else {
            jac(2) = f * (-denom_inv);
        }

        // df/dp3
        if (x > p1) {
            jac(3) = f * (-denom_inv + (x - p1) * (x - p1) / (p3 * p3 * p3));
        }
        else {
            jac(3) = f * (-denom_inv);
        }

        return jac;
    }

    struct asymmetricGaussianFunctor {
        asymmetricGaussianFunctor(const std::vector<cv::Point2f>& inPts) : mPts(inPts) {};
        const std::vector<cv::Point2f>& mPts;

        int values() const { return mPts.size(); }

        int operator()(const Eigen::VectorXd& params, Eigen::VectorXd& fvec) const {
            for (int i = 0; i < values(); i++) {
                fvec(i) = mPts[i].y - asymmetricGaussianFunction(mPts[i].x, params);
            }
            return 0;
        }

        int df(const Eigen::VectorXd& params, Eigen::MatrixXd& fjac) const {
            for (int i = 0; i < values(); i++) {
                fjac.row(i) = -asymmetricGaussianJacobian(mPts[i].x, params).transpose();
            }
            return 0;
        }
    };

    inline Eigen::VectorXd asymmetricGaussianFit(const std::vector<cv::Point2f>& points, const Eigen::VectorXd& initParam) {
        Eigen::VectorXd params = initParam;
        asymmetricGaussianFunctor functor(points);
        Eigen::LevenbergMarquardt<asymmetricGaussianFunctor> lm(functor);
        //lm.parameters.maxfev = 100;
        //lm.parameters.ftol = 1e-4;
        //lm.parameters.xtol = 1e-4;
        //lm.parameters.gtol = 1e-4;
        //lm.parameters.factor = 100.;
        int ret = lm.minimize(params);
        return params;
    }


//////*****************    平滑非对称    ******************//////
    inline double sigmoid(double x) {
        return 1.0 / (1.0 + std::exp(-x));
    }

    inline double smoothAsymmetricFunction(double x, const Eigen::VectorXd& params) {
        const double u = x - params(1);
        const double term1 = params(2) * sigmoid(u);
        const double term2 = params(3) * sigmoid(-u);
        const double exponent = u * u * (term1 + term2);
        return params(0) * std::exp(-exponent);
    }

    inline Eigen::VectorXd smoothAsymmetricJacobian(double x, const Eigen::VectorXd& params) {
        double u = x - params(1);
        double sigma_u = sigmoid(u);
        double sigma_neg_u = sigmoid(-u);
        double sigma_u_deriv = sigma_u * (1.0 - sigma_u);
        double sigma_neg_u_deriv = -sigma_neg_u * (1.0 - sigma_neg_u);

        double g = params(2) * sigma_u + params(3) * sigma_neg_u;
        double g_prime = params(2) * sigma_u_deriv + params(3) * sigma_neg_u_deriv;

        double f = smoothAsymmetricFunction(x, params);

        Eigen::VectorXd jac(4);
        jac(0) = f / params(0);  // df/dp0
        jac(1) = f * (2.0 * u * g + u * u * g_prime);  // df/dp1
        jac(2) = -f * u * u * sigma_u;  // df/dp2
        jac(3) = -f * u * u * sigma_neg_u;  // df/dp3

        return jac;
    }

    struct smoothAsymmetricFunctor {
        smoothAsymmetricFunctor(const std::vector<cv::Point2f>& inPts) : mPts(inPts) {};
        const std::vector<cv::Point2f>& mPts;

        int values() const { return mPts.size(); }

        int operator()(const Eigen::VectorXd& params, Eigen::VectorXd& fvec) const {
            for (int i = 0; i < values(); i++) {
                fvec(i) = mPts[i].y - smoothAsymmetricFunction(mPts[i].x, params);
            }
            return 0;
        }

        int df(const Eigen::VectorXd& params, Eigen::MatrixXd& fjac) const {
            for (int i = 0; i < values(); i++) {
                fjac.row(i) = -smoothAsymmetricJacobian(mPts[i].x, params).transpose();
            }
            return 0;
        }
    };

    inline Eigen::VectorXd smoothAsymmetricFit(const std::vector<cv::Point2f>& points, const Eigen::VectorXd& initParam) {
        Eigen::VectorXd params = initParam;
        smoothAsymmetricFunctor functor(points);
        Eigen::LevenbergMarquardt<smoothAsymmetricFunctor> lm(functor);
        //lm.parameters.maxfev = 100;
        //lm.parameters.ftol = 1e-6;
        //lm.parameters.xtol = 1e-6;
        //lm.parameters.gtol = 1e-6;
        //lm.parameters.factor = 100.;
        int ret = lm.minimize(params);
        return params;
    }


//////*****************    距离    ******************//////
    inline double distanceFunction(const Eigen::VectorXd& input, const Eigen::VectorXd& params, const cv::Point2f& inScaleFactor) {
        // parameter
        const double deltaX = params[0];
        const double deltaY = params[1];
        const double deltaT = params[2];

        // input
        const double x = input[0];
        const double y = input[1];
        const double resX = input[2];
        const double resY = input[3];
        const double resGX = input[4];
        const double resGY = input[5];

        const double RT = CV_PI / 180.;
        const double sint = std::sin(deltaT * RT);
        const double cost = std::cos(deltaT * RT);
        const double x_ = (cost * x - sint * y) * inScaleFactor.x + deltaX;
        const double y_ = (sint * x + cost * y) * inScaleFactor.y + deltaY;
        const double res = pow((x_ - resX) * resGX + (y_ - resY) * resGY, 2);
        return res;
    }

    inline Eigen::VectorXd distanceJacobian(const Eigen::VectorXd& input, const Eigen::VectorXd& params, const cv::Point2f& inScaleFactor) {
        // parameter
        const double deltaX = params[0];
        const double deltaY = params[1];
        const double deltaT = params[2];

        // input
        const double x = input[0];
        const double y = input[1];
        const double resX = input[2];
        const double resY = input[3];
        const double resGX = input[4];
        const double resGY = input[5];

        const double RT = CV_PI / 180.;
        const double sint = std::sin(deltaT * RT);
        const double cost = std::cos(deltaT * RT);
        const double x_ = (cost * x - sint * y) * inScaleFactor.x + deltaX;
        const double y_ = (sint * x + cost * y) * inScaleFactor.y + deltaY;
        const double item = (x_ - resX) * resGX + (y_ - resY) * resGY;
        Eigen::Vector3d jac;
        jac(0) = 2 * item * resGX;  // df/ddeltaX
        jac(1) = 2 * item * resGY;  // df/ddeltaY
        //jac(2) = 2 * item * (cost * (x * inScaleFactor.y * resGY - y * inScaleFactor.x * resGX) - sint * (x * inScaleFactor.x * resGX + y * inScaleFactor.y * resGY));  // df/dt
        jac(2) = 2 * item * (
            - inScaleFactor.x * resGX * RT * sint * x
            - inScaleFactor.x * resGX * RT * cost * y
            + inScaleFactor.y * resGY * RT * cost * x
            - inScaleFactor.y * resGY * RT * sint * y
        );  // df/ddeltaT

        return jac;
    }

    struct distanceFunctor {
        distanceFunctor(const std::vector<cv::Mat>& inMat, const cv::Point2f& inScaleFactor) : inMat(inMat), inScaleFactor(inScaleFactor) {}
        const std::vector<cv::Mat>& inMat;
        const cv::Point2f& inScaleFactor;

        int values() const { return inMat.size(); }

        int operator()(const Eigen::VectorXd& params, Eigen::VectorXd& fvec) const {
            for (int cNo = 0; cNo < values(); cNo++) {
                Eigen::VectorXd input(6);
                input <<
                    inMat[0].ptr<float>()[cNo],
                    inMat[1].ptr<float>()[cNo],
                    inMat[2].ptr<float>()[cNo],
                    inMat[3].ptr<float>()[cNo],
                    inMat[4].ptr<float>()[cNo],
                    inMat[5].ptr<float>()[cNo];
                double y_predicted = distanceFunction(input, params, inScaleFactor);
                fvec(cNo) = 0 - y_predicted;
            }
            return 0;
        }

        int df(const Eigen::VectorXd& params, Eigen::MatrixXd& fjac) const {
            for (int cNo = 0; cNo < values(); cNo++) {
                Eigen::VectorXd input(6);
                input <<
                    inMat[0].ptr<float>()[cNo],
                    inMat[1].ptr<float>()[cNo],
                    inMat[2].ptr<float>()[cNo],
                    inMat[3].ptr<float>()[cNo],
                    inMat[4].ptr<float>()[cNo],
                    inMat[5].ptr<float>()[cNo];
                fjac.row(cNo) = -distanceJacobian(input, params, inScaleFactor).transpose();
            }
            return 0;
        }
    };

    inline Eigen::VectorXd distanceFit(const std::vector<cv::Mat>& inMat, const Eigen::VectorXd& initParams, const cv::Point2f& inScaleFactor) {
        Eigen::VectorXd params = initParams;
        distanceFunctor functor(inMat, inScaleFactor);
        Eigen::LevenbergMarquardt<distanceFunctor> lm(functor);
        //lm.parameters.maxfev = 100;
        //lm.parameters.ftol = 1e-4;
        //lm.parameters.xtol = 1e-4;
        //lm.parameters.gtol = 1e-4;
        //lm.parameters.factor = 100.;
        int ret = lm.minimize(params);
        return params;
    }
}

#endif
