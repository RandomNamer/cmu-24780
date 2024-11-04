#include <stdio.h>
#include <algorithm>
#include "fssimplewindow.h"
#include "yspng.h"

class Histogram
{
protected:
    unsigned int hist[256];
    bool histReady;

public:
    Histogram() {
        memset(hist, 0, sizeof(hist));
        histReady = false;
    }

    void Make(const YsRawPngDecoder &png) {
        memset(hist, 0, sizeof(hist));
        for (int i = 0; i < png.wid * png.hei; ++i) {
            const auto r = png.rgba[i * 4];
            const auto g = png.rgba[i * 4 + 1];
            const auto b = png.rgba[i * 4 + 2];
            hist[r] += 1;
            hist[g] += 1;
            hist[b] += 1;
        }
        histReady = true;
    }

    void Print() const {
        if (!histReady) {
            printf("Histogram not ready.\n");
            return;
        }

        int maxNum = 0;
        for (const unsigned int i : hist) {
            maxNum = std::max<unsigned int>(maxNum, i);
        }

        for (int i = 0; i < 256; ++i) {
            printf("%3d:", i);

            int len = hist[i] * 80 / maxNum;
            for (int j = 0; j < len; ++j) {
                printf("#");
            }
            printf("\n");
        }
    }

    void Draw(int posX, int posY) const {
        if (!histReady) {
            return;
        }

        int maxNum = 0;
        for (const unsigned int i : hist) {
            maxNum = std::max<unsigned int>(maxNum, i);
        }

        glColor3ub(255, 255, 255); // Set drawing color to white

        glBegin(GL_LINES);
        for (int i = 0; i < 256; ++i) {
            int len = hist[i] * 80 / maxNum; // Histogram max height is 80 pixels

            glVertex2i(posX + i, posY);       // Start from specified position
            glVertex2i(posX + i, posY + len); // Draw upwards
        }
        glEnd();
    }
};


int main(int argc, char *argv[]) {
    FsChangeToProgramDir();

    if (argc < 2) {
        printf("Need input");
        return 1;
    }

    const auto filename = argv[1];

    YsRawPngDecoder png;
    if (YSOK == png.Decode(filename)) {
        printf("File Load OK: %s Width: %d Height: %d\n", filename, png.wid, png.hei);
    } else {
        printf("Cannot open file %s.\n", filename);
        return 1;
    }
    png.Flip();

    Histogram hist;
    hist.Make(png);
    hist.Print();

    // Window size matches image size
    FsOpenWindow(0, 0, png.wid, png.hei, 1);
    for (;;) {
        FsPollDevice();
        if (FSKEY_ESC == FsInkey()) {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, png.wid, png.hei);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, png.wid, 0, png.hei);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glRasterPos2i(0, 0);
        glDrawPixels(png.wid, png.hei, GL_RGBA, GL_UNSIGNED_BYTE, png.rgba);

        hist.Draw(0, 0);

        FsSwapBuffers();
    }

    FsCloseWindow();

    return 0;
}

