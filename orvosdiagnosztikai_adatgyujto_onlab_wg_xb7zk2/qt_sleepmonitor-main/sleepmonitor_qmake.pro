QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cameraclass.cpp \
    main.cpp \
    sleepmonitormain.cpp

HEADERS += \
    cameraclass.h \
    sleepmonitormain.h

FORMS += \
    sleepmonitormain.ui
	
INCLUDEPATH += C:\opencv\build\include
INCLUDEPATH += C:\Spinnaker\include


LIBS += C:\opencv\build\x64\vc15\lib\opencv_world460.lib
LIBS += C:\opencv\build\x64\vc15\lib\opencv_world460d.lib
LIBS += C:\Spinnaker\lib64\vs2015\Spinnaker_v140.lib
LIBS += C:\Spinnaker\lib64\vs2015\SpinVideo_v140.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Assets/Icons/connectIcon.png \
    Assets/Icons/pauseIcon.png \
    Assets/Icons/playIcon.png \
    Assets/Icons/previewIcon.png \
    Assets/Icons/recordIcon.png \
    Assets/Icons/stopIcon.png \
    Assets/Icons/stopPreviewIcon.png

RC_ICONS = Assets/Icons/thermalIcon.ico
