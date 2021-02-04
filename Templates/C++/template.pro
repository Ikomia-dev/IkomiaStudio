#-------------------------------------------------
#
# Project created by Ikomia Studio
#
#-------------------------------------------------
defineReplace(changePathQt) {
    var1 = $$1
    var2 = $${var1}.framework/Versions/5/$$var1
    var3 = $$2
    var4 = $$quote(install_name_tool -change /usr/local/opt/qt/lib/$$var2 @executable_path/../Frameworks/$$var2 $$var3 $$escape_expand(\n\t))

    return($$var4)
}

QT += gui core widgets sql

win32: DESTDIR = $$(USERPROFILE)/Ikomia/Plugins/C++/$$TARGET
unix: DESTDIR = $$(HOME)/Ikomia/Plugins/C++/$$TARGET

TEMPLATE = lib
VERSION = 1.0.0
CONFIG += plugin
CV_VERSION = 4.0.1
OPENCV_VERSION = 401

DEFINES += _PluginNameUpperCase__LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx{
    QMAKE_CC = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
}

CONFIG += c++14

# Enable OpenMP
msvc {
  QMAKE_CXXFLAGS += -openmp -arch:AVX2 -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}

unix: QMAKE_CXXFLAGS += -fopenmp
unix:!macx: QMAKE_LFLAGS +=  -fopenmp
macx: LIBS += -lomp

#Source files
SOURCES += \
        _PluginName_.cpp

HEADERS += \
        _PluginName_.h \
        _PluginName_Global.hpp

# Main Include path
INCLUDEPATH += /usr/local/include

# Ikomia Path
LIBS += -L_APIFolder_/lib/
macx:LIBS += -L/Applications/Ikomia.app/Contents/Frameworks/

#Global lib path
win32:LIBS += -Lc:/msys64/mingw/lib/
unix:!macx:LIBS += -L/usr/local/lib64/
macx:LIBS += -L/usr/local/lib/

#Dynamic link to Boost
win32:LIBS += -L$$(PROGRAMFILES)/Boost/lib
win32:LIBS += -lboost_filesystem-mt -lboost_system-mt
unix:LIBS += -lboost_filesystem -lboost_system

#Dynamic link with OpenCV
win32: INCLUDEPATH += $$(PROGRAMFILES)/OpenCV/include
unix:!macx: INCLUDEPATH += _APIFolder_/include/opencv4
macx:INCLUDEPATH += _APIFolder_/include/opencv4

win32:LIBS += -L$$(PROGRAMFILES)/OpenCV/x64/vc15/lib
win32:LIBS += -lopencv_core343 -lopencv_objdetect343 -lopencv_photo343 -lopencv_ximgproc343 -lopencv_imgproc343
unix:!macx:LIBS += -lopencv_core -lopencv_imgproc
macx:LIBS += -lopencv_core.$${CV_VERSION} -lopencv_imgproc.$${CV_VERSION}

#Dynamic link with Ikomia Utils
win32:LIBS += -lUtils
unix:!macx:LIBS += -lUtils
macx:LIBS += -lUtils.1
INCLUDEPATH += _APIFolder_/include/Utils

#Dynamic link with Ikomia Core
win32:LIBS += -lCore
unix:!macx:LIBS += -lCore
macx:LIBS += -lCore.1
INCLUDEPATH += _APIFolder_/include/Core

#Dynamic link with Ikomia DataProcess
win32:LIBS += -lDataProcess
unix:!macx:LIBS += -lDataProcess
macx:LIBS += -lDataProcess.1
INCLUDEPATH += _APIFolder_/include/DataProcess

DEPENDPATH += _APIFolder_

# DEPLOYMENT
macx {
    makeDeploy.path = $$DESTDIR
    TARGET_LIB = $$DESTDIR/$${QMAKE_PREFIX_SHLIB}$${TARGET}.$${VERSION}.$${QMAKE_EXTENSION_SHLIB}
    # QtWidgets
    makeDeploy.commands += $$changePathQt(QtWidgets,$$TARGET_LIB)
    # QtCore
    makeDeploy.commands += $$changePathQt(QtCore,$$TARGET_LIB)
    # QtGui
    makeDeploy.commands += $$changePathQt(QtGui,$$TARGET_LIB)
    # QtSql
    makeDeploy.commands += $$changePathQt(QtSql,$$TARGET_LIB)
    INSTALLS += makeDeploy
}


