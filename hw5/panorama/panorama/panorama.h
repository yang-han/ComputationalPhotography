#ifndef PANORAMA
#define PANORAMA
#include "hw6_pa.h"
using namespace std;
using namespace cv;


class Panorama4251 : public CylindricalPanorama {
public:
	Panorama4251() {};
	~Panorama4251() {};
	bool makePanorama(std::vector<cv::Mat>& img_vec, cv::Mat& img_out, double f) {
		Mat o = cylinder(img_vec[0], f);
		cout << img_vec[0].size() << endl;
		cout << o.size() << endl;
		imshow("hello", o);
		imshow("hi", img_vec[0]);
		vector<Mat> img_cylinder;
		for (int i = 0; i < img_vec.size(); ++i) {
			img_cylinder.push_back(cylinder(img_vec[i], f));
		}
		//int dx = getOffset(img_cylinder[0], img_cylinder[1]);
		//Mat result = stitch(img_cylinder[0], img_cylinder[1], dx);
		Mat result = img_cylinder[0];
		for (int i = 1; i < img_cylinder.size(); ++i) {
			cout << "now: " << i << endl;
			int dx = getOffset(result, img_cylinder[i]);
			result = stitch(result, img_cylinder[i], dx);
		}
		imshow("result", result);


		return true; 
	}; 
	
	//Point2i getOffset(Mat& img, Mat& img1)
	//{
	//	Mat templ(img1, Rect(0, 0.4*img1.rows, 0.2*img1.cols, 0.2*img1.rows));
	//	//Mat result(img.cols - templ.cols + 1, img.rows - templ.rows + 1, CV_8UC1);//result存放匹配位置信息
	//	Mat result;
	//	matchTemplate(img, templ, result, CV_TM_CCORR_NORMED);
	//	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	//	double minVal; double maxVal; Point minLoc; Point maxLoc; Point matchLoc;
	//	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//	matchLoc = maxLoc;//获得最佳匹配位置
	//	int dx = matchLoc.x;
	//	int dy = matchLoc.y - 0.4*img1.rows;//右图像相对左图像的位移
	//	Point2i a(dx, dy);
	//	cout << "delta: " << a << endl;
	//	return a;
	//};

	int getOffset(Mat& img, Mat& img1)
	{
		Mat templ(img1, Rect(0, 0.4*img1.rows, 0.2*img1.cols, 0.2*img1.rows));
		Mat result;
		matchTemplate(img, templ, result, CV_TM_CCORR_NORMED);
		normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
		double minVal; double maxVal; Point minLoc; Point maxLoc; Point matchLoc;
		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
		cout << "delta: " << maxLoc.x << endl;
		return maxLoc.x;
	};
	Mat cylinder(Mat& img, double f) {
		Mat output;
		int cols = (int)2 * f * atan(0.5*img.cols / f);
		int rows = (int)img.rows;
		cout << cols << " " << rows << endl;
		cout << img.type() << endl;
		output.create(rows, cols, CV_8UC3);
		cout << output.type() << " " << output.size() << endl;
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				int x = (int)(f * tan((float)(j - cols * 0.5)/f) + img.cols*0.5);
				int y = (int)((i-0.5*rows)*sqrt(pow(x-img.cols*0.5, 2)+f*f)/f+0.5*img.rows);
				if (0 <= x && x < img.cols && 0 <= y && y < img.rows) {
					output.at<Vec3b>(i, j)[0] = img.at<Vec3b>(y, x)[0];
					output.at<Vec3b>(i, j)[1] = img.at<Vec3b>(y, x)[1];
					output.at<Vec3b>(i, j)[2] = img.at<Vec3b>(y, x)[2];
				}
				else {
					output.at<Vec3b>(i, j)[0] = 0;
					output.at<Vec3b>(i, j)[1] = 0;
					output.at<Vec3b>(i, j)[2] = 0;
				}
			}
		}
		return output;
	}
	Mat stitch(Mat& img1, Mat& img2, int dx) {
		assert(img1.rows == img2.rows);
		//Mat result(img1.rows, img1.cols + img2.rows - dx, CV_8UC3);
		Mat result;
		cout << 1 << endl;
		//Mat _img(img2, Rect(dx, 0, img2.cols-dx, img2.rows));
		Mat _img1(img1, Rect(0, 0, dx, img1.rows));

		cout << 2 << endl;
		cout << img1.dims << " " << img1.size() << " " << img1.type() << endl;
		cout << img2.dims << " " << img2.size() << " " << img2.type() << endl;
		hconcat(_img1, img2, result);

		cout << 3 << endl;
		return result;

	}


	Mat linearStitch(Mat img, Mat img1, Point2i a)
	{
		int d = img.cols - a.x;//过渡区宽度
		int ms = img.rows - abs(a.y);//拼接图行数
		int ns = img.cols + a.x;//拼接图列数
		Mat stitch = Mat::zeros(ms, ns, CV_8UC3);
		//拼接
		Mat_<uchar> ims(stitch);
		Mat_<uchar> im(img);
		Mat_<uchar> im1(img1);

		if (a.y >= 0)
		{
			Mat roi1(stitch, Rect(0, 0, a.x, ms));
			img(Range(a.y, img.rows), Range(0, a.x)).copyTo(roi1);
			Mat roi2(stitch, Rect(img.cols, 0, a.x, ms));
			img1(Range(0, ms), Range(d, img1.cols)).copyTo(roi2);
			for (int i = 0; i < ms; i++)
				for (int j = a.x; j < img.cols; j++)
					ims(i, j) = uchar((img.cols - j) / float(d)*im(i + a.y, j) + (j - a.x) / float(d)*im1(i, j - a.x));

		}
		else
		{
			Mat roi1(stitch, Rect(0, 0, a.x, ms));
			img(Range(0, ms), Range(0, a.x)).copyTo(roi1);
			Mat roi2(stitch, Rect(img.cols, 0, a.x, ms));
			img1(Range(-a.y, img.rows), Range(d, img1.cols)).copyTo(roi2);
			for (int i = 0; i < ms; i++)
				for (int j = a.x; j < img.cols; j++)
					ims(i, j) = uchar((img.cols - j) / float(d)*im(i, j) + (j - a.x) / float(d)*im1(i + abs(a.y), j - a.x));
		}


		return stitch;
	}

};

#endif // !PANORAMA

