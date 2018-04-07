#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
using std::vector;
using std::cout;
using std::endl;

class SparseMatrix
{
public:
	vector<double> vals;
	vector<int> rows;
	vector<int> cols;

public:
	int row_num = 0;
	int col_num = 0;

public:
	SparseMatrix(int row = 0, int col = 0);
	~SparseMatrix();

	double at(int row, int col) const{
		for (int i = 0; i < rows.size(); ++i) {
			if (rows[i] == row && cols[i] == col)return vals[i];
		}
		return 0;
	}

	void insert(double val, int row, int col) {
		if (val == 0) {
			return; 
		}
		for (int i = 0; i < rows.size(); ++i) {
			if (rows[i] == row && cols[i] == col) {
				vals[i] = val;
				return;
			}
		}
		rows.insert(rows.end(), row);
		cols.insert(cols.end(), col);
		vals.insert(vals.end(), val);
	}

	void initializeFromVector(vector<int>& rows, vector<int>& cols, vector<double>& vals, int row_num = 0, int col_num = 0) {
		this->vals = vals;
		this->rows = rows;
		this->cols = cols;
		if (row_num == 0 && rows.size() != 0) this->row_num = *(rows.end() - 1) + 1;
		else this->row_num = row_num;
		if (col_num == 0 && cols.size() != 0) this->col_num = *(cols.end() - 1) + 1;
		else this->col_num = col_num;
	};

	void print() const {
		std::cout << "Shape: (" << this->row_num << ", " << this->col_num << ")" << std::endl;
		for (int i = 0; i < row_num; ++i) {
			for (int j = 0; j < col_num; j++)
			{
				std::cout << this->at(i, j) << "\t";
			}
			std::cout << std::endl;
		}
	}

	void setShape(int row_num, int col_num) {
		this->row_num = row_num;
		this->col_num = col_num;
	}

	SparseMatrix invLowTriangle() const{
		SparseMatrix B(this->row_num, this->col_num);
		for (int i = 0; i < B.row_num; ++i) {
			B.insert(1.0 / this->at(i, i), i, i);
		}
		for (int i = 1; i < B.row_num; ++i) {
			for (int j = 0; j < i; ++j) {
				double val = 0;
				for (int k = j; k <= i - 1; ++k) {
					//cout << "A " << this->at(i, k) << " B " << B.at(k, j) << endl;
					val += this->at(i, k) * B.at(k, j);
				}

				//cout << "B " << B.at(i, i) << " val " << val << endl;
				B.insert(-1.0*B.at(i, i)*val, i, j);
			}
		}
		return B;
	}

	SparseMatrix transpose() {
		SparseMatrix B(this->col_num, this->row_num);
		for (int i = 0; i < this->rows.size(); ++i) {
			B.insert(this->vals[i], this->cols[i], this->rows[i]);
		}
		return B;
	}

	SparseMatrix operator*(const SparseMatrix& m2) {
		SparseMatrix res(this->row_num, m2.col_num);
		if (this->col_num != m2.row_num) {
			std::cout << "Shape doesn't match when calculating * ! (";
			std::cout << this->row_num << ", " << this->col_num << ")  (";
			std::cout << m2.row_num << ", " << m2.col_num << ")" << std::endl;
			return res;
		};
		int p_i = 0, p_j = 0;
		double res_val;
		for (int i = 0; i < res.row_num; ++i) {
			for (int j = 0; j < res.col_num; ++j) {
				res_val = 0;
				for (int m = 0; m < this->col_num; ++m) {
					res_val += this->at(i, m) * m2.at(m, j);
				}
				if (res_val != 0) {
					res.insert(res_val, i, j);
				}
			}
		}
		return res;
	}

	SparseMatrix operator+(const SparseMatrix& m2) {
		SparseMatrix res(this->row_num, this->col_num);
		if (this->row_num != m2.row_num || this->col_num != m2.col_num) {
			std::cout << "Shape doesn't match when calculating + ! (";
			std::cout << this->row_num << ", " << this->col_num << ")  (";
			std::cout << m2.row_num << ", " << m2.col_num << ")" << std::endl;
		}
		for (int i = 0; i < this->row_num; ++i) {
			for (int j = 0; j < this->col_num; ++j) {
				res.insert(this->at(i, j) + m2.at(i, j), i, j);
			}
		}
		return res;
	}

	SparseMatrix operator-(const SparseMatrix& m2) {
		SparseMatrix res(this->row_num, this->col_num);
		if (this->row_num != m2.row_num || this->col_num != m2.col_num) {
			std::cout << "Shape doesn't match when calculating - ! (";
			std::cout << this->row_num << ", " << this->col_num << ")  (";
			std::cout << m2.row_num << ", " << m2.col_num << ")" << std::endl;
		}
		for (int i = 0; i < this->row_num; ++i) {
			for (int j = 0; j < this->col_num; ++j) {
				res.insert(this->at(i, j) - m2.at(i, j), i, j);
			}
		}
		return res;
	}

	bool operator==(const SparseMatrix& m2) {
		if (this->row_num != m2.row_num || this->col_num != m2.col_num ) {
			return false;
		}
		for (int i = 0; i < this->row_num; ++i) {
			for (int j = 0; j < this->col_num; ++j) {
				if (abs(this->at(i, j) - m2.at(i, j)) > 1e-7) {
					return false;
				}
			}
		}
		return true;
	}

	friend SparseMatrix operator*(double a, SparseMatrix x) {
		vector<double> vals;
		for (int i = 0; i < x.vals.size(); ++i) {
			vals.push_back(x.vals[i] * a);
		}
		SparseMatrix res(x.row_num, x.col_num);
		res.initializeFromVector(x.rows, x.cols, vals, x.row_num, x.col_num);
		return res;
	}
};

