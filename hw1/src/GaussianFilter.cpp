#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;


const double PI = 3.14159;

double G(double x2, double& sigma) {
	return (1.0 / 2 / PI / sigma) * exp(-1.0*(x2) / (2.0*sigma*sigma));
}

void myGaussianFilter(Mat& src, Mat& dst, double sigma) {
	int size = 5;
	int mid = size / 2;
	Mat mask(Size(size, size), CV_32FC1);
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			mask.at<float>(i, j) = (float)G(pow(i - mid, 2) + pow(j - mid, 2), sigma);
		}
	}
	double s = sum(mask)[0];
	mask = mask / s;
	filter2D(src, dst, src.depth(), mask);
}

int main(int argc, char** argv) {
	if (argc != 4) {
		cout << "Input Illegal!" << endl;
	}
	char* input = argv[1];
	char* output = argv[2];
	int sigma = atoi(argv[3]);
	Mat mat = imread(input);
	Mat dst;
	cout << mat.size() << " " << mat.type() << endl;
	myGaussianFilter(mat, dst, sigma);
	imwrite(output, dst);
	return 0;
}
