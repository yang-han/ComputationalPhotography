
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cassert>
#include "KMeansImpl.h"

using namespace std;


KMeansImpl::KMeansImpl(int dimNum, int clusterNum) {
    dimensionNumber = dimNum;
    clusterNumber = clusterNum;

    means = new double *[clusterNumber];
    for (int i = 0; i < clusterNumber; i++) {
        means[i] = new double[dimensionNumber];
        memset(means[i], 0, sizeof(double) * dimensionNumber);
    }

}

KMeansImpl::~KMeansImpl() {
    for (int i = 0; i < clusterNumber; i++) {
        delete[] means[i];
    }
    delete[] means;
}

void KMeansImpl::cluster(double *data, int N, int *Label) {
    int size = 0;
    size = N;

    assert(size >= clusterNumber);

    init(data, N);

    double *x = new double[dimensionNumber];
    int label = -1;
    double cnt = 0;
    double lastCost = 0;
    double currCost = 0;
    int unchanged = 0;
    int *counts = new int[clusterNumber];
    double **nextMeans = new double *[clusterNumber];
    for (int i = 0; i < clusterNumber; i++) {
        nextMeans[i] = new double[dimensionNumber];
    }

    for(;;) {
        memset(counts, 0, sizeof(int) * clusterNumber);
        for (int i = 0; i < clusterNumber; i++) {
            memset(nextMeans[i], 0, sizeof(double) * dimensionNumber);
        }

        lastCost = currCost;
        currCost = 0;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < dimensionNumber; j++)
                x[j] = data[i * dimensionNumber + j];

            currCost += getLabel(x, &label);

            counts[label]++;
            for (int d = 0; d < dimensionNumber; d++) {
                nextMeans[label][d] += x[d];
            }
        }
        currCost /= size;

        for (int i = 0; i < clusterNumber; i++) {
            if (counts[i] > 0) {
                for (int d = 0; d < dimensionNumber; d++) {
                    nextMeans[i][d] /= counts[i];
                }
                memcpy(means[i], nextMeans[i], sizeof(double) * dimensionNumber);
            }
        }

        cnt++;
        if (fabs(lastCost - currCost) < eps * lastCost) {
            unchanged++;
        }
        if (cnt >= UB || unchanged >= 3) {
            break;
        }

    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < dimensionNumber; j++)
            x[j] = data[i * dimensionNumber + j];
        getLabel(x, &label);
        Label[i] = label;
    }
    delete[] counts;
    delete[] x;
    for (int i = 0; i < clusterNumber; i++) {
        delete[] nextMeans[i];
    }
    delete[] nextMeans;
}

void KMeansImpl::init(double *data, int N) {
    int size = N;
    double *sample = new double[dimensionNumber];

    for (int i = 0; i < clusterNumber; i++) {
        int select = i * size / clusterNumber;
        for (int j = 0; j < dimensionNumber; j++)
            sample[j] = data[select * dimensionNumber + j];
        memcpy(means[i], sample, sizeof(double) * dimensionNumber);
    }

    delete[] sample;
}

double KMeansImpl::getLabel(const double *sample, int *label) {
    double distance = -1;
    for (int i = 0; i < clusterNumber; i++) {
        double temp = calcDistance(sample, means[i], dimensionNumber);
        if (temp < distance || distance == -1) {
            distance = temp;
            *label = i;
        }
    }
    return distance;
}

double KMeansImpl::calcDistance(const double *x, const double *u, int dimNum) {
    double temp = 0;
    for (int d = 0; d < dimNum; d++) {
        temp += (x[d] - u[d]) * (x[d] - u[d]);
    }
    return sqrt(temp);
}