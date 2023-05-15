#ifndef MODELDETECTIONS_H
#define MODELDETECTIONS_H

#include <fstream>
#include <opencv2/opencv.hpp>
#include <vector>

struct Detection
{
    int class_id;
    float confidence;
    cv::Rect box;
};

std::vector<std::string> load_class_list();

void load_net(cv::dnn::Net& net, bool is_cuda);

cv::Mat format_yolov5(const cv::Mat& source);

void detect(cv::Mat& image, cv::dnn::Net& net, std::vector<Detection>& output, const std::vector<std::string>& className, int dimension);
#endif // !MODELDETECTIONS_H