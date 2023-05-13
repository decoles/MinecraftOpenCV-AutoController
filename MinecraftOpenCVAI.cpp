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

enum MODES
{
    NONE = 0,
    PATHING = 1,
    FIGHT = 2,
    MINE = 3,
    HARVEST = 4,
    SENTRY = 5,
};

enum KEYS
{
    ONE = 0x31,
    TWO = 0x32,
    THREE = 0x33,
    FOUR = 0x34,

};

vector<string>MODENAMES = { "NONE", "PATHING", "FIGHT", "MINE", "HARVEST", "SENTRY" };

const float THRESHOLD = 0.85; //For match templage
int frameHeight = 540, frameWidth = 960, CURRENTMODE = NONE;

Mat returnImage(bool& val);
void returnMatchTemplate(Mat img, Mat templ, int& food, vector<Rect>& foodOutline);
void detectBeingsAttack(Mat& frame, Net &net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
void detectBeingsAvoidance(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);

DWORD WINAPI thred(__in LPVOID lpParameter) //Testing multithread in windows
{
    MouseRightClickAndHold();
   // KeyActionDown(0x33);//3 key
    //Sleep(1);
    //KeyActionUp(0x33);
    return 0;
}

int main()
{
    bool gameWindowFocus = false, is_cuda = true;
    int counter = 0, fpsCounter = 0, timePassed = 0, currentFood = 0, foodTimeCounter = 0;
    auto start_time = high_resolution_clock::now();
    auto FoodTimer = high_resolution_clock::now();
    time_t start = time(0), FoodTimeStart = time(0);
    unsigned int diffsum, maxdiff;
    double percent_diff;
    Mat matGray, matDiff, matGrayPrev, frame;
    string line;
    std::vector<std::string> class_list = load_class_list();
    const std::vector<cv::Scalar> colors = { cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0) };
    vector<Rect> foodOutline;
    cv::dnn::Net net;
    load_net(net, is_cuda);
    
    frame = returnImage(gameWindowFocus); //initial frame
    //Inital diff frame TODO get unstuck with diff
    cvtColor(frame, matGray, COLOR_BGR2GRAY);
    matDiff = matGray.clone();
    matGrayPrev = matGray.clone();
    maxdiff = (matDiff.cols) * (matDiff.rows) * 255;

    Mat templ = imread("MatchTemplate/food.png"); //For telling how much food exists
    Mat img;
    bool KeyFlag = true;

    DWORD dwThreadId;
    static HANDLE hThread = NULL;

    while (1)
    {
        if (GetKeyState('X') & 0x8000 && KeyFlag == true) //Press X key to change working state VERY FINICKY
        {
            KeyFlag = false;
            if (CURRENTMODE == 5)
                CURRENTMODE = 0;
            else
                CURRENTMODE++; 
            cout << "CURRENT MODDE: " << MODENAMES[CURRENTMODE] << endl;
            KeyActionUp(0x57);
            KeyActionUp(VK_SPACE);
            MouseLeftClickUp();
        }
        frame = returnImage(gameWindowFocus);
        // Wait indefinitely for a key press
        int key = cv::waitKey(1);
        if (key == 27) // break if escape key is pressed
            break;

        //gameWindowFocus = true;

        //DOWN SCALE AND USE FRAME
        cvtColor(frame, frame, COLOR_BGRA2BGR); //Go from 8UC4 to 8UC3
        resize(frame, frame, Size(960, 540), INTER_LINEAR);

        //cvtColor(frame, matGray, COLOR_BGR2GRAY);
        //absdiff(matGrayPrev, matGray, matDiff);
        //diffsum = (unsigned int)sum(matDiff)[0];
       // percent_diff = ((double)diffsum / (double)maxdiff) * 100;
        //cout << percent_diff << endl;

        if (gameWindowFocus) { //Main window foucs
            if (CURRENTMODE == NONE)
            {
                cv::putText(frame, "NO MODE SELECTED", Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            }
            if (CURRENTMODE == PATHING)
            {
                KeyActionDown(0x57);
                KeyActionDown(VK_SPACE);
                detectBeingsAvoidance(frame, net, colors, class_list);
            }
            else if (CURRENTMODE == FIGHT)
            {
                detectBeingsAttack(frame, net, colors, class_list);
            }
            else if (CURRENTMODE == MINE)
            {
                //MouseLeftClick();
            }

            else if (CURRENTMODE == HARVEST)
            {

            }
            else if(CURRENTMODE == SENTRY)
            {
                
            }
            //frame = returnMatchTemplate(frame.clone(), templ, currentFood); //Find hunger
            //detectBeings(frame, net, colors, class_list);

            //Only feed if window is in focus
            auto currentTimeFood = high_resolution_clock::now();
            auto foodElapseTime = duration_cast<seconds>(currentTimeFood - FoodTimer).count();
            if (foodElapseTime >= 3)
            {
                returnMatchTemplate(frame.clone(), templ, currentFood, foodOutline); //Find hunger and get shape
                FoodTimer = currentTimeFood;
                if (currentFood > 0) //Dont want this happening if the food level is gone or character is dead
                {
                    if (currentFood <= 8 && hThread == NULL)
                    {
                        hThread = CreateThread(NULL, 0, thred, NULL, 0, &dwThreadId);
                    }
                    if (currentFood >= 9 && hThread != NULL) //If thread is occupied and food is good cancel thread
                    {
                        CloseHandle(hThread);
                        hThread = NULL;
                    }
                }
            }
            if (foodOutline.size() > 0)
            {
                for (int k = 0; k < foodOutline.size(); k++)
                {
                    rectangle(frame, Rect(foodOutline[k].x, foodOutline[k].y, foodOutline[k].height, foodOutline[k].width), Scalar(0, 255, 0), 2);
                }
            }
        }
        else
        {
            KeyActionUp(0x57);
            KeyActionUp(VK_SPACE);
            MouseLeftClickUp();
            CloseHandle(hThread);

        }
        
        
        //cv::rectangle(frame, Point(0, 0), Point(frameWidth * 0.2, frameHeight * 0.3), Scalar(0, 0, 0), 3);
        //cv::rectangle(frame, Point(240, 469), Point(605, 515), Scalar(0, 0, 0), 3); //For hotbar
        imshow("Game Actual", frame);
        //imshow("diff", matDiff);

        //matGrayPrev = matGray.clone();
        //cout << percent_diff << endl;

  
        


        counter++;
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
        if (elapsed_time >= 1) {
            cout << "Loop completed " << counter << " times in " << elapsed_time << " seconds." << endl;
            KeyFlag = true; //Resets keypress timer so it dosent get spammed
            counter = 0; //For fps counter
            start_time = current_time;
        }
    }
    //
    return 0;
}

//returnImage: 
Mat returnImage(bool &val)
{
    HWND hwnd = FindWindow(NULL, L"Minecraft* 1.19.2 - Singleplayer");
    HWND temp = GetForegroundWindow(); //Gets current window for comparing to needed window

    if (hwnd == temp)//Window Focus
        val = true; //Sets true if window is in focus so controls dont interact with the system.
    else
        val = false;

    HDC hdc = GetDC(hwnd); // Get the device context of the window

    RECT rect;
    GetClientRect(hwnd, &rect); // Get the dimensions of the client area of the window

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    // global variables for current frame dimensions
    //frameHeight = height;
    //frameWidth = width; 
    
    HDC hdcMem = CreateCompatibleDC(hdc); // Create a compatible device context
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height); // Create a compatible bitmap

    // Select the bitmap into the device context
    SelectObject(hdcMem, hBitmap);

    // Copy the contents of the window to the bitmap
    BitBlt(hdcMem, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

    // Create a new frame object to store the captured image, unsinged int 8bit four channels
    Mat frame(height, width, CV_8UC4);

    // Copy the bitmap data to the frame object
    GetBitmapBits(hBitmap, width * height * 4, frame.data); 

    //cleanup
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdc);
    return frame;
}
//Import frame and a template image to match to 
void returnMatchTemplate(Mat img, Mat templ, int &food, vector<Rect> &foodOutline)
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
    for (int i = 0; i < contours.size(); i++)
    {
        Mat1b mask(result.rows, result.cols, uchar(0));
        drawContours(mask, contours, i, Scalar(255), FILLED);
        double minVal; double maxVal; Point minLoc; Point maxLoc;
        Point matchLoc;
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, mask);
        //rectangle(img2, Rect(maxLoc.x, maxLoc.y, templ2.cols, templ2.rows), Scalar(0,255,0), 2);
        food++;
        foodOutline.push_back(Rect(maxLoc.x, maxLoc.y, templ2.cols, templ2.rows)); //get the boxes around the food showing all the time
    }
}

void detectBeingsAttack(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
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
        int centerX = box.x + box.width / 2;
        int centerY = box.y + box.height / 2;
        //cout << box.height << " BOX HEIGHT" << endl;
        //cout << frameHeight << " FRAME HEIGHT " << endl;
        float upperBound = frameHeight - (frameHeight * .30);
        //When in creatures face stop and attack
        if (upperBound < box.height)
        {
            MouseLeftClick();
            KeyActionUp(0x57); //Stop running
            KeyActionUp(VK_SPACE);

        }
        else
        {
            MouseLeftClickUp(); //cancel input
            KeyActionDown(0x57);
            KeyActionDown(VK_SPACE);
        }
            if (centerX > frameWidth / 2)
            {
                MouseMove(2, 0);
            }
            if (centerX < frameWidth / 2)
            {
                MouseMove(-2, 0);
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

void detectBeingsAvoidance(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
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
        circle(frame, Point2i(box.x + box.width / 2, box.y + box.height / 2), 5, Scalar(0, 125, 230), 4, 3); //Center enemy
        circle(frame, Point2i(frameWidth / 2, frameHeight / 2), 5, Scalar(0, 125, 230), 4, 3); //Center screen
        cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

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
                MouseMove(2, 0);
            }
            if(centerX < frameWidth / 2)
            {
                MouseMove(-2, 0);
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


