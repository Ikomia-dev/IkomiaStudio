// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CCameraDlg.h"
#include <QGridLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QGroupBox>
#include <regex>
#include "CDataVideoBuffer.h"

#ifdef Q_OS_WIN
#include "CWindowsDeviceEnumerator.h"
#endif

CCameraDlg::CCameraDlg(QWidget* parent, Qt::WindowFlags f) :
    CDialog(tr("Camera choice"), parent, DEFAULT_FIXED|EFFECT_ENABLED, f)
{
    init();
    addManualCam();
    addIPCam();
}

void CCameraDlg::addCam(const QString& name, int id)
{
    // Add cam with a name and an available id for opencv videoCapture
    QRadioButton* pRadio = new QRadioButton(name);

    m_pLayout->insertWidget(0, pRadio);

    connect(pRadio, &QRadioButton::clicked, [=]{
        m_cameraMode = QString::number(id);
        m_cameraName = pRadio->text(); });
}

void CCameraDlg::addCam(const QString& name, const QString& path)
{
    // Add cam with a name and an available path for opencv videoCapture
    QRadioButton* pRadio = new QRadioButton(name);

    m_pLayout->insertWidget(0, pRadio);

    connect(pRadio, &QRadioButton::clicked, [=]{
        m_cameraMode = path;
        m_cameraName = pRadio->text(); });
}

void CCameraDlg::findCamera()
{
    std::vector<QString> camName, camList;
    if(m_bUseOpenNI)
    {
        camName = {"Kinect Depth", "Kinect Color"};
        camList = {QString::number(cv::CAP_OPENNI + cv::CAP_OPENNI_DEPTH_MAP),
                                        QString::number(cv::CAP_OPENNI + cv::CAP_OPENNI_BGR_IMAGE)};
    }

    try
    {
#if defined(_WIN32)
    addCamUSB_Win(camName, camList);
#elif defined(__MACH__)
    addCamUSB_Mac(camName, camList);
#elif (defined(linux) || defined(__linux__) || defined(__linux))
    addCamUSB_Linux(camName, camList);
#endif
    }
    catch(std::exception& e)
    {
        qCritical().noquote() << QString(e.what());
    }

    int index = 0;
    // find cameras from list
    for(auto&& id : camList)
    {
        CDataVideoBuffer camTester;
        try
        {
            camTester.setVideoPath(id.toStdString());
            addCam(camName[index], id);
        }
        catch(std::exception& e)
        {
            qDebug() << e.what();
        }
        index++;
    }
}

QString CCameraDlg::getCameraName()
{
    return m_cameraName;
}

QString CCameraDlg::getCameraMode()
{
    return m_cameraMode;
}

void CCameraDlg::init()
{
    // Main layout with Ok/Cancel buttons
    m_pLayout = new QVBoxLayout;

    auto pLayout = getContentLayout();

    QPushButton* pOkBtn = new QPushButton(tr("Ok"));
    QPushButton* pCancelBtn = new QPushButton(tr("Cancel"));

    QGroupBox* pGroupBox = new QGroupBox(tr("Available cameras"));
    pGroupBox->setLayout(m_pLayout);

    QHBoxLayout* pHBox = new QHBoxLayout;

    pHBox->addWidget(pOkBtn);
    pHBox->addWidget(pCancelBtn);

    pLayout->addWidget(pGroupBox);
    pLayout->addLayout(pHBox);

    connect(pOkBtn, &QPushButton::clicked, this, &CDialog::accept);
    connect(pCancelBtn, &QPushButton::clicked, this, &CDialog::reject);
}

void CCameraDlg::addIPCam()
{
    QRadioButton* pRadio = new QRadioButton(tr("IP Cam"));
    QLineEdit* pLink = new QLineEdit;
    pLink->setPlaceholderText(tr("<Cam IP Address>"));
    pLink->setEnabled(false);

    QHBoxLayout* pHBox = new QHBoxLayout;
    pHBox->addWidget(pRadio, 1);
    pHBox->addWidget(pLink, 1);

    m_pLayout->insertLayout(0, pHBox);

    connect(pRadio, &QRadioButton::toggled, [=](bool checked){
        pLink->setEnabled(checked);
        if(checked)
            m_cameraName = pRadio->text();
    });
    connect(pLink, &QLineEdit::textChanged, [=](const QString& str){ m_cameraMode = str; });
}

void CCameraDlg::addManualCam()
{
    QRadioButton* pRadio = new QRadioButton(tr("Manual Cam"));
    QLineEdit* pName = new QLineEdit;
    pName->setPlaceholderText(tr("<Cam path or id>"));
    pName->setEnabled(false);

    QHBoxLayout* pHBox = new QHBoxLayout;
    pHBox->addWidget(pRadio, 1);
    pHBox->addWidget(pName, 1);

    m_pLayout->insertLayout(0, pHBox);

    connect(pRadio, &QRadioButton::toggled, [=](bool checked){
        pName->setEnabled(checked);
        if(checked)
            m_cameraName = pRadio->text();
    });
    connect(pName, &QLineEdit::textChanged, [=](const QString& str){ m_cameraMode = str; });
}

void CCameraDlg::addCamUSB_Linux(std::vector<QString>& camName, std::vector<QString>& camPath)
{
    QString path = "/dev/v4l/by-id/";
    QDir dir(path);
    QStringList filters;
    filters << "*video*";
    auto camList = dir.entryInfoList(filters);

    for(auto&& it : camList)
    {
        auto target = it.canonicalFilePath();
        camName.emplace_back(target);
        camPath.emplace_back(target);
    }
}

void CCameraDlg::addCamUSB_Mac(std::vector<QString> &camName, std::vector<QString> &camPath)
{
    int number = 1;// 0 is for integrated camera and we don't want it for the moment
    CDataVideoBuffer camTester;
    while(true)
    {
        try
        {
            camTester.openStreamFromId(number);
            camName.emplace_back("Cam_"+QString::number(number));
            camPath.emplace_back(QString::number(number));
            number++;
        }
        catch(CException& /*e*/)
        {
            qDebug() << "Available cameras: " << number;
            break;
        }

    }

}

void CCameraDlg::addCamUSB_Win(std::vector<QString>& camName, std::vector<QString>& camPath)
{
#if defined(Q_OS_WIN)
    CWindowsDeviceEnumerator deviceEnum;

    std::map<int, Device> devices = deviceEnum.getVideoDevicesMap();

    for (auto const &device : devices)
    {
        camName.emplace_back(QString::fromStdString(device.second.deviceName));
        camPath.emplace_back(QString::number(device.first));
    }
#else
    Q_UNUSED(camName);
    Q_UNUSED(camPath);
#endif
}
