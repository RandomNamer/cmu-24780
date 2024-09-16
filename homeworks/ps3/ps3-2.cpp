//
// Created by Zeyu Zhang on 9/12/24.
// parse SVG path and draw using pure GL_LINES primitives.
// In real world SVG path can be multi-lined and have more than one command in one line, In this application I only handle simple situations.
// SVG paths are preprocessed to split commands into separate lines using JavaScript.
// Change sample to draw other.
//
#define USE_BUNDLED_SAMPLES
#define ARC_STEPS 0
#define BEZIER_STEPS 20


#include "fssimplewindow.h"
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "math.h"

using namespace std;

#include "wireframe-samples.h"

const string& SAMPLE = SAMPLE_PATH_SPARKLE_A;

struct Point2i {
    int x;
    int y;
};
struct Point2f {
    float x;
    float y;

    operator Point2i() const {
        return {static_cast<int>(round(x)), static_cast<int>(round(y))};
    }

    Point2f operator+ (const Point2f& other) const {
        return {x + other.x, y + other.y};
    }
};


vector<Point2f> interpolateBezier(Point2f startPosition, Point2f endPosition, Point2f control1, Point2f control2, bool relative = false, int steps = 0) {
    vector<Point2f> points = { startPosition, startPosition };

    if (relative) {
        control1 = control1 + startPosition;
        control2 = control2 + startPosition;
        endPosition = endPosition + startPosition;
    }

    for (int i=1; i<=steps; i++) {
        float t = static_cast<float>(i) / steps;

        float x = (1-t)*(1-t)*(1-t)*startPosition.x + 3*(1-t)*(1-t)*t*control1.x + 3*(1-t)*t*t*control2.x + t*t*t*endPosition.x;
        float y = (1-t)*(1-t)*(1-t)*startPosition.y + 3*(1-t)*(1-t)*t*control1.y + 3*(1-t)*t*t*control2.y + t*t*t*endPosition.y;

        points.push_back(points.back());
        points.push_back({x, y});
    }
//    points.push_back(endPosition);
    return points;
}

vector<Point2f> interpolateArc(Point2f startPosition, Point2f endPosition, float radiusX, float radiusY, float xAxisRotate, float largeArcFlag, float sweepFlag,bool relative = false, int steps = 0) {
    if (steps == 0) {
        return {startPosition, relative ? startPosition+endPosition : endPosition};
    }
    //This is bad, should use to step zero
    vector<Point2f> points;
    for (int i=0; i<=steps; i++) {
        float t = static_cast<float>(i) / steps;
        float angle = atan2(endPosition.y - startPosition.y, endPosition.x - startPosition.x);
        float x = cos(angle) * radiusX * cos(t) - sin(angle) * radiusY * sin(t);
        float y = sin(angle) * radiusX * cos(t) + cos(angle) * radiusY * sin(t);
        if (relative) {
            x += startPosition.x;
            y += startPosition.y;
        }
        points.push_back({x, y});
    }
    return points;
}

template<size_t N>
std::array<float, N> extractFloatNumbers(const string& line) {
    std::array<float, N> numbers;
    std::string currentNumber;
    int index = 0;

    for (char ch : line) {
        if (ch == ',' || ch == '-' || ch == ' ') {
            if (!currentNumber.empty()) {
                if (index < N) numbers[index++] = std::stof(currentNumber);
                currentNumber.clear();
            }
            if (ch == '-') {
                currentNumber += ch;
            }
        } else {
            if ((ch >= '0' && ch <= '9' )|| ch == '.') currentNumber += ch;
        }
    }

    if (!currentNumber.empty() && index < N) {
        numbers[index] = std::stof(currentNumber);
    }

    return numbers;
}

vector<Point2f> parsePathData(const string& rawPath) {
    istringstream allss(rawPath);
    char command;
    Point2f currentPosition = {0 , 0};
    vector<Point2f> points;
    string line;
    while(!allss.eof() && getline(allss, line)) {
PARSE_START:
        if (line.empty()) continue;
        command = line[0];
        cout<<"Parsing: "<<line<<endl;
        switch (command) {
            case 'M': {
                auto [x, y] = extractFloatNumbers<2>(line.substr(1));
                currentPosition = {x, y};
                break;
            }
            case 'L':
            case 'l': {
                auto [x, y] = extractFloatNumbers<2>(line.substr(1));
                points.push_back(currentPosition);
                if (command == 'l') {
                    x += currentPosition.x;
                    y += currentPosition.y;
                }
                currentPosition = {x, y};
                points.push_back(currentPosition);
                break;
            }
            case 'h':
            case 'H': {
                auto [x] = extractFloatNumbers<1>(line.substr(1));
                points.push_back(currentPosition);
                if (command == 'h') {
                    x += currentPosition.x;
                }
                currentPosition = {x, currentPosition.y};
                points.push_back(currentPosition);
                break;
            }
            case 'V':
            case 'v': {
                auto [y] = extractFloatNumbers<1>(line.substr(1));
                points.push_back(currentPosition);
                if (command == 'v') {
                    y += currentPosition.y;
                }
                currentPosition = {currentPosition.x, y};
                points.push_back(currentPosition);
                break;
            }
            case 'C':
            case 'c': {
                auto [control1x, control1y, control2x, control2y, x, y] = extractFloatNumbers<6>(line.substr(1));
                const auto &bezierPoints = interpolateBezier(currentPosition, {x, y}, {control1x, control1y}, {control2x, control2y}, command == 'c', BEZIER_STEPS);
                points.insert(points.end(), bezierPoints.begin(), bezierPoints.end());
                currentPosition = bezierPoints.back();
                break;
            }
            case 'A':
            case 'a': {
                auto [radiusX, radiusY, xAxisRotation, largeArcFlag, sweepFlag, x, y] = extractFloatNumbers<7>(line.substr(1));
                const auto& arcPoints = interpolateArc(currentPosition, {x, y}, radiusX, radiusY, xAxisRotation, largeArcFlag, sweepFlag, command == 'a', ARC_STEPS);
                points.insert(points.end(), arcPoints.begin(), arcPoints.end());
                currentPosition = arcPoints.back();
                break;
            }
            case 'Z':
            case 'z':
                if (!points.empty()) {
                    points.push_back(currentPosition);
                    points.push_back(points.front());
//                    currentPosition = points.front();
                }
                if (line.length() > 2) {
                    cout<<"Extra characters after Z/z: "<<line.substr(1)<<endl;
                    line = line.substr(1);
                    goto PARSE_START;
                }
                break;
            default:
                break;
        }
    }
    return points;
}

void glDrawWireframeArt(const vector<Point2f>& points) {
    glColor3ub(0,0,0);
    glBegin(GL_LINES);
    for (const auto& point : points) {
        glVertex2i(point.x, point.y);
    }
    glEnd();
}

array<float, 4> calculateOffsets(const vector<Point2f>& points, int wndSizeX, int wndSizeY) {
    int border = 20;
    float minX = points[0].x, minY = points[0].y, maxX = points[0].x, maxY = points[0].y;
    for (auto& p: points) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }
    float scaleX = (wndSizeX - 2*border) /  (maxX - minX);
    float scaleY = (wndSizeY - 2*border) / (maxY - minY);
    float offsetX = -minX * scaleX + border;
    float offsetY = -minY * scaleY + border;
    return {scaleX, scaleY, offsetX, offsetY};
}


int main() {
    FsOpenWindow(16,16,800,600,1);
    auto parsedLines = parsePathData(SAMPLE);
    auto [scaleX, scaleY, offsetX, offsetY] = calculateOffsets(parsedLines, 800, 600);
    for (auto& p: parsedLines) {
        p.x = p.x * scaleX + offsetX;
        p.y = p.y * scaleY + offsetY;
    }
    FsPollDevice();
    while(FSKEY_ESC!=FsInkey()) {
        FsPollDevice();
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glDrawWireframeArt(parsedLines);
//        glDrawWireframeArt({{0, 0}, {800, 0}, {800, 600}, {0, 600}, {0, 0}});
        FsSwapBuffers();
        FsSleep(25);
    }
    return 0;
}
