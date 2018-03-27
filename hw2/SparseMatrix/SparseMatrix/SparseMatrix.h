#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
using std::vector;

class SparseMatrix
{
private:
	vector<double> vals;
	vector<int> rows;
	vector<int> cols;

public:
	int row_num = 0;
	int col_num = 0;

public:
	SparseMatrix();
	~SparseMatrix();

	double at(int row, int col) {
		for (int i = 0; i < rows.size(); ++i) {
			if (rows[i] == row && cols[i] == col)return vals[i];
		}
		return 0;
	}

	void insert(double val, int row, int col) {
		for (int i = 0; i < rows.size(); ++i) {
			if (rows[i] == row && cols[i] == col) {
				vals[i] = val;
				break;
			}
			if (rows[i] == row) {
				if (i == rows.size() - 1 || cols[i]<col && cols[i + 1]>col) {
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
		if(row_num == 0 && rows.size() != 0) this->row_num = *(rows.end() - 1) + 1;
		else this->row_num = row_num;
		if(col_num == 0 && cols.size() != 0) this->col_num = *(cols.end() - 1) + 1;
		else this->col_num = col_num;

	};

	void print() {
		std::cout << "Shape: (" << this->row_num << ", " << this->col_num << ")" << std::endl;
		for (int i = 0; i < row_num; ++i) {
			for (int j = 0; j < col_num; j++)
			{
				std::cout << this->at(i, j) << "\t";
			}
			std::cout << std::endl;
		}
	}

	SparseMatrix mul(SparseMatrix& m) {
		SparseMatrix res;
		if (this->col_num != m.row_num) {
			std::cout << "Shape doesn't match! (";
			std::cout << this->row_num << ", " << this->col_num << ")  (";
			std::cout << m.row_num <<", " << m.col_num  << ")" << std::endl;
			res.initializeFromVector(vector<int>({}), vector<int>({}), vector<double>({}), 0, 0);
			return res;
		};
		int res_row_num = this->row_num;
		int res_col_num = m.col_num;
		for (int i = 0; i < res_row_num; ++i) {
			for (int j = 0; j < res_col_num; ++j) {

			}
		}
		//x.print();
		return res;
	}

};

