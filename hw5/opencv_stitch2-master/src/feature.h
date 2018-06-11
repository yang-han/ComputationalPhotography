#include "opencv2/opencv.hpp"
#include <vector>
#include "projection.h"

// using namespace cv::xfeatures2d;

Mat stitch(Mat& input1, Mat& input2) {
	//cout << "begin stitcher1\n";
	Mat img_1 = Mat(input1.rows, input1.cols, 1);
	Mat img_2 = Mat(input2.rows, input2.cols, 1);
	cvtColor(input1, img_1, CV_BGR2GRAY);
	cvtColor(input2, img_2, CV_BGR2GRAY);

	//cout << "begin stitcher2\n";
	Ptr<SURF> detector =  cv::xfeatures2d::SURF::create();
  	std::vector<KeyPoint> keypoints_1, keypoints_2, kp1, kp2;

	detector->detect( img_1, kp1);
	detector->detect( img_2, kp2);

	//cout << "begin stitcher3\n";
	//优化KeyPoint
	int thv = input2.cols > input1.cols ? input1.cols : input2.cols;
	thv /= 2;
	int range = 3;
	for (int i = 0; i < kp1.size(); i ++) {
		if(kp1[i].pt.x > input1.cols - thv) {
			int count = 0;
			for(int p = - range; p <= range; p ++)
				for (int q = - range; q <= range; q ++) {
					if(kp1[i].pt.y + p >= 0 && kp1[i].pt.y + p < input1.rows && kp1[i].pt.x + q >= 0 && kp1[i].pt.x + q < input1.cols && input1.at<Vec3b>(kp1[i].pt.y + p, kp1[i].pt.x + q) == Vec3b(0,0,0))
						count++;
				}
			
			if (count <= 6) keypoints_1.push_back(kp1[i]);
		}
	}
	//cout << "begin stitcher4\n";
	cout << kp2.size() << endl;
	for (int i = 0; i < kp2.size(); i ++) {
		if(kp2[i].pt.x < thv) {
			int count = 0;
			for(int p = - range; p <= range; p ++)
				for (int q = - range; q <= range; q ++) {
					if(kp2[i].pt.y + p >= 0 && kp2[i].pt.y + p < input2.rows && kp2[i].pt.x + q >= 0 && kp2[i].pt.x + q < input2.cols && input2.at<Vec3b>(kp2[i].pt.y + p, kp2[i].pt.x + q) == Vec3b(0,0,0))
						count++;
				}
			
			if (count <= 6) keypoints_2.push_back(kp2[i]);
		}
	}

	//cout << "begin stitcher5\n";
	//-- Draw keypoints
	Mat img_keypoints_1; 
	Mat img_keypoints_2;
	drawKeypoints( img_1, keypoints_1, img_keypoints_1, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
	drawKeypoints( img_2, keypoints_2, img_keypoints_2, Scalar::all(-1), DrawMatchesFlags::DEFAULT );

	//-- Show detected (drawn) keypoints
	//imshow("Keypoints 1", img_keypoints_1 );
	//imshow("Keypoints 2", img_keypoints_2 );
  	
  	//描述子
  	Mat descriptors_1, descriptors_2;
  	detector->compute(img_1, keypoints_1, descriptors_1);
	detector->compute(img_2, keypoints_2, descriptors_2);

	//match
	FlannBasedMatcher matcher;
  	vector< DMatch > matches;
  	matcher.match(descriptors_1, descriptors_2, matches);
  	Mat matche_result;
  	drawMatches(img_1, keypoints_1,  img_2, keypoints_2, matches, matche_result);
	//imshow("##", matche_result);

  	cout << "begin stitcher6\n";
	//设定阈值，提取更好的匹配点
	float min_dis = 10000, max_dis = 0;
	for (int i  = 0; i < matches.size(); i ++) {
		if (matches[i].distance < min_dis) min_dis = matches[i].distance;
		if (matches[i].distance > max_dis) max_dis = matches[i].distance;
	}

	float threshold =  3 * min_dis;
	//float threshold =  max_dis;
	cout << "min_dis = " << min_dis << "   max_dis = " << max_dis << endl;
	std::vector<DMatch> betterMatcher;
	for (int i = 0; i < matches.size(); i ++) {
		if (matches[i].distance < threshold) {
			betterMatcher.push_back(matches[i]);
		}
	}
	
	drawMatches(img_1, keypoints_1,  img_2, keypoints_2, betterMatcher, matche_result);
	imshow("betterMatcher", matche_result);

	cout << "begin stitcher7\n";
	vector<Point2f> KP1, KP2;
	for (int i = 0; i < betterMatcher.size(); i ++) {
		KP1.push_back(keypoints_1[betterMatcher[i].queryIdx].pt);
		KP2.push_back(keypoints_2[betterMatcher[i].trainIdx].pt);
	}
	Mat H = findHomography( KP1, KP2, CV_RANSAC);

	cout << "H : " << H.rows << " " << H.cols << " " << H.channels() << endl;
	for (int i = 0; i < H.rows; i ++)
		for (int j = 0; j < H.cols; j ++) {
			cout << int(H.ptr<uchar>(i)[j]) << " ";
		}
	cout << endl;

	 //用得到的H矩阵  来进行透视矩阵变换  用到的是perspectiveTransform函数  
    	vector<Point2f> g_vCorners1(4);  
    	vector<Point2f> g_vCorners2(4);  
    	g_vCorners1[0] = Point2f(0, 0);  
    	g_vCorners1[1] = Point2f((float)img_1.cols, 0);  
    	g_vCorners1[2] = Point2f((float)img_1.cols, (float)img_1.rows);  
    	g_vCorners1[3] = Point2f(0, (float)img_1.rows);  
  	
  	cout << "begin stitcher8\n";
    	perspectiveTransform(g_vCorners1, g_vCorners2, H);
    	cout << "img1: " << img_1.rows << "   " << img_1.cols << endl;
    	cout << "g_vCorners1: " << endl;
    	for (int i = 0; i < g_vCorners1.size(); i ++) {
    		cout << "( " << g_vCorners1[i].x << " " << g_vCorners1[i].y << ")" << endl;
    	}

    	cout << "g_vCorners2: " << endl;
    	for (int i = 0; i < g_vCorners2.size(); i ++) {
    		cout << "( " << g_vCorners2[i].x << " " << g_vCorners2[i].y << ")" << endl;
    	}
  	

  	line( matche_result, g_vCorners2[0] + Point2f( img_1.cols, 0), g_vCorners2[1] + Point2f( img_1.cols, 0), Scalar(0, 255, 0), 4 );
  	line( matche_result, g_vCorners2[1] + Point2f( img_1.cols, 0), g_vCorners2[2] + Point2f( img_1.cols, 0), Scalar( 0, 255, 0), 4 );
  	line( matche_result, g_vCorners2[2] + Point2f( img_1.cols, 0), g_vCorners2[3] + Point2f( img_1.cols, 0), Scalar( 0, 255, 0), 4 );
  	line( matche_result, g_vCorners2[3] + Point2f( img_1.cols, 0), g_vCorners2[0] + Point2f( img_1.cols, 0), Scalar( 0, 255, 0), 4 );

  	imshow("betterMatcher", matche_result);

  	//blend
  	float minCols = g_vCorners2[0].x > g_vCorners2[3].x ? g_vCorners2[3].x : g_vCorners2[0].x;
  	int minX = int(minCols) + img_1.cols;
  	Mat stitch_img = Mat::zeros(img_2.rows, img_1.cols + img_2.cols - minX, CV_8UC3);
  	//Mat stitch_img = Mat(img_2.rows, img_1.cols + img_2.cols - minX, CV_8UC3);
  	float alpha = 0.5;
  	for(int i = 0; i < stitch_img.rows; i ++) {
  		for (int j = 0; j < stitch_img.cols + minX; j ++) {
  			if (j < img_1.cols) {
  				vector<Point2f> v1, v2;
    				v1.push_back(Point2f(j, i));
    				perspectiveTransform(v1, v2, H);
    				int a[2], b[2];
	    			a[0] = int(floor(v2[0].y));
	    			a[1] = int(ceil(v2[0].y));
	    			b[0] = int(floor(v2[0].x + img_1.cols));
	    			b[1] = int(ceil(v2[0].x + img_1.cols));
	    			for (int p = 0; p < 2; p ++)
	    				for (int q = 0; q < 2; q ++) {
	    					if (a[p] >= 0 && b[q] - minX >= 0 && a[p] < stitch_img.rows && b[q] - minX < stitch_img.cols && input1.at<Vec3b>(i, j) != Vec3b(0, 0, 0))
	    						stitch_img.at<Vec3b>(a[p] , b[q] - minX) = input1.at<Vec3b>(i, j);
	    				}
  			}
  			else {
  				if (input2.at<Vec3b>(i, j - img_1.cols) != Vec3b(0, 0, 0))
  					if (stitch_img.at<Vec3b>(i , j - minX) == Vec3b(0, 0, 0))
  						stitch_img.at<Vec3b>(i , j - minX) = input2.at<Vec3b>(i, j - img_1.cols);
  					else {
  						for (int k = 0; k < stitch_img.channels(); k ++) {
  							int taget1 = stitch_img.at<Vec3b>(i , j - minX)[k];
  							int taget2 = input2.at<Vec3b>(i, j - img_1.cols)[k];
	  						stitch_img.at<Vec3b>(i , j - minX)[k] = taget1 > taget2 ? taget2 : taget1;
	  					}
  					}
  			}
  		}
  	}

  	
  	imshow("result1", stitch_img);
  	waitKey(0);
  	return stitch_img;
}