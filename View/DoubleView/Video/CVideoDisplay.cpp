#include "CVideoDisplay.h"
#include "Protocol/CViewPropertyIO.h"
#include "View/Common/CDialog.h"
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QProgressDialog>
#include "Main/AppTools.hpp"

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

void CVideoDisplay::initLayout()
{
    setObjectName("CVideoDisplay");
    m_pImgDisplay = new CImageDisplay(this, "",
                                      CImageDisplay::ZOOM_BUTTONS |
                                      CImageDisplay::GRAPHICS_BUTTON |
                                      CImageDisplay::SAVE_BUTTON |
                                      CImageDisplay::EXPORT_BUTTON);
    m_pImgDisplay->setSelectOnClick(false);

    auto str = QDateTime::fromTime_t(0).toUTC().toString("mm:ss");
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
    connect(m_pSliderPos, &QSlider::sliderMoved, this, &CVideoDisplay::onUpdateVideoPos);
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
    connect(this, &CVideoDisplay::doStartPlayer, this, &CVideoDisplay::onStartPlayer);
    connect(this, &CVideoDisplay::doStopPlayer, this, &CVideoDisplay::onStopPlayer);
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

void CVideoDisplay::setStreamOptions(bool bEnable)
{
    int index;
    m_bIsStream = bEnable;

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

bool CVideoDisplay::isPlaying() const
{
    return m_pTimer->isActive();
}

void CVideoDisplay::onSetSliderLength(int length)
{
    m_length = length;
    m_pSliderPos->setRange(0, length-1);
    // For synchronization
    emit doSetSliderLength(length);
}

void CVideoDisplay::onSetSliderPos(int pos)
{
    if(m_bIsPaused == false && m_bLastFrame == true)
    {
        stopVideo();
        m_bLastFrame = false;
    }
    else
    {
        m_currentPos = pos;
        m_pSliderPos->setValue(pos);
        int maxPos = m_pSliderPos->maximum();
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

void CVideoDisplay::onSetTotalTime(int totalTime)
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

void CVideoDisplay::onSetCurrentTime(int currentTime)
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

void CVideoDisplay::onSetSliderPosSync(int pos)
{
    m_currentPos = pos;
    m_pSliderPos->setValue(pos);
}

void CVideoDisplay::onUpdateVideoPos(int pos)
{
    emit doUpdateVideoPos(pos);
}

void CVideoDisplay::onStartPlayer(int msec)
{
    emit doNotifyVideoStart();
    m_pTimer->start(msec);
}

void CVideoDisplay::onStopPlayer()
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
            emit doStartPlayer((int)(1000/m_fps));
    }
    else
    {
        m_bIsPaused = true;
        m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
        emit doStopPlayer();
    }
}

void CVideoDisplay::onSyncSetPlayVideo(bool bPlay)
{
    if(m_bIsPaused == bPlay)
        onSyncPlayVideo();
}

void CVideoDisplay::onSyncStopVideo()
{
    emit doStopPlayer();
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    m_pSliderPos->setValue(0);
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
    QSettings IkomiaSettings;

    auto fileName = Utils::File::saveFile(this, tr("Save Video"), IkomiaSettings.value(_DefaultDirVideoExport).toString(), tr("avi Files (*.avi)"), QStringList("avi"), ".avi");
    if(fileName.isEmpty())
        return;

    IkomiaSettings.setValue(_DefaultDirVideoExport, QFileInfo(fileName).path());
    emit doExportVideo(fileName);
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
        emit doStartPlayer((int)(1000/m_fps));

    // For synchronization
    emit doSyncPlay();
}

void CVideoDisplay::pauseVideo()
{
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    emit doStopPlayer();

    // For synchronization
    emit doSyncPlay();
}

void CVideoDisplay::stopVideo()
{
    emit doStopPlayer();
    emit doStopVideo();
    m_bIsPaused = true;
    m_pPlayBtn->setIcon(QIcon(":/Images/play.png"));
    m_pSliderPos->setValue(0);
    emit doUpdateVideoPos(0);
    // For synchronization
    emit doSyncStop();
}
