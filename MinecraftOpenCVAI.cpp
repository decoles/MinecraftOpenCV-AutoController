#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/dnn.hpp>
#include <Windows.h>
#include <opencv2/dnn/all_layers.hpp>
#include <ModelDetections.h>
#include <string>
#include <time.h>
#include <chrono>
#include "wtypes.h"

using namespace cv::dnn; //OpenCV ML
using namespace std::chrono; //Timing
using namespace std;
using namespace cv;

const float THRESHOLD = 0.85; //For match templage
int frameHeight;
int frameWidth;


Mat returnImage(bool& val);
Mat returnMatchTemplate(Mat img, Mat templ);
void detectBeings(Mat& frame, Net &net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
void MouseMove(int x, int y);

int main()
{
    bool gameWindowFocus = false;
    int counter = 0;
    auto start_time = high_resolution_clock::now();
    Mat dst, cdst, cdstP;
    int fpsCounter = 0;
    time_t start = time(0);

    string line;
    Mat frame;

    std::vector<std::string> class_list = load_class_list();
    const std::vector<cv::Scalar> colors = { cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0) };

    bool is_cuda = true;
    cv::dnn::Net net;
    load_net(net, is_cuda);

    frame = returnImage(gameWindowFocus);

    Mat templ = imread("MatchTemplate/food.png");
    Mat img;
    while (1)
    {
        //keybd_event(0x57, 0, 0, 0);
        frame = returnImage(gameWindowFocus);
       // cout << "IS WINDOW IN FOCUS " << gameWindowFocus << endl;
        //frame = img.clone();
        //img = frame.clone();
        
        // Wait indefinitely for a key press
        int key = cv::waitKey(1);
        if (key == 27) // break if escape key is pressed
            break;
        //gameWindowFocus = true;

        if (gameWindowFocus) { //Main window foucs code
            cvtColor(frame, frame, COLOR_BGRA2BGR); //Go from 8UC4 to 8UC3

            detectBeings(frame, net, colors, class_list);

           
            frame = returnMatchTemplate(frame.clone(), templ); //Find hunger
            
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
             //putText(
        }
        cv::rectangle(frame, Point(0, 0), Point(frameWidth * 0.2, frameHeight * 0.3), Scalar(0, 0, 0), 3);
        //cv::rectangle(frame, Point(240, 469), Point(605, 515), Scalar(0, 0, 0), 3); //For hotbar
        cv::putText(frame, "test", Point(0,0), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
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
Mat returnImage(bool &val)
{
    HWND hwnd = FindWindow(NULL, L"Minecraft* 1.19.2 - Singleplayer");

    HWND temp = GetForegroundWindow();
    int len = GetWindowTextLength(temp) + 1;
    LPTSTR title = new TCHAR[len];
    GetWindowText(temp, title, len);
    if (hwnd == temp)//Window Focus
    {
        //cout << "same window" << endl;
        val = true; //Sets true if window is in focus so controls dont interact with the system.
    }
    else
        val = false;

    HDC hdc = GetDC(hwnd); // Get the device context of the window

    RECT rect;
    GetClientRect(hwnd, &rect); // Get the dimensions of the client area of the window

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    frameHeight = height;
    frameWidth = width; // global variables

    //cout << "width: " << width << " " << "height: " << height << endl;
    
    HDC hdcMem = CreateCompatibleDC(hdc); // Create a compatible device context
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height); // Create a compatible bitmap

    // Select the bitmap into the device context
    SelectObject(hdcMem, hBitmap);

    // Copy the contents of the window to the bitmap
    BitBlt(hdcMem, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

    // Create a new frame object to store the captured image, unsinged int 8bit four channels
    Mat frame(height, width, CV_8UC4);

    GetBitmapBits(hBitmap, width * height * 4, frame.data); // Copy the bitmap data to the frame object

    //cleanup
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdc);
    return frame;
}

//Import frame and a template image to match to 
Mat returnMatchTemplate(Mat img, Mat templ)
{
    Mat3b img2 = img.clone();
    Mat3b templ2 = templ.clone();

    Mat1b img_gray;
    Mat1b templ_gray;
    cvtColor(img2, img_gray, COLOR_BGR2GRAY);
    cvtColor(templ2, templ_gray, COLOR_BGR2GRAY);


    Mat1f result;

    cvtColor(img, img, COLOR_BGRA2BGR);

    int result_cols = img.cols - templ.cols + 1;
    int result_rows = img.rows - templ.rows + 1;
    //result.create(result_rows, result_cols, CV_32FC1);

    matchTemplate(img2, templ2, result, TM_CCOEFF_NORMED);
    //normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
    threshold(result, result, 0.7, 1., THRESH_BINARY);

    Mat1b resb;
    result.convertTo(resb, CV_8U, 255);

    vector<vector<Point>> contours;
    findContours(resb, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    //cout << contours.size() << endl;
    for (int i = 0; i < contours.size(); i++)
    {
        Mat1b mask(result.rows, result.cols, uchar(0));
        drawContours(mask, contours, i, Scalar(255), FILLED);
        double minVal; double maxVal; Point minLoc; Point maxLoc;
        Point matchLoc;
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, mask);
        rectangle(img2, Rect(maxLoc.x, maxLoc.y, templ2.cols, templ2.rows), Scalar(0,255,0), 2);
    }
    return img2;
}

void detectBeings(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
{
    std::vector<Detection> output;
    detect(frame, net, output, class_list);
    int detections = output.size();
    for (int i = 0; i < detections; ++i)
    {
        auto box = output[i].box;
        auto classId = output[i].class_id;
        const auto color = colors[classId % colors.size()];
        cv::rectangle(frame, box, color, 3);
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
        circle(frame, Point2i(box.x + box.width/2, box.y + box.height/2), 5, Scalar(0, 125, 230), 4, 3);
        circle(frame, Point2i(frameWidth / 2, frameHeight / 2), 5, Scalar(0, 125, 230), 4, 3);

        int centerX = box.x + box.width / 2;
        int centerY = box.y + box.height / 2;
        cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        //cout << class_list[classId].c_str() << endl;
        //keybd_event(0x57, 0, 0, 0);
        //keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
        //MouseMove(box.x, box.y);
        //MouseMove(box.x, box.y);
        //cout << "x " << box.x << " y " << box.y << endl;
        if (centerX > frameWidth/2)
        {
            MouseMove(1, 0);
        }
        if (centerX < frameWidth/2)
        {
            MouseMove(-1, 0);
        }
    }
}

void MouseMove(int x, int y)
{
    double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
    double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
    double fx = x * (65535.0f / fScreenWidth);
    double fy = y * (65535.0f / fScreenHeight);
    cout << "FX " << fx << " FY " << fy << endl;
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    Input.mi.dx = fx;
    Input.mi.dy = fy;
    ::SendInput(1, &Input, sizeof(INPUT));
}
