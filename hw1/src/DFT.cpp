#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;


void fftshift(const Mat &src, Mat &dst) {
	dst.create(src.size(), src.type());
	int rows = src.rows, cols = src.cols;
	Rect roiTopBand, roiBottomBand, roiLeftBand, roiRightBand;
	if (rows % 2 == 0) {
		roiTopBand = Rect(0, 0, cols, rows / 2);
		roiBottomBand = Rect(0, rows / 2, cols, rows / 2);
	}
	else {
		roiTopBand = Rect(0, 0, cols, rows / 2 + 1);
		roiBottomBand = Rect(0, rows / 2 + 1, cols, rows / 2);
	}
	if (cols % 2 == 0) {
		roiLeftBand = Rect(0, 0, cols / 2, rows);
		roiRightBand = Rect(cols / 2, 0, cols / 2, rows);
	}
	else {
		roiLeftBand = Rect(0, 0, cols / 2 + 1, rows);
		roiRightBand = Rect(cols / 2 + 1, 0, cols / 2, rows);
	}
	Mat srcTopBand = src(roiTopBand);
	Mat dstTopBand = dst(roiTopBand);
	Mat srcBottomBand = src(roiBottomBand);
	Mat dstBottomBand = dst(roiBottomBand);
	Mat srcLeftBand = src(roiLeftBand);
	Mat dstLeftBand = dst(roiLeftBand);
	Mat srcRightBand = src(roiRightBand);
	Mat dstRightBand = dst(roiRightBand);
	flip(srcTopBand, dstTopBand, 0);
	flip(srcBottomBand, dstBottomBand, 0);
	flip(dst, dst, 0);
	flip(srcLeftBand, dstLeftBand, 1);
	flip(srcRightBand, dstRightBand, 1);
	flip(dst, dst, 1);
}


void myDFT(Mat& src, Mat& dst) {
	int m = getOptimalDFTSize(src.rows);
	int n = getOptimalDFTSize(src.cols);
	Mat padded;
	copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.rows, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(),CV_32F) };
	Mat complexMat;
	merge(planes, 2, complexMat);

	dft(complexMat, complexMat, DFT_COMPLEX_OUTPUT);

	fftshift(complexMat, complexMat);

	split(complexMat, planes);
	magnitude(planes[0], planes[1], planes[0]);
	Mat mag = planes[0];
	mag += Scalar::all(1);

	log(mag, mag);

	normalize(mag, mag, 0, 1, CV_MINMAX);
	mag.convertTo(dst, CV_8U, 255);
}


int main(int argc, char** argv) {
	if (argc != 3) {
		cout << "Input Illegal!" << endl;
	}
	char* input = argv[1];
	char* output = argv[2];
	Mat mat = imread(input, CV_LOAD_IMAGE_GRAYSCALE);
	Mat dst;

	cout << mat.size() << " " << mat.type() << endl;
	myDFT(mat, dst);

	imwrite(output, dst);
	return 0;
}
