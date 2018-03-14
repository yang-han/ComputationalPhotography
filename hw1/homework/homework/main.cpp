#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;


const double PI = 3.14159;

//实现盒状均值滤波
//实现高斯滤波
//实现中值滤波
//实现简单的双边滤波
//利用傅里叶变换完成图像的频域变换

void myBoxFilter(Mat& src, Mat& dst, int w, int h) {
	Mat mask(Size(h, w), CV_32FC1, Scalar(1.0 / w / h));
	filter2D(src, dst, src.depth(), mask);
}

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

int medianProcess(Mat& src, int y, int x, int w, int h, int ch) {
	vector<int> pixels;
	int mid_w = w / 2;
	int mid_h = h / 2;
	Size size = src.size();
	for (int i = y - mid_h; i <= y + mid_h; ++i) {
		for (int j = x - mid_w; j <= x + mid_w; ++j) {
			if (i >= 0 && i < size.height && j >= 0 && j < size.width) {
				pixels.push_back(src.at<Vec3b>(i, j)[ch]);
			}
			else {
				continue;
			}
		}
	}
	//cout << "- " << pixels.size() << " "<< y << "  " << x << endl;
	//cout << pixels.size() << endl;
	//sort(pixels.begin(), pixels.end());
	return pixels.at(pixels.size() / 2);
}


void myMedianFilter(Mat& src, Mat& dst, int w, int h) {
	//medianBlur(src, des, w);
	Size size = src.size();
	dst.create(size, src.type());
	//cout << src << endl;
	for (int i = 0; i < size.height; ++i) {
		for (int j = 0; j < size.width; ++j) {
			//cout << "processing " << i << ' ' << j << endl;
			dst.at<Vec3b>(i, j)[0] = medianProcess(src, i, j, w, h, 0);
			dst.at<Vec3b>(i, j)[1] = medianProcess(src, i, j, w, h, 1);
			dst.at<Vec3b>(i, j)[2] = medianProcess(src, i, j, w, h, 2);
			//cout << "srccc " << src << endl;
		}
	}
	//cout << des.size() << ' ' << des.type() << endl;
	//cout << des << endl;
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
	mag += Scalar::all(0);

	log(mag, mag);

	normalize(mag, dst, 0, 1, CV_MINMAX);
}


int main() {
	//Mat mat = imread("C:\\Users\\sleepy\\OneDrive\\Pictures\\2615063031ohkhke.jpg", CV_LOAD_IMAGE_COLOR);
	Mat mat = imread("C:\\Users\\sleepy\\OneDrive\\Pictures\\2615063031ohkhke.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	Mat dst;

	cout << mat.size() << " " << mat.type() << endl;
	//myBoxFilter(mat, dst, 3, 3);
	//myGaussianFilter(mat, dst, 10);
	//myMedianFilter(mat, dst, 3, 3);
	//myBilateralFilter(mat, dst, 5, 1.0, 1000);
	myDFT(mat, dst);


	//cout << dst << endl;
	imshow("src", mat);
	imshow("dst", dst);

	cv::waitKey(0);
	cv::destroyAllWindows();
	return 0;
}
