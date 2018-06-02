#include <iostream>
#include "solver.h"


int main(int argc, char *argv[])
{

	Function* f = new Function();
	double* X = new double[f->nX()];
	X[0] = 1;
	X[1] = 1;
	X[2] = 1;

	Solver4251* solver = new Solver4251();
	GaussNewtonParams params;
	params.verbose = true;

	double result = solver->solve(f, X, params, new GaussNewtonReport());
	cout << "Final value: " << result << endl;

	cout << "--------------------" << endl;
	cout << "Linear Check:" << endl;
	f->linearCheck();

	delete[] X;
	delete f;
	delete solver;
	return 0;
}
