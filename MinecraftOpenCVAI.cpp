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
#include <KeySimulation.h>
#include <vector>

using namespace cv::dnn; //OpenCV ML
using namespace std::chrono; //Timing
using namespace std;
using namespace cv;

const float THRESHOLD = 0.85; //For match templage
int frameHeight;
int frameWidth;

enum MODES
{
    PATHING = 0,
    FIGHT = 1,
    MINE = 2,
    HARVEST = 3,
    NONE = 4,
};

vector<string>MODENAMES = {"PATHING, FIGHT", "MINE", "HARVEST", "NONE"};

int CURRENTMODE = NONE;


Mat returnImage(bool& val);
Mat returnMatchTemplate(Mat img, Mat templ, int& food);
void detectBeings(Mat& frame, Net &net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
DWORD WINAPI thred(__in LPVOID lpParameter)
{
    KeyActionDown(VK_SPACE);
    Sleep(1);
    KeyActionUp(VK_SPACE);
    return 0;
}

int main()
{
    bool gameWindowFocus = false;
    int counter = 0;
    auto start_time = high_resolution_clock::now();
    int fpsCounter = 0;
    time_t start = time(0);
    unsigned int diffsum, maxdiff;
    double percent_diff;
    Mat matGray, matDiff, matGrayPrev;
    string line;
    Mat frame;
    int timePassed = 0;
    int currentFood = 0;
    std::vector<std::string> class_list = load_class_list();
    const std::vector<cv::Scalar> colors = { cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0) };
    bool is_cuda = true;
    cv::dnn::Net net;
    load_net(net, is_cuda);
    
    frame = returnImage(gameWindowFocus);
    cvtColor(frame, matGray, COLOR_BGR2GRAY);

    matDiff = matGray.clone();
    matGrayPrev = matGray.clone();

    maxdiff = (matDiff.cols) * (matDiff.rows) * 255;
    Mat templ = imread("MatchTemplate/food.png");
    Mat img;
    bool KeyFlag = true;

    DWORD myThreadID;


    while (1)
    {
        if (GetAsyncKeyState('X') & 0x8000 && KeyFlag == true)
        {
            KeyFlag = false;
            if (CURRENTMODE == 4)
                CURRENTMODE = 0;
            else
                CURRENTMODE++; 

            //cout << "Current Mode Switch To: " << MODENAMES[CURRENTMODE] << endl ;

        }
        //Sleep(5000);
        frame = returnImage(gameWindowFocus);
        // Wait indefinitely for a key press
        int key = cv::waitKey(1);
        if (key == 27) // break if escape key is pressed
            break;
        //gameWindowFocus = true;
        cvtColor(frame, frame, COLOR_BGRA2BGR); //Go from 8UC4 to 8UC3
        resize(frame, frame, Size(960, 540), INTER_LINEAR);

        //cvtColor(frame, matGray, COLOR_BGR2GRAY);
        //absdiff(matGrayPrev, matGray, matDiff);

        //diffsum = (unsigned int)sum(matDiff)[0];

       // percent_diff = ((double)diffsum / (double)maxdiff) * 100;

        //cout << percent_diff << endl;

        if (gameWindowFocus) { //Main window foucs code
            if (CURRENTMODE == PATHING)
            {
                KeyActionDown(0x57);
                KeyActionDown(VK_SPACE);
                detectBeings(frame, net, colors, class_list);
            }
            else if (CURRENTMODE == MINE)
            {
                
            }
            else if (CURRENTMODE == FIGHT)
            {
                KeyActionDown(0x57);
                detectBeings(frame, net, colors, class_list);


            }
            else if (CURRENTMODE == HARVEST)
            {

            }
            else if (CURRENTMODE == NONE)
            {
                cv::putText(frame, "NO MODE SELECTED", Point(0, 0), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(255, 255, 0));

               //HANDLE myhandle = CreateThread(0, 0, thred, 0, 0 ,&myThreadID);

            }
               //SpacebarAction();

            //detectBeings(frame, net, colors, class_list);
            frame = returnMatchTemplate(frame.clone(), templ, currentFood); //Find hunger

            cout << currentFood << endl;
             //putText(
        }
        cv::rectangle(frame, Point(0, 0), Point(frameWidth * 0.2, frameHeight * 0.3), Scalar(0, 0, 0), 3);
        //cv::rectangle(frame, Point(240, 469), Point(605, 515), Scalar(0, 0, 0), 3); //For hotbar
        imshow("Game Actual", frame);
        //imshow("diff", matDiff);
        counter++;
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
        //matGrayPrev = matGray.clone();
        //cout << percent_diff << endl;

        if (elapsed_time >= 1) {
            cout << "Loop completed " << counter << " times in " << elapsed_time << " seconds." << endl;
            timePassed++; //Increments each second up to 3 seconds;
            if (timePassed >= 1)
                timePassed = 0;
            KeyFlag = true; //Resets keypress timer so it dosent get spammed
            //KeyActionUp(0x57);
           // KeyActionUp(VK_SPACE);


            counter = 0;
            start_time = current_time;
        }
    }


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
Mat returnMatchTemplate(Mat img, Mat templ, int &food)
{
    food = 0;
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
        food++;
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
        circle(frame, Point2i(box.x + box.width/2, box.y + box.height/2), 5, Scalar(0, 125, 230), 4, 3); //Center enemy
        circle(frame, Point2i(frameWidth / 2, frameHeight / 2), 5, Scalar(0, 125, 230), 4, 3); //Center screen
        cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        //cout << class_list[classId].c_str() << endl;
        //keybd_event(0x57, 0, 0, 0);
        //keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
        //MouseMove(box.x, box.y);
        //MouseMove(box.x, box.y);
        //cout << "x " << box.x << " y " << box.y << endl;

        int centerX = box.x + box.width / 2;
        int centerY = box.y + box.height / 2;
        if (CURRENTMODE == PATHING || CURRENTMODE == MINE || CURRENTMODE == HARVEST)
        {

                MouseMove(5, 0);
   
        }
        else if (CURRENTMODE == FIGHT)
        {
            if (centerX > frameWidth / 2)
            {
                MouseMove(1, 0);
            }
            if (centerX < frameWidth / 2)
            {
                MouseMove(-1, 0);
            }
            if (centerY > frameHeight / 2)
            {
                MouseMove(0, 1);
            }
            if (centerY < frameHeight / 2)
            {
                MouseMove(0, -1);
            }
        }
    }
}


