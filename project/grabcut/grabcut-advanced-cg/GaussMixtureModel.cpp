#include <cmath>
#include <cstring>
#include <cassert>
#include <iostream>
#include "GaussMixtureModel.h"
#include "KMeansImpl.h"

using namespace std;

#define max(a,b) ((a)<(b)?(b):(a))

GaussMixtureModel::GaussMixtureModel(int _dimensionNumber, int _mixtureNumber) {
    dimensionNumber = _dimensionNumber;
    mixtureNumber = _mixtureNumber;

    maxIterationNumber = 100;
    endError = 0.001;

    allocate();

    for (int i = 0; i < mixtureNumber; i++) {
        priors[i] = 1.0 / mixtureNumber;

        for (int d = 0; d < dimensionNumber; d++) {
            means[i][d] = 0;
            vars[i][d] = 1;
        }
    }
}

GaussMixtureModel::~GaussMixtureModel() {
    dispose();
}

void GaussMixtureModel::allocate() {
    priors = new double[mixtureNumber];
    means = new double *[mixtureNumber];
    vars = new double *[mixtureNumber];

    for (int i = 0; i < mixtureNumber; i++) {
        means[i] = new double[dimensionNumber];
        vars[i] = new double[dimensionNumber];
    }

    minVars = new double[dimensionNumber];
}

void GaussMixtureModel::dispose() {
    delete[] priors;

    for (int i = 0; i < mixtureNumber; i++) {
        delete[] means[i];
        delete[] vars[i];
    }
    delete[] means;
    delete[] vars;

    delete[] minVars;
}

double GaussMixtureModel::getProbability(const double *sample) {
    double p = 0;
    for (int i = 0; i < mixtureNumber; i++) {
        p += priors[i] * getProbability(sample, i);
    }
    return p;
}

double GaussMixtureModel::getProbability(const double *x, int j) {
    double p = 1;
    for (int d = 0; d < dimensionNumber; d++) {
        p *= 1 / sqrt(2 * 3.1415926535 * vars[j][d]);
        p *= exp(-0.5 * (x[d] - means[j][d]) * (x[d] - means[j][d]) / vars[j][d]);
    }
    return p;
}

void GaussMixtureModel::train(double *data, int N) {
    init(data, N);

    int size = N;

    double iterNum = 0;
    double lastL = 0;
    double currL = 0;
    int unchanged = 0;
    double *x = new double[dimensionNumber];
    double *nextPriors = new double[mixtureNumber];
    double **nextVars = new double *[mixtureNumber];
    double **nextMeans = new double *[mixtureNumber];

    for (int i = 0; i < mixtureNumber; i++) {
        nextMeans[i] = new double[dimensionNumber];
        nextVars[i] = new double[dimensionNumber];
    }

    for(;;) {
        memset(nextPriors, 0, sizeof(double) * mixtureNumber);
        for (int i = 0; i < mixtureNumber; i++) {
            memset(nextVars[i], 0, sizeof(double) * dimensionNumber);
            memset(nextMeans[i], 0, sizeof(double) * dimensionNumber);
        }

        lastL = currL;
        currL = 0;

        for (int k = 0; k < size; k++) {
            for (int j = 0; j < dimensionNumber; j++)
                x[j] = data[k * dimensionNumber + j];
            double p = getProbability(x);

            for (int j = 0; j < mixtureNumber; j++) {
                double pj = getProbability(x, j) * priors[j] / p;

                nextPriors[j] += pj;

                for (int d = 0; d < dimensionNumber; d++) {
                    nextMeans[j][d] += pj * x[d];
                    nextVars[j][d] += pj * x[d] * x[d];
                }
            }

            currL += (p > 1E-20) ? log10(p) : -20;
        }
        currL /= size;

        for (int j = 0; j < mixtureNumber; j++) {
            priors[j] = nextPriors[j] / size;

            if (priors[j] > 0) {
                for (int d = 0; d < dimensionNumber; d++) {
                    means[j][d] = nextMeans[j][d] / nextPriors[j];
                    vars[j][d] = nextVars[j][d] / nextPriors[j] - means[j][d] * means[j][d];
                    if (vars[j][d] < minVars[d]) {
                        vars[j][d] = minVars[d];
                    }
                }
            }
        }

        iterNum++;
        if (fabs(currL - lastL) < endError * fabs(lastL)) {
            unchanged++;
        }
        if (iterNum >= maxIterationNumber || unchanged >= 3) {
            break;
        }
    }
    delete[] nextPriors;
    for (int i = 0; i < mixtureNumber; i++) {
        delete[] nextMeans[i];
        delete[] nextVars[i];
    }
    delete[] nextMeans;
    delete[] nextVars;
    delete[] x;
}

void GaussMixtureModel::init(double *data, int N) {
    const double MIN_VAR = 1E-10;
    int *Label = new int[N];
    KMeansImpl kmeans = KMeansImpl(dimensionNumber, mixtureNumber);
    kmeans.cluster(data, N, Label);

    int *counts = new int[mixtureNumber];
    double *overMeans = new double[dimensionNumber];
    for (int i = 0; i < mixtureNumber; i++) {
        counts[i] = 0;
        priors[i] = 0;
        memcpy(means[i], kmeans.means[i], sizeof(double) * dimensionNumber);
        memset(vars[i], 0, sizeof(double) * dimensionNumber);
    }
    memset(overMeans, 0, sizeof(double) * dimensionNumber);
    memset(minVars, 0, sizeof(double) * dimensionNumber);

    int size = 0;
    size = N;

    double *x = new double[dimensionNumber];
    int label = -1;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < dimensionNumber; j++)
            x[j] = data[i * dimensionNumber + j];
        label = Label[i];

        counts[label]++;
        double *m = kmeans.means[label];
        for (int d = 0; d < dimensionNumber; d++) {
            vars[label][d] += (x[d] - m[d]) * (x[d] - m[d]);
        }

        for (int d = 0; d < dimensionNumber; d++) {
            overMeans[d] += x[d];
            minVars[d] += x[d] * x[d];
        }
    }

    for (int d = 0; d < dimensionNumber; d++) {
        overMeans[d] /= size;
        minVars[d] = max(MIN_VAR, 0.01 * (minVars[d] / size - overMeans[d] * overMeans[d]));
    }

    for (int i = 0; i < mixtureNumber; i++) {
        priors[i] = 1.0 * counts[i] / size;

        if (priors[i] > 0) {
            for (int d = 0; d < dimensionNumber; d++) {
                vars[i][d] = vars[i][d] / counts[i];

                if (vars[i][d] < minVars[d]) {
                    vars[i][d] = minVars[d];
                }
            }
        } else {
            memcpy(vars[i], minVars, sizeof(double) * dimensionNumber);
        }
    }
    delete[] x;
    delete[] counts;
    delete[] overMeans;
    delete[] Label;

}
