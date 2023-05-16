#ifndef MODELDETECTIONS_H
#define MODELDETECTIONS_H

#include <fstream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

struct Detection
{
    int class_id;
    float confidence;
    cv::Rect box;
};

vector<string> load_class_list(string val);

void load_net(dnn::Net& net, bool is_cuda, string modelName);

Mat format_yolov5(const Mat& source);

void detect(Mat& image, dnn::Net& net, vector<Detection>& output, const vector<string>& className, int dimension);
#endif // !MODELDETECTIONS_H