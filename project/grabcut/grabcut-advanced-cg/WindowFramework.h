#pragma once

#include <iostream>
#include "cvui.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "GrabCut.h"

using namespace std;
using namespace cv;

#define WINDOW_NAME "Implementation of Grabcut"

enum drawState {
    READY,
    DRAWING,
};

class WindowFramework {
public:
    void initWindow();
    void loadImage();
    void initState();
    void display();
    void listenMouse();
    void doGrabCut();
    void listenButton();
    bool shouldExit();
    bool initGrabCut();
    GrabCutImplementation grabCutImplementation;
private:
    static const Scalar PEN_BGD_COLOR;
    static const Scalar PEN_FGD_COLOR;
    static const Scalar PEN_RECT;

    static const int DRAW_BGD;
    static const int DRAW_FGD;
    static const int DRAW_RECT;

    Mat frame;
    Mat image;
    Mat mask;

    Rect choosingBox;
    vector<Point> foregroundPixels, backgroundPixels;
    int drawingFlag;
    bool exitFlag;
    drawState drawingState;
    bool beforeGraphCut;
    bool hasMask;
};