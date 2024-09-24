//
// Created by Zeyu Zhang on 9/11/24.
//

#include <iostream>
#include <utility>
#include <array>
#include <exception>
using namespace std;

const int ALT_AXIS[7] = {0, 2000, 4000, 6000, 8000, 10000, 12000};
const int TEMP_AXIS[4] = {-20, 0, 20, 40};
const int ROC_VALUES[7][4] = {
        {830, 770, 705, 640},
        {720, 655, 595, 535},
        {645, 685, 525, 465},
        {530, 475, 425, 360},
        {420, 365, 210, 250},
        {310, 255, 200, 145},
        {200, 145, 0, 0}
};


struct Point3d{
    double x,y,z;
};
struct Point3i{
    int x,y,z;
    operator Point3d() const {
        return {static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)};
    }
};

double bilinearInterpolate(int x, int y, Point3d p00, Point3d p01, Point3d p10, Point3d p11) {
    double xw = (x - p00.x) / (p01.x - p00.x);
    double yw = (y - p00.y) / (p10.y - p00.y);
    return (1-xw) * (1-yw) * p00.z + (1-xw) * yw * p10.z + xw * (1-yw) * p01.z + xw * yw * p11.z;
}

template<size_t XLENGTH, size_t YLENGTH>
array<Point3i, 4> search4AdjPoints(int x, int y, const int (& xAxis)[XLENGTH], const int (& yAxis)[YLENGTH], const int (& values)[XLENGTH][YLENGTH]) {
    int xIdx, yIdx = -1;
    for (int i=0; i < XLENGTH; i++) {
        if (xAxis[i] >= x) { xIdx = i; break; }
    }
    for (int i=0; i < YLENGTH; i++) {
        if (yAxis[i] >= y) {yIdx = i; break; }
    }
    if ( xIdx > 0 && yIdx > 0 && xIdx < XLENGTH && yIdx < YLENGTH) {
        //Valid
        return {{
                {xAxis[xIdx-1], yAxis[yIdx-1], values[xIdx-1][yIdx-1]},
                {xAxis[xIdx], yAxis[yIdx-1], values[xIdx][yIdx-1]},
                {xAxis[xIdx-1], yAxis[yIdx], values[xIdx-1][yIdx]},
                {xAxis[xIdx], yAxis[yIdx], values[xIdx][yIdx]}
            }};
    } else {
        throw std::out_of_range("Invalid input");
        return {{
                {0, 0, 0},
                {0, 0, 0},
                {0, 0, 0},
                {0, 0, 0}
            }};
    }
}

int main() {
    int alt, temp;
    START:
    cout<<"Enter Altitude and Temperature: ";
    cin>>alt>>temp;
    if (alt < 0 || alt > 10000 || temp < -20 || temp > 40) {
        cout<<"Re-enter the values";
        goto START;
    }
    auto adjPoints = search4AdjPoints<7,4>(alt, temp, ALT_AXIS, TEMP_AXIS, ROC_VALUES);
    auto roc = bilinearInterpolate(alt, temp, adjPoints[0], adjPoints[1], adjPoints[2], adjPoints[3]);
    cout<<"Expected Climb Rate="<<roc<<"ft/min"<<endl;
}
