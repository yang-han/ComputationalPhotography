#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
using std::vector;

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
		if (val == 0)return;
		if (this->rows.size() == 0) {
			rows.insert(rows.begin(), row);
			cols.insert(cols.begin(), col);
			vals.insert(vals.begin(), val);
		}
		for (int i = 0; i < rows.size(); ++i) {
			if (rows[i] == row && cols[i] == col) {
				vals[i] = val;
				break;
			}
			if (rows[i] == row) {
				if (i == rows.size() - 1 || cols[i]<col && cols[i + 1] > col || cols[i]<col && rows[i+1]>row){
					rows.insert(rows.begin() + i + 1, row);
					cols.insert(cols.begin() + i + 1, col);
					vals.insert(vals.begin() + i + 1, val);
					break;
				}
			}
			else if (i == rows.size() - 1 || rows[i]<row && rows[i + 1] >row) {
				rows.insert(rows.begin() + i + 1, row);
				cols.insert(cols.begin() + i + 1, col);
				vals.insert(vals.begin() + i + 1, val);
				break;
			}
		}
	}

	void initializeFromVector(vector<int>& rows, vector<int>& cols, vector<double>& vals, int row_num = 0, int col_num = 0) {
		this->vals = vals;
		this->rows = rows;
		this->cols = cols;
		//for each (auto var in rows)
		//{
		//	this->row = this->row > var ? this->row : var;
		//}
		//for each (auto var in cols)
		//{
		//	this->col = this->col > var ? this->col : var;
		//}
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

	SparseMatrix inv() {
		SparseMatrix B(this->row_num, this->col_num);
		for (int i = 0; i < B.row_num; ++i) {
			B.insert(1.0 / this->at(i, i), i, i);
		}
		for (int i = 1; i < B.row_num; ++i) {
			for (int j = 0; j < i; ++j) {
				double val = 0;
				for (int k = j; k <= i - 1; ++k) {
					val += this->at(i, k) * B.at(k, j);
				}
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

	SparseMatrix mul(const SparseMatrix& m2) {
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
				/*p_i = 0;
				while (this->rows[p_i] < i)
				{
					++p_i;
				}
				if (this->rows[p_i] > i) continue;
				while (this->rows[p_i] == i) {
					res_val += vals[p_i] * m2.at(cols[p_i], j);
					++p_i;
				}*/
				//std::cout << i << "... " << j << ".... " << res_val << std::endl;
				if (res_val != 0) {
					res.insert(res_val, i, j);
				}
			}
		}
		return res;
	}

	SparseMatrix operator*(const SparseMatrix& m2) {
		return this->mul(m2);
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

	friend SparseMatrix operator*(double a, SparseMatrix x) {
		//std::cout << "*****!: " << a << std::endl;
		vector<double> vals;
		for (int i = 0; i < x.vals.size(); ++i) {
			vals.push_back(x.vals[i] * a);
		}
		SparseMatrix res(x.row_num, x.col_num);
		res.initializeFromVector(x.rows, x.cols, vals, x.row_num, x.col_num);
		return res;
	}
};

