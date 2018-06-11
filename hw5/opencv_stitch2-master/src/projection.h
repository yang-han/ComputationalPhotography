#ifndef PROJECTION_H
#define PROJECTION_H
#include <iostream>
#include <cmath>
#include <string>
#include "opencv2/opencv.hpp"

#define PI 3.1415926

using namespace cv;
using namespace std;

//柱面投影
Mat Cylinder_projection(Mat& input) {
	int width = input.cols;
	int height = input.rows;
	int centerX = width / 2;
	int centerY = height / 2;

	Mat output = input.clone();
	double f = width / (2 * tan(PI / 6 / 2)); //视角设为30度

	double theta, pointX, pointY;
	for (int i = 0; i < height; i ++) {
		for (int j = 0; j < width; j ++) {
			theta = asin((j - centerX) / f);
			pointY = f * tan((j - centerX) / f) + centerX;
			pointX = (i - centerY) / cos(theta) + centerY;
			for (int k = 0; k < input.channels(); k ++) {
				if (pointX >= 0 && pointX <= height && pointY >= 0 && pointY <= width)
					output.at<Vec3b>(i, j)[k] = input.at<Vec3b>(pointX, pointY)[k];
				else 
					output.at<Vec3b>(i, j)[k] = 0;
			}
		}
	}
	return output;
}

string turnIntoStr(int num) {
	string ss;
	int a = num / 10;
	int b = num % 10;
	ss.push_back('0'+a);
	ss.push_back('0'+b);
	return ss;
}

//dataset 2 预处理
Mat pre_processing(Mat input) {
	//缩小为1/4
	//逆时针90度
	int row = input.rows / 4;
	int col = input.cols / 4;
	Mat output = Mat::zeros(col, row, CV_8UC3);
	for (int i = 0; i < col; i ++) {
		for (int j = 0; j < row; j ++) {
			if(input.rows - 4 * j >= 0 && input.rows - 4 * j < input.rows && input.cols - 4 * i >= 0 && input.cols - 4 * i < input.cols)
				output.at<Vec3b>(i, j) = input.at<Vec3b>(input.rows - 4 * j, input.cols - 4 * i);
		}
	}
	return output;
}

#endif
