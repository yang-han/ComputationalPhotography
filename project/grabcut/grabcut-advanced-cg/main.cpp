#include "WindowFramework.h"
#define CVUI_IMPLEMENTATION
#include "cvui.h"

WindowFramework windowFramework;

int main(int argc, char **argv) {
    windowFramework.initWindow();
    windowFramework.loadImage();
    windowFramework.initState();
    for (;;) {
        windowFramework.listenButton();
        windowFramework.listenMouse();
        windowFramework.display();
        if (cv::waitKey(20) == 27 || windowFramework.shouldExit()) {
            break;
        }
    }
    return 0;
}