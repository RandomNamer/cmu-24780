#include <iostream>
#include <math.h>
#include "fssimplewindow.h"

#define DRAW_TEST

const double tolerance=1e-6;

const int canvasH = 600;
const int canvasW = 600;

class Equation {
public:
    double a, b, c;

    Equation(double a, double b, double c) : a(a), b(b), c(c) {}

    void Plot(float R, float G, float B, int range = 30) const {
        glColor3f(R, G, B);
        glLineWidth(2);
        glBegin(GL_LINES);

        if (b != 0) {
            // y = (-a/b)x + (c/b)
            double y1 = (-a / b) * -range + (c / b);
            double y2 = (-a / b) * range + (c / b);
            glVertex2f(-1.f, y1 / range);
            glVertex2f(1.f, y2 / range);
        } else if (a != 0) {
            // x = c/a
            double x = c / a;
            glVertex2f(x / range, -1.f);
            glVertex2f(x / range, 1.f);
        }

        glEnd();
    }
};

class SimultaneousEquation {
public:
    Equation eqn[2];

    SimultaneousEquation(const Equation& eq1, const Equation& eq2) : eqn{eq1, eq2} {}

    SimultaneousEquation(): eqn{Equation(0, 0, 0), Equation(0, 0, 0)} {}

    bool Solve(double& x, double& y) const {
        double a1 = eqn[0].a, b1 = eqn[0].b, c1 = eqn[0].c;
        double a2 = eqn[1].a, b2 = eqn[1].b, c2 = eqn[1].c;

        double D = a1 * b2 - b1 * a2;
        if (std::fabs(D) < 1e-6) {
            return false;
        }

        x = (c1 * b2 - b1 * c2) / D;
        y = (a1 * c2 - c1 * a2) / D;
        return true;
    }

    void Plot() const {
        eqn[0].Plot(1.0f, 0.0f, 0.0f); // Red
        eqn[1].Plot(0.0f, 0.0f, 1.0f); // Blue
    }
};

class Axes {
public:
    int density = 30;

    Axes(int d): density(d) {}

    void Draw() const {
        drawAxes();
        drawGrid(density);
    }
private:
    void drawAxes() const {
        glColor3ub(0, 0, 0);
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex2f(-1.f, 0.f);
        glVertex2f(1.f, 0.f);
        glVertex2f(0.f, -1.f);
        glVertex2f(0.f, 1.f);
        glEnd();
    }

    void drawGrid(int density) const {
        glLineWidth(1);
        for (int i = -density; i <= density; ++i) {
            if (i == 0) {
                continue;
            }
            glColor3f(0.7, 0.7, 0.7);
            glBegin(GL_LINES);
            glVertex2f(-1.f, i / static_cast<float>(density));
            glVertex2f(1.f, i / static_cast<float>(density));
            glVertex2f(i / static_cast<float>(density), -1.f);
            glVertex2f(i / static_cast<float>(density), 1.f);
            glEnd();
        }
    }

};
int main(void) {
    SimultaneousEquation eqn;
    double x, y;
#ifndef DRAW_TEST
    std::cout << "ax+by=c\n";
    std::cout << "dx+ey=f\n";
    std::cout << "Enter a b c d e f:";
    std::cin >>
             eqn.eqn[0].a >> eqn.eqn[0].b >> eqn.eqn[0].c >>
             eqn.eqn[1].a >> eqn.eqn[1].b >> eqn.eqn[1].c;
    if (true == eqn.Solve(x, y)) {
        std::cout << "x=" << x << " y=" << y << '\n';
    } else {
        std::cout << "No solution.\n";
    }
#else
    eqn = SimultaneousEquation(Equation(1, 1, 0), Equation(-1, 1, 4));
#endif
    Axes axes(30);
    FsOpenWindow(0, 0, canvasH, canvasW, 1);
    for (;;) {
        FsPollDevice();
        if (FSKEY_ESC == FsInkey()) {
            break;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //gl y coord is the opposite of the canvas y coord
        glScalef(canvasW / 2.f, -canvasH / 2.f, 1.f);
        glTranslatef(1.f, -1.f, 0.f);

        axes.Draw();
        eqn.Plot();
        FsSwapBuffers();
    }
    return 0;
}
