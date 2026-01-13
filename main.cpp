#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include "GestureRecognizer.h"
#include "TTS.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Cannot open camera" << endl;
        return -1;
    }

    GestureRecognizer recognizer;
    Mat frame, debugFrame;
    
    string lastDetected = "";
    string stableGesture = "";
    auto gestureStartTime = chrono::steady_clock::now();
    bool spoken = false;

    // Window setup
    namedWindow("GestureTalk", WINDOW_AUTOSIZE);

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Clone for debug drawing
        debugFrame = frame.clone();
        
        // Flip for mirror effect
        flip(debugFrame, debugFrame, 1);
        
        // Detect
        string currentDetection = recognizer.detect(frame, debugFrame); 
        // Note: 'detect' uses raw frame (not flipped) usually for logic, but drawing on flipped 'debugFrame' requires coord mapping or flip input.
        // Let's pass 'frame' (unflipped) to detect, but we want to draw on the visual output. 
        // Better: Flip first.
        
        flip(frame, frame, 1);
        debugFrame = frame.clone();
        currentDetection = recognizer.detect(frame, debugFrame);

        // Debounce Logic
        auto now = chrono::steady_clock::now();
        if (currentDetection == lastDetected && !currentDetection.empty()) {
            auto duration = chrono::duration_cast<chrono::milliseconds>(now - gestureStartTime).count();
            if (duration > 300) { // 300ms persistence
                if (stableGesture != currentDetection) {
                    stableGesture = currentDetection;
                    spoken = false;
                }
            }
        } else {
            lastDetected = currentDetection;
            gestureStartTime = now;
        }

        // Trigger Action
        if (!stableGesture.empty() && !spoken) {
            cout << "Gesture Detected: " << stableGesture << endl;
            TTS::speak(stableGesture);
            spoken = true;
        }

        // Overlay State
        if (!stableGesture.empty()) {
             // Green box or text
             putText(debugFrame, "Detected: " + stableGesture, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        } else {
             putText(debugFrame, "Waiting...", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        }

        // Show
        imshow("GestureTalk", debugFrame);

        if (waitKey(30) == 'q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
