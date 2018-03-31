#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;


const double PI = 3.14159;

double G(double x2, double& sigma) {
	return (1.0 / 2 / PI / sigma) * exp(-1.0*(x2) / (2.0*sigma*sigma));
}

double bilateralProcess(Mat& src, int y, int x, int kernel_size, int ch, double sigma_s, double sigma_r) {
	Mat mask = Mat(Size(kernel_size, kernel_size), CV_32FC1);
	Size size = src.size();
	double result = 0;
	double factor = 0;
	double total_factor = 0;
	Vec3b color = src.at<Vec3b>(y, x);
	for (int i = y - kernel_size / 2; i <= y + kernel_size / 2; ++i) {
		for (int j = x - kernel_size / 2; j <= x + kernel_size / 2; ++j) {
			if (i >= 0 && i < src.rows && j >= 0 && j < src.cols) {
				factor = G(pow(i - y, 2) + pow(j - x, 2), sigma_s) * G(pow(src.at<Vec3b>(i, j)[ch] - color[ch], 2), sigma_r);
				result += factor * src.at<Vec3b>(i, j)[ch];
				total_factor += factor;
			}
			else {
				continue;
			}
		}
	}
	//cout << (int)src.at<Vec3b>(y, x)[ch] << " " << result <<" " << total_factor<< " " << result / total_factor << endl;
	return result / total_factor;
}

void myBilateralFilter(Mat& src, Mat& dst, int kernel_size, double sigma_s, double sigma_r) {
	dst.create(src.size(), src.type());
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			dst.at<Vec3b>(i, j)[0] = (char)bilateralProcess(src, i, j, kernel_size, 0, sigma_s, sigma_r);
			dst.at<Vec3b>(i, j)[1] = (char)bilateralProcess(src, i, j, kernel_size, 1, sigma_s, sigma_r);
			dst.at<Vec3b>(i, j)[2] = (char)bilateralProcess(src, i, j, kernel_size, 2, sigma_s, sigma_r);
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 5) {
		cout << "Input Illegal!" << endl;
	}
	char* input = argv[1];
	char* output = argv[2];
	int sigma_s = atoi(argv[3]);
	int sigma_r = atoi(argv[4]);
	Mat mat = imread(input);
	Mat dst;

	cout << mat.size() << " " << mat.type() << endl;
	myBilateralFilter(mat, dst, 5, sigma_s, sigma_r);
	imwrite(output, dst);
	return 0;
}
