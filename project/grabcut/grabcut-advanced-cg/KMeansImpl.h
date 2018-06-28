#pragma once

#include <cstring>
#include <fstream>

#define eps (1e-3)
#define UB (100)

class KMeansImpl {
public:

    KMeansImpl(int dimNum = 1, int clusterNum = 1);

    ~KMeansImpl();

    void init(double *data, int N);

    void cluster(double *data, int N, int *Label);

    double **means;

private:
    int dimensionNumber;
    int clusterNumber;

    double getLabel(const double *x, int *label);

    double calcDistance(const double *x, const double *u, int dimNum);
};
