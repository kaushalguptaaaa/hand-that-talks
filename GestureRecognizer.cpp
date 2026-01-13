#include "GestureRecognizer.h"
#include <cmath>

GestureRecognizer::GestureRecognizer() {}

double getAngle(cv::Point s, cv::Point f, cv::Point e) {
    double l1 = cv::norm(f - s);
    double l2 = cv::norm(f - e);
    double dot = (s.x - f.x) * (e.x - f.x) + (s.y - f.y) * (e.y - f.y);
    double angle = acos(dot / (l1 * l2));
    return angle * 180.0 / CV_PI;
}

std::string GestureRecognizer::detect(const cv::Mat& frame, cv::Mat& debugFrame) {
    // Preprocessing
    cv::Mat blur, hsv, mask;
    cv::GaussianBlur(frame, blur, cv::Size(5, 5), 0);
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);

    // Skin color segmentation (HSV)
    // Adjust these values if detection is poor!
    // Ranges: Hue 0-20, Sat 30-255, Val 30-255 is generic skin
    cv::inRange(hsv, cv::Scalar(0, 30, 60), cv::Scalar(20, 255, 255), mask);
    
    // Morphology
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::erode(mask, mask, kernel, cv::Point(-1, -1), 2);
    cv::dilate(mask, mask, kernel, cv::Point(-1, -1), 2);

    // Contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    int maxAreaIndex = -1;
    double maxArea = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > maxArea && area > 1000) { // Filter small noise
            maxArea = area;
            maxAreaIndex = i;
        }
    }

    if (maxAreaIndex != -1) {
        std::vector<cv::Point> handContour = contours[maxAreaIndex];
        std::vector<int> hullIndices;
        std::vector<cv::Point> hullPoints;

        // Convex Hull
        cv::convexHull(handContour, hullIndices, false);
        cv::convexHull(handContour, hullPoints, true);

        // Draw context
        cv::drawContours(debugFrame, std::vector<std::vector<cv::Point>>{handContour}, -1, cv::Scalar(0, 255, 0), 2);
        cv::drawContours(debugFrame, std::vector<std::vector<cv::Point>>{hullPoints}, -1, cv::Scalar(0, 0, 255), 2);

        // Count fingers
        int fingers = countFingers(handContour, hullIndices);
        GestureType g = mapToGesture(fingers);

        // Bounding box for "Okay" check (simple heuristic)
        // If fingers == 1, check orientation?
        // For now, map:
        // 0 -> Yes (Fist)
        // 1 -> Help (Point)
        // 2-3 -> Okay (Thumbs up / Peace approximation)
        // 4-5 -> Stop (Palm)
        
        switch (g) {
            case GestureType::STOP: return "Stop";
            case GestureType::YES: return "Yes";
            case GestureType::HELP: return "Help";
            case GestureType::OKAY: return "Okay";
            default: return "";
        }
    }

    return "";
}

int GestureRecognizer::countFingers(const std::vector<cv::Point>& contour, const std::vector<int>& hull_indices) {
    if (hull_indices.empty()) return 0;

    std::vector<cv::Vec4i> defects;
    cv::convexityDefects(contour, hull_indices, defects);

    int fingers = 0;
    for (const auto& v : defects) {
        float depth = v[3] / 256.0; // depth is in fixed point format
        if (depth > 20) { // arbitrary depth threshold
            int startidx = v[0];
            int endidx = v[1];
            int faridx = v[2];

            cv::Point start = contour[startidx];
            cv::Point end = contour[endidx];
            cv::Point far = contour[faridx];

            if (getAngle(start, far, end) < 100.0) { // Filter shallow angles
                fingers++;
            }
        }
    }
    
    // defects count "valleys".
    // 0 valleys = 0 or 1 finger (usually).
    // if area is large and no valleys -> 0 or 1.
    // Let's rely on defects count + 1 logic approximation?
    // Actually, "Fist" has 0 extended fingers -> 0 deep defects.
    // "1 finger" -> 0-1 deep defects depending on hull shape.
    // This is tricky.
    // If defects > 3 -> Open Palm.
    
    // Simplification for hackathon:
    return fingers;
    // Real "fingers count" is usually associated with:
    // 0 defects -> 1 finger or fist?
    // 1 defect -> 2 fingers
    // 4 defects -> 5 fingers.
}

GestureType GestureRecognizer::mapToGesture(int defectsCount) {
    // Heuristics
    // 0 defects -> Fist (Yes) or Point (Help)?
    // Usually Point has at least 1 significant defect near the thumb base if the hand is upright?
    // Let's assume:
    // Fist -> 0 defects.
    // Point -> 1 defect ? (Thumb-Index gap?)
    // Peace -> 1 defect (Index-Middle gap).
    // Stop -> 4 defects (gaps between 5 fingers).

    if (defectsCount >= 4) return GestureType::STOP;
    if (defectsCount == 0) return GestureType::YES; // Fist
    if (defectsCount == 1 || defectsCount == 2) return GestureType::HELP; // Point / "Okay" approx
    if (defectsCount == 3) return GestureType::OKAY; // "Okay" (Three fingers? Close enough)
    
    return GestureType::NONE;
}
