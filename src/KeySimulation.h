#ifndef KEYSIMULATION_H
#define KEYSIMULATION_H

#include <Windows.h>
#include <iostream>
using namespace std;

void MouseMove(int x, int y)
{
    double fScreenWidth = GetSystemMetrics(SM_CXSCREEN) - 1;
    double fScreenHeight = GetSystemMetrics(SM_CYSCREEN) - 1;
    double fx = x * 10;
    double fy = y * 10; //Turns speeds
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    Input.mi.dx = fx;
    Input.mi.dy = fy;
    SendInput(1, &Input, sizeof(INPUT));
}

void MouseRightClickAndHold()
{
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &Input, sizeof(INPUT));
    Sleep(4000);
    Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &Input, sizeof(INPUT));
}

void MouseLeftClick()
{
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &Input, sizeof(INPUT));
}

void MouseLeftClickUp()
{
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &Input, sizeof(INPUT));
}

void MouseRightClick()
{
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &Input, sizeof(INPUT));
}

void KeyActionDown(int key)
{
    INPUT Input = { 0 };
    Input.type = INPUT_KEYBOARD;
    Input.ki.wVk = key;
    SendInput(1, &Input, sizeof(INPUT)); // Send KeyDown
}

void KeyActionUp(int key)
{
    INPUT Input = { 0 };
    Input.type = INPUT_KEYBOARD;
    Input.ki.wVk = key;
    Input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &Input, sizeof(INPUT)); // Send KeyUp
}
#endif