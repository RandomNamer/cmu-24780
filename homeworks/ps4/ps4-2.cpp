//
// Created by Zeyu Zhang on 9/24/24.
//

//#define RESTART_INSTEAD_OF_QUIT
#define FPS 40
//#define USE_VSYNC //If defined, the FPS should be set to vsync freq.


#ifdef _WIN32
#define M_PI 3.1415926
#endif 

#include <iostream>
#include <utility>
#include <algorithm>
#include <array>
#include <vector>
#include "time.h"
#include "fssimplewindow.h"
#include <cmath>
#include <chrono>
#include <cstdlib>
using namespace std;

//using Point2i = std::array<int, 2>;
//using Point2f = std::array<float, 2>;

//This is how bidrectional implicit conversion works
struct Point2i;
struct Point2f {
    float x;
    float y;
    operator Point2i() const;
};

struct Point2i {
    int x;
    int y;

    operator Point2f() const {
        return {static_cast<float>(x), static_cast<float>(y)};
    }
};

Point2f::operator Point2i()  const {
    return {static_cast<int>(round(x)), static_cast<int>(round(y))};
}


struct Color3i {
    int r,g,b;

};

struct RectF {
    Point2f topLeft;
    Point2f bottomRight;

    std::array<float, 4> getBounds() const {
        return {topLeft.x, bottomRight.x, topLeft.y, bottomRight.y};
    }
};

void drawLine(Point2f start, Point2f end, Color3i color={0, 0, 0}) {
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_LINES);
    glVertex2i(start.x, start.y);
    glVertex2i(end.x, end.y);
    glEnd();
}

void drawRectFilled(Point2f topLeft, Point2f bottomRight, Color3i color) {
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_QUADS);
    glVertex2i(topLeft.x, topLeft.y);
    glVertex2i(bottomRight.x, topLeft.y);
    glVertex2i(bottomRight.x, bottomRight.y);
    glVertex2i(topLeft.x, bottomRight.y);
    glEnd();
}

void drawCircle(Point2f center, int radius, Color3i color) {
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 360; ++i) {
        float angle = i * M_PI / 180.0f;
        glVertex2i(center.x + radius * cos(angle), center.y + radius * sin(angle));
    }
    glEnd();
}

void drawLineStrip(const std::vector<Point2f>& points, Color3i color={0, 0, 0}) {
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_LINE_STRIP);
    for (const auto& point : points) {
        glVertex2i(point.x, point.y);
    }
    glEnd();
}

void drawTarget(Point2f center, int size=20) {
    drawRectFilled({center.x - size, center.y - size}, {center.x + size, center.y + size}, {255, 0, 0});
}

void drawObstacle(Point2f center, int size=20, bool isHit=false) {
    int thickness = 10;
    Color3i color = isHit ? Color3i{0, 255, 0} : Color3i{0, 255, 255};
    drawRectFilled({center.x - size, center.y - size}, {center.x + size, center.y + size}, color);
}

struct Obstacle {
    RectF rect;
    int width, height;
    int hitsRemaining;
};



class CannonBallGame {
public:

    struct CannonConfig {
        float shieldSize;
        float barrelLength;
        float calibre;
    };

    Point2f cannonPos;
    int cannonAngle;
    bool isFiring;
    Point2f cannonBallPos;
    Point2f cannonBallVel;
    std::vector<Point2f> cannonBallTrail;
    std::vector<Obstacle> obstacles;
    Point2f targetPos;
    int targetVel;
    int shotsFired;
    float G;
    float initialVelocity;
    float canvasScaleX;
    float canvasScaleY;
    constexpr const static float worldWidth = 80.0f;
    constexpr const static float worldHeight = 60.0f;
    CannonConfig cannon = {2, 5, 0.1};
    Point2f barrelEnd;

    CannonBallGame(float g = 9.8f, float velocity = 40.0f, int canvasWidth = 800, int canvasHeight = 600)
            : G(g), initialVelocity(velocity), canvasScaleX(static_cast<float>(canvasWidth) / worldWidth), canvasScaleY(static_cast<float>(canvasHeight) / worldHeight) {
        cannonPos = {3.f, worldHeight - 6.f};
        updateCannonAngle(45);
        isFiring = false;
        shotsFired = 0;
        targetPos = {worldWidth - 5.0f, 5.0f};
        targetVel = 10;
        srand(time(nullptr));
        initializeObstacles();
    }

    void initializeObstacles() {
        obstacles.clear();
        for (int i = 0; i < 5; ++i) {
            int width, height;
            Point2f topLeft, bottomRight;
            width = 8 + rand() % 8; // Width between 8m and 15m
            height = 8 + rand() % 8; // Height between 8m and 15m
            // in range (20, worldWidth - 10) and (20, worldHeight - 10), thus ensured not too close to the edge
            topLeft = Point2i{20 + rand() % static_cast<int>(worldWidth - 30),
                              20 + rand() % static_cast<int>(worldHeight - 30)};
            bottomRight = {topLeft.x + width, topLeft.y + height};
            obstacles.push_back({{topLeft, bottomRight}, width, height, 2});
        }
    }

    bool doFrame(int key, int frameTimeMs) {
        handleInput(key);
        bool gameEnded = updateGame(static_cast<float>(frameTimeMs) / 1000.0f);
        drawGame();
        return gameEnded;
    }

    void handleInput(int key) {
        if (key == FSKEY_UP && cannonAngle < 90) {
            updateCannonAngle(cannonAngle + 3);
        } else if (key == FSKEY_DOWN && cannonAngle > 0) {
            updateCannonAngle(cannonAngle - 3);
        } else if (key == FSKEY_SPACE && !isFiring) {
            isFiring = true;
            float rad = cannonAngle * M_PI / 180.0f;
            cannonBallVel = {initialVelocity * cos(rad), -initialVelocity * sin(rad)};
            cannonBallPos = barrelEnd;
            shotsFired++;
        }
    }

    bool updateGame(float frameTime) {
        if (isFiring) {
            cannonBallVel.y += G * frameTime;
            cannonBallPos.x += cannonBallVel.x * frameTime;
            cannonBallPos.y += cannonBallVel.y * frameTime;
            cannonBallTrail.push_back(cannonBallPos);
            if (cannonBallTrail.size() > 10) {
                cannonBallTrail.erase(cannonBallTrail.begin());
            }
            for (auto& obs : obstacles) {
                if (obs.hitsRemaining > 0 && hit(obs.rect)) {
                    obs.hitsRemaining--;
                    removeCannonBall();
                }
            }
            if (cannonBallPos.x < 0 || cannonBallPos.x > worldWidth || cannonBallPos.y > worldHeight || cannonBallPos.y < 0) {
                //missed
                removeCannonBall();
            }

            //target Hit detection:
            if (hit(getCurrentTargetRect())) {
                return true;
            }
        }
        targetPos.y += targetVel * frameTime;
        if (targetPos.y + 5.0f > worldHeight || targetPos.y < 5.0f) {
            //reverse direction
            targetVel = -targetVel;
        }
        return false;
    }

    void drawGame() {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        drawCannon(cannonPos,cannonAngle);
        drawObstacles();
        drawTarget();
        if (isFiring) {
            drawCannonBall();
        }
        FsSwapBuffers();
    }

private:

    bool hit(RectF rect) {
        auto b = rect.getBounds();
        return cannonBallPos.x > b[0] && cannonBallPos.x < b[1] && cannonBallPos.y > b[2] && cannonBallPos.y < b[3];
    }

    void removeCannonBall() {
        isFiring = false;
        cannonBallTrail.clear();
    }

    void updateCannonAngle(int deg) {
        cannonAngle = deg;
        updateBarrelEnd();
    }

    void updateBarrelEnd() {
        float rad = static_cast<float>(cannonAngle) * M_PI / 180.0f;
        barrelEnd = {cannonPos.x + cannon.barrelLength * cos(rad) + cannon.shieldSize / 2.f, cannonPos.y - cannon.barrelLength * sin(rad) + cannon.shieldSize / 2.f};
    }

    Point2f worldToCanvas(const Point2f& world) {
        return {world.x * canvasScaleX, world.y * canvasScaleY};
    }

    void drawCannon(Point2f pos, int degree=0) {
        // auto [x, y] = pos;
        float x = pos.x;
        float y = pos.y;
        // auto [shieldSize, barrelLength, calibre] = cannon;
        float shieldSize = cannon.shieldSize;
        float barrelLength = cannon.barrelLength;
        float calibre = cannon.calibre;
        //Black barrel, rotatable
//        drawRectFilled(worldToCanvas({x - calibre, y - calibre}), worldToCanvas({x + barrelEndX + calibre, y + barrelEndY + calibre}), {0, 0, 0});
        glLineWidth(calibre * canvasScaleX);
        drawLine(
                worldToCanvas({x + shieldSize / 2, y + shieldSize / 2}),
                worldToCanvas(barrelEnd),
                {0, 0, 0}
                );
        // Blue color for the square
        drawRectFilled(worldToCanvas(pos), worldToCanvas({x + shieldSize, y + shieldSize}), {0, 0, 255});
    }

    void drawCannonBall(int size=1) {
        size *= canvasScaleX;
        Color3i color;
        switch (shotsFired) {
            case 1: color = {0, 0, 255}; break;
            case 2: color = {0, 255, 255}; break;
            case 3: color = {255, 255, 0}; break;
            case 4: color = {255, 0, 255}; break;
            default: color = {255, 0, 0}; break;
        }

        drawCircle(worldToCanvas(cannonBallPos), size, color);

        std::vector<Point2f> trailInCanvas(cannonBallTrail.size());
        std::transform(cannonBallTrail.begin(), cannonBallTrail.end(), trailInCanvas.begin(), [&] (auto& worldCoord) {
            return worldToCanvas(worldCoord);
        } );

        glLineWidth(1);
        drawLineStrip(trailInCanvas, color);
    }

    void drawObstacles() {
        for (const auto& obs : obstacles) {
            auto color = obs.hitsRemaining == 2 ?  Color3i{0, 255, 0} : Color3i{255, 255, 0};
            if (obs.hitsRemaining > 0) {
                drawRectFilled(worldToCanvas(obs.rect.topLeft), worldToCanvas(obs.rect.bottomRight), color);
            }
        }
    }

    RectF getCurrentTargetRect() {
        return RectF{Point2f{targetPos.x - 5.0f, targetPos.y - 5.0f}, Point2f{targetPos.x + 5.0f, targetPos.y + 5.0f}};
    }

    void drawTarget() {
        auto targetRect = getCurrentTargetRect();
        drawRectFilled(worldToCanvas(targetRect.topLeft), worldToCanvas(targetRect.bottomRight), {255, 0, 0});
        // drawRectFilled(worldToCanvas(lt), worldToCanvas(rb), {255, 0, 0});
    }

};

int main() {
    FsOpenWindow(16,16,800,600,1);
    auto game = CannonBallGame();
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    FsPollDevice();
    int targetFrameTimeMs = 1000 / FPS;
    int lastFrameTimeMs = 0;
    for(;;) {
        FsPollDevice();
        int key = FsInkey();
        // cout<<key<<endl;
        if (key == FSKEY_1) {
            game = CannonBallGame();
        } else if (key == FSKEY_ESC) {
            break;
        }
        auto start = chrono::high_resolution_clock::now();
        bool gameEnded = game.doFrame(key, targetFrameTimeMs);
        lastFrameTimeMs = round(chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count() / 1000.f);
        cout << "Game do frame with input " << key << ", frameTime: " << lastFrameTimeMs << "ms."<<endl;
        if (gameEnded) {
#ifdef RESTART_INSTEAD_OF_QUIT
            game = CannonBallGame();
#else
            break;
#endif
        }
#ifndef USE_VSYNC
        FsSleep(std::max(1, targetFrameTimeMs - lastFrameTimeMs - 1));
#endif
    }

}