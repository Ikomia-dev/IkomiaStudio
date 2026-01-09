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

#include "CVideoDisplay.h"
#include "Workflow/CViewPropertyIO.h"
#include "View/Common/CDialog.h"
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QProgressDialog>
#include "Main/AppTools.hpp"
#include "View/DoubleView/Image/CImageExportDlg.h"

CVideoDisplay::CVideoDisplay(QWidget* parent, const QString& name, int flags) : CDataDisplay(parent, name, flags)
{
    initLayout();
    initConnections();

    if(flags & PLAYER_SYNC)
        initPlayer();

    m_typeId = DisplayType::VIDEO_DISPLAY;
}

CVideoDisplay::~CVideoDisplay()
{
}

bool CVideoDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doToggleMaximize(this);
            return true;
        }
    }
    return CDataDisplay::eventFilter(obj, event);
}

void CVideoDisplay::applyViewProperty()
{
    CDataDisplay::applyViewProperty();

    if(m_bUpdateViewProperty == true)
    {
        m_pImgDisplay->applyZoomViewProperty();
        m_bUpdateViewProperty = false;
    }
}

void CVideoDisplay::stopPlayer()
{
    emit doStopTimer();
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    setSliderPos(0);
    // For synchronization
    emit doSyncStop();
}

void CVideoDisplay::enableGraphicsInteraction(bool bEnable)
{
    if(m_pImgDisplay)
        m_pImgDisplay->enableGraphicsInteraction(bEnable);
}

void CVideoDisplay::initLayout()
{
    setObjectName("CVideoDisplay");
    m_pImgDisplay = new CImageDisplay(this, "",
                                      CImageDisplay::ZOOM_BUTTONS |
                                      CImageDisplay::GRAPHICS_BUTTON |
                                      CImageDisplay::SAVE_BUTTON |
                                      CImageDisplay::EXPORT_BUTTON);
    m_pImgDisplay->setSelectOnClick(false);

    auto str = QDateTime::fromSecsSinceEpoch(0).toUTC().toString("mm:ss");
    m_pTotalTime = new QLabel(str);
    m_pCurrentTime = new QLabel(str);

    m_pPlayBtn = createButton(QIcon(":/Images/play.png"), tr("Play"));
    m_pStopBtn = createButton(QIcon(":/Images/stop.png"), tr("Stop"));
    m_pRecordBtn = createButton(QIcon(":/Images/record.png"), tr("Record"));

    m_pSliderPos = new QSlider(Qt::Horizontal);
    m_pSliderPos->setValue(0);

    int index = 1;
    m_pHbox->insertStretch(index++, 1);
    m_pHbox->insertWidget(index++, m_pPlayBtn);
    m_pHbox->insertWidget(index++, m_pStopBtn);
    m_pHbox->insertWidget(index++, m_pRecordBtn);
    m_pHbox->insertWidget(index++, m_pSliderPos, 2);
    m_pHbox->insertWidget(index++, m_pCurrentTime);
    m_pHbox->insertWidget(index++, new QLabel("/"));
    m_pHbox->insertWidget(index++, m_pTotalTime);

    if(m_flags & RESULTS_BTN)
    {
        m_pSaveBtn = createButton(QIcon(":/Images/save.png"), tr("Save video"));
        m_pExportBtn = createButton(QIcon(":/Images/export.png"), tr("Export video"));

        m_pHbox->insertStretch(index++, 1);
        m_pHbox->insertWidget(index++, m_pSaveBtn);
        m_pHbox->insertWidget(index++, m_pExportBtn);
    }
    m_pLayout->addWidget(m_pImgDisplay);
}

void CVideoDisplay::initConnections()
{
    //Inner image display signals
    connect(m_pImgDisplay, &CImageDisplay::doDoubleClicked, [&]{ emit doImageDoubleClicked(); });
    connect(m_pImgDisplay, &CImageDisplay::doSave, [&]{ emit doSaveCurrentFrame(); });
    connect(m_pImgDisplay, &CImageDisplay::doExport, [&](const QString& path, bool bWithGraphics){ emit doExportCurrentFrame(path, bWithGraphics); });
    connect(m_pImgDisplay, &CImageDisplay::doToggleGraphicsToolbar, [&]{ emit doToggleGraphicsToolbar(); });

    //Video specific signals
    connect(m_pPlayBtn, &QPushButton::clicked, this, &CVideoDisplay::onPlayVideo);
    connect(m_pStopBtn, &QPushButton::clicked, this, &CVideoDisplay::onStopVideo);
    connect(m_pSliderPos, &QSlider::valueChanged, this, &CVideoDisplay::onUpdateVideoPos);
    connect(m_pRecordBtn, &QPushButton::clicked, this, &CVideoDisplay::onRecordVideo);

    if(m_flags & RESULTS_BTN)
    {
        connect(m_pSaveBtn, &QPushButton::clicked, this, &CVideoDisplay::onSaveVideo);
        connect(m_pExportBtn, &QPushButton::clicked, this, &CVideoDisplay::onExportVideo);
    }

    connect(this, &CVideoDisplay::doSelect, [&](CDataDisplay* pDisplay)
    {
        Q_UNUSED(pDisplay);
        setSelected(true);
    });
}

void CVideoDisplay::initPlayer()
{
    m_pTimer = new QTimer(this);
    m_pTimer->setTimerType(Qt::PreciseTimer);
    connect(m_pTimer, &QTimer::timeout, this, &CVideoDisplay::onLoadNextFrame);
    connect(this, &CVideoDisplay::doStartTimer, this, &CVideoDisplay::onStartTimer);
    connect(this, &CVideoDisplay::doStopTimer, this, &CVideoDisplay::onStopTimer);
}

void CVideoDisplay::emitVideoInfo()
{
    emit doSetSliderLength(m_length);
    emit doSetSliderPos(m_currentPos);
    emit doSetFPS(m_fps);
    emit doSetTotalTime(m_totalTime);
    emit doSetCurrentTime(m_currentTime);
    emit doSetPlayVideo(!m_bIsPaused);
}

CImageDisplay* CVideoDisplay::getImageDisplay() const
{
    return m_pImgDisplay;
}

void CVideoDisplay::setSourceType(const CDataVideoBuffer::Type& type)
{
    int index;
    m_sourceType = type;
    m_bIsStream = (type == CDataVideoBuffer::OPENNI_STREAM || type == CDataVideoBuffer::ID_STREAM ||
                   type == CDataVideoBuffer::IP_STREAM || type == CDataVideoBuffer::PATH_STREAM);

    if(m_bIsStream)
    {
        index = 4;
        m_pHbox->insertWidget(index, m_pRecordBtn);

        if(m_flags & RESULTS_BTN)
        {
            m_pHbox->removeWidget(m_pSaveBtn);
            m_pHbox->removeWidget(m_pExportBtn);
            m_pSaveBtn->setParent(nullptr);
            m_pExportBtn->setParent(nullptr);
        }
    }
    else
    {
        m_pHbox->removeWidget(m_pRecordBtn);
        m_pRecordBtn->setParent(nullptr);

        if(m_flags & RESULTS_BTN)
        {
            index = 9;
            m_pHbox->insertWidget(index++, m_pSaveBtn);
            m_pHbox->insertWidget(index++, m_pExportBtn);
        }
    }
}

void CVideoDisplay::setImage(CImageScene *pScene, QImage image, QString name, bool bZoomFit)
{
    // Get image display and put new image in it
    m_pImgDisplay->setViewProperty(m_pViewProperty);
    m_pImgDisplay->setName(name);

    if(pScene)
    {
        m_pImgDisplay->setImage(pScene, image, bZoomFit);
        auto pLayer = pScene->getCurrentGraphicsLayer();
        m_pImgDisplay->getView()->setCurrentGraphicsLayer(pLayer);
    }
    else
        m_pImgDisplay->setImage(image, bZoomFit);
}

void CVideoDisplay::setViewSpaceShared(bool bShared)
{
    m_pImgDisplay->setViewSpaceShared(bShared);
}

void CVideoDisplay::setSelected(bool bSelect)
{
    if(bSelect)
    {
        QString css = QString("CVideoDisplay { border: 2px solid %1; }").arg(qApp->palette().highlight().color().name());
        setStyleSheet(css);
    }
    else
    {
        QString css = QString("CVideoDisplay { border: 2px solid %1; }").arg(qApp->palette().base().color().name());
        setStyleSheet(css);
    }
}

void CVideoDisplay::setActive(bool bActive)
{
    CDataDisplay::setActive(bActive);
    if(!bActive)
        stopPlayer();
}

bool CVideoDisplay::isPlaying() const
{
    return m_pTimer && m_pTimer->isActive();
}

void CVideoDisplay::onSetSliderLength(size_t length)
{
    m_length = length;
    m_pSliderPos->setRange(0, length-1);
    // For synchronization
    emit doSetSliderLength(length);
}

void CVideoDisplay::onSetSliderPos(size_t pos)
{
    if(m_bIsPaused == false && m_bLastFrame == true)
    {
        stopVideo();
        m_bLastFrame = false;
    }
    else
    {
        m_currentPos = pos;
        setSliderPos(pos);
        size_t maxPos = m_pSliderPos->maximum();
        m_bLastFrame = (maxPos > 0 && pos == maxPos);
        // For synchronization
        emit doSetSliderPos(pos);
    }
}

void CVideoDisplay::onSetFPS(double fps)
{
    m_fps = fps;
    // For synchronization
    emit doSetFPS(fps);
}

void CVideoDisplay::onSetTotalTime(size_t totalTime)
{
    QString timePattern;
    if(totalTime < 3600)
        timePattern = "mm:ss";
    else
        timePattern = "hh:mm:ss";

    m_totalTime = totalTime;
    auto str = QDateTime::fromSecsSinceEpoch(totalTime).toUTC().toString(timePattern);
    m_pTotalTime->setText(str);
    // For synchronization
    emit doSetTotalTime(totalTime);
}

void CVideoDisplay::onSetCurrentTime(size_t currentTime)
{
    QString timePattern;
    if(m_totalTime < 3600)
        timePattern = "mm:ss";
    else
        timePattern = "hh:mm:ss";

    m_currentTime = currentTime;
    auto str = QDateTime::fromSecsSinceEpoch(currentTime).toUTC().toString(timePattern);
    m_pCurrentTime->setText(str);
    // For synchronization
    emit doSetCurrentTime(currentTime);
}

bool CVideoDisplay::hasStreamOptions() const
{
    return m_bIsStream;
}

void CVideoDisplay::onSetSliderPosSync(size_t pos)
{
    m_currentPos = pos;
    setSliderPos(pos);
}

void CVideoDisplay::onUpdateVideoPos(size_t pos)
{
    emit doUpdateVideoPos(pos);
}

void CVideoDisplay::onStartTimer(int msec)
{
    emit doNotifyVideoStart();
    m_pTimer->start(msec);
}

void CVideoDisplay::onStopTimer()
{
    m_pTimer->stop();
}

void CVideoDisplay::onSyncPlayVideo()
{
    if(m_bIsPaused == true)
    {
        m_bIsPaused = false;
        m_pPlayBtn->setIcon(QIcon(":/Images/pause.png"));

        if(m_fps == 0)
        {
            qDebug() << "Wrong fps number.";
            return;
        }
        else
            emit doStartTimer((int)(1000/m_fps));
    }
    else
    {
        m_bIsPaused = true;
        m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
        emit doStopTimer();
    }
}

void CVideoDisplay::onSyncSetPlayVideo(bool bPlay)
{
    if(m_bIsPaused == bPlay)
        onSyncPlayVideo();
}

void CVideoDisplay::onSyncStopVideo()
{
    emit doStopTimer();
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    setSliderPos(0);
}

void CVideoDisplay::onPlayVideo()
{
    if(m_bIsPaused == true)
        playVideo();
    else
        pauseVideo();
}

void CVideoDisplay::onStopVideo()
{
    stopVideo();

    if(m_bIsRecording == true)
    {
        m_bIsRecording = false;
        m_pRecordBtn->setIcon(QIcon(":/Images/record.png"));
        emit doRecordVideo(m_bIsRecording);
    }
}

void CVideoDisplay::onRecordVideo()
{
    if(m_bIsRecording == true)
    {
        // Stop record
        m_bIsRecording = false;
        m_pRecordBtn->setIcon(QIcon(":/Images/record.png"));
        m_pRecordBtn->setToolTip(tr("Record video"));
    }
    else
    {
        // start record
        m_bIsRecording = true;
        m_pRecordBtn->setIcon(QIcon(":/Images/stop-record.png"));
        m_pRecordBtn->setToolTip(tr("Stop record"));

        if(m_bIsPaused == true)
            playVideo();
    }
    emit doRecordVideo(m_bIsRecording);
}

void CVideoDisplay::onSaveVideo()
{
    emit doSaveVideo();
}

void CVideoDisplay::onExportVideo()
{
    auto dataType = (m_sourceType == CDataVideoBuffer::IMAGE_SEQUENCE ? CImageExportDlg::IMAGE_SEQUENCE : CImageExportDlg::VIDEO);
    CImageExportDlg exportDlg(tr("Export video"), dataType, this);

    if(exportDlg.exec() == QDialog::Accepted)
        emit doExportVideo(exportDlg.getFileName(), exportDlg.isGraphicsExported());
}

void CVideoDisplay::onLoadNextFrame()
{
    emit doPlayVideo();
}

void CVideoDisplay::playVideo()
{
    if(m_bIsPaused == false)
        return;

    m_bIsPaused = false;
    m_pPlayBtn->setIcon(QIcon(":/Images/pause.png"));

    if(m_fps == 0)
    {
        qDebug() << "Wrong fps number.";
        return;
    }
    else
        emit doStartTimer((int)(1000/m_fps));

    // For synchronization
    emit doSyncPlay();
}

void CVideoDisplay::pauseVideo()
{
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    emit doStopTimer();

    // For synchronization
    emit doSyncPlay();
}

void CVideoDisplay::stopVideo()
{
    emit doStopTimer();
    emit doStopVideo();
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    setSliderPos(0);
    // For synchronization
    emit doSyncStop();
}

void CVideoDisplay::setSliderPos(size_t pos)
{
    bool bState = m_pSliderPos->blockSignals(true);
    m_pSliderPos->setValue(pos);
    m_pSliderPos->blockSignals(bState);
}
