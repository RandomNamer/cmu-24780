//
// Created by Zeyu Zhang on 11/4/24.
//

#include <iostream>
#include <string>
#include <memory>
#include "fssimplewindow.h"
#include "yspng.h"
#define USE_MORPH_EDGE

std::shared_ptr<const unsigned char[]> binaryFilter(const unsigned char* rgba, int wid, int hei, unsigned char threshold) {
    std::shared_ptr <unsigned char[]> binary(new unsigned char[wid * hei]());
    memset(binary.get(), 0, wid * hei);
    for (int p = 0; p < wid * hei; ++p) {
        const auto r = rgba[p * 4];
        const auto g = rgba[p * 4 + 1];
        const auto b = rgba[p * 4 + 2];
        if (r > threshold && g > threshold && b > threshold) {
            binary[p] = 255;
        }
    }
    return binary;
}

std::shared_ptr<const unsigned char[]> subtractionFilter(const unsigned char* bin1, const unsigned char* bin2, int wid, int hei) {
    std::shared_ptr<unsigned char[]> subtracted(new unsigned char[wid * hei]());
    memset(subtracted.get(), 0, wid * hei);
    for (int i = 0; i < wid * hei; ++i) {
        subtracted[i] = bin1[i] - bin2[i];
    }
    return subtracted;
}


template<size_t SE_SIZE>
std::shared_ptr<const unsigned char[]> erodeFilter(const unsigned char* bin, int wid, int hei, const int se[SE_SIZE][SE_SIZE]) {
    std::shared_ptr<unsigned char[]> eroded(new unsigned char[wid * hei]());
    memset(eroded.get(), 0, wid * hei);
   for (int x = 0; x < wid; ++x) {
       for (int y = 0; y < hei; ++y) {
           bool match = true;
           for (int i = 0; i < SE_SIZE; ++i) {
               for (int j = 0; j < SE_SIZE; ++j) {
                   if (se[i][j] == 1) {
                       if (x + i >= wid || y + j >= hei || bin[(y + j) * wid + x + i] == 0) {
                           match = false;
                           break;
                       }
                   }
               }
               if (!match) {
                   break;
               }
           }
           if (match) {
               eroded[y * wid + x] = 255;
           }
       }
   }
    return eroded;
}

template<size_t SE_SIZE>
std::shared_ptr<const unsigned char[]> morphologicalEdgeFilter(const unsigned char* bin, int wid, int hei, const int se[SE_SIZE][SE_SIZE]) {
    auto eroded = erodeFilter<SE_SIZE>(bin, wid, hei, se);
    auto edge = subtractionFilter(bin, eroded.get(), wid, hei);
    return edge;
}

std::shared_ptr<unsigned char[]> binaryContourFilter(const unsigned char* binary, int width, int height) {
    std::shared_ptr<unsigned char[]> contour(new unsigned char[width * height]());
    memset(contour.get(), 0, width * height);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;

            // Check if the current pixel is foreground
            if (binary[idx] > 0) {
                //4-neighbors
                if (binary[idx - 1] == 0 || binary[idx + 1] == 0 || binary[idx - width] == 0 || binary[idx + width] == 0) {
                    contour[idx] = 255;
                }
            }
        }
    }

    return contour;
}


enum DisplayState {
    ORIGINAL,
    BINARY,
    EDGE
};

int main() {
    YsRawPngDecoder png;
    FsChangeToProgramDir();
    if (YSOK != png.Decode("ps9.png")) {
        std::cout << "Failed to load image." << std::endl;
        return 1;
    }
    png.Flip();

    auto binImage = binaryFilter(png.rgba, png.wid, png.hei, 219);
    std::cout<<"Successfully built binarized image."<<std::endl;

    int SE_CROSS[3][3] = {{0, 1, 0}, {1, 1, 1}, {0, 1, 0}};
    int SE_SQUARE[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

#ifdef USER_MORPH_EDGE
    auto edgeImage = morphologicalEdgeFilter(binImage.get(), png.wid, png.hei, SE_CROSS);
#else
    auto edgeImage = binaryContourFilter(binImage.get(), png.wid, png.hei);
#endif

    auto state = ORIGINAL;
    FsOpenWindow(0, 0, png.wid, png.hei, 1);
    for(;;) {
        FsPollDevice();
        switch (FsInkey()) {
            case FSKEY_ESC:
                return 0;
            case FSKEY_SPACE:
                state = BINARY;
                break;
            case FSKEY_ENTER:
                state = EDGE;
                break;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, png.wid, png.hei);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, png.wid, 0, png.hei);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


        glRasterPos2i(0, 0);
        switch (state) {
            case ORIGINAL:
                glDrawPixels(png.wid, png.hei, GL_RGBA, GL_UNSIGNED_BYTE, png.rgba);
                break;
            case BINARY:
                glDrawPixels(png.wid, png.hei, GL_LUMINANCE, GL_UNSIGNED_BYTE, binImage.get());
                break;
            case EDGE:
                glDrawPixels(png.wid, png.hei, GL_LUMINANCE, GL_UNSIGNED_BYTE, edgeImage.get());
                break;
        }
        FsSwapBuffers();
        FsSleep(25);
    }
    FsCloseWindow();
}
