#include <chrono>
#include <opencv2/imgproc/imgproc.hpp>
#include "GrabCut.h"
#include "GMM.h"
#include "graph.h"

GrabCut2D::~GrabCut2D(void)
{
}

//1.Load Input Image: 加载输入颜色图像;
//2.Init Mask: 用矩形框初始化Mask的Label值（确定背景：0， 确定前景：1，可能背景：2，可能前景：3）,矩形框以外设置为确定背景，矩形框以内设置为可能前景;
//3.Init GMM: 定义并初始化GMM(其他模型完成分割也可得到基本分数，GMM完成会加分）
//4.Sample Points:前背景颜色采样并进行聚类（建议用kmeans，其他聚类方法也可)
//5.Learn GMM(根据聚类的样本更新每个GMM组件中的均值、协方差等参数）
//4.Construct Graph（计算t-weight(数据项）和n-weight（平滑项））
//7.Estimate Segmentation(调用maxFlow库进行分割)
//8.Save Result输入结果（将结果mask输出，将mask中前景区域对应的彩色图像保存和显示在交互界面中）
void GrabCut2D::GrabCut(const cv::Mat &img, cv::Mat &mask, cv::Rect rect,
                        cv::Mat &bgdModel, cv::Mat &fgdModel,
                        int iterCount, int mode)
{
    std::cout << "Execute GrabCut Function" << std::endl;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();


    if (img.empty())
        CV_Error(CV_StsBadArg, "image is empty");
    if (img.type() != CV_8UC3)
        CV_Error(CV_StsBadArg, "image mush have CV_8UC3 type");

    GMM bgdGMM, fgdGMM;
    cv::Mat compIdxs(img.size(), CV_32SC1);

    if (mode == GC_WITH_RECT || mode == GC_WITH_MASK) {
        if (mode == GC_WITH_RECT)
            initMaskWithRect(mask, img.size(), rect);
        else // flag == GC_WITH_MASK
            checkMask(img, mask);
        initGMMs(img, mask, bgdGMM, fgdGMM);
    }
    else //mode == GC_CUT
    {
        checkMask(img, mask);
        bgdGMM = GMM::fromModel(bgdModel);
        fgdGMM = GMM::fromModel(fgdModel);
    }

    if (iterCount <= 0)
        return;

    const double gamma = 50;
    const double lambda = 9 * gamma;
    const double beta = calcBeta(img);

    cv::Mat leftW, upleftW, upW, uprightW;
    calcNWeights(img, leftW, upleftW, upW, uprightW, beta, gamma);
    int vtxCount = img.cols * img.rows;
    int edgeCount = 2 * (4 * img.cols * img.rows - 3 * (img.cols + img.rows) + 2);

    for (int i = 0; i < iterCount; i++) {
        Graph<double, double, double> graph(vtxCount, edgeCount);
        assignGMMsComponents(img, mask, bgdGMM, fgdGMM, compIdxs);
        learnGMMs(img, mask, compIdxs, bgdGMM, fgdGMM);
        constructGCGraph(img, mask, bgdGMM, fgdGMM, lambda, leftW, upleftW, upW, uprightW, graph);
        estimateSegmentation(graph, mask);
    }
    fgdModel = fgdGMM.exportModel();
    bgdModel = bgdGMM.exportModel();

    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
}

void GrabCut2D::initMaskWithRect(cv::Mat &mask, cv::Size size, cv::Rect rect)
{
    mask.create(size, CV_8UC1);
    mask.setTo(cv::GC_BGD);

    rect.x = std::max(0, rect.x);
    rect.y = std::max(0, rect.y);
    rect.width = std::min(rect.width, size.width - rect.x);
    rect.height = std::min(rect.height, size.height - rect.y);

    (mask(rect)).setTo(cv::Scalar(cv::GC_PR_FGD));
}

void GrabCut2D::checkMask(const cv::Mat &img, const cv::Mat &mask)
{
    if (mask.empty())
        CV_Error(CV_StsBadArg, "mask is empty");
    if (mask.type() != CV_8UC1)
        CV_Error(CV_StsBadArg, "mask must have CV_8UC1 type");
    if (mask.cols != img.cols || mask.rows != img.rows)
        CV_Error(CV_StsBadArg, "mask must have as many rows and cols as img");
    for (int y = 0; y < mask.rows; y++) {
        for (int x = 0; x < mask.cols; x++) {
            uchar val = mask.at<uchar>(y, x);
            if (val != cv::GC_BGD && val != cv::GC_FGD && val != cv::GC_PR_BGD && val != cv::GC_PR_FGD)
                CV_Error(CV_StsBadArg, "mask element value must be equel"
                    "GC_BGD or GC_FGD or GC_PR_BGD or GC_PR_FGD");
        }
    }
}

void GrabCut2D::initGMMs(const cv::Mat &img, const cv::Mat &mask, GMM &fgdGMM, GMM &bgdGMM)
{
    std::vector<cv::Vec3f> bgdSamples;
    std::vector<cv::Vec3f> fgdSamples;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            if (mask.at<uchar>(i, j) == cv::GC_BGD || mask.at<uchar>(i, j) == cv::GC_PR_BGD)
                bgdSamples.push_back((cv::Vec3f) img.at<cv::Vec3b>(i, j));
            else
                fgdSamples.push_back((cv::Vec3f) img.at<cv::Vec3b>(i, j));
        }
    }
    CV_Assert(!bgdSamples.empty() && !fgdSamples.empty());
    cv::Mat bgdSampleMat((int) bgdSamples.size(), 3, CV_32FC1, &bgdSamples[0][0]);
    cv::Mat fgdSampleMat((int) fgdSamples.size(), 3, CV_32FC1, &fgdSamples[0][0]);
    cv::Mat bgdLabels;
    cv::Mat fgdLabels;
    cv::kmeans(bgdSampleMat, bgdGMM.getComponentsCount(), bgdLabels,
               cv::TermCriteria(cv::TermCriteria::MAX_ITER, 10, 0.0), 0, cv::KMEANS_PP_CENTERS);
    cv::kmeans(fgdSampleMat, fgdGMM.getComponentsCount(), fgdLabels,
               cv::TermCriteria(cv::TermCriteria::MAX_ITER, 10, 0.0), 0, cv::KMEANS_PP_CENTERS);

    bgdGMM.initLearning();
    for (int i = 0; i < bgdSamples.size(); i++) {
        bgdGMM.addSample(bgdLabels.at<int>(i, 0), bgdSamples[i]);
    }
    bgdGMM.endLearning();

    fgdGMM.initLearning();
    for (int i = 0; i < fgdSamples.size(); i++) {
        fgdGMM.addSample(fgdLabels.at<int>(i, 0), fgdSamples[i]);
    }
    fgdGMM.endLearning();
}

double GrabCut2D::calcBeta(const cv::Mat &img)
{
    double beta = 0;
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            cv::Vec3d color = img.at<cv::Vec3b>(y, x);
            if (x > 0) // left
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y, x - 1);
                beta += diff.dot(diff);
            }
            if (y > 0 && x > 0) // upleft
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y - 1, x - 1);
                beta += diff.dot(diff);
            }
            if (y > 0) // up
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y - 1, x);
                beta += diff.dot(diff);
            }
            if (y > 0 && x < img.cols - 1) // upright
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y - 1, x + 1);
                beta += diff.dot(diff);
            }
        }
    }
    if (beta <= std::numeric_limits<double>::epsilon())
        beta = 0;
    else
        beta = 1.f / (2 * beta / (4 * img.cols * img.rows - 3 * img.cols - 3 * img.rows + 2));

    return beta;
}

void GrabCut2D::calcNWeights(const cv::Mat &img, cv::Mat &leftW, cv::Mat &upleftW,
                             cv::Mat &upW, cv::Mat &uprightW, double beta, double gamma)
{
    leftW.create(img.rows, img.cols, CV_64FC1);
    upleftW.create(img.rows, img.cols, CV_64FC1);
    upW.create(img.rows, img.cols, CV_64FC1);
    uprightW.create(img.rows, img.cols, CV_64FC1);
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            cv::Vec3d color = img.at<cv::Vec3b>(y, x);
            if (x > 0) // left
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y, x - 1);
                leftW.at<double>(y, x) = gamma * exp(-beta * diff.dot(diff));
            }
            else
                leftW.at<double>(y, x) = 0;
            if (x > 0 && y > 0) // upleft
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y - 1, x - 1);
                upleftW.at<double>(y, x) = gamma / sqrt(2.0) * exp(-beta * diff.dot(diff));
            }
            else
                upleftW.at<double>(y, x) = 0;
            if (y > 0) // up
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y - 1, x);
                upW.at<double>(y, x) = gamma * exp(-beta * diff.dot(diff));
            }
            else
                upW.at<double>(y, x) = 0;
            if (x < img.cols - 1 && y > 0) // upright
            {
                cv::Vec3d diff = color - (cv::Vec3d) img.at<cv::Vec3b>(y - 1, x + 1);
                uprightW.at<double>(y, x) = gamma / sqrt(2.0) * exp(-beta * diff.dot(diff));
            }
            else
                uprightW.at<double>(y, x) = 0;
        }
    }

}

void GrabCut2D::assignGMMsComponents(const cv::Mat &img, const cv::Mat &mask,
                                     const GMM &bgdGMM, const GMM &fgdGMM, cv::Mat &compIdxs)
{
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            cv::Vec3d color = img.at<cv::Vec3b>(y, x);
            compIdxs.at<int>(y, x) = mask.at<uchar>(y, x) == cv::GC_BGD
                                         || mask.at<uchar>(y, x) == cv::GC_PR_BGD ?
                                     bgdGMM.whichComponent(color) : fgdGMM.whichComponent(color);
        }
    }
}

void GrabCut2D::learnGMMs(const cv::Mat &img, const cv::Mat &mask, const cv::Mat &compIdxs,
                          GMM &bgdGMM, GMM &fgdGMM)
{
    bgdGMM.initLearning();
    fgdGMM.initLearning();
    for (int ci = 0; ci < bgdGMM.getComponentsCount(); ci++) {
        for (int y = 0; y < img.rows; y++) {
            for (int x = 0; x < img.cols; x++) {
                if (compIdxs.at<int>(y, x) == ci) {
                    if (mask.at<uchar>(y, x) == cv::GC_BGD || mask.at<uchar>(y, x) == cv::GC_PR_BGD)
                        bgdGMM.addSample(ci, img.at<cv::Vec3b>(y, x));
                    else
                        fgdGMM.addSample(ci, img.at<cv::Vec3b>(y, x));
                }
            }
        }
    }
    bgdGMM.endLearning();
    fgdGMM.endLearning();
}

void GrabCut2D::constructGCGraph(const cv::Mat &img, const cv::Mat &mask, const GMM &bgdGMM,
                                 const GMM &fgdGMM, double lambda, const cv::Mat &leftW,
                                 const cv::Mat &upleftW, const cv::Mat &upW, const cv::Mat &uprightW,
                                 Graph<double, double, double> &graph)
{
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            // add node
            int vtxIdx = graph.add_node();
            cv::Vec3b color = img.at<cv::Vec3b>(y, x);

            // set t-weights
            double fromSource, toSink;
            if (mask.at<uchar>(y, x) == cv::GC_PR_BGD || mask.at<uchar>(y, x) == cv::GC_PR_FGD) {
                fromSource = -log(bgdGMM(color));
                toSink = -log(fgdGMM(color));
            }
            else if (mask.at<uchar>(y, x) == cv::GC_BGD) {
                fromSource = 0;
                toSink = lambda;
            }
            else // GC_FGD
            {
                fromSource = lambda;
                toSink = 0;
            }
            graph.add_tweights(vtxIdx, fromSource, toSink);

            // set n-weights
            if (x > 0) {
                double w = leftW.at<double>(y, x);
                graph.add_edge(vtxIdx, vtxIdx - 1, w, w);
            }
            if (x > 0 && y > 0) {
                double w = upleftW.at<double>(y, x);
                graph.add_edge(vtxIdx, vtxIdx - img.cols - 1, w, w);
            }
            if (y > 0) {
                double w = upW.at<double>(y, x);
                graph.add_edge(vtxIdx, vtxIdx - img.cols, w, w);
            }
            if (x < img.cols - 1 && y > 0) {
                double w = uprightW.at<double>(y, x);
                graph.add_edge(vtxIdx, vtxIdx - img.cols + 1, w, w);
            }
        }
    }
}

void GrabCut2D::estimateSegmentation(Graph<double, double, double> &graph, cv::Mat &mask)
{
    graph.maxflow();
    for (int y = 0; y < mask.rows; y++) {
        for (int x = 0; x < mask.cols; x++) {
            if (mask.at<uchar>(y, x) == cv::GC_PR_BGD || mask.at<uchar>(y, x) == cv::GC_PR_FGD) {
                if (graph.what_segment(y * mask.cols + x /*vertex index*/)
                    == Graph<double, double, double>::SOURCE)
                    mask.at<uchar>(y, x) = cv::GC_PR_FGD;
                else
                    mask.at<uchar>(y, x) = cv::GC_PR_BGD;
            }
        }
    }
}

