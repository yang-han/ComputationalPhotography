#ifndef PANORAMA
#define PANORAMA
#include "hw6_pa.h"
#include <opencv2/xfeatures2d.hpp>
using namespace std;
using namespace cv;


class Panorama4251 : public CylindricalPanorama {
public:
	Panorama4251() {};
	~Panorama4251() {};
	bool makePanorama(std::vector<cv::Mat>& img_vec, cv::Mat& img_out, double f) {
		vector<Mat> img_cylinder;
		vector<Mat> results;
		for (int i = 0; i < img_vec.size(); ++i) {
			img_cylinder.push_back(cylinder(img_vec[i], f));
		}
		//int dx = getOffset(img_cylinder[0], img_cylinder[1]);
		//Mat result = stitch(img_cylinder[0], img_cylinder[1], dx);
		//results.push_back(img_cylinder[0]);
		Mat last_result = img_cylinder[0];
		//for (int i = 1; i < 4; ++i) {
		//	cout << "now: " << i << endl;
		//	Point2i dx = getOffset(result, img_cylinder[i]);
		//	linearStitch(result, img_cylinder[i], result, dx);
		//}
		for (int i = 1; i < img_cylinder.size(); ++i) {
			Mat img_1 = last_result;
			Mat img_2 = img_cylinder[i];
			Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
			vector<KeyPoint> kps_0, kps_1;
			f2d->detect(img_1, kps_0);
			f2d->detect(img_2, kps_1);

			Mat descriptors_0, descriptors_1;
			f2d->compute(img_1, kps_0, descriptors_0);
			f2d->compute(img_2, kps_1, descriptors_1);

			FlannBasedMatcher matcher;
			//BFMatcher matcher;
			vector<DMatch> matches;
			matcher.match(descriptors_0, descriptors_1, matches);
			sort(matches.begin(), matches.end());
			float min_v = numeric_limits<float>::max();
			float max_v = 0;
			for (int i = 0; i < matches.size(); ++i) {
				min_v = min(min_v, matches[i].distance);
				max_v = max(max_v, matches[i].distance);
			}
			vector<Point2f> ps_0, ps_1;
			//assert(matches.size() > 500);
			cout << "min_v " << min_v << endl;
			cout << "max_v " << max_v << endl;
			for (int i = 0; i<matches.size(); ++i) {
				DMatch m = matches[i];
				if (m.distance > max_v / 2 )continue;
				ps_0.push_back(kps_0[m.queryIdx].pt);
				ps_1.push_back(kps_1[m.trainIdx].pt);
			}
			cout << "total_size... " << matches.size() << endl;
			cout << "size... " << ps_0.size() << endl;
			Mat rev_H = findHomography(ps_1, ps_0, RANSAC);
			Mat H = findHomography(ps_0, ps_1, RANSAC);

			cout << "begin stitcher....  " << i << endl;
			vector<Point2f> corners_1(4);
			vector<Point2f> corners_2(4);
			corners_1[0] = Point2f(0, 0);
			corners_1[1] = Point2f((float)img_1.cols, 0);
			corners_1[2] = Point2f((float)img_1.cols, (float)img_1.rows);
			corners_1[3] = Point2f(0, (float)img_1.rows);

			perspectiveTransform(corners_1, corners_2, H);
			int down_rows = (int)min(corners_2[0].y, corners_2[1].y);
			down_rows = min(0, down_rows) * -1;
			int right_cols = (int)min(corners_2[0].x, corners_2[3].x);
			right_cols = min(0, right_cols) * -1;
			//int extend_cols = (int)max(g_vCorners2[1].x, g_vCorners1[2].x);
			//extend_cols = max(extend_cols, img_1.cols) + right_cols;
			//int extend_rows = (int)max(g_vCorners2[2].y, g_vCorners1[3].y);
			//extend_rows = max(extend_rows, img_1.rows) + down_rows;
			Mat stitch_img = Mat::zeros(img_2.rows+down_rows, img_2.cols+right_cols, CV_8UC3);
			img_2.copyTo(Mat(stitch_img, Rect(right_cols, down_rows, img_2.cols, img_2.rows)));
			for (int i = 0; i < stitch_img.rows; ++i) {
				for (int j = 0; j < stitch_img.cols; ++j) {
					if (stitch_img.at<Vec3b>(i, j) != Vec3b(0, 0, 0)) {
						continue;
					}
					int x0 = j - right_cols;
					int y0 = i - down_rows;
					vector<Point2f> pix, dst;
					pix.emplace_back(x0, y0);
					perspectiveTransform(pix, dst, rev_H);
					Point2f pos = dst[0];
					//cout << pos << endl;
					int x = (int)floor(pos.x);
					int y = (int)floor(pos.y);
					if (0 < y && y < img_1.rows && 0 < x && x < img_1.cols && img_1.at<Vec3b>(y,x) != Vec3b(0,0,0) ) {
						Vec3b c = img_1.at<Vec3b>(y, x);
						//if (stitch_img.at<Vec3b>(i,j) != Vec3b(0, 0, 0)) { c += (stitch_img.at<Vec3b>(i,j)-c)/2; }
						stitch_img.at<Vec3b>(i, j) = c;
					}
				}
			}
			cout << "stitched" << endl;
			//imshow("ffff", stitch_img);
			//waitKey(0);
			//destroyWindow("ffff");
			last_result = stitch_img;

		}
		cout << "finished" << endl;
		//imshow("result", last_result);
		//waitKey(0);
		last_result.copyTo(img_out);
		return true;
	};
	Point2f getTransformPoint(const Point2f originalPoint, const Mat &transformMaxtri)
	{
		Mat originelP, targetP;
		originelP = (Mat_<double>(3, 1) << originalPoint.x, originalPoint.y, 1.0);
		targetP = transformMaxtri*originelP;
		float x = targetP.at<double>(0, 0) / targetP.at<double>(2, 0);
		float y = targetP.at<double>(1, 0) / targetP.at<double>(2, 0);
		return Point2f(x, y);
	}

	Point2i getOffset(Mat& img, Mat& img1)
	{
		Mat templ(img1, Rect(0, 0.4*img1.rows, 0.2*img1.cols, 0.2*img1.rows));
		//Mat result(img.cols - templ.cols + 1, img.rows - templ.rows + 1, CV_8UC1);//result存放匹配位置信息
		Mat result;
		matchTemplate(img, templ, result, CV_TM_CCORR_NORMED);
		normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
		double minVal; double maxVal; Point minLoc; Point maxLoc; Point matchLoc;
		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
		matchLoc = maxLoc;//获得最佳匹配位置
		int dx = matchLoc.x;
		int dy = matchLoc.y - 0.4*img1.rows;//右图像相对左图像的位移
		Point2i a(dx, dy);
		cout << "delta: " << a << endl;
		return a;
	};

	//int getOffset(Mat& img, Mat& img1)
	//{
	//	Mat templ(img1, Rect(0, 0.4*img1.rows, 0.2*img1.cols, 0.2*img1.rows));
	//	Mat result;
	//	matchTemplate(img, templ, result, CV_TM_CCORR_NORMED);
	//	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	//	double minVal; double maxVal; Point minLoc; Point maxLoc; Point matchLoc;
	//	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//	cout << "delta: " << maxLoc.x << endl;
	//	return maxLoc.x;
	//};
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
				int x = (int)(f * tan((float)(j - cols * 0.5) / f) + img.cols*0.5);
				int y = (int)((i - 0.5*rows)*sqrt(pow(x - img.cols*0.5, 2) + f*f) / f + 0.5*img.rows);
				if (0 <= x && x < img.cols && 0 <= y && y < img.rows) {
					output.at<Vec3b>(i, j) = img.at<Vec3b>(y, x);
				}
				else {
					output.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
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


	void linearStitch(Mat& img, Mat& img1, Mat& output, Point2i a)
	{
		int d = img.cols - a.x;//过渡区宽度
		int ms = img.rows - abs(a.y);//拼接图行数
		int ns = img.cols + a.x;//拼接图列数
		Mat stitch = Mat::zeros(ms, ns, CV_8UC3);
		//拼接
		Mat ims(stitch);
		Mat im(img);
		Mat im1(img1);

		if (a.y >= 0)
		{
			Mat roi1(stitch, Rect(0, 0, a.x, ms));
			img(Range(a.y, img.rows), Range(0, a.x)).copyTo(roi1);
			Mat roi2(stitch, Rect(img.cols, 0, a.x, ms));
			img1(Range(0, ms), Range(d, img1.cols)).copyTo(roi2);
			for (int i = 0; i < ms; i++)
				for (int j = a.x; j < img.cols; j++) {
					ims.at<Vec3b>(i, j)[0] = uchar((img.cols - j) / float(d)*im.at<Vec3b>(i + a.y, j)[0] + (j - a.x) / float(d)*im1.at<Vec3b>(i, j - a.x)[0]);
					ims.at<Vec3b>(i, j)[1] = uchar((img.cols - j) / float(d)*im.at<Vec3b>(i + a.y, j)[1] + (j - a.x) / float(d)*im1.at<Vec3b>(i, j - a.x)[1]);
					ims.at<Vec3b>(i, j)[2] = uchar((img.cols - j) / float(d)*im.at<Vec3b>(i + a.y, j)[2] + (j - a.x) / float(d)*im1.at<Vec3b>(i, j - a.x)[2]);
				}
		}
		else
		{
			Mat roi1(stitch, Rect(0, 0, a.x, ms));
			img(Range(0, ms), Range(0, a.x)).copyTo(roi1);
			Mat roi2(stitch, Rect(img.cols, 0, a.x, ms));
			img1(Range(-a.y, img.rows), Range(d, img1.cols)).copyTo(roi2);
			for (int i = 0; i < ms; i++)
				for (int j = a.x; j < img.cols; j++) {
					ims.at<Vec3b>(i, j)[0] = uchar((img.cols - j) / float(d)*im.at<Vec3b>(i, j)[0] + (j - a.x) / float(d)*im1.at<Vec3b>(i + abs(a.y), j - a.x)[0]);
					ims.at<Vec3b>(i, j)[1] = uchar((img.cols - j) / float(d)*im.at<Vec3b>(i, j)[1] + (j - a.x) / float(d)*im1.at<Vec3b>(i + abs(a.y), j - a.x)[1]);
					ims.at<Vec3b>(i, j)[2] = uchar((img.cols - j) / float(d)*im.at<Vec3b>(i, j)[2] + (j - a.x) / float(d)*im1.at<Vec3b>(i + abs(a.y), j - a.x)[2]);
				}
		}
		cout << ims.size() << endl;
		ims.copyTo(output);
		//return stitch;
	}

};

#endif // !PANORAMA

