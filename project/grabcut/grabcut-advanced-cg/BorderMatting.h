#pragma once

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define safeDiv(a,b) ((b)?(a)/(b):(0))
const double PI = 3.14159;

class BorderMatting {

public:
    Mat borderMattingResult;

    double linear(double r, double delta, double sigma);

    double gaussian(double r, double del, double sigma);

    double mMean(double r, double fMean, double bMean);

    double mVar(double r, double fVar, double bVar);

    struct Values {
        double bVar;
        double fVar;

        Vec3b bMean;
        Vec3b fMean;
    };

    class borderPoint {
    public:
        int delta, sigma;
        int dis;
        double alpha;

        Point2d p;
        Values para;

        borderPoint(int x, int y, int dis = 0) : p(Point2d(x, y)), dis(dis) {}

        borderPoint() {};

        int distance(borderPoint t) {
            return sqrt((this->p.x - t.p.x) * (this->p.x - t.p.x) + (this->p.y - t.p.y) * (this->p.y - t.p.y));
        }
    };

    class PointArray {
    public:
        borderPoint p;
        vector<borderPoint> neighbor;

        PointArray(borderPoint p) : p(p) {}
    };

    uchar toGray(Vec3b tmp) {
        return (tmp[2] * 299 + tmp[1] * 587 + tmp[0] * 114 + 500) / 1000;
    }

    void push(borderPoint p, vector<PointArray> &list, int threshold);

    void getL(borderPoint p, Values &result);

    vector<PointArray> pointArray;

    void push(borderPoint p, int threshold);

    void constructStrip();

    double dataTermPoint(borderPoint _ip, uchar _I, int _delta, int _sigma, Values &para);

    void init(const Mat &originImage, const Mat &mask);

    void run();

    int checkDis(borderPoint pt);

    void constructContour();

private:
    Mat edge;

    Mat payloadMask;
    Mat payloadImage;

    double g[8][32][16];
};

