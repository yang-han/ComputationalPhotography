#include "BorderMatting.h"

using namespace cv;

double BorderMatting::linear(double r, double delta, double sigma) {
    double p = 0;
    if (sigma >= 0.1)
        p = 1 / sigma;
    if (r < delta - sigma / 2)
        return 0;
    if (r >= delta + sigma / 2)
        return 1;
    return 0.5 + p * (r - delta);
}

void BorderMatting::init(const Mat &originImage, const Mat &mask) {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 32; j++)
            for (int k = 0; k < 16; k++)              g[i][j][k] = linear(i, j * 0.2, k * 0.6);
    mask.copyTo(payloadMask);
    payloadMask = payloadMask & 1;
    originImage.copyTo(payloadImage);
    Canny(payloadMask, edge, 1, 5, 3);
    for (int row = 0; row < payloadImage.rows; row++)
        for (int col = 0; col < payloadImage.cols; col++)
            if (edge.at<uchar>(row, col))
                push(borderPoint(col, row), pointArray, 2);
    for (int row = 0; row < payloadImage.rows; row++)
        for (int col = 0; col < payloadImage.cols; col++)
            if (edge.at<uchar>(row, col) == 0 && payloadMask.at<uchar>(row, col) == 1) {
                borderPoint pt(col, row);
                int dis = checkDis(pt);
                if (dis > 0) push(pt, 3);
            }
}

int BorderMatting::checkDis(borderPoint pt) {
    for (int row = 0; row < pointArray.size(); row++) {
        if (pt.distance(pointArray[row].p) < 3)
            return pt.distance(pointArray[row].p);
    }
    return -1;
}

void BorderMatting::push(borderPoint p, vector<PointArray> &list, int threshold) {
    for (int i = 0; i < list.size(); i++) {
        if (p.distance(list[i].p) <= threshold) {
            list.insert(list.begin() + i, PointArray(p));
            return;
        }
    }
    list.emplace_back(PointArray(p));
}

void BorderMatting::push(borderPoint p, int threshold) {
    int min = -1;
    for (int i = 0; i < pointArray.size(); i++) {
        if (p.distance(pointArray[i].p) <= threshold) {
            if (p.distance(pointArray[i].p) < threshold) {
                min = i;
                threshold = p.distance(pointArray[i].p);
            }
        }
    }

    if (min == -1)
        return;
    else {
        p.dis = threshold;
        pointArray[min].neighbor.push_back(p);
    }
}


void BorderMatting::getL(borderPoint p, Values &result) {
    int startCol = (p.p.x - 20 < 0) ? 0 : (int) p.p.x - 20;
    int colLen = (startCol + 41 < payloadImage.cols) ? 41 : payloadImage.cols - startCol;
    int startRow = (p.p.y - 20 < 0) ? 0 : (int) p.p.y - 20;
    int rowLen = (startRow + 41 < payloadImage.rows) ? 41 : payloadImage.rows - startRow;
    Mat next = payloadImage(Rect(startCol, startRow, colLen, rowLen));
    Vec3i bMean, fMean;
    double bVar = 0, fVar = 0;
    int fNum = 0, bNum = 0;
    for (int row = 0; row < next.rows; row++)
        for (int col = 0; col < next.cols; col++) {
            Vec3i tmp(next.at<Vec3b>(row, col)[0], next.at<Vec3b>(row, col)[1], next.at<Vec3b>(row, col)[2]);
            if (edge.at<uchar>(startRow + row, startCol + col) == 1) {
                fMean += tmp;
                fNum++;
            } else {
                bMean += tmp;
                bNum++;
            }
        }
    result.fMean = fMean = safeDiv(fMean, fNum);
    result.bMean = bMean = safeDiv(bMean, bNum);
    for (int row = 0; row < next.rows; row++)
        for (int col = 0; col < next.cols; col++) {
            Vec3i tmp(next.at<Vec3b>(row, col)[0], next.at<Vec3b>(row, col)[1], next.at<Vec3b>(row, col)[2]);
            if (edge.at<uchar>(startRow + row, startCol + col) == 1)
                fVar += (fMean - tmp).dot(fMean - tmp);
            else
                bVar += (tmp - bMean).dot(tmp - bMean);
        }
    result.fVar = safeDiv(fVar, fNum);
    result.bVar = safeDiv(bVar, bNum);
}

double BorderMatting::gaussian(double r, double del, double sigma) {
    return 1.0 / (pow(sigma, 0.5) * pow(2.0 * PI, 0.5)) * exp(-(pow(r - del, 2.0) / (2.0 * sigma)));
}

double BorderMatting::mMean(double r, double fMean, double bMean) {
    return (1.0 - r) * bMean + r * fMean;
}

double BorderMatting::mVar(double r, double fVar, double bVar) {
    return (1.0 - r) * (1.0 - r) * bVar + r * r * fVar;
}

double BorderMatting::dataTermPoint(borderPoint _ip, uchar _I, int _delta, int _sigma, Values &para) {
    double alpha = g[_ip.dis][_delta][_sigma];
    double t = gaussian(_I, mMean(alpha, toGray(para.fMean), toGray(para.bMean)), mVar(alpha, para.fVar, para.bVar));
    t = -log(t) / log(2.0);
    return t;
}

const long long __NaN = 0xFFF8000000000000;

void BorderMatting::run() {

    int delta = 15, sigma = 5;
    double eMin = *((double *) &__NaN);
    for (int n = 0; n < 30; n++)
        for (int m = 0; m < 10; m++) {
            double t = dataTermPoint(pointArray[0].p,
                                     toGray(payloadImage.at<Vec3b>(pointArray[0].p.p.y, pointArray[0].p.p.x)),
                                     n, m,
                                     pointArray[0].p.para);
            for (int j = 0; j < pointArray[0].neighbor.size(); j++) {
                borderPoint &p = pointArray[0].neighbor[j];
                getL(p, p.para);
                t += dataTermPoint(p, toGray(payloadImage.at<Vec3b>(p.p.y, p.p.x)), n, m, p.para);
            }
            if (t < eMin) {
                eMin = t;
                delta = n;
                sigma = m;
            }
        }

    for (int i = 1; i < pointArray.size(); i++) {
        Values para;
        getL(pointArray[i].p, para);
        pointArray[i].p.para = para;
        for (int j = 0; j < pointArray[i].neighbor.size(); j++) {
            borderPoint &p = pointArray[i].neighbor[j];
            getL(p, para);
            p.para = para;
        }
        double min = INT_MAX;
        for (int n = 0; n < 30; n++)
            for (int m = 0; m < 10; m++) {
                double t = dataTermPoint(pointArray[i].p,
                                         toGray(payloadImage.at<Vec3b>(pointArray[i].p.p.y, pointArray[i].p.p.x)), n,
                                         m, pointArray[i].p.para);
                for (int j = 0; j < pointArray[i].neighbor.size(); j++) {
                    borderPoint &p = pointArray[i].neighbor[j];
                    t += dataTermPoint(p, toGray(payloadImage.at<Vec3b>(p.p.y, p.p.x)), n, m, p.para);
                }
                double V = 2 * (n - delta) * (n - delta) + 360 * (sigma - m) * (sigma - m);
                if (t + V < min) {
                    min = t + V;
                    pointArray[i].p.delta = n;
                    pointArray[i].p.sigma = m;
                }
            }
        sigma = pointArray[i].p.sigma;
        delta = pointArray[i].p.delta;
        pointArray[i].p.alpha = g[0][delta][sigma];
        for (int j = 0; j < pointArray[i].neighbor.size(); j++) {
            borderPoint &p = pointArray[i].neighbor[j];
            p.alpha = g[p.dis][delta][sigma];
        }
    }
    Mat _alphaMask = Mat(payloadMask.size(), CV_32FC1, Scalar(0));
    for (int i = 0; i < payloadMask.rows; i++)
        for (int j = 0; j < payloadMask.cols; j++)
            _alphaMask.at<float>(i, j) = payloadMask.at<uchar>(i, j);
    for (int i = 0; i < pointArray.size(); i++) {
        _alphaMask.at<float>(pointArray[i].p.p.y, pointArray[i].p.p.x) = pointArray[i].p.alpha;
        for (int j = 0; j < pointArray[i].neighbor.size(); j++) {
            borderPoint &p = pointArray[i].neighbor[j];
            _alphaMask.at<float>(p.p.y, p.p.x) = p.alpha;
        }
    }

    Mat result = Mat(payloadImage.size(), CV_8UC4);
    for (int i = 0; i < result.rows; i++)
        for (int j = 0; j < result.cols; j++) {
            result.at<Vec4b>(i, j) = Vec4b(payloadImage.at<Vec3b>(i, j)[0], payloadImage.at<Vec3b>(i, j)[1],
                                           payloadImage.at<Vec3b>(i, j)[2],
                                           _alphaMask.at<float>(i, j) * 255);
        }
    vector<int> compressionParams;
    compressionParams.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compressionParams.push_back(9);
    imwrite("BorderMatting.png", result, compressionParams);
    result.copyTo(borderMattingResult);
    printf("Boder Matting: done.");
}