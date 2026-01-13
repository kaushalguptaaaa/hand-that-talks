#ifndef GESTURE_RECOGNIZER_H
#define GESTURE_RECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <map>

enum class GestureType {
    NONE,
    STOP,
    YES,
    HELP,
    OKAY
};

class GestureRecognizer {
public:
    GestureRecognizer();
    std::string detect(const cv::Mat& frame, cv::Mat& debugFrame);

private:
    cv::Mat skinMask;
    
    // Thresholds
    int h_min = 0, h_max = 20;
    int s_min = 30, s_max = 255;
    int v_min = 30, v_max = 255;

    int countFingers(const std::vector<cv::Point>& contour, const std::vector<int>& hull_indices);
    GestureType mapToGesture(int fingers);
};

#endif
