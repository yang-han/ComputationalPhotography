#include "SparseMatrix.h"

int main() {
	SparseMatrix sp;
	vector<int> rows({ 0,0,0,1,1,1,1,2,2,2,2,3,3,3 });
	vector<int> cols({ 0,1,2,0,1,2,3,0,1,2,3,1,2,3 });
	vector<double> vals({ 10,-1,2,-1,11,1,3,2,-1,10,-1,3,-1,8 });
	sp.initializeFromVector(rows, cols,vals);
	sp.print();
	sp.insert(0.38, 0, 0);
	sp.print();
	sp.insert(0.99, 0, 3);
	sp.print();
	SparseMatrix matrix2;
	matrix2.initializeFromVector(vector<int>({}), vector<int>({}), vector<double>({}), 9, 10);
	SparseMatrix m = sp.mul(matrix2);
	m.print();
	system("pause");


	return 0;
}