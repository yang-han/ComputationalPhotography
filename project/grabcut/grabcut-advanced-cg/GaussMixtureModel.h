#pragma once

class GaussMixtureModel {
public:
    GaussMixtureModel(int _dimensionNumber = 3, int _mixtureNumber = 5);

    ~GaussMixtureModel();

    double getProbability(const double *sample);

    void init(double *data, int N);

    void train(double *data, int N);

private:
    int dimensionNumber;
    int mixtureNumber;
    double *priors;
    double **means;
    double **vars;


    double *minVars;
    int maxIterationNumber;
    double endError;

    double getProbability(const double *x, int j);

    void allocate();

    void dispose();
};
