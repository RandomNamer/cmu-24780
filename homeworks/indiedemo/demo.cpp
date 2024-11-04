
#include "fssimplewindow.h"
#include "yssimplesound.h"
#include "ysglfontdata.h"
#include <cmath>
#include <GLUT/glut.h>
#include <vector>
#include<array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <complex>
#include <iostream>
#include <numeric>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FREQ_ANALYSIS_WINDOW_SIZE 1024
#define LEVELS_ANALYSIS_WINDOW_SIZE 256

 enum TestMode {
    FFT_CALC,
    UI_HOMEPAGE,
    PLAYBACK_CROSSFADE,
    FEATURE_HISTOGRAM,
    FEATURE_WAVEFORM,
    FEATURE_LEVEL_GAUGE,
    FEATURE_BACKGROUND,
};

//#define TEST_SWITCH
constexpr TestMode TEST_MODE = FFT_CALC;


//Animator Framework
template<typename T>
class ValueAnimator {
public:
    using InterpolatorFunc = std::function<float(float)>;
    using EvaluatorFunc = std::function<T(float, const T&, const T&)>;

    // Constructor with functional interpolator and templated evaluator
    ValueAnimator(T start, T end, int durationMillis,
                  InterpolatorFunc interpolator = linearInterpolator,
                  EvaluatorFunc evaluator = linearEvaluator)
            : startValue(start), endValue(end), durationMillis(durationMillis),
              interpolator(interpolator), evaluator(evaluator), animationEnded(false) {}

    // Set the update listener
    void setUpdateListener(std::function<void(T)> listener) {
        updateListener = std::move(listener);
    }

    // Implement the update function from VSyncListener
    void update(int currentTimeMillis)  {
        if (animationEnded) return;

        int elapsed = currentTimeMillis - startTimeMillis;
        if (elapsed >= durationMillis) elapsed = durationMillis;

        float fraction = interpolator(static_cast<float>(elapsed) / durationMillis);
        T animatedValue = evaluator(fraction, startValue, endValue);

        if (updateListener) {
            updateListener(animatedValue);
        }

        if (elapsed >= durationMillis) {
            animationEnded = true;
        }
    }

    bool hasEnded() const {
        return animationEnded;
    }

    // Built-in interpolators (float-to-float)
    static float linearInterpolator(float t) { return t; }
   static float easeInOutInterpolator(float t) {
        return 0.5f * (1.0f + std::sin((t - 0.5f) * M_PI));
    }
    static float accelerateDecelerateInterpolator(float t) {
        return (cos((t + 1) * M_PI) / 2) + 0.5;
    }
    static float fastOutSlowInInterpolator(float t) {
        // Control points for FastOutSlowIn Bezier curve
        float p0 = 0.0f, p1 = 0.4f, p2 = 0.0f, p3 = 1.0f;
        return cubicBezierInterpolate(t, p0, p1, p2, p3);
    }


    // Built-in evaluators (templated)
    static T linearEvaluator(float fraction, const T& start, const T& end) {
        return start + fraction * (end - start);
    }

private:
    T startValue;
    T endValue;
    int durationMillis;
    int startTimeMillis = 0;
    bool animationEnded = false;

    InterpolatorFunc interpolator;
    EvaluatorFunc evaluator;
    std::function<void(T)> updateListener;

    static float cubicBezierInterpolate(float t, float p0, float p1, float p2, float p3) {
        float u = 1 - t;
        return 3 * u * u * t * p1 + 3 * u * t * t * p2 + t * t * t;
    }
};

// Specialization for ARGB color (std::array<int, 4>)
template <>
std::array<int, 4> ValueAnimator<std::array<int, 4>>::linearEvaluator(float fraction, const std::array<int, 4>& start, const std::array<int, 4>& end) {
    std::array<int, 4> result;
    for (int i = 0; i < 4; ++i) {
        result[i] = static_cast<int>(start[i] + fraction * (end[i] - start[i]));
    }
    return result;
};

// Metrics calculator
class AsyncMetricsCalculator {
public:
    AsyncMetricsCalculator(int sampleRate) : sampleRate(sampleRate) {}

    void onUpdate(unsigned char* currentFramePointer) {
        std::lock_guard<std::mutex> lock(mutex);
        std::copy(currentFramePointer, currentFramePointer + fftFrameSize, currentFrame.begin());
        if (calculationThread.joinable()) {
            calculationThread.join();
        }
        calculationThread = std::thread(&AsyncMetricsCalculator::calculateMetrics, this);
    }

    std::vector<double> getRawFreqDistribution() const {
        std::lock_guard<std::mutex> lock(mutex);
        return std::vector(freqDistribution); //copy constructor
    }

    template<size_t BINS>
    std::array<double, BINS> getHistogram() const {
        std::lock_guard<std::mutex> lock(mutex);
        std::array<double, BINS> histogram = {0};
        double binWidth = (sampleRate / 2.0) / BINS;
        for (double value : freqDistribution) {
            int bin = static_cast<int>(value / binWidth);
            if (bin < BINS) {
                histogram[bin]++;
            }
        }
        return histogram;
    }

    std::array<double, 2> getLevelsInDecibles() const {
        std::lock_guard<std::mutex> lock(mutex);
        return levels;
    }

private:
    int sampleRate;
    int fftFrameSize = FREQ_ANALYSIS_WINDOW_SIZE;
    int levelsFrameSize = LEVELS_ANALYSIS_WINDOW_SIZE;
    std::vector<unsigned char> currentFrame = std::vector<unsigned char>(fftFrameSize);
    std::vector<double> freqDistribution;
    std::array<double, 2> levels = {0.0, 0.0};
    std::thread calculationThread;
    mutable std::mutex mutex;

    void calculateMetrics() {

        std::vector<double> leftChannel, rightChannel;
        for (size_t i = 0; i < currentFrame.size(); i += 2) {
            leftChannel.push_back(static_cast<double>(currentFrame[i]));
            rightChannel.push_back(static_cast<double>(currentFrame[i + 1]));
        }

        std::vector<std::complex<double>> leftFFT = fft(leftChannel.data(), leftChannel.size());
        std::vector<std::complex<double>> rightFFT = fft(rightChannel.data(), rightChannel.size());
        std::vector<double> leftFreqDist = getFrequencyDistribution(leftFFT);
        std::vector<double> rightFreqDist = getFrequencyDistribution(rightFFT);
        freqDistribution.resize(leftFreqDist.size());
        std::transform(leftFreqDist.begin(), leftFreqDist.end(), rightFreqDist.begin(), freqDistribution.begin(), std::plus<double>());

        double leftSum = std::accumulate(leftChannel.begin(), leftChannel.end(), 0.0);
        double rightSum = std::accumulate(rightChannel.begin(), rightChannel.end(), 0.0);
        double leftAvg = leftSum / leftChannel.size();
        double rightAvg = rightSum / rightChannel.size();
        levels[0] = 20 * std::log10(leftAvg);
        levels[1] = 20 * std::log10(rightAvg);
    }

    std::vector<std::complex<double>> fft(const double* input, int n) const {
        if (n == 1) {
            return { std::complex<double>(input[0], 0.0) };
        }

        std::vector<std::complex<double>> evenFFT = fft(input, n / 2);
        std::vector<std::complex<double>> oddFFT = fft(input + 1, n / 2);

        std::vector<std::complex<double>> result(n);
        for (int k = 0; k < n / 2; ++k) {
            std::complex<double> t = std::polar(1.0, -2.0 * M_PI * k / n) * oddFFT[k];
            result[k] = evenFFT[k] + t;
            result[k + n / 2] = evenFFT[k] - t;
        }

        return result;
    }

    std::vector<double> getFrequencyDistribution(const std::vector<std::complex<double>>& fftResult) const {
        int n = fftResult.size();
        std::vector<double> magnitudes(n);
        for (int i = 0; i < n; ++i) {
            magnitudes[i] = std::abs(fftResult[i]);
        }
        return magnitudes;
    }
};

// UI Framework
struct Point2i {
    int x, y;
};

struct Color4i {
    int r, g, b, a;
    Color4i(int r, int g, int b) : r(r), g(g), b(b), a(255) {};
    Color4i(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {};
};

class View {
public:
    int x = 0, y = 0, width = 0, height = 0;

    void setDebug(bool d) {
        this->debug = d;
    }

    void setPosition(int x, int y) {
        this->x = x;
        this->y = y;
    }

    void setSize(int width, int height) {
        this->width = width;
        this->height = height;
    }

    void setVisible(bool visible) {
        this->visible = visible;
    }

    void setAlpha(float alpha) {
        this->alpha = alpha;
    }

    void addChild(std::shared_ptr<View> child) {
        children.push_back(child);
    }

    void draw(int parentX = 0, int parentY = 0) {
        if (!visible) return;

        globalX = parentX + x;
        globalY = parentY + y;

        // Draw the current view (placeholder for actual drawing logic)
        drawSelf();

        if (debug) drawBorder({255, 0, 0, 255});

        // Draw children
        for (auto it = children.begin(); it != children.end();) {
            auto& child = *it;
            child->draw(globalX, globalY);
            if (child->globalX < globalX || child->globalY < globalY ||
                child->globalX + child->width > globalX + width ||
                child->globalY + child->height > globalY + height) {
                it = children.erase(it);
            } else {
                ++it;
            }
        }
    }

    void drawCircle(Point2i center, int radius, Color4i color) {
        glColor4ub(color.r, color.g, color.b, static_cast<unsigned char>(color.a * alpha));
        glBegin(GL_TRIANGLE_FAN);
        for (int i = 0; i < 360; ++i) {
            float angle = i * M_PI / 180.0f;
            glVertex2i(globalX + center.x + radius * cos(angle), globalY + center.y + radius * sin(angle));
        }
        glEnd();
    }

    void drawCircleHollowed(Point2i center, int radius, int thickness, Color4i color, int segments=100) {
        glLineWidth(thickness);
        glColor4ub(color.r, color.g, color.b, static_cast<unsigned char>(color.a * alpha));
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            float theta = 2.0f * M_PI * float(i) / float(segments);
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);
            glVertex2f(globalX + center.x + x, globalY + center.y + y);
        }
        glEnd();
    }

    void drawRectFilled(Point2i topLeft, Point2i bottomRight, Color4i color) {
        glColor4ub(color.r, color.g, color.b, static_cast<unsigned char>(color.a * alpha));
        glBegin(GL_QUADS);
        glVertex2i(globalX + topLeft.x, globalY + topLeft.y);
        glVertex2i(globalX + bottomRight.x, globalY + topLeft.y);
        glVertex2i(globalX + bottomRight.x, globalY + bottomRight.y);
        glVertex2i(globalX + topLeft.x, globalY + bottomRight.y);
        glEnd();
    }

    void drawLine(Point2i start, Point2i end, Color4i color) {
        glLineWidth(2);
        glColor4ub(color.r, color.g, color.b, static_cast<unsigned char>(color.a * alpha));
        glBegin(GL_LINES);
        glVertex2i(globalX + start.x, globalY + start.y);
        glVertex2i(globalX + end.x, globalY + end.y);
        glEnd();
    }

    void drawLineStrip(const std::vector<Point2i>& points, Color4i color) {
        glLineWidth(2);
        glColor4ub(color.r, color.g, color.b, static_cast<unsigned char>(color.a * alpha));
        glBegin(GL_LINE_STRIP);
        for (const auto& point : points) {
            glVertex2i(globalX + point.x, globalY + point.y);
        }
        glEnd();
    }

    void drawBorder(Color4i color, bool regardlessAlpha = false) {
        glLineWidth(1);
        auto finalAlpha = regardlessAlpha ? 1.0f : this->alpha;
        glColor4ub(color.r, color.g, color.b, static_cast<unsigned char>(color.a * finalAlpha));
        glBegin(GL_LINE_LOOP);
        glVertex2i(globalX, globalY);
        glVertex2i(globalX + width, globalY);
        glVertex2i(globalX + width, globalY + height);
        glVertex2i(globalX, globalY + height);
        glEnd();
    }

protected:
    bool visible = true;
    float alpha = 1.0f;
    int globalX, globalY;
    bool debug = false;
    std::vector<std::shared_ptr<View>> children;

    virtual void drawSelf() {
        // Placeholder for actual drawing logic
        std::cout << "Drawing view at (" << globalX << ", " << globalY << ") with alpha " << alpha << std::endl;
    }

};

class ConstraintLayout : public View {
public:
    enum Alignment {
        ALIGN_PARENT_LEFT,
        ALIGN_PARENT_RIGHT,
        ALIGN_PARENT_TOP,
        ALIGN_PARENT_BOTTOM,
        ALIGN_PEER_LEFT,
        ALIGN_PEER_RIGHT,
        ALIGN_PEER_TOP,
        ALIGN_PEER_BOTTOM
    };

    void addChild(std::shared_ptr<View> child, Alignment alignment, int padding = 0, std::shared_ptr<View> peer = nullptr) {
        switch (alignment) {
            case ALIGN_PARENT_LEFT:
                child->setPosition(padding, child->y);
                break;
            case ALIGN_PARENT_RIGHT:
                child->setPosition(width - child->width - padding, child->y);
                break;
            case ALIGN_PARENT_TOP:
                child->setPosition(child->x, padding);
                break;
            case ALIGN_PARENT_BOTTOM:
                child->setPosition(child->x, height - child->height - padding);
                break;
            case ALIGN_PEER_LEFT:
                if (peer) child->setPosition(peer->x - child->width - padding, child->y);
                break;
            case ALIGN_PEER_RIGHT:
                if (peer) child->setPosition(peer->x + peer->width + padding, child->y);
                break;
            case ALIGN_PEER_TOP:
                if (peer) child->setPosition(child->x, peer->y - child->height - padding);
                break;
            case ALIGN_PEER_BOTTOM:
                if (peer) child->setPosition(child->x, peer->y + peer->height + padding);
                break;
        }
        View::addChild(child);
    }
};

class AbsoluteLayout : public View {
public:
    void addChild(std::shared_ptr<View> child, int x, int y) {
        child->setPosition(x, y);
        View::addChild(child);
    }
};

class TextView : public View {
public:
    //wid, height
    enum FontFace {
        BITMAPFONT_6x8,
        BITMAPFONT_8x12,
        BITMAPFONT_10x14,
        BITMAPFONT_12x16,
        BITMAPFONT_16x24,
        BITMAPFONT_20x32,
        BITMAPFONT_28x44
    };

    TextView(FontFace fontface) {
        switch (fontface) {
            case BITMAPFONT_6x8:
                charWidth = 6;
                charHeight = 8;
                renderFunc = YsGlDrawFontBitmap6x8;
                break;
            case BITMAPFONT_8x12:
                charWidth = 8;
                charHeight = 12;
                renderFunc = YsGlDrawFontBitmap8x12;
                break;
            case BITMAPFONT_10x14:
                charWidth = 10;
                charHeight = 14;
                renderFunc = YsGlDrawFontBitmap10x14;
                break;
            case BITMAPFONT_12x16:
                charWidth = 12;
                charHeight = 16;
                renderFunc = YsGlDrawFontBitmap12x16;
                break;
            case BITMAPFONT_16x24:
                charWidth = 16;
                charHeight = 24;
                renderFunc = YsGlDrawFontBitmap16x24;
                break;
            case BITMAPFONT_20x32:
                charWidth = 20;
                charHeight = 32;
                renderFunc = YsGlDrawFontBitmap20x32;
                break;
            case BITMAPFONT_28x44:
                charWidth = 28;
                charHeight = 44;
                renderFunc = YsGlDrawFontBitmap28x44;
                break;
        }
        height = charHeight;
    }

    void setText(const std::string& text) {
        this->text = text;
        width = charWidth * text.size();
        height = charHeight;
    }

    void setTextColor(Color4i color) {
        this->textColor = color;
    }

    void setTextRenderingDirectly(void (*f)(const char*)) {
        this->renderFunc = f;
    }

private:
    std::string text;
    Color4i textColor = {255, 255, 255, 255};
    void (*renderFunc)(const char*) = nullptr;
    int charWidth, charHeight;

    void drawSelf() override {
        if (renderFunc) {
            return;
        } else {
            glColor4ub(textColor.r, textColor.g, textColor.b, static_cast<unsigned char>(textColor.a * alpha));
            glRasterPos2i(globalX, globalY);
            renderFunc(text.c_str());
        }
    }
};

class MainActivity {

};

int main() {
    FsOpenWindow(16,16,800,600,1);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    FsPollDevice();
    FsPassedTime();
    constexpr auto clk = std::chrono::high_resolution_clock();
#ifdef TEST_SWITCH
    TestRunner();
    return -1;
#endif
    for (;;) {
        FsPollDevice();
        int key = FsInkey();
        if (FSKEY_ESC == key) {
            break;
        }
    }

}