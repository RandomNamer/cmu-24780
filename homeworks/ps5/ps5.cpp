//
// Created by Zeyu Zhang on 9/30/24.
//

#include "fssimplewindow.h"
#include "yssimplesound.h"
#include "ysglfontdata.h"
#include <cmath>
#include "stdlib.h"

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <queue>
#include <array>
#include <algorithm>



using namespace std;

const auto RES_DIR = R"(W:\Sync\Courses\24 Fall\24780\cmu-24780\homeworks\ps5\res\)"s;

#ifdef _WIN32
#define M_PI 3.1415926
#endif

struct MouseEvent {
    int lb;
    int mb;
    int rb;
    int mx;
    int my;
    int eventType;
};

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

struct Rect {
    Point2i topLeft;
    Point2i bottomRight;

    std::array<int, 4> getBounds() const {
        return {topLeft.x, bottomRight.x, topLeft.y, bottomRight.y};
    }
};

void drawCircle(Point2f center, int radius, Color3i color, unsigned int alpha=255) {
    glColor4ub(color.r, color.g, color.b, alpha);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 360; ++i) {
        float angle = i * M_PI / 180.0f;
        glVertex2i(center.x + radius * cos(angle), center.y + radius * sin(angle));
    }
    glEnd();
}

void drawCircleHollowed(Point2f center, int radius, int thickness, Color3i color, unsigned int alpha=255, int segments=100) {
    glLineWidth(thickness);
    glColor4ub(color.r, color.g, color.b, alpha);
    glBegin(GL_LINE_LOOP);  // Use GL_LINE_LOOP to create a non-filled circle
    for (int i = 0; i < segments; ++i) {
        // Calculate the angle for each point
        float theta = 2.0f * M_PI * float(i) / float(segments);

        // Calculate the x and y coordinates based on the center and radius
        float x = radius * cosf(theta);  // X coordinate
        float y = radius * sinf(theta);  // Y coordinate

        // Specify the vertex on the circumference
        glVertex2f(center.x + x, center.y + y);
    }
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

void drawLine(Point2f start, Point2f end, Color3i color={0, 0, 0}) {
    glLineWidth(2);
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_LINES);
    glVertex2i(start.x, start.y);
    glVertex2i(end.x, end.y);
    glEnd();
}

void drawLineStrip(const std::vector<Point2f>& points, Color3i color={0, 0, 0}) {
    glLineWidth(2);
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_LINE_STRIP);
    for (const auto& point : points) {
        glVertex2i(point.x, point.y);
    }
    glEnd();
}


struct WeaponConfig {
    string name;
    int fireIntervalMs;
    //No difference
    int reloadTimeMs;
    int magSize;
    //aim down sight
    float adsFovFactor;
    int adsTimeMs;
    //gun kick is in gunkickTime scope size from 1x to gunkickViewMul then back to 1x
    float gunkickViewMul;
    int gunkickTimeMs;
    //We assume only headshot and torso shot, legs are actually lines
    int damage;
    float hsMultiplier;
    float projectileSpeed;
};

struct GameConfig {
    vector<WeaponConfig> weapons;
    //random target spawn
    int targetCount;
    int targetDistanceMax = 10;
    float targetSpeedMin = 20;
    float targetSpeedMax = 100;
    float mouseSensitivity;
    float adsSenstivityMultiplier = 2.f;
    //By default gunkick is only scope magnification
    bool gunkickUseFovChange = false;
//    int fov = 120;
};

const WeaponConfig AK = WeaponConfig{
    "Kastov 762",
    100,
    2550,
    30,
    1.25,
    260,
    1.05,
    150,
    39,
    1.3,
    800
};

const WeaponConfig M200 = WeaponConfig {
    "FJX Imperium",
    1300,
    4500,
    5,
    6,
    580,
    1.2,
    200,
    150,
    3.3,
    1000
};

const WeaponConfig M249 = WeaponConfig {
    "Bruen Mk9",
    70,
    5800,
    200,
    1.25,
    350,
    1.03,
    100,
    35,
    1.2,
    800

};

const vector<WeaponConfig> weaponList = {AK, M200, M249};

//Snipe against multiple targets
//TODO: hip fire spread; wasd move; bullet drop; more sound fx; more resolution support
class CodParodyGame {
public:

    struct Target {
        float distance;
        int x;
        int y;
        float moveSpeed;
        int health = 150;
    };

    enum EventType {
        ADS_EXIT_COMPLETED,
        GUNKICK_ANIM_FINISHED,
//        PLAY_HITMARKER,
        HITMARKER_FINISHED,
//        PLAY_FINISHING_SOUND,
        FINISHING_SOUND_FINISHED,
        RELOAD,
        RELOAD_FINISHED,
        FIRE_READY,
        UPDATE_CENTER,
        FIRE,
    };

    //Everything related to time
    struct Event {
        int time;
        EventType type;
        vector<void*> arguments = {};

        bool operator< (const Event& other) const {
            return time > other.time;
        }
    };

    //states
    int currentViewPortWidth = 800;
    int currentViewPortHeight = 600;
    Rect currentViewPort = {{800, 600}, {1600, 1200}};
    int currentCenterX = currentViewPortWidth / 2 + 800;
    int currentCenterY = currentViewPortHeight / 2 + 600;
    float currentFovFactor = 1;
    float currentProjectileDistance = 0;
    bool isFiring = false;
    //is in ADS
    bool isAiming = false;
    bool isAdsExiting = false;
    bool isReloading = false;
    bool isGunkickInEffect = false;

    //Current frame props
    bool shouldDisplayHitmarker = false;
    bool isHitmarkerHeadshot = false;
    bool isHitmarkerFromKill = false;
    vector<int> damageList;

    int remainingBullets = 0;
    vector<Target> targets;
     
    WeaponConfig currentWeapon;
    int currentWeaponIdx = 0;
//    int BASE_FOV = 120;
    int baseHealth = 150;
    int currentGameDurationMs = 0;
    float currentAdsPercentage = 0;
    int adsChangeStartTime = 0;
    int gunkickStartTime = 0;
    float mouseSensitivity = 1;


    //For all usages that are rare
    const GameConfig rawConfig;

    priority_queue<Event> eventQueue;
    YsSoundPlayer player;
    YsSoundPlayer::SoundData hitmarkerSound;
    YsSoundPlayer::SoundData finishingSound1;
    YsSoundPlayer::SoundData finishingSound2;


    CodParodyGame(GameConfig config): rawConfig(config), mouseSensitivity(config.mouseSensitivity) {
        srand(time(nullptr));
        spawnTargets(config.targetCount, config.targetDistanceMax, config.targetSpeedMin, config.targetSpeedMax);
        setWeapon(0);
        if (YSOK != hitmarkerSound.LoadWav((RES_DIR + "hitmarker.wav").c_str())) {
            cout << "Failed to load hitmarker sound" << endl;
        }
        if (YSOK != finishingSound1.LoadWav((RES_DIR + "finisher_01.wav").c_str())) {
            cout << "Failed to load finishing sound 1" << endl;
        }
        if (YSOK != finishingSound2.LoadWav((RES_DIR + "finisher_02.wav").c_str())) {
            cout << "Failed to load finishing sound 2" << endl;
        }
        player.Start();
        playFinishingSound(true);
        FsSleep(1000);
    }

    bool doFrame(MouseEvent mev, int kev, int frameTimeMs) {
        handleInput(mev, kev);
        bool gameEnded = update(frameTimeMs);
        draw();
        return gameEnded;
    }

private:

    enum HigPriorityEventIndex {
        IDX_AIM,
        IDX_FIRE
    };

    enum HitType {
        HEADSHOT,
        TORSO,
        NO_HIT
    };

    void handleInput(MouseEvent mev, int kev) {
        if (mev.rb) {
            if (!isAiming) {
                isAiming = true;
                isAdsExiting = false;
                adsChangeStartTime = currentGameDurationMs;
            }
        } else {
            if (isAiming && !isAdsExiting) {
                eventQueue.push({currentGameDurationMs + currentWeapon.adsTimeMs, ADS_EXIT_COMPLETED});
                //Assume previous ads is not when exiting
                float currentAdsPercentage = interpolateAds();
                adsChangeStartTime = currentGameDurationMs - static_cast<int>(currentWeapon.adsTimeMs * (1.f - currentAdsPercentage));
                isAdsExiting = true;
            }
        }

        if (mev.lb) {
            if (!isFiring && !isReloading && remainingBullets > 0) {
                fireCurrentWeapon();
            }
        }

        if (kev == FSKEY_R) {
            reload();
        }

        if (kev == FSKEY_1) {
            setWeapon(++currentWeaponIdx);
        }

        eventQueue.push(Event { IDX_AIM, UPDATE_CENTER, {&mev}});
    }

    bool update(int frameTimeMs) {

        currentGameDurationMs += frameTimeMs;

        updateFov();

        while (!eventQueue.empty() && eventQueue.top().time <= currentGameDurationMs) {
            Event event = eventQueue.top();
            eventQueue.pop();
            switch (event.type) {
                case ADS_EXIT_COMPLETED:
                    isAiming = false;
                    isAdsExiting = false;
                    break;
                case GUNKICK_ANIM_FINISHED:
                    isGunkickInEffect = false;
                    break;
                case FIRE_READY:
                    isFiring = false;
                    break;
                case RELOAD_FINISHED:
                    isReloading = false;
                    remainingBullets = currentWeapon.magSize;
                    break;
                case HITMARKER_FINISHED:
                    // Hide hitmarker
                    shouldDisplayHitmarker = false;
                    // player.Stop(hitmarkerSound);
                    break;
                case FINISHING_SOUND_FINISHED:
                    // Stop finishing sound
                    shouldDisplayHitmarker = false;
                    // player.Stop(finishingSound1);
                    // player.Stop(finishingSound2);
                    break;
                case UPDATE_CENTER:
                    updateCenter(*static_cast<MouseEvent*>(event.arguments[0]));
                    break;
                case FIRE:
                    // Fire bullet
                    handleAfterFireEvent();
                    break;
            }
        }

        bool hasAliveTarget = false;

        for (auto &target : targets) {
            if (target.health > 0) {
                hasAliveTarget = true;
            }
            target.x += target.moveSpeed * frameTimeMs / 1000.0;
            if (target.x < 0 || target.x > 2400) {
                target.moveSpeed = -target.moveSpeed;
            }
        }

        return !hasAliveTarget;
    }

    void draw() {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        for (auto &target : targets) {
            if (target.health > 0) drawTarget(target);
        }

        if (isAiming) drawScope(interpolateAds());
        else drawCrosshair(); // hip fire
//        drawProjectile();
        if (shouldDisplayHitmarker) {
            drawHitmarker(isHitmarkerHeadshot, isHitmarkerFromKill ? Color3i{255, 0, 0} : Color3i{0, 0, 0});
        }

        drawUI();
        FsSwapBuffers();
    }

    void updateFov() {
//        if (!isAiming) return;
        // Calculate current FOV
        float adsFovFactor = isAiming ? evaluateAdsFovFactor() : 1.0f;
        float gunkickFovFactor = isGunkickInEffect && rawConfig.gunkickUseFovChange ? evaluateGunkickFovFactor(): 1.0f;
        currentFovFactor = adsFovFactor * gunkickFovFactor;

        // Update viewport based on current FOV
        currentViewPortWidth = static_cast<int>(800 / currentFovFactor);
        currentViewPortHeight = static_cast<int>(600 / currentFovFactor);
        currentViewPort = {{currentCenterX - currentViewPortWidth / 2, currentCenterY - currentViewPortHeight / 2},
                           {currentCenterX + currentViewPortWidth / 2, currentCenterY + currentViewPortHeight / 2}};
    }

    void updateCenter(const MouseEvent &mev) {
        currentCenterX += round(static_cast<float>(mev.mx) * mouseSensitivity / currentFovFactor);
        currentCenterY += round(static_cast<float>(mev.my) * mouseSensitivity / currentFovFactor);

        currentCenterX = std::max(currentViewPortWidth / 2, std::min(currentCenterX, 2400 - currentViewPortWidth / 2));
        currentCenterY = std::max(currentViewPortHeight / 2, std::min(currentCenterY, 1800 - currentViewPortHeight / 2));
    }

    void fireCurrentWeapon() {
        isFiring = true;
        remainingBullets--;
        if (remainingBullets == 0) reload();

        float currentGunkickProgress = interpolateGunkick();
        if (currentGunkickProgress > 0.5) currentGunkickProgress = 1 - currentGunkickProgress;
        //consistent gunkick
        gunkickStartTime = currentGameDurationMs - static_cast<int>(currentWeapon.gunkickTimeMs * currentGunkickProgress);
        isGunkickInEffect = true;
        tryRemoveNextEventOfType(GUNKICK_ANIM_FINISHED);
        eventQueue.push(Event {currentGameDurationMs + currentWeapon.gunkickTimeMs, GUNKICK_ANIM_FINISHED});

        // post fire logic
        eventQueue.push(Event {IDX_FIRE, FIRE});
        eventQueue.push(Event{currentGameDurationMs + currentWeapon.fireIntervalMs, FIRE_READY});
    }

    bool tryRemoveNextEventOfType(EventType type) {
        vector<Event> removed;
        bool found = false;
        while (!eventQueue.empty()) {
            if(eventQueue.top().type == type) {
                eventQueue.pop();
                found = true;
            } else {
                removed.push_back(eventQueue.top());
                eventQueue.pop();
            }
        }
        if (!removed.empty()) {
            for (auto &event : removed) {
                eventQueue.push(event);
            }
        }
        return false;
    }

    void enqueueSingletonEvent(Event ev) {
        while(tryRemoveNextEventOfType(ev.type)) {}
        eventQueue.push(ev);
    }

    // Use the current center as the projectile's position
    void handleAfterFireEvent() {
        damageList.clear();
        for (auto &target : targets) {
            // Check if the projectile intersects with the target
            if (target.health <= 0 ) continue;
            auto hitType = getHitType(target);
            if (hitType != NO_HIT) {
                onNewHit(target, hitType == HEADSHOT);
                // break; // Assuming one hit per shot
            }
        }
    }

    // Destructuring does not work in C++ 11
    inline bool hit(Rect rect, Point2i point) {
        // auto [x1, x2, y1, y2] = rect.getBounds();
        // auto [x, y] = point;
        // return x > x1 && x < x2  && y > y1 && y < y2;
        auto bounds = rect.getBounds();
        return point.x > bounds[0] && point.x < bounds[1] && point.y > bounds[2] && point.y < bounds[3];
    }

    HitType getHitType(const Target &target) {
        // Simple bounding box collision detection
        auto r = 100.f / target.distance * currentFovFactor;
        auto viewportX = (target.x - currentCenterX) * currentFovFactor + 400;
        auto viewportY = (target.y - currentCenterY) * currentFovFactor + 300;

        // Draw the target as a rectangle
        auto torsoHitBox = Rect{Point2f{viewportX - r / 2.f, viewportY - r}, Point2f{viewportX + r / 2.f, viewportY + r}};

        auto headHitBox = Rect{Point2f{viewportX - r / 2.f, viewportY - r - r}, Point2f{viewportX + r / 2.f, viewportY - r}};

        if (hit(torsoHitBox, {400, 300})) {
            return TORSO;
        } else if (hit(headHitBox, {400, 300})) {
            return HEADSHOT;
        } else {
            return NO_HIT;
        }
    }

    void onNewHit(Target& target, bool isHeadShot) {
        int damage = currentWeapon.damage * (isHeadShot ? currentWeapon.hsMultiplier : 1);
        target.health -= damage;
        if (target.health <= 0) {
            //Dead
            playFinishingSound(isHeadShot);
            isHitmarkerFromKill = true;
            enqueueSingletonEvent(Event {currentGameDurationMs + 125, FINISHING_SOUND_FINISHED});
        } else {
            playHitmarkerSound();
            isHitmarkerFromKill = false;
        }
        shouldDisplayHitmarker = true;
        isHitmarkerHeadshot = isHeadShot;
        enqueueSingletonEvent(Event {currentGameDurationMs + 67, HITMARKER_FINISHED});
        damageList.emplace_back(damage);
    }

    void setWeapon(int idx) {
        idx = idx % weaponList.size();
        currentWeapon = weaponList[idx];
        currentWeaponIdx = idx;
        remainingBullets = currentWeapon.magSize;
        isAiming = false;
        updateFov();
    }

    //All linear interpolation
    float interpolateAds() {
        int elapsedTime = currentGameDurationMs - adsChangeStartTime;
        float adsTime = static_cast<float>(currentWeapon.adsTimeMs);
        float adsPercentage = std::min(1.0f,std::max(0.0f, static_cast<float>(elapsedTime) / adsTime));
        return adsPercentage;
    }

    float evaluateAdsFovFactor() {
        float adsPercentage = interpolateAds();
        if (isAdsExiting) {
            adsPercentage = 1.0f - adsPercentage;
        }
        return 1.0f + (currentWeapon.adsFovFactor - 1.0f) * adsPercentage;
    }

    //Coerce gunkick to 0-1
    float interpolateGunkick() {
        if (!isGunkickInEffect) return 0;
        int elapsedTime = currentGameDurationMs - gunkickStartTime;
        float gunkickTime = static_cast<float>(currentWeapon.gunkickTimeMs);
        float gunkickPercentage = std::min(1.0f, std::max(0.0f, static_cast<float>(elapsedTime) / gunkickTime));
        return gunkickPercentage;
    }

    float evaluateGunkickFovFactor() {
        float gunkickProgress = interpolateGunkick();
        if (gunkickProgress == 0.f  || gunkickProgress == 1.f)
            return 1.0f;

        if (gunkickProgress < 0.5) {
            return 1.0f + (currentWeapon.gunkickViewMul - 1.0f) * gunkickProgress * 2;
        } else {
            return 1.0f + (currentWeapon.gunkickViewMul - 1.0f) * (1 - gunkickProgress) * 2;
        }
    }

    void reload() {
        if (!isReloading) {
            isReloading = true;
            eventQueue.push(Event{ currentGameDurationMs + currentWeapon.reloadTimeMs, RELOAD_FINISHED });
        }
    }

    void spawnTargets(int count, int maxDistance, float minSpeed, float maxSpeed, bool staticTarget = false) {
        targets.clear();
        for (int i = 0; i < count; ++i) {
            float distance = static_cast<float>(rand() % maxDistance + 1);
            int x = rand() % (2400 - 400) + 200; // Ensure at least 200 units from the edges
            int y = rand() % (1800 - 400) + 200; // Ensure at least 200 units from the edges
            float moveSpeed = static_cast<float>(rand()) / RAND_MAX * (maxSpeed - minSpeed) + minSpeed;
            if (staticTarget) moveSpeed = 0;
            targets.push_back({distance, x, y, moveSpeed});
        }
    }

    void drawScope(float adsPercentage) {
        if (isAdsExiting) adsPercentage = 1.0f - adsPercentage;
        float gunkickFactor = evaluateGunkickFovFactor();
        Point2i center = {400, 300};
        // Draw a simple scope overlay
        int scopeRadius = static_cast<int>((120 * adsPercentage + 80) * gunkickFactor);
        drawCircleHollowed(Point2i{center.x, center.y }, scopeRadius, 3, {0, 0, 0}); // outer
//        drawCircle(Point2i{center.x, center.y }, scopeRadius - 5, {0, 0, 0}, 0); // inner
        drawLine(Point2i{center.x - scopeRadius, center.y}, Point2i{center.x + scopeRadius, center.y}, {0, 0, 0});
        drawLine(Point2i{center.x, center.y - scopeRadius}, Point2i{center.x, center.y + scopeRadius}, {0, 0, 0});
    }

//    void drawTarget(const Target& target) {
//        // Draw the target as a rectangle
//        auto r = 100.f / target.distance;
//        auto torsoHitBox = Rect{Point2f{target.x - r / 2.f, target.y - r}, Point2f{target.x + r / 2.f, target.y + r}};
//        drawRectFilled(torsoHitBox.topLeft, torsoHitBox.bottomRight, {30, 90, 0});
//        auto headCenter = Point2f{static_cast<float>(target.x), target.y - r - r / 2};
//        drawCircle(headCenter, static_cast<int>(r / 2), {180, 220, 0});
//
//
//        // Draw legs
//        Point2f leftLegStart = {target.x - r / 4.f, target.y + r};
//        Point2f leftLegEnd = {target.x - r / 4.f, target.y + r + r / 2.f};
//        Point2f rightLegStart = {target.x + r / 4.f, target.y + r};
//        Point2f rightLegEnd = {target.x + r / 4.f, target.y + r + r / 2.f};
//
//        drawLine(leftLegStart, leftLegEnd, {0, 0, 0});
//        drawLine(rightLegStart, rightLegEnd, {0, 0, 0});
//
//        // Draw arms
//        Point2f leftArmStart = {target.x - r / 2.f, target.y - r / 2.f};
//        Point2f leftArmEnd = {target.x - r, target.y - r / 2.f};
//        Point2f rightArmStart = {target.x + r / 2.f, target.y - r / 2.f};
//        Point2f rightArmEnd = {target.x + r, target.y - r / 2.f};
//
//        drawLine(leftArmStart, leftArmEnd, {0, 0, 0});
//        drawLine(rightArmStart, rightArmEnd, {0, 0, 0});
//    }

    void drawTarget(const Target& target) {
        // Transform world coordinates to viewport coordinates
        auto r = 100.f / target.distance * currentFovFactor;
        auto viewportX = (target.x - currentCenterX) * currentFovFactor + 400;
        auto viewportY = (target.y - currentCenterY) * currentFovFactor + 300;

        // Draw the target as a rectangle
        auto torsoHitBox = Rect{Point2f{viewportX - r / 2.f, viewportY - r}, Point2f{viewportX + r / 2.f, viewportY + r}};
        drawRectFilled(torsoHitBox.topLeft, torsoHitBox.bottomRight, {30, 90, 0});
        auto headCenter = Point2f{viewportX, viewportY - r - r / 2};
        drawCircle(headCenter, static_cast<int>(r / 2), {180, 220, 0});

        // Draw legs
        Point2f leftLegStart = {viewportX - r / 4.f, viewportY + r};
        Point2f leftLegEnd = {viewportX - r / 4.f, viewportY + r + r / 2.f};
        Point2f rightLegStart = {viewportX + r / 4.f, viewportY + r};
        Point2f rightLegEnd = {viewportX + r / 4.f, viewportY + r + r / 2.f};

        drawLine(leftLegStart, leftLegEnd, {0, 0, 0});
        drawLine(rightLegStart, rightLegEnd, {0, 0, 0});
			
        // Draw arms
        Point2f leftArmStart = {viewportX - r / 2.f, viewportY - r / 2.f};
        Point2f leftArmEnd = {viewportX - r, viewportY - r / 2.f};
        Point2f rightArmStart = {viewportX + r / 2.f, viewportY - r / 2.f};
        Point2f rightArmEnd = {viewportX + r, viewportY - r / 2.f};

        drawLine(leftArmStart, leftArmEnd, {0, 0, 0});
        drawLine(rightArmStart, rightArmEnd, {0, 0, 0});
    }

    void drawCrosshair() {
        // Draw a simple crosshair at the center of the screen
        int centerX = 400;
        int centerY = 300;
        int crosshairSize = 30;
        drawLine(Point2i{centerX - crosshairSize, centerY}, Point2i{centerX + crosshairSize, centerY}, {0, 255, 0});
        drawLine(Point2i{centerX, centerY - crosshairSize}, Point2i{centerX, centerY + crosshairSize}, {0, 255, 0});
    }

    void drawHitmarker(bool isHeadShot, Color3i color) {
        // Draw a hitmarker at the center of the screen
        int centerX = 400;
        int centerY = 300;
        int hitmarkerSize = 30;
        int gapSize = 10; // 20, 10

        // Top-left to center-left
        drawLine(Point2i{centerX - hitmarkerSize, centerY - hitmarkerSize}, Point2i{centerX - gapSize, centerY - gapSize}, color);
        // Top-right to center-right
        drawLine(Point2i{centerX + hitmarkerSize, centerY - hitmarkerSize}, Point2i{centerX + gapSize, centerY - gapSize}, color);
        // Bottom-left to center-left
        drawLine(Point2i{centerX - hitmarkerSize, centerY + hitmarkerSize}, Point2i{centerX - gapSize, centerY + gapSize}, color);
        // Bottom-right to center-right
        drawLine(Point2i{centerX + hitmarkerSize, centerY + hitmarkerSize}, Point2i{centerX + gapSize, centerY + gapSize}, color);
    }

    void drawUI() {
        string info = "Current weapon: " + currentWeapon.name + "\n";
        string magazine;
        if (isReloading) {
            magazine += "Reloading";
        } else {
            magazine = to_string(remainingBullets) +" / " + to_string(currentWeapon.magSize);
        }
//        cout<<info<<endl;
        glColor3ub(0, 0, 0);
        glRasterPos2i(10, 20);
        YsGlDrawFontBitmap10x14(info.c_str());
        glRasterPos2i(640, 570);
        YsGlDrawFontBitmap16x20(magazine.c_str());
        int dmgIndicatorPosY = 330;
        for (int dmg: damageList) {
            glRasterPos2i(400, dmgIndicatorPosY);
            YsGlDrawFontBitmap8x8(std::to_string(dmg).c_str());
            dmgIndicatorPosY += 10;
        }
    }

    void playHitmarkerSound() {
        player.MakeCurrent();
        // player.Start();
        player.PlayOneShot(hitmarkerSound);
    }

    void playFinishingSound(bool isHeadshot) {
        player.MakeCurrent();
        // player.Start();
        player.PlayOneShot(isHeadshot ? finishingSound2 : finishingSound1);
    }
};

int main() {
    FsOpenWindow(16,16,800,600,1);
    auto gameConfig = GameConfig {
        weaponList,
        50,
        5,
        60,
        200,
        3.f,
        3.7f,
        true
    };
    CodParodyGame game(gameConfig);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    FsPollDevice();
    FsPassedTime();
    MouseEvent mouseState = {0, 0, 0, 0, 0, 0};
    FsGetMouseEvent(mouseState.lb, mouseState.mb, mouseState.rb, mouseState.mx, mouseState.my);

    //For render time calc
    constexpr auto clk = chrono::high_resolution_clock();

    for (;;) {
        FsPollDevice();
        int key = FsInkey();
        if (FSKEY_ESC == key) {
            break;
        }
        auto newState = MouseEvent {0, 0, 0, 0, 0, 0};
        FsGetMouseEvent(newState.lb, newState.mb, newState.rb, newState.mx, newState.my);

        auto start = clk.now();

        bool gameEnded = game.doFrame(MouseEvent {
                newState.lb, newState.mb, newState.rb, newState.mx - mouseState.mx, newState.my - mouseState.my, 0
            }, key, FsPassedTime());
        mouseState = newState;

        auto renderTimeUs = chrono::duration_cast<chrono::microseconds>(clk.now() - start).count();
//        cout << "Render time: " << renderTimeUs << "us" << endl;

        if (gameEnded) {
            break;
        }
    }
}