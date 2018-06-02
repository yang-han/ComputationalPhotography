#include "hw3_gn.h"
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

class Solver4251 :public GaussNewtonSolver {
private:
	double* R;
	double* J;
public:
	Solver4251() {}
	~Solver4251() {
		delete[] R;
		delete[] J;
	};

	virtual double solve(
		ResidualFunction *f, // 目标函数
		double *X,           // 输入作为初值，输出作为结果
		GaussNewtonParams param = GaussNewtonParams(), // 优化参数
		GaussNewtonReport *report = nullptr // 优化结果报告
	)
	{
		int nR = f->nR();
		int nX = f->nX();
		R = new double[nR];
		J = new double[nR*nX];
		Mat Jmat(nR, nX, CV_64FC1);
		Mat Xmat(nX, 1, CV_64FC1);
		Mat Rmat(nR, 1, CV_64FC1);
		Mat delta;
		int max_iter = param.max_iter;
		int i = 0;
		for (i = 0; i < max_iter; i++)
		{
			f->eval(R, J, X);
			convertToMat(J, Jmat);
			convertToMat(R, Rmat);
			convertToMat(X, Xmat);
			cv::solve(Jmat, -1 * Rmat, delta, CV_SVD);
			double residual = norm(delta, NORM_L2);
			if (residual < param.residual_tolerance) {
				if (report != nullptr) {
					report->stop_type = GaussNewtonReport::STOP_RESIDUAL_TOL;
					report->n_iter = i;
				}
				break;
			}
			double gradient = norm(Jmat, NORM_L2);
			if (gradient < param.gradient_tolerance) {
				if (report != nullptr) {
					report->stop_type = GaussNewtonReport::STOP_GRAD_TOL;
					report->n_iter = i;
				}

			}
			Xmat += delta;
			X = Xmat.ptr<double>(0, 0);
			if (param.verbose) {
				cout << i + 1 << " time iter result: " << endl;
				cout << Xmat << endl;
				cout << "------------" << endl;
			}
		}

		if (i == max_iter) {
			if (report != nullptr) {
				report->stop_type = GaussNewtonReport::STOP_NO_CONVERGE;
				report->n_iter = i;
			}
		}

		cout << "Result get after " << i << " iterations: " << endl;
		cout << Xmat << endl;

		double result = norm(Rmat, NORM_L2);
		result = pow(result, 2);
		return result;
	}

private:
	void convertToMat(double* data, Mat& mat) {
		int n = mat.rows;
		int m = mat.cols;
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				mat.at<double>(i, j) = data[i*m + j];
			}

		}
	}
};


class Function : public ResidualFunction {
private:
	double params[753][3];
public:
	Function() {
		ifstream data("ellipse753.txt");
		if (!data.is_open()) {
			cout << "Open data failed." << endl;
			return;
		}
		for (int i = 0; i < 753; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				data >> params[i][j];
			}
		}		
	}

	virtual int nX() const {
		return 3;
	}

	virtual int nR() const {
		return 753;
	}

	virtual void eval(double *R, double *J, double *X) {
		int nR = this->nR();
		int nX = this->nX();
		// -2(xi^2)/A^3
		for (int i = 0; i < nR; i++)
		{
			for (int j = 0; j < nX; j++)
			{
				J[i*nX + j] = -2 * pow(params[i][j], 2) / pow(X[j], 3);
			}
		}
		for (int i = 0; i < nR; i++)
		{
			double res = 0;
			for (int j = 0; j < nX; j++)
			{
				res += pow(params[i][j], 2) / pow(X[j], 2);
			}
			R[i] = res - 1;
		}
	}

	virtual void linearCheck() {
		Mat data_mat(753, 3, CV_64FC1);

		for (int i = 0; i < 753; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				data_mat.at<double>(i, j) = pow(params[i][j], 2);
			}
		}

		Mat ymat(753, 1, CV_64FC1, Scalar(1.0));
		Mat res(3, 1, CV_64FC1);
		cv::solve(data_mat, ymat, res, CV_SVD);
		//cout << res << endl;
		for (int i = 0; i < 3; ++i) {
			double xi = res.at<double>(i, 0);
			cout << sqrt(1/xi) << endl;
		}

	}
};
