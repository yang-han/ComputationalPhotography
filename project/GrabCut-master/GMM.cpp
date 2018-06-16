//
// Created by zouyao on 6/8/17.
//

#include "GMM.h"
#include <limits>
#include <iostream>

GMM::Component::Component()
{
    mean = cv::Vec3d::all(0);
    cov = cv::Matx33d::zeros();
    inverseCov = cv::Matx33d::zeros();
    covDeterminant = 0;
    sampleCount = 0;
}

GMM::Component::Component(const cv::Mat &modelComponent)
{
    mean = modelComponent(cv::Rect(0, 0, 3, 1));
    cov = modelComponent(cv::Rect(3, 0, 9, 1)).reshape(1, 3);
    calcInverseAndDet();
}

void GMM::Component::initLearning()
{
    mean = cv::Vec3d::all(0);
    cov = cv::Matx33d::zeros();
    sampleCount = 0;
}

void GMM::Component::addSample(cv::Vec3d color)
{
    mean += color;
    cov += color * color.t();
    sampleCount++;
}

void GMM::Component::endLearning()
{
    const double variance = 0.01;
    mean /= sampleCount;
    cov = (1.0 / sampleCount) * cov;
    cov -= mean * mean.t();
    const double det = cv::determinant(cov);
    if (det <= std::numeric_limits<double>::epsilon()) {
        cov += variance * cv::Matx33d::eye();
    }
    calcInverseAndDet();
}

double GMM::Component::operator()(const cv::Vec3d &color) const
{
    cv::Vec3d diff = color - mean;
    double mult = (diff.t() * inverseCov * diff)(0);
    double result = 1.0 / sqrt(covDeterminant) * exp(-0.5 * mult);
    return result;
}

int GMM::Component::getSampleCount() const
{
    return sampleCount;
}

cv::Mat GMM::Component::exportModel() const
{
    cv::Mat meanMat = cv::Mat(mean.t());
    cv::Mat covMat = cv::Mat(cov).reshape(1, 1);
    cv::Mat model;
    cv::hconcat(meanMat, covMat, model);
    return model;
}

void GMM::Component::calcInverseAndDet()
{
    covDeterminant = cv::determinant(cov);
    inverseCov = cov.inv();
}

GMM::GMM(int componentsCount)
    : components(size_t(componentsCount)), coefs(size_t(componentsCount)), totalSampleCount(0)
{

}

GMM GMM::fromModel(const cv::Mat &model)
{
    const int modelSize = 3/*mean*/ + 9/*covariance*/ + 1/*component weight*/;
    if ((model.type() != CV_64FC1) || (model.rows != 1) || (model.cols % modelSize != 0))
        CV_Error(CV_StsBadArg, "_model must have CV_64FC1 type, rows == 1 and cols == 13*componentsCount");
    int componentCount = model.cols / modelSize;
    GMM result(componentCount);
    for (int i = 0; i < componentCount; i++) {
        cv::Mat componentModel = model(cv::Rect(i, 0, modelSize, 1));
        result.coefs[i] = componentModel.at<double>(0, 0);
        result.components[i] = GMM::Component(componentModel(cv::Rect(1, 0, 12, 1)));
    }
    return result;
}

cv::Mat GMM::exportModel() const
{
    cv::Mat result;
    for (size_t i = 0; i < components.size(); i++) {
        cv::Mat coefMat(1, 1, CV_64F, cv::Scalar(coefs[i]));
        cv::Mat componentMat = components[i].exportModel();
        cv::Mat combinedMat;
        cv::hconcat(coefMat, componentMat, combinedMat);
        if (result.empty()) {
            result = combinedMat;
        }
        else {
            cv::hconcat(result, combinedMat, result);
        }
    }
    return result;
}

int GMM::getComponentsCount() const
{
    return (int) components.size();
}

double GMM::operator()(const cv::Vec3d color) const
{
    double res = 0;
    for (size_t i = 0; i < components.size(); i++) {
        if (coefs[i] > 0)
            res += coefs[i] * components[i](color);
    }
    return res;
}

int GMM::whichComponent(const cv::Vec3d color) const
{
    int k = 0;
    double max = 0;
    for (int ci = 0; ci < components.size(); ci++) {
        double p = components[ci](color);
        if (p > max) {
            k = ci;
            max = p;
        }
    }
    return k;
}

void GMM::initLearning()
{
    for (int ci = 0; ci < components.size(); ci++) {
        components[ci].initLearning();
        coefs[ci] = 0;
    }
    totalSampleCount = 0;
}

void GMM::addSample(int ci, const cv::Vec3d color)
{
    components[ci].addSample(color);
    totalSampleCount++;
}

void GMM::endLearning()
{
    for (int ci = 0; ci < components.size(); ci++) {
        int n = components[ci].getSampleCount();
        if (n == 0) {
            coefs[ci] = 0;
        }
        else {
            coefs[ci] = (double) n / totalSampleCount;
            components[ci].endLearning();
        }
    }
}


