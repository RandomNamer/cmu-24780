#include "fssimplewindow.h"
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <algorithm>

std::array<int, 2> parseInput(const std::string &input) {
    std::array<int, 2> result;
    auto firstInputDelimiter = input.find_first_of(' ');
    auto secondInputDelimiter = input.find_last_of(' ');
    if (firstInputDelimiter != std::string::npos && secondInputDelimiter != std::string::npos) {
        result[0] = std::stoi(input.substr(0, firstInputDelimiter));
        result[1] = std::stoi(input.substr(secondInputDelimiter + 1, input.length()));
    }
    return result;
}

class CharBitmap {
public:
    int wid, hei;
    char *pix;

    CharBitmap() : wid(0), hei(0), pix(nullptr) {}

    ~CharBitmap() {
        CleanUp();
    }

    void CleanUp() {
        if (pix) {
            delete[] pix;
            pix = nullptr;
        }
        wid = 0;
        hei = 0;
    }

    void Create(int w, int h) {
        CleanUp();
        wid = w;
        hei = h;
        pix = new char[wid * hei]();
    }

    void CreateFromFile() {
        std::ifstream file("pattern.txt");
        if (file.is_open()) {
            std::string line;
            std::getline(file, line);
            auto dimensions = parseInput(line);
            Create(dimensions[0], dimensions[1]);
            for (int y = 0; y < hei; ++y) {
                std::getline(file, line);
                for (int x = 0; x < wid; ++x) {
                    SetPixel(x, y, line[x]);
                }
            }
            file.close();
        }
    }

    void SaveToFile() {
        std::ofstream file("pattern.txt");
        if (file.is_open()) {
            file << wid << " " << hei << std::endl;
            for (int y = 0; y < hei; ++y) {
                for (int x = 0; x < wid; ++x) {
                    file << GetPixel(x, y);
                }
                file << std::endl;
            }
            file.close();
        }
    }

    void SetPixel(int x, int y, char p) {
        if (p > 7) {
            p = 7;
        }
        if (x >= 0 && x < wid && y >= 0 && y < hei) {
            pix[y * wid + x] = p;
        }
    }

    char GetPixel(int x, int y) const {
        if (x >= 0 && x < wid && y >= 0 && y < hei) {
            return pix[y * wid + x];
        }
        return 0;
    }

    void Draw() const {
        DrawBitmap();
        DrawLattice();
    }

private:

    void DrawBitmap() const {
        for (int y = 0; y < hei; ++y) {
            for (int x = 0; x < wid; ++x) {
                char p = GetPixel(x, y);
                switch (p) {
                    case 0: glColor3f(0, 0, 0); break;
                    case 1: glColor3f(0, 0, 1); break;
                    case 2: glColor3f(1, 0, 0); break;
                    case 3: glColor3f(1, 0, 1); break;
                    case 4: glColor3f(0, 1, 0); break;
                    case 5: glColor3f(0, 1, 1); break;
                    case 6: glColor3f(1, 1, 0); break;
                    case 7: glColor3f(1, 1, 1); break;
                }
                glBegin(GL_QUADS);
                glVertex2i(x * 20, y * 20);
                glVertex2i((x + 1) * 20, y * 20);
                glVertex2i((x + 1) * 20, (y + 1) * 20);
                glVertex2i(x * 20, (y + 1) * 20);
                glEnd();
            }
        }
    }

    void DrawLattice() const {
        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        for (int i = 0; i <= wid; ++i) {
            glVertex2i(i * 20, 0);
            glVertex2i(i * 20, hei * 20);
        }
        for (int i = 0; i <= hei; ++i) {
            glVertex2i(0, i * 20);
            glVertex2i(wid * 20, i * 20);
        }
        glEnd();
    }
};


int main() {

    CharBitmap bitmap;
    int width, height;

    while (true) {
        std::cout << "Enter Dimension> ";
        std::string input;
        std::getline(std::cin, input);
        auto dimensions = parseInput(input);
        if (dimensions[0] > 0 && dimensions[1] > 0) {
            width = dimensions[0];
            height = dimensions[1];
            std::cout<<"Got valid config. Width: "<<width<<", Height: "<<height<<std::endl;
            break;
        }
    }

    bitmap.Create(width, height);
    FsOpenWindow(16, 16, width * 20, height * 20, 1);

    int key = FSKEY_NULL;
    while (key != FSKEY_ESC) {
        FsPollDevice();
        key = FsInkey();

        if (key == FSKEY_S) {
            bitmap.SaveToFile();
            continue;
        } else if (key == FSKEY_L) {
            bitmap.CreateFromFile();
            continue;
        }

        int lb, mb, rb, mx, my;
        FsGetMouseState(lb, mb, rb, mx, my);

        if (key >= FSKEY_0 && key <= FSKEY_7) {
            int colorCode = key - FSKEY_0;
            int x = mx / 20;
            int y = my / 20;
            bitmap.SetPixel(x, y, colorCode);
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        bitmap.Draw();

        glFlush();
        FsSwapBuffers();
        FsSleep(50);
    }

    return 0;
}