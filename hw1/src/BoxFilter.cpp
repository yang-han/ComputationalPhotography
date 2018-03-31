#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;


void myBoxFilter(Mat& src, Mat& dst, int w, int h) {
	Mat mask(Size(h, w), CV_32FC1, Scalar(1.0 / w / h));
	filter2D(src, dst, src.depth(), mask);
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
	myBoxFilter(mat, dst, w, h);
	imwrite(output, dst);
	return 0;
}
