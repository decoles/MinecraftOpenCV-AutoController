#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <Windows.h>
#include <opencv2/dnn/all_layers.hpp>
#include <string>
#include <time.h>
//using namespace dnn;

using namespace cv::dnn;
using namespace std;
using namespace cv;


int main()
{
    Mat dst, cdst, cdstP;
    int fpsCounter = 0;
    time_t start = time(0);
    while (1)
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
        /*
        std::string test = "best.onnx";
        // Load the YOLOv5 ONNX model
        cv::dnn::Net net = cv::dnn::readNet(test);

        // Set backend and target to use OpenCV's CPU backend
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        cv::frame blob = cv::dnn::blobFromImage(frame, 1 / 255.0, cv::Size(128  , 212), cv::Scalar(),false, false);
        net.setInput(blob);
        if (blob.empty())
        {
            return -1;
        }
        try {
        cv::frame output = net.forward();
        // process output
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
        //net.setInput(blob);

       // cv::frame detection = net.forward();
       */


        // Wait indefinitely for a key press
        int key = cv::waitKey(1);
        if (key == 27) // break if escape key is pressed
            break;
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdc);


        Canny(frame, dst, 50, 200, 3);
        cvtColor(dst, cdstP, COLOR_GRAY2BGR);

        vector<Vec4i> linesP; // will hold the results of the detection
        HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 130, 2); // runs the actual detection
        // Draw the lines
        for (size_t i = 0; i < linesP.size(); i++)
        {
            Vec4i l = linesP[i];
            line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
        }

        imshow("Game Window", cdstP); // Display the captured image
        imshow("Game Actual", frame);
    }


    // Clean up resources


    return 0;
}