#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include "panorama.h"
using namespace std;
using namespace cv;

int main() {
	Panorama4251* panorama = new Panorama4251();
	vector<string> file_list;
	string base_dir = "C:\\yh\\3rdyear_spr_sum\\ComputationalPhotography\\hw5\\panorama\\panorama-data1\\";

	vector<Mat> img_list;
	for (int i = 1538; i < 1550; ++i) {
	//for (int i = 1599; i < 1619; ++i) {
		ostringstream ss;
		ss << base_dir << "DSC0" << i << ".JPG";
		Mat img = imread(ss.str(), CV_LOAD_IMAGE_COLOR);
		//cout << img.empty() << endl;
		img_list.push_back(img);
		//imshow(ss.str(), img);
	}

	//for (int i = 0; i < img_list.size(); ++i) {
	//	ostringstream ss;
	//	ss << "DSC0" << i << ".JPG";
	//	imshow(ss.str(), img_list[i]);
	//}

	Mat output;
	double f;
	ifstream f_file;
	f_file.open(base_dir + "K.txt");
	f_file >> f;
	cout << f << endl;
	panorama->makePanorama(img_list, output, f);


	waitKey(0);
	destroyAllWindows();
	return 0;
}