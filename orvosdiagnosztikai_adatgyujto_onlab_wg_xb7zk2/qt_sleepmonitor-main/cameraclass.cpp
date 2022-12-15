#include "cameraclass.h"

CameraClass::CameraClass()
{
}

CameraClass::~CameraClass()
{
    camPtr->DeInit();
    camList.Clear();
    system->ReleaseInstance();
}

int CameraClass::GetCamera()
{
    int result = 0;
    try
    {
        std::cout << "Connection started...\n";
        // Retrieve singleton reference to system object
        system = System::GetInstance();

        // Retrieve list of cameras from the system
        camList = system->GetCameras();
        unsigned int numCameras = camList.GetSize();

        std::cout << "Number of cameras detected: " << numCameras << "\n\n";

        // Finish if there are no cameras
        if (numCameras == 0)
        {
            std::cout << "Camera not connected!\n";
            return 1;
        }

        std::cout << "Camera detected. Starting initialization...\n";

        camPtr = camList.GetByIndex(0);
        result = result | InitCamera();

        //      NOT SURE IF NEEDED, need more testing
        // Clear camera list before releasing system
        //std::cout << "clearing list\n";
        //camList.Clear();

        std::cout << "Returning camera pointer\n";
        return result;
    }
    catch(Spinnaker::Exception& e)
    {
        std::cout << "Error:" << e.what() << "\n";
        return -1;
    }
}

int CameraClass::InitCamera()
{
    int result = 0;
    std::cout << "\nInitializing camera\n";
    try
    {
        camPtr->Init();

        INodeMap& nodeMap = camPtr->GetNodeMap();

        // Configure camera settings (fps, acquisition mode, etc.)
        //result = result | ConfigureCamera(nodeMap);
        ConfigureCamera(nodeMap);

        //camPtr->DeInit();

        std::cout << "Initialization ended\n";

        return result;
    }
    catch (Spinnaker::Exception& e)
    {
        std::cout << "Error at initializing: " << e.what() << "\n";
        return -1;
    }

}

int CameraClass::StartRecording(int recordSeconds, int numParts, QLabel* previewFrame)
{
    std::cout << "Recording started\n";

    isRecording = true;
    SpinVideo video;

    totalFrames = ((recordSeconds * 10) + 24) * numParts;
    currentFrameCount = 0;

    try
    {
        int curPart = 0;
        while (curPart < numParts && isRecording)
        {
            curPart++;
            if (!camPtr->IsInitialized()) camPtr->Init();

            // Calculate required number of frames for 1 video file
            const int numImages = (FRAMERATE * recordSeconds) + 24;

            // Begin acquiring images
            if (!camPtr->IsStreaming()) CameraClass::camPtr->BeginAcquisition();
            std::cout << "Acquisition started..." << "\n" << "\n";

            // *** NOTES ***
            // By default, if no specific color processing algorithm is set, the image
            // processor will default to NEAREST_NEIGHBOR method.
            ImageProcessor processor;
            processor.SetColorProcessing(HQ_LINEAR);

            //const unsigned int k_videoFileSize_MB = 4096;
            //video.SetMaximumFileSize(k_videoFileSize_MB);

            Video::H264Option option;
            option.frameRate = FRAMERATE;
            option.bitrate = BITRATE;
            option.height = HEIGHT;
            option.width = WIDTH;

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point elapsed;

            std::string videoFilename = "Video_" + std::to_string(curPart) + "_" + std::to_string(time(0));
            video.Open(videoFilename.c_str(), option);

            // *** IMPORTAN NOTE TO SELF ***
            // The first 24 frames in the MP4 file won't be saved
            // and at least 24 frames must be appended to the video
            int imageCnt = 0;
            while(imageCnt < numImages && isRecording)
            {
                imageCnt++;
                currentFrameCount++;
                try
                {
                    // Retrieve the next received image
                    ImagePtr pResultImage = CameraClass::camPtr->GetNextImage(1000);

                    if (pResultImage->IsIncomplete())
                    {
                        std::cout << "Image "<< imageCnt << " is incomplete with image status " << pResultImage->GetImageStatus() << "..." << "\n"
                            << "\n";
                    }
                    else
                    {
                        std::cout << "------------------------" << "\n";
                        std::cout << "Grabbed image " << imageCnt << "/" << numImages << "\n";

                        currentFrame = cv::Mat(480, 640, CV_16UC1, pResultImage->GetData(), pResultImage->GetStride());
                        currentFrame -= offset;
                        currentFrame *= gain;

                        if (isPreview)
                        {
                            previewFrame->setPixmap(QPixmap::fromImage(QImage(currentFrame.data,
                                                                                currentFrame.cols,
                                                                                currentFrame.rows,
                                                                                currentFrame.step,
                                                                                QImage::Format_Grayscale16)));
                        }



                        // Deep copy image into mp4 file
                        video.Append(processor.Convert(pResultImage, PixelFormat_Mono8));
                        std::cout << "Appended image " << imageCnt << "/" << numImages << " to part:" << curPart << "\n";

                        // Release image
                        pResultImage->Release();

                        elapsed = std::chrono::steady_clock::now();
                        std::cout << "Recording time elapsed: "
                                  << std::chrono::duration_cast<std::chrono::seconds>(elapsed - begin).count()
                                  << "s" << "\n";

                        //int processPercent = imageCnt*100/numImages;
                        //progressBar->setValue(processPercent);

                        std::cout << "------------------------" << "\n";
                    }
                }
                catch (Spinnaker::Exception& e)
                {
                    std::cout << "Error: " << e.what() << "\n";
                }
            }
            video.Close();

            //const char* avi_filename = videoFilename.c_str() + '.avi';
            //const char* mp4_filename = videoFilename.c_str() + '.mp4';
            //rename(avi_filename, mp4_filename);

            std::cout << "\n" << "Video saved at " << videoFilename << ".avi" << "\n";

            // End acquisition
            if (!isPreview)
            {
                CameraClass::camPtr->EndAcquisition();
                CameraClass::camPtr->DeInit();
            }
        }

        std::cout << "Recording ended\n";
        isRecording = false;
        return 0;
    }
    catch (Spinnaker::Exception& e)
    {
        std::cout << "Error at recording: " << e.GetFullErrorMessage() << "\n";
        video.Close();
        isRecording = false;
        if (e.GetError() == -1002)
        {
            std::cout << "\n\nError -1002 caught, which means the camera was disconnected\n\n";
            return -1;
        }
    }
    return 1;
}

int CameraClass::ConfigureCamera(INodeMap& nodeMap)
{
    std::cout << "Camera config started:\n\n";
    try
    {
        std::cout << "Setting acquisition mode to continuous...\n";
        // Set acquisition mode to continuous
        CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
        if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
        {
            std::cout << "Unable to set acquisition mode to continuous (node retrieval). Aborting...\n";
            return 1;
        }

        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
        {
            std::cout << "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting...\n";
            return 1;
        }

        int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
        std::cout << "Setting done\n\n";

        std::cout << "Setting acquisition framerate to 10 fps...\n";
        CFloatPtr ptrFramerate = nodeMap.GetNode("AcquisitionFrameRate");
        ptrFramerate->SetValue(FRAMERATE);
        std::cout << "Setting done\n";

        std::cout << "\nCamera config ended\n";
        return 0;
    }
    catch (Spinnaker::Exception& e)
    {
        std::cout << "Error at configuration: " << e.what() << "\n";
        return -1;
    }

}
