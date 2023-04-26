#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc.hpp"
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

const float THRESHOLD = 0.85;

Mat returnImage();

int main()
{
    int counter = 0;
    auto start_time = high_resolution_clock::now();   
    Mat dst, cdst, cdstP;
    int fpsCounter = 0;
    time_t start = time(0);

    //vector<string> class_list;
    //ifstream ifs("coco.names");
    string line;

    //Net net;
   // net = readNet("yolov5s.onnx");

    //while (getline(ifs, line))
   // {
     //   class_list.push_back(line);
   // }
    Mat templ = imread("creeperforward.jpg", IMREAD_GRAYSCALE);
    Mat imgDisplay;
    Mat result;
    Mat frame;
    Mat img;
    cout << CV_VERSION << endl;

    while (1)
    {
        //keybd_event(0x57, 0, 0, 0);
        frame = returnImage();
        img = frame.clone();

        // Wait indefinitely for a key press
        int key = cv::waitKey(1);
        if (key == 27) // break if escape key is pressed
            break;
        cvtColor(img, img, COLOR_RGB2GRAY);
        //Mat img = frame.clone();
        
        //Mat img = imread("test.png", IMREAD_COLOR);
        
        //cvtColor(img, img, COLOR_RGB2GRAY);
        // Mat img = post_process(frame, detections, class_list);
       // cout << templ.type() << " " << img.type() << " " << endl;

        img.copyTo(imgDisplay);


        int result_cols = img.cols - templ.cols + 1;
        int result_rows = img.rows - templ.rows + 1;
        result.create(result_rows, result_cols, CV_32FC1);
        try {
            matchTemplate(img, templ, result, TM_CCOEFF_NORMED);

           //normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
            double minVal; double maxVal; Point minLoc; Point maxLoc;
            Point matchLoc;

            minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

            matchLoc = maxLoc;


            if (maxVal > 0.2)
            {
                rectangle(img, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
            }
            cout << maxVal << " " << minVal << endl;
       }
       catch(exception e)
       {
           cout << "no" << endl;
       }



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

        imshow("Game Actual", img);
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