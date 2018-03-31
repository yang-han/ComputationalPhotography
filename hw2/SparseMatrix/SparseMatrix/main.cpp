#include "SparseMatrix.h"
using namespace std;


SparseMatrix GaussianSiedel(SparseMatrix& A, SparseMatrix& b, SparseMatrix& x) {
	SparseMatrix L;
	SparseMatrix U;
	vector<int> L_row;
	vector<int> L_col;
	vector<double> L_val;
	vector<int> U_row;
	vector<int> U_col;
	vector<double> U_val;
	for (int i = 0; i < A.rows.size(); ++i) {
		if (A.rows[i] >= A.cols[i]) {
			L_row.insert(L_row.end(), A.rows[i]);
			L_col.insert(L_col.end(), A.cols[i]);
			L_val.insert(L_val.end(), A.vals[i]);
		}
		else {
			U_row.insert(U_row.end(), A.rows[i]);
			U_col.insert(U_col.end(), A.cols[i]);
			U_val.insert(U_val.end(), A.vals[i]);
		}
	}
	L.initializeFromVector(L_row, L_col, L_val, A.row_num, A.col_num);
	U.initializeFromVector(U_row, U_col, U_val, A.row_num, A.col_num);
	L.print();
	U.print();
	SparseMatrix L_rev = L.inv();
	L_rev.print();
	U.print();
	b.print();
	SparseMatrix T = L_rev * U;
	SparseMatrix C = L_rev * b;
	T.print();
	C.print();
	int max_times = 500;
	int times = max_times;
	while (times--) {
		x.print();
		SparseMatrix new_x = C - T * x;
		if (new_x == x) {
			cout << "Iterate times: " << max_times - times << endl;
			break;
		}
		else {
			x = new_x;
		}
	}
	return x;
}

SparseMatrix ConjugateGradient(SparseMatrix& A, SparseMatrix& b, SparseMatrix& x) {
	SparseMatrix r = b - A*x;
	SparseMatrix p = r;
	r.print();
	int max_times = 500;
	int times = max_times;
	double a = 0;
	while (times--) {
		x.print();
		a = (r.transpose() * r).at(0, 0) / (p.transpose()*A*p).at(0,0);
		SparseMatrix new_x = x + a*p;
		if (new_x == x) {
			cout << "Iterate times: " << max_times - times << endl;
			break;
		}
		else {
			x = new_x;
		}
		SparseMatrix prev_r = r;
		r = r - a*A*p;
		double beta = (r.transpose() * r).at(0, 0) / (prev_r.transpose()*prev_r).at(0, 0);
		p = r - beta*p;
	}
	return x;
}

int main() {
	SparseMatrix A;
	vector<int> rows({ 0,0,0,1,1,1,1,2,2,2,2,3,3,3 });
	vector<int> cols({ 0,1,2,0,1,2,3,0,1,2,3,1,2,3 });
	vector<double> vals({ 10,-1,2,-1,11,-1,3,2,-1,10,-1,3,-1,8 });
	A.initializeFromVector(rows, cols,vals);
	A.print();
	SparseMatrix b(4, 1);
	b.initializeFromVector(vector<int>({ 0,1,2,3 }), vector<int>({ 0,0,0,0 }), vector<double>({ 6,25,-11,15 }), 4, 1);
	b.print();
	SparseMatrix x(4, 1);
	SparseMatrix res;

	//res = GaussianSiedel(A, b, x);
	res = ConjugateGradient(A, b, x);

	cout << "Result:" << endl;
	res.print();
	
	
	system("pause");
	return 0;
}