#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;

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
	sort(pixels.begin(), pixels.end());
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

int main(int argc, char** argv) {
	if (argc != 5) {
		cout << "Input Illegal!" << endl;
	}
	char* input = argv[1];
	char* output = argv[2];
	int w = atoi(argv[3]);
	int h = atoi(argv[4]);
	Mat mat = imread(input);
	Mat dst;

	cout << mat.size() << " " << mat.type() << endl;
	myMedianFilter(mat, dst, w, h);
	imwrite(output, dst);
	return 0;
}
