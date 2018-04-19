#include <iostream>
#include <opencv2\opencv.hpp>

int main(int argc, char *argv[])
 {
    cv::Mat frame;
    cv::VideoCapture camera(0);

    while(1)
    {
        camera >> frame; 
        cv::imshow("camera", frame);

        cv::waitKey(30);
    }

    return 0;
}