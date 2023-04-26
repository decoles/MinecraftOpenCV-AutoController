#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <Windows.h>
#include <opencv2/dnn/all_layers.hpp>
#include <string>
#include <time.h>
#include <chrono>

using namespace cv::dnn; //OpenCV ML
using namespace std::chrono; //Timing
using namespace std;
using namespace cv;

Mat returnImage();
vector<Mat> pre_process(Mat& input_image, Net& net, int height, int width)
{
    // Convert to blob.
    Mat blob;
    blobFromImage(input_image, blob, 1.0 / 255., Size(height, width), Scalar(), true, false);

    net.setInput(blob);

    // Forward propagate.
    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    return outputs;
}



int main()
{
    int counter = 0;
    auto start_time = high_resolution_clock::now();   
    Mat dst, cdst, cdstP;
    int fpsCounter = 0;
    time_t start = time(0);

    vector<string> class_list;
    ifstream ifs("coco.names");
    string line;

    Net net;
    net = readNet("yolov5s.onnx");

    while (getline(ifs, line))
    {
        class_list.push_back(line);
    }

    while (1)
    {
        //keybd_event(0x57, 0, 0, 0);
        Mat frame = returnImage();

        // Wait indefinitely for a key press
        int key = cv::waitKey(1);
        if (key == 27) // break if escape key is pressed
            break;



        vector<Mat> detections;
        detections = pre_process(frame, net, frame.size[0], frame.size[1]);













       // Canny(frame, dst, 50, 200, 3);
        //cvtColor(dst, cdstP, COLOR_GRAY2BGR);

        //vector<Vec4i> linesP; // will hold the results of the detection
        //HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 130, 2); // runs the actual detection
        // Draw the lines
        //for (size_t i = 0; i < linesP.size(); i++)
        //{
         //   Vec4i l = linesP[i];
        //    line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
        //}

        //imshow("Game Window", cdstP); // Display the captured image
        //putText(frame, "test", Point(10, frame.rows / 2), FONT_HERSHEY_DUPLEX, 1.0, CV_RGB(118, 185, 0), 2);

        imshow("Game Actual", frame);
        counter++;
                auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

        if (elapsed_time >= 1) {
            cout << "Loop completed " << counter << " times in " << elapsed_time << " seconds." << endl;
            counter = 0;
            start_time = current_time;
        }
    }


    // Clean up resources


    return 0;
}

//returnImage: 
Mat returnImage()
{
    HWND hwnd = FindWindow(NULL, L"Minecraft* 1.19.2 - Singleplayer");

    HDC hdc = GetDC(hwnd); // Get the device context of the window

    RECT rect;
    GetClientRect(hwnd, &rect); // Get the dimensions of the client area of the window

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcMem = CreateCompatibleDC(hdc); // Create a compatible device context
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height); // Create a compatible bitmap

    // Select the bitmap into the device context
    SelectObject(hdcMem, hBitmap);

    // Copy the contents of the window to the bitmap
    BitBlt(hdcMem, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

    // Create a new frame object to store the captured image, unsinged int 8bit four channels
    Mat frame(height, width, CV_8UC4);

    GetBitmapBits(hBitmap, width * height * 4, frame.data); // Copy the bitmap data to the frame object
    
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdc);
    return frame;
}