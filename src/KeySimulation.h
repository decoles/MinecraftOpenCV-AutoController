#ifndef KEYSIMULATION_H
#define KEYSIMULATION_H

#include <Windows.h>
#include <iostream>
using namespace std;


void MouseMove(int x, int y)
{
    double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
    double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
    double fx = x * 10;
    double fy = y * 10; //Turns speeds
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    Input.mi.dx = fx;
    Input.mi.dy = fy;
    ::SendInput(1, &Input, sizeof(INPUT));
}

void KeyActionDown(int key)
{
    INPUT space = { 0 };
    space.type = INPUT_KEYBOARD;
    space.ki.wVk = key;
    SendInput(1, &space, sizeof(INPUT)); // Send KeyDown
}
void KeyActionUp(int key)
{
    INPUT space = { 0 };
    space.type = INPUT_KEYBOARD;
    space.ki.wVk = key;
    space.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &space, sizeof(INPUT)); // Send KeyUp
}
#endif