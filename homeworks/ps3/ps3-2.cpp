//
// Created by Zeyu Zhang on 9/12/24.
//
#include "fssimplewindow.h"
#include <iostream>
#include <utility>

using namespace std;

int main() {
    FsOpenWindow(16,16,800,600,1);
    FsPollDevice();
    while(FSKEY_NULL==FsInkey()) {
        FsPollDevice();
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glColor3ub(255,255,255);
        glBegin(GL_QUADS);
        glVertex2i(100,100);
        glVertex2i(700,100);
        glVertex2i(700,500);
        glVertex2i(100,500);
        glEnd();
        FsSwapBuffers();
        FsSleep(25);
    }
    return 0;
}
