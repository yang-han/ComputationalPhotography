#include "WindowFramework.h"

void WindowFramework::initWindow() {
    cvui::init(WINDOW_NAME);
    frame = cv::Mat(1000, 1000, CV_8UC3);
    frame = cv::Scalar(49, 52, 49);
}

void WindowFramework::loadImage() {
	string filename = "C:\\yh\\download.jpg";
    image = imread(filename, 1);
    mask.create(image.size(), CV_8UC1);
}

void WindowFramework::initState() {
    backgroundPixels.clear();
    foregroundPixels.clear();

    beforeGraphCut = true;
    drawingFlag = DRAW_RECT;
    drawingState = READY;
    hasMask = false;

    mask.setTo(Scalar::all(0));
}

void WindowFramework::listenMouse() {
    cv::Point cursor = cvui::mouse();
    int x = cursor.x - 200, y = cursor.y;
    static int startx = 0, starty = 0;
    if (x > 0)
        if (cvui::mouse(cvui::LEFT_BUTTON, cvui::DOWN)) {
            if (drawingState == READY) {
                drawingState = DRAWING;
                startx = x, starty = y;
            }
        } else if (cvui::mouse(cvui::LEFT_BUTTON, cvui::UP)) {
            if (drawingState == DRAWING) {
                if (drawingFlag == DRAW_RECT) {
                    mask.setTo(GC_BGD);
                    (mask(choosingBox)).setTo(Scalar(GC_PR_FGD));
                    drawingState = READY;
                    choosingBox = Rect(Point(startx, starty), Point(x, y));
                } else {
                    hasMask = true;
                }
            }
        } else if (cvui::mouse(cvui::IS_DOWN)) {
            if (drawingState == DRAWING) {
                if (drawingFlag == DRAW_RECT)
                    choosingBox = Rect(Point(startx, starty), Point(x, y));
                else if (drawingFlag == DRAW_BGD) {
                    backgroundPixels.emplace_back(Point(x, y));
                    circle(mask, Point(x, y), 2, GC_BGD);
                } else if (drawingFlag == DRAW_FGD) {
                    foregroundPixels.emplace_back(Point(x, y));
                    circle(mask, Point(x, y), 2, GC_FGD);
                }
            }
        }
}

void WindowFramework::display() {
    Mat imageDisplay;
    if (image.empty()) {
        return;
    }
    if (beforeGraphCut)
        image.copyTo(imageDisplay);
    else
        image.copyTo(imageDisplay, mask & 1);

    for (int i = 0; i < backgroundPixels.size(); ++i)
        circle(imageDisplay, backgroundPixels[i], 2, PEN_BGD_COLOR);
    for (int i = 0; i < foregroundPixels.size(); ++i)
        circle(imageDisplay, foregroundPixels[i], 2, PEN_FGD_COLOR);
    if (!choosingBox.empty())
        rectangle(imageDisplay, Point(choosingBox.x, choosingBox.y),
                  Point(choosingBox.x + choosingBox.width, choosingBox.y + choosingBox.height), PEN_RECT, 1);

    cvui::image(frame, 200, 0, imageDisplay);

    cvui::update();
    cvui::imshow(WINDOW_NAME, frame);

}

const Scalar WindowFramework::PEN_RECT = Scalar(255, 255, 255);
const Scalar WindowFramework::PEN_FGD_COLOR = Scalar(255, 0, 0);
const Scalar WindowFramework::PEN_BGD_COLOR = Scalar(0, 0, 255);

const int WindowFramework::DRAW_RECT = 0;
const int WindowFramework::DRAW_BGD = 1;
const int WindowFramework::DRAW_FGD = 2;

void WindowFramework::listenButton() {
    if (cvui::button(frame, 0, 0, "background")) {
        printf("press background");
        drawingFlag = DRAW_BGD;
    }
    if (cvui::button(frame, 0, 30, "foreground")) {
        printf("press foreground");
        drawingFlag = DRAW_FGD;
    }
    if (cvui::button(frame, 0, 60, "reset")) {
        printf("press initState");
        initState();
    }
    if (cvui::button(frame, 0, 90, "do grabcut")) {
        printf("press do grabcut button\n");
        if (initGrabCut()) doGrabCut();
        else printf("Init failed.");
    }
    if (cvui::button(frame, 0, 120, "border matting")) {
        printf("press border matting");
        grabCutImplementation.doBorderMatting();
    }
    if (cvui::button(frame, 0, 150, "exit")) {
        exitFlag = true;
    }
}

bool WindowFramework::initGrabCut() {
    if (choosingBox.empty()) return false;
    printf("Init Grabcut\n");
    if (hasMask) {
        grabCutImplementation.grabCut(image, mask, 1);
    } else {
        grabCutImplementation.calMaskByBox(mask, choosingBox);
        grabCutImplementation.grabCut(image, mask, 1);
    }
    return true;
}

void WindowFramework::doGrabCut() {
    printf("do grabcut\n");
    grabCutImplementation.grabCut(image, mask, 1);
    printf("grabcut done\n");
    beforeGraphCut = false;
    backgroundPixels.clear();
    foregroundPixels.clear();
}

bool WindowFramework::shouldExit() {
    return exitFlag;
}
