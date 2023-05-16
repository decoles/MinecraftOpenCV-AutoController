#include "ModelDetections.h"

const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;
const float NMS_THRESHOLD = 0.4;
const float CONFIDENCE_THRESHOLD = 0.4;
const int ROWS = 25200;

//Reads class list line by line and loads it into vector
vector<string> load_class_list(string val)
{
    vector<string> class_list;
    ifstream ifs(val);
    string line;
    while (getline(ifs, line))
    {
        class_list.push_back(line);
    }
    return class_list;
}

//Loads the model and determines if CUDA will be used or not
void load_net(dnn::Net& net, bool is_cuda, string modelName)
{
    auto result = dnn::readNet(modelName);
    if (is_cuda) //In case I want to use cpu later
    {
        result.setPreferableBackend(dnn::DNN_BACKEND_CUDA);
        result.setPreferableTarget(dnn::DNN_TARGET_CUDA);
    }
    net = result;
}

//Format images for processing 640x640
Mat format_yolov5(const Mat& source) {
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    Mat result = Mat::zeros(_max, _max, CV_8UC3);
    source.copyTo(result(Rect(0, 0, col, row)));
    return result;
}

void detect(Mat& image, dnn::Net& net, vector<Detection>& output, const vector<string>& className, int dimension) {
    Mat blob;

    auto input_image = format_yolov5(image); //formatted 640x640 image

    dnn::blobFromImage(input_image, blob, 1. / 255., Size(INPUT_WIDTH, INPUT_HEIGHT), Scalar(), true, false);
    net.setInput(blob);
    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    float x_factor = input_image.cols / INPUT_WIDTH;
    float y_factor = input_image.rows / INPUT_HEIGHT;

    float* data = (float*)outputs[0].data; //contains an array of all data such as x, y coordiantes as well as confidcence and sizes

    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;

    for (int i = 0; i < ROWS; ++i) { //Loop through all detections

        float confidence = data[4];

        if (confidence >= CONFIDENCE_THRESHOLD) {

            float* classes_scores = data + 5;
            Mat scores(1, className.size(), CV_32FC1, classes_scores);
            Point class_id;
            double max_class_score;
            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
            if (max_class_score > SCORE_THRESHOLD) { //find highest value in scene
                confidences.push_back(confidence);
                class_ids.push_back(class_id.x);
                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];
                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
        data += dimension; //MUST BE OUTPUT SIZE CAN BE FOUND WITH ONLINE MODEL VIEWER
    }

    vector<int> nms_result;
    dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result); //Generates all the boxes and where they go
    for (int i = 0; i < nms_result.size(); i++) {
        int idx = nms_result[i];
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        output.push_back(result);
    }
}