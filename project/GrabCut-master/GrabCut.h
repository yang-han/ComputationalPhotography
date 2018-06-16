#pragma once
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
enum
{
	GC_WITH_RECT  = 0, 
	GC_WITH_MASK  = 1, 
	GC_CUT        = 2  
};
class GMM;
template <typename captype, typename tcaptype, typename flowtype> class Graph;
class GrabCut2D
{
public:
    //输入：
    //cv::InputArray _img,     :输入的color图像(类型-cv:Mat)
    //cv::Rect rect            :在图像上画的矩形框（类型-cv:Rect)
    //int iterCount :           :每次分割的迭代次数（类型-int)


    //中间变量
    //cv::InputOutputArray _bgdModel ：   背景模型（推荐GMM)（类型-13*n（组件个数）个double类型的自定义数据结构，可以为cv:Mat，或者Vector/List/数组等）
    //cv::InputOutputArray _fgdModel :    前景模型（推荐GMM) （类型-13*n（组件个数）个double类型的自定义数据结构，可以为cv:Mat，或者Vector/List/数组等）


    //输出:
    //cv::InputOutputArray _mask  : 输出的分割结果 (类型： cv::Mat)
	void GrabCut( const cv::Mat &img, cv::Mat &mask, cv::Rect rect,
		cv::Mat &bgdModel,cv::Mat &fgdModel,
		int iterCount, int mode );  

	~GrabCut2D(void);
private:
    void initMaskWithRect(cv::Mat &mask, cv::Size size, cv::Rect rect);
    void checkMask(const cv::Mat &img, const cv::Mat &mask);
    void initGMMs(const cv::Mat &img, const cv::Mat &mask, GMM &fgdGMM, GMM &bgdGMM);
    double calcBeta(const cv::Mat &img);
    void calcNWeights( const cv::Mat& img, cv::Mat& leftW, cv::Mat& upleftW,
                       cv::Mat& upW, cv::Mat& uprightW, double beta, double gamma);
    void assignGMMsComponents( const cv::Mat& img, const cv::Mat& mask,
                          const GMM& bgdGMM, const GMM& fgdGMM, cv::Mat& compIdxs);
    void learnGMMs( const cv::Mat& img, const cv::Mat& mask, const cv::Mat& compIdxs,
                          GMM& bgdGMM, GMM& fgdGMM);
    void constructGCGraph( const cv::Mat& img, const cv::Mat& mask, const GMM& bgdGMM,
                           const GMM& fgdGMM, double lambda, const cv::Mat& leftW,
                           const cv::Mat& upleftW, const cv::Mat& upW, const cv::Mat& uprightW,
                           Graph<double, double, double>& graph );
    void estimateSegmentation( Graph<double, double, double>& graph, cv::Mat& mask );

};

