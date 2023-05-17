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
int CURRENTMODE = NONE, CURRENTKEY = ONE;
const int FRAMEHEIGHT = 540, FRAMEWIDTH = 960, DIMENSIONBEINGS = 10, DIMENSIONMINING = 12, DIMENSIONTREE = 7;

Mat returnImage(bool& val);
void returnMatchTemplate(Mat img, Mat templ, int& food, vector<Rect>& foodOutline);
void detectBeingsAttack(Mat& frame, Net &net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
void detectBeingsAvoidance(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
void detectBeingsSentry(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
void detectTree(Mat& frame, Net& netBeings, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);
void detectLight(Mat& frame);
void detectOre(Mat& frame, Net& netBeings, std::vector<cv::Scalar> colors, std::vector<std::string> class_list);


DWORD WINAPI eatThread(__in LPVOID lpParameter) //performs eating action while doing other tasks
{
    KeyActionDown(THREE);//Press three key for food
    Sleep(1);
    KeyActionUp(THREE);
    MouseRightClickAndHold();
    KeyActionDown(CURRENTKEY);
    Sleep(1);
    KeyActionUp(CURRENTKEY);//Put key back to whichever place it was in
    return 0;
}

DWORD WINAPI changeCurrentKey(__in LPVOID lpParameter) //changes key and sleeps while performing other tasks
{
    KeyActionDown(CURRENTKEY);
    Sleep(10);
    KeyActionUp(CURRENTKEY);//Put key back to whichever place it was in
    return 0;
}

int main()
{
    bool gameWindowFocus = false, is_cuda = true;
    int counter = 0, fpsCounter = 0, timePassed = 0, currentFood = 0, foodTimeCounter = 0;
    auto start_time = high_resolution_clock::now(); //Main loops fps timer
    auto FoodTimer = high_resolution_clock::now(); //Timer to feed character
    auto stuckTimer = high_resolution_clock::now();//Determine when to unstuck
    unsigned int diffsum, maxdiff;
    double percent_diff;
    Mat matGray, matDiff, matGrayPrev, frame;
    string line;

    //For storing food outlines
    vector<Rect> foodOutline; 

    //Load the model(s) & information
    cv::dnn::Net netBeings;
    cv::dnn::Net netTrees;
    cv::dnn::Net netOre;

    load_net(netBeings, is_cuda, "DetectEnemies.onnx");
    load_net(netTrees, is_cuda, "DetectTrees.onnx");
    load_net(netOre, is_cuda, "DetectOre.onnx");

    std::vector<std::string> class_list_mobs = load_class_list("classesMobs.txt");
    std::vector<std::string> class_list_trees = load_class_list("classesTrees.txt");
    std::vector<std::string> class_list_ores = load_class_list("classesOres.txt");

    const std::vector<cv::Scalar> colors = { cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0) };

    //INITAL SIZEING CODE
    frame = returnImage(gameWindowFocus); //initial frame
    cvtColor(frame, frame, COLOR_BGRA2BGR); //Go from 8UC4 to 8UC3
    resize(frame, frame, Size(960, 540), INTER_LINEAR);

    
    cvtColor(frame, matGray, COLOR_BGR2GRAY);
    matDiff = matGray.clone();
    matGrayPrev = matGray.clone();
    maxdiff = (matDiff.cols) * (matDiff.rows) * 255;
    
    Mat templ = imread("MatchTemplate/food.png"); //For telling how much food exists

    bool KeyFlag = true;
    DWORD dwThreadId;
    DWORD dwThreadTWO;
    static HANDLE hThread = NULL;
    static HANDLE keyThread = NULL;

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
            //Verify none of existing keys are still pressed
            KeyActionUp(0x57);
            //KeyActionUp(VK_SPACE);
            MouseLeftClickUp();
            switch (CURRENTMODE)
            {
            case 2:
                CURRENTKEY = TWO;
                break;
            case 3:
                CURRENTKEY = ONE;
                break;
            case 4:
                CURRENTKEY = FOUR;
                break;
            default: //Defaults to pickaxe or 1 key 
                CURRENTKEY = ONE;
                break;
            }
            keyThread = CreateThread(NULL, 0, changeCurrentKey, NULL, 0, &dwThreadTWO);

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

        cvtColor(frame, matGray, COLOR_BGR2GRAY);
        absdiff(matGrayPrev, matGray, matDiff);
        diffsum = (unsigned int)sum(matDiff)[0];
        percent_diff = ((double)diffsum / (double)maxdiff) * 100;
        if (gameWindowFocus) { //Main window foucs
            if (CURRENTMODE == NONE)
            {
                cv::putText(frame, "NO MODE SELECTED", Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            }
            if (CURRENTMODE == PATHING)
            {
                KeyActionDown(0x57);
                //KeyActionDown(VK_SPACE);
                detectBeingsAvoidance(frame, netBeings, colors, class_list_mobs);
            }
            else if (CURRENTMODE == FIGHT)
            {
               detectBeingsAttack(frame, netBeings, colors, class_list_mobs);
            }
            else if (CURRENTMODE == MINE)
            {
                detectOre(frame, netOre, colors, class_list_ores);
            }

            else if (CURRENTMODE == HARVEST)
            {
                detectTree(frame, netTrees, colors, class_list_trees);

            }
            else if(CURRENTMODE == SENTRY)
            {
                detectBeingsSentry(frame, netBeings, colors, class_list_mobs);
            }
            //Only feed if window is in focus
            auto currentTimeFood = high_resolution_clock::now();
            auto foodElapseTime = duration_cast<seconds>(currentTimeFood - FoodTimer).count();
            //Feed every 3 seconds or more
            if (foodElapseTime >= 3)
            {
                returnMatchTemplate(frame.clone(), templ, currentFood, foodOutline); //Find hunger and get shape
                FoodTimer = currentTimeFood;
                if (currentFood > 0) //Dont want this happening if the food level is gone or character is dead
                {
                    if (currentFood <= 8 && hThread == NULL)
                    {
                        hThread = CreateThread(NULL, 0, eatThread, NULL, 0, &dwThreadId);
                    }
                    if (currentFood >= 9 && hThread != NULL) //If thread is occupied and food is good cancel thread
                    {
                        CloseHandle(hThread);
                        hThread = NULL;
                    }
                }
                else
                {
                    foodOutline.clear();
                }
            }
            if (foodOutline.size() > 0)
            {
                for (int k = 0; k < foodOutline.size(); k++)
                {
                    rectangle(frame, Rect(foodOutline[k].x, foodOutline[k].y, foodOutline[k].height, foodOutline[k].width), Scalar(0, 255, 0), 2);
                }
            }

            //For light detection dot NONFUNCTIONAL
            detectLight(frame);
            //Gets character unstuck
            auto currentStuckTime = high_resolution_clock::now();
            auto stuckElapsedTime = duration_cast<seconds>(currentStuckTime - stuckTimer).count();
            if (percent_diff < 1)
            {
                if (stuckElapsedTime >= 5)
                {
                    MouseMove(90, 0);
                }
            }
            else
            {
                stuckTimer = currentStuckTime;
            }
        }
        else //If window is out of focus make sure no keys are still going.
        {
            KeyActionUp(0x57);
            //KeyActionUp(VK_SPACE);
            MouseLeftClickUp();
            CloseHandle(hThread);
        }

        //imshow("diff", matDiff);



  
        
        matGrayPrev = matGray.clone();
        imshow("Game Overlay", frame);
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

void detectBeingsAttack(Mat& frame, Net& netBeings, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
{
    std::vector<Detection> output;
    detect(frame, netBeings, output, class_list, DIMENSIONBEINGS);
    int detections = output.size();
    for (int i = 0; i < detections; ++i)
    {
        auto box = output[i].box;
        auto classId = output[i].class_id;
        const auto color = colors[classId % colors.size()];
        cv::rectangle(frame, box, color, 3);
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
        circle(frame, Point2i(box.x + box.width / 2, box.y + box.height / 2), 5, Scalar(0, 125, 230), 4, 3); //Center enemy
        circle(frame, Point2i(FRAMEWIDTH / 2, FRAMEHEIGHT / 2), 5, Scalar(0, 125, 230), 4, 3); //Center screen
        cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        float upperBound = FRAMEHEIGHT - (FRAMEHEIGHT * .50);
        if (classId != 1) {
            //When in creatures in face stop and attack
            if (upperBound < box.height)
            {
                MouseLeftClick(); //attack
                KeyActionUp(0x57); //Stop running
                //KeyActionUp(VK_SPACE); //stop jumping
            }
            else
            {
                MouseLeftClickUp(); //cancel input
                KeyActionDown(0x57); //walk towards
            }
            if (box.x > FRAMEWIDTH / 2)
            {
                MouseMove(4, 0);
            }
            else if (box.x + box.width < FRAMEWIDTH / 2)
            {
                MouseMove(-4, 0);
            }
            if (box.y > FRAMEHEIGHT / 2)
            {
                MouseMove(0, 2);
            }
            else if (box.y + box.height < FRAMEHEIGHT / 2)
            {
                MouseMove(0, -2);
            }
        }
    }
    if (detections == 0) // & 
    {
        MouseLeftClickUp(); //cancel input
        KeyActionDown(0x57); //walk towards
        //KeyActionDown(VK_SPACE); //jump
    }
    //else
        //KeyActionDown(VK_SPACE); //jump

}

void detectBeingsAvoidance(Mat& frame, Net& netBeings, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
{
    std::vector<Detection> output;
    detect(frame, netBeings, output, class_list, DIMENSIONBEINGS);
    int detections = output.size();
    for (int i = 0; i < detections; ++i)
    {
        auto box = output[i].box;
        auto classId = output[i].class_id;
        const auto color = colors[classId % colors.size()];
        rectangle(frame, box, color, 3);
        rectangle(frame, Point(box.x, box.y - 20), Point(box.x + box.width, box.y), color, FILLED);
        circle(frame, Point2i(box.x + box.width / 2, box.y + box.height / 2), 5, Scalar(0, 125, 230), 4, 3); //Center enemy
        circle(frame, Point2i(FRAMEWIDTH / 2, FRAMEHEIGHT / 2), 5, Scalar(0, 125, 230), 4, 3); //Center screen
        putText(frame, class_list[classId].c_str(), Point(box.x, box.y - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));

        int centerX = box.x + box.width / 2;
        // move left or right depening if the enemy is on the left or right side
        if (centerX > FRAMEWIDTH / 2)
        {
            MouseMove(-5, 0);
        }
        if (centerX < FRAMEWIDTH / 2)
        {
            MouseMove(5, 0);
        }
    }
}

void detectBeingsSentry(Mat& frame, Net& netBeings, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
{
    std::vector<Detection> output;
    detect(frame, netBeings, output, class_list, DIMENSIONBEINGS);
    int detections = output.size();
    if (detections >= 1)
    {
        CURRENTMODE = PATHING;
    }
    MouseMove(3, 0);

}

//Detects brightest light in scene, partially created by chatGPT
void detectLight(Mat& frame) 
{
    Mat binary;
    Mat tempFrame = frame.clone();
    cvtColor(tempFrame, tempFrame, COLOR_RGB2GRAY);
    cv::threshold(tempFrame, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Find the contours in the binary image
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Iterate through the contours to find the contour with the largest area
    double max_area = 0.0;
    int max_area_contour_idx = -1;
    for (int i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > max_area) {
            max_area = area;
            max_area_contour_idx = i;
        }
    }

    // Use the moments of the largest contour to calculate the centroid of the brightest spot
    if (max_area_contour_idx != -1) {
        cv::Moments moments = cv::moments(contours[max_area_contour_idx], false);
        double centroid_x = moments.m10 / moments.m00;
        double centroid_y = moments.m01 / moments.m00;
        cv::Point brightest_spot(centroid_x, centroid_y);

        // Draw a circle around the brightest spot on the original frame
        cv::circle(frame, brightest_spot, 10, cv::Scalar(0, 255, 0), -1);
    }

}

void detectTree(Mat& frame, Net& net, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
{
    std::vector<Detection> output;
    detect(frame, net, output, class_list, DIMENSIONTREE);
    int detections = output.size();
    float upperBound = FRAMEHEIGHT - (FRAMEHEIGHT * .70);

    if(detections != 0){
        auto box = output[0].box;
        auto classId = output[0].class_id;
        const auto color = colors[classId % colors.size()];
        cv::rectangle(frame, box, color, 3);
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
        circle(frame, Point2i(box.x + box.width / 2, box.y + box.height / 2), 5, Scalar(0, 125, 230), 4, 3); //Center enemy
        circle(frame, Point2i(FRAMEWIDTH / 2, FRAMEHEIGHT / 2), 5, Scalar(0, 125, 230), 4, 3); //Center screen
        cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        if (upperBound < box.height)
        {
            MouseLeftClick(); //attack
        }
        else
        {
            MouseLeftClickUp(); //cancel input
        }
        if (box.x > FRAMEWIDTH/2)
        {
            MouseMove(1, 0);
        }
        else if (box.x + box.width < FRAMEWIDTH / 2)
        {
            MouseMove(-1, 0);
        }
        if (box.y > FRAMEHEIGHT / 2)
        {
            MouseMove(0, 1);
        }
        else if (box.y + box.height < FRAMEHEIGHT / 2)
        {
            MouseMove(0, -1);
        }
    }
}

void detectOre(Mat& frame, Net& netOre, std::vector<cv::Scalar> colors, std::vector<std::string> class_list)
{
    std::vector<Detection> output;
    detect(frame, netOre, output, class_list, DIMENSIONMINING);
    int detections = output.size();
    float upperBound = FRAMEHEIGHT - (FRAMEHEIGHT * .70);

    // for (int i = 0; i < detections; ++i)
    // {
    if (detections != 0) {
        auto box = output[0].box;
        auto classId = output[0].class_id;
        const auto color = colors[classId % colors.size()];
        cv::rectangle(frame, box, color, 3);
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
        circle(frame, Point2i(box.x + box.width / 2, box.y + box.height / 2), 5, Scalar(0, 125, 230), 4, 3); //Center enemy
        circle(frame, Point2i(FRAMEWIDTH / 2, FRAMEHEIGHT / 2), 5, Scalar(0, 125, 230), 4, 3); //Center screen
        cv::putText(frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        if (upperBound < box.height)
        {
            MouseLeftClick(); //attack
        }
        else
        {
            MouseLeftClickUp(); //cancel input
        }
        if (box.x > FRAMEWIDTH / 2)
        {
            MouseMove(1, 0);
        }
        else if (box.x + box.width < FRAMEWIDTH / 2)
        {
            MouseMove(-1, 0);
        }
        if (box.y > FRAMEHEIGHT / 2)
        {
            MouseMove(0, 1);
        }
        else if (box.y + box.height < FRAMEHEIGHT / 2)
        {
            MouseMove(0, -1);
        }
    }
}

