//
// Created by zouyao on 6/8/17.
//

#ifndef GRABCUT_GMM_H
#define GRABCUT_GMM_H

#include <opencv2/core/core.hpp>
#include <vector>


class GMM
{
public:
    class Component
    {
    public:
        Component();
        Component(const cv::Mat &modelComponent);
        cv::Mat exportModel() const;
        void initLearning();
        void addSample(cv::Vec3d color);
        void endLearning();
        double operator()(const cv::Vec3d &color) const;
        int getSampleCount() const;
    private:
        void calcInverseAndDet();
        cv::Vec3d mean;
        cv::Matx33d cov;
        cv::Matx33d inverseCov;
        double covDeterminant;
        int sampleCount;
    };
public:
    GMM(int componentsCount = 5);
    static GMM fromModel(const cv::Mat &model);
    cv::Mat exportModel() const;
    int getComponentsCount() const;
    double operator()( const cv::Vec3d color ) const;
    int whichComponent( const cv::Vec3d color ) const;

    void initLearning();
    void addSample( int ci, const cv::Vec3d color );
    void endLearning();

private:
    std::vector<Component> components;
    std::vector<double> coefs;
    int totalSampleCount;

};


#endif //GRABCUT_GMM_H
