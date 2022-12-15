#ifndef CAMERACLASS_H
#define CAMERACLASS_H

#define FRAMERATE 10
#define BITRATE 10000000
#define HEIGHT 480
#define WIDTH 640

#include <QMainWindow>
#include <QObject>
#include <QLabel>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include "Spinnaker.h"
#include "SpinVideo.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace Spinnaker::Video;

class CameraClass
{

public:
    CameraClass();
    ~CameraClass();

    /* Functions */
    int ConfigureCamera(INodeMap& nodeMap);
    int StartRecording(int recordLength, int numParts, QLabel *previewFrame);
    int InitCamera();
    int GetCamera();
    //void GetSingleImage();

    /* Variables */
    bool isRecording = false;
    bool isPreview = false;
    int offset = 23800;
    int gain = 50;
    int totalFrames;
    int currentFrameCount;

    cv::Mat currentFrame;
    CameraPtr camPtr;
    SystemPtr system;
    CameraList camList;
private:

signals:
    void CameraDisconnected();
    void UpdateProgressbar();

private slots:

};

#endif // CAMERACLASS_H
