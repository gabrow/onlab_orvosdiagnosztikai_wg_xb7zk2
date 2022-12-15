#include "sleepmonitormain.h"
#include "ui_sleepmonitormain.h"


using namespace cv;

SleepMonitorMain::SleepMonitorMain(QWidget *parent, CameraClass *cam) : QMainWindow(parent), ui(new Ui::SleepMonitorMain), camera(cam)
{
    ui->setupUi(this);

    ui->recordingFinishedLabel->hide();
    //ui->recordingProgressBar->hide();
    ui->disconnectedLabel->hide();

    QIcon recordIcon(QPixmap("../Assets/Icons/recordIcon.png"));
    QIcon stopIcon(QPixmap("../Assets/Icons/stopIcon.png"));
    QIcon previewIcon(QPixmap("../Assets/Icons/previewIcon.png"));
    QIcon stopPreviewIcon(QPixmap("../Assets/Icons/stopPreviewIcon.png"));
    QIcon connectIcon(QPixmap("../Assets/Icons/connectIcon.png"));

    ui->startRecordingButton->setIcon(recordIcon);
    ui->stopRecordingButton->setIcon(stopIcon);
    ui->showPreviewButton->setIcon(previewIcon);
    ui->hidePreviewButton->setIcon(stopPreviewIcon);
    ui->connectButton->setIcon(connectIcon);

    ui->startRecordingButton->setText("");
    ui->stopRecordingButton->setText("");
    ui->showPreviewButton->setText("");
    ui->hidePreviewButton->setText("");
    ui->connectButton->setText("");

    //ui->connectButton->setIconSize(QSize(50,50));

    connect(this, &SleepMonitorMain::ConnectionFinished, this, &SleepMonitorMain::onConnectionFinished);
    connect(this, &SleepMonitorMain::CameraDisconnected, this, &SleepMonitorMain::onCameraDisconnected);

    connect(this, &SleepMonitorMain::UpdateProgressbar, this, &SleepMonitorMain::onUpdateProgressbar);

    connect(this, &SleepMonitorMain::RecordingEnded, this, &SleepMonitorMain::onRecordingEnded);
    connect(this, &SleepMonitorMain::RecordingStarted, this, &SleepMonitorMain::onRecordingStarted);

    connect(this, &SleepMonitorMain::PreviewEnded, this, &SleepMonitorMain::onPreviewEnded);
    connect(this, &SleepMonitorMain::PreviewStarted, this, &SleepMonitorMain::onPreviewStarted);
}

SleepMonitorMain::~SleepMonitorMain()
{
    //delete ui;

    camera->isRecording = false;
    camera->isPreview = false;
    isClosing = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (statusCheckThread.joinable()) statusCheckThread.join();
    if (previewThread.joinable()) previewThread.join();
    if (recordingThread.joinable()) recordingThread.join();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SleepMonitorMain::UpdateDisplayedRecordTime(int recordTime)
{
    ui->timeValueLabel->setText(QString::number(recordTime/60) + "h " + QString::number(recordTime%60) + "m");
}

void SleepMonitorMain::on_recordTimeMin_valueChanged(int arg1)
{
    recordMinute = arg1;
    SleepMonitorMain::UpdateDisplayedRecordTime((recordMinute + recordHour * 60) * recordParts);

    if (!recordMinute && !recordHour) isTimeNull = true;
    else isTimeNull = false;

    if(isConnected && !isTimeNull) ui->startRecordingButton->setEnabled(true);
    else ui->startRecordingButton->setEnabled(false);
}

void SleepMonitorMain::on_recordTimeHour_valueChanged(int arg1)
{
    recordHour = arg1;
    SleepMonitorMain::UpdateDisplayedRecordTime((recordMinute + recordHour * 60) * recordParts);

    if (recordMinute == 0 && recordHour == 0) isTimeNull = true;
    else isTimeNull = false;

    if(isConnected && !isTimeNull)    ui->startRecordingButton->setEnabled(true);
    else ui->startRecordingButton->setEnabled(false);
}

void SleepMonitorMain::on_recordParts_valueChanged(int arg1){
    recordParts = arg1;
    SleepMonitorMain::UpdateDisplayedRecordTime((recordMinute + recordHour * 60) * recordParts);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SleepMonitorMain::on_startRecordingButton_clicked()
{
    if (camera->isRecording) return;
    if (recordingThread.joinable()) recordingThread.join();

    progress = 0;
    ui->recordingFinishedLabel->hide();
    ui->recordingProgressBar->setValue(0);
    ui->recordingProgressBar->show();
    ui->startRecordingButton->setEnabled(false);
    ui->stopRecordingButton->setEnabled(true);

    recordingThread = std::thread(&SleepMonitorMain::StartRecording, this);
}

void SleepMonitorMain::on_stopRecordingButton_clicked()
{
    camera->isRecording = false;
}

void SleepMonitorMain::StartRecording()
{
    emit RecordingStarted();

    int recordSeconds = recordHour*60 + recordMinute; // recordHour*3600 + recordMinute * 60;
    int result = camera->StartRecording(recordSeconds, recordParts, ui->imageLabel);

    emit RecordingEnded();
    if (result == -1) emit CameraDisconnected();
    return;
}

void SleepMonitorMain::onRecordingStarted()
{
    //int recordSeconds = (recordHour * 60 + recordMinute + 2) * recordParts;
    //QTimer *timer = new QTimer(this);
    //connect(timer, &QTimer::timeout, this, QOverload<>::of(&SleepMonitorMain::UpdateProgressbar));
    //timer->start(recordSeconds*10);
}

void SleepMonitorMain::onRecordingEnded()
{
    std::cout << "recording ended, gui got signal\n\n";
    ui->startRecordingButton->setEnabled(true);
    ui->stopRecordingButton->setEnabled(false);
    ui->recordingProgressBar->setValue(100);
    ui->recordingFinishedLabel->show();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SleepMonitorMain::on_connectButton_clicked()
{
    // one of this will be useful someday
    ///    cameraThread = std::thread(startCamera);
    ///    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ///
    ///    std::thread *th = new std::thread(GetCamera);
    ///    th->join();
    ///
    ///    QTimer::singleShot(2000, this, &SleepMonitorMain::onConnectionFinished);

    int result = 1;

    if(isConnecting) return;
    if(statusCheckThread.joinable()) statusCheckThread.join();

    isConnecting = true;
    statusCheckThread = std::thread(&SleepMonitorMain::GUIGetCamera, this, &result);

    ui->connectButton->setEnabled(false);
    ui->connectionStateLabel->setText("Connecting...");
}

void SleepMonitorMain::onConnectionFinished(int result)
{
    if (result == 0)
    {
        isConnected = true;
        if(!isTimeNull) ui->startRecordingButton->setEnabled(true);

        std::cout << "Connection finished. Camera is now accessible.\n";
        ui->connectButton->setEnabled(false);
        ui->connectionStateLabel->setText("Connected");
        //ui->connectionStateLabel->setFont(QFont("Arial", 12));
        ui->connectionStateLabel->setStyleSheet("color: green");

        ui->showPreviewButton->setEnabled(true);
        ui->disconnectedLabel->hide();


        if(statusCheckThread.joinable()) statusCheckThread.join();
        statusCheckThread = std::thread(&SleepMonitorMain::StatusCheck, this);
        //QTimer *timer = new QTimer(this);
        //connect(timer, &QTimer::timeout, this, QOverload<>::of(&SleepMonitorMain::ConnectionCheck));
        //timer->start(1000);
    }
    else
    {
        isConnected = false;
        ui->connectButton->setEnabled(true);
        ui->connectionStateLabel->setText("Could not connect\nPress to try again");

        std::cout << "Connection failed.\n";

        ui->showPreviewButton->setEnabled(false);
    }
}

void SleepMonitorMain::GUIGetCamera(int* result)
{
    *result = camera->GetCamera();
    isConnecting = false;
    emit ConnectionFinished(*result);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SleepMonitorMain::on_showPreviewButton_clicked()
{
    if(camera->isPreview) return;
    if(previewThread.joinable()) previewThread.join();

    previewThread = std::thread(&SleepMonitorMain::DisplayPreview, this);
    //SleepMonitorMain::CameraTest();
}

void SleepMonitorMain::on_hidePreviewButton_clicked()
{
    emit PreviewEnded();
}

void SleepMonitorMain::DisplayPreview()
{
    try
    {
        if (!camera->camPtr->IsInitialized()) camera->camPtr->Init();
        if (!camera->camPtr->IsStreaming()) camera->camPtr->BeginAcquisition();

        camera->isPreview = true;
        Spinnaker::ImagePtr spinImgPtr;
        Mat previewFrame;

        emit PreviewStarted();

        while (camera->isPreview)
        {
            if(!camera->isRecording)
            {
                spinImgPtr = camera->camPtr->GetNextImage(1000);
                previewFrame = (cv::Mat(480, 640, CV_16UC1, spinImgPtr->GetData(), spinImgPtr->GetStride()) - camera->offset) * camera->gain;
                //previewFrame -= camera->offset;
                //previewFrame *= camera->gain;
                ui->imageLabel->setPixmap(QPixmap::fromImage(QImage(previewFrame.data,
                                                                    previewFrame.cols,
                                                                    previewFrame.rows,
                                                                    previewFrame.step,
                                                                    QImage::Format_Grayscale16)));
                spinImgPtr->Release();
            }
        }
        if (!camera->isRecording) camera->camPtr->EndAcquisition();
    }
    catch(Spinnaker::Exception& e)
    {
        std::cout << "error at preview loop: " << e.what() << " code: " << e.GetFullErrorMessage() << "\n";
        emit PreviewEnded();
        if (e.GetError() == -1002 || e.GetError() == -1010)
        {
            emit CameraDisconnected();
        }
        else
        {
            camera->camPtr->EndAcquisition();
        }
    }
}

void SleepMonitorMain::onPreviewStarted()
{
    camera->isPreview = true;
    ui->showPreviewButton->setEnabled(false);
    ui->hidePreviewButton->setEnabled(true);
    ui->imageLabel->show();
}

void SleepMonitorMain::onPreviewEnded()
{
    camera->isPreview = false;
    ui->showPreviewButton->setEnabled(true);
    ui->hidePreviewButton->setEnabled(false);
    ui->imageLabel->hide();
}

void SleepMonitorMain::CameraTest()
{
    camera->isPreview = true;

    cv::VideoCapture webcam(0);
    if (!webcam.isOpened())
    {
        std::cerr << "ERROR: Could not open camera" << std::endl;
        return;
    }

    cv::Mat frame;

    while (camera->isPreview)
    {
        webcam >> frame;

        cv::cvtColor(frame, frame, COLOR_BGR2GRAY);

        frame -= camera->offset;
        frame *= camera->gain;

        ui->imageLabel->setPixmap(QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_Grayscale8)));

        if (cv::waitKey(10) == 27)
            break;
    }
    webcam.release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SleepMonitorMain::on_offsetSlider_valueChanged(int value)
{
    ui->offsetValueLabel->setText(QString::number(value));
    //this->offset = value;
    camera->offset = value;
}

void SleepMonitorMain::on_gainSlider_valueChanged(int value)
{
    ui->gainValueLabel->setText(QString::number(value));
    //this->gain = value;
    camera->gain = value;
}

void SleepMonitorMain::on_defaultButton_clicked()
{
    on_gainSlider_valueChanged(50);
    ui->gainSlider->setValue(50);

    on_offsetSlider_valueChanged(23750);
    ui->offsetSlider->setValue(23750);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SleepMonitorMain::onCameraDisconnected()
{
    ui->disconnectedLabel->show();
    isConnected = false;
    camera->isPreview = false;
    camera->isRecording = false;

    ui->connectButton->setEnabled(true);
    ui->connectionStateLabel->setText("Disconnected\nTry to connect again");
    //ui->connectionStateLabel->setFont(QFont("Arial", 10));
    ui->connectionStateLabel->setStyleSheet("color: black");

    ui->startRecordingButton->setEnabled(false);
    ui->stopRecordingButton->setEnabled(false);
    ui->showPreviewButton->setEnabled(false);
    ui->hidePreviewButton->setEnabled(false);
}

void SleepMonitorMain::StatusCheck()
{
    int progress = 0;
    while (!isClosing && isConnected)
    {
        //if (camera->system->GetCameras().GetSize() == 0)
        //{
        //    emit CameraDisconnected();
        //    break;
        //}
        if (camera->isRecording)
        {
            progress = (camera->currentFrameCount * 100) / camera->totalFrames;
            emit UpdateProgressbar(progress);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void SleepMonitorMain::onUpdateProgressbar(int progress)
{
    ui->recordingProgressBar->setValue(progress);
}
