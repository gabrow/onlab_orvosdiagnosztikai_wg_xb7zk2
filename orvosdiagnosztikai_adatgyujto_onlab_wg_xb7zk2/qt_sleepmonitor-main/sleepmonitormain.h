#ifndef SLEEPMONITORMAIN_H
#define SLEEPMONITORMAIN_H

#include <thread>
#include <QMainWindow>
#include <QProgressDialog>
#include <QTimer>

#include "cameraclass.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SleepMonitorMain; }
QT_END_NAMESPACE

class SleepMonitorMain : public QMainWindow
{
    Q_OBJECT

public:

    SleepMonitorMain(QWidget *parent = nullptr, CameraClass *cam = nullptr);
    ~SleepMonitorMain();

    /* Functions */
    void GUIGetCamera(int *result);
    void DisplayPreview();
    void StartRecording();
    void StatusCheck();

    void CameraTest();

    /* Variables */

private slots:
    void on_startRecordingButton_clicked();
    void on_recordTimeMin_valueChanged(int arg1);
    void on_recordTimeHour_valueChanged(int arg1);
    void on_connectButton_clicked();
    void on_recordParts_valueChanged(int arg1);
    void on_showPreviewButton_clicked();
    void on_hidePreviewButton_clicked();
    void on_gainSlider_valueChanged(int value);
    void on_offsetSlider_valueChanged(int value);
    void on_defaultButton_clicked();
    void on_stopRecordingButton_clicked();



private:
    Ui::SleepMonitorMain *ui;
    CameraClass *camera;

    /* Functions */
    void UpdateDisplayedRecordTime(int recordTime); 

    /* Variables */
    bool isConnected = false;
    bool isTimeNull = true;
    bool isPreviewActive = false;
    bool isConnecting = false;
    bool isClosing = false;

    int recordMinute = 0;
    int recordHour = 0;
    int recordParts = 1;
    int recordTime = 0;

    int progress = 0;


    /* Threads */
    std::thread statusCheckThread;
    std::mutex statusCheckMutex;

    std::thread recordingThread;
    std::mutex recordingMutex;

    std::thread previewThread;
    std::mutex previewMutex;

public slots:
    void onConnectionFinished(int result);
    void onCameraDisconnected();

    void onUpdateProgressbar(int progress);

    void onRecordingEnded();
    void onRecordingStarted();

    void onPreviewStarted();
    void onPreviewEnded();



signals:
    void ConnectionFinished(int result);
    void CameraDisconnected();

    void UpdateProgressbar(int progress);

    void RecordingStarted();
    void RecordingEnded();

    void PreviewStarted();
    void PreviewEnded();

};
#endif // SLEEPMONITORMAIN_H
