#include "feature.h"

string Name(int dataSet, int num) {
	string temp;
	if (dataSet == 1) {
		temp = "images/pano1_00" + turnIntoStr(num) + ".bmp";
	}
	else {
		temp = "images/100NIKON-DSCN00" + turnIntoStr(num) + "_DSCN00" + turnIntoStr(num) + ".JPG";
	}
	return temp;
}

int main(int argc, char** argv )
{
	//读入input.png
	cout << "which data set do you wana test ? (1 or 2 :" << endl;
	int dataSet;
	cin >> dataSet;
	cout << "please input the start number and end number of images : " << endl;
	int start, end;
	cin >> start >> end;

	/*~~~~~~~~~~~~~~~~~~从两端开始拼接~~~~~~~~~~~~~~~~~~*/
	string name1, name2;
	name1 = Name(dataSet, start);
	name2 = Name(dataSet, end);
	Mat image1 = imread(name1.data(), -1);
	Mat image2 = imread(name2.data(), -1);
	if (dataSet != 1) {
		image1 = pre_processing(image1);
		image2 = pre_processing(image2);
	}
	image1 = Cylinder_projection(image1);
	image2 = Cylinder_projection(image2);
	Mat result;
	cout << name1 << endl;
	cout << name2 << endl;

	while (start != end - 1 && start != end) {
		Mat temp1;
		start ++;
		name1 = Name(dataSet, start);
		cout << name1 << endl;
		temp1 = imread(name1.data(), -1);
		if (dataSet != 1) {
			temp1 = pre_processing(temp1);
		}
		temp1 = Cylinder_projection(temp1);
		image1 = (stitch(image1, temp1)).clone();

		if (start == end - 1 || start == end)break;
		else {
			Mat temp2;
			end --;
			name2 = Name(dataSet, end);
			cout << name2 << endl;
			temp2 = imread(name2.data(), -1);
			if (dataSet != 1) {
				temp2 = pre_processing(temp2);
			}
			temp2 = Cylinder_projection(temp2);
			image2 = (stitch(temp2, image2)).clone();
		}
	}
	result = stitch(image1, image2);
	
	/*~~~~~~~~~~~~~~~从左到右，单张拼接~~~~~~~~~~~~~~~~`*/
	/*
	Mat image1, image2;
	string name;
	Mat result;
	if (dataSet == 1)
		result = imread("images/pano1_00" + turnIntoStr(start) + ".bmp", -1);
	else {
		result = imread("images/100NIKON-DSCN00" + turnIntoStr(start) + "_DSCN00" + turnIntoStr(start) + ".JPG", -1);
		result = pre_processing(result);
	}

	result = Cylinder_projection(result);
	
	for (int i = start + 1; i <= end; i ++) {
		image1 = result.clone();
		if (dataSet == 1) {
			name = "images/pano1_00" + turnIntoStr(i) + ".bmp";
			image2 = imread(name.data(), -1);
		}
		else {
			name = "images/100NIKON-DSCN00" + turnIntoStr(i) + "_DSCN00" + turnIntoStr(i) + ".JPG";
			image2 = imread(name.data(), -1);
			image2 = pre_processing(image2);
		}
		if ( !image1.data || !image2.data) {
			printf("No image data \n");
			return -1;
		}
		image2 = Cylinder_projection(image2);
		if (dataSet == 1)
			result = (stitch(image1, image2)).clone();
		else
			result = (stitch(image1, image2)).clone();
	}
	*/
	if (dataSet == 1)
		imwrite("images/result1.bmp", result);
	else 
		imwrite("images/result2.JPG", result);

    	waitKey(0);
  	return 0;
}