#ifndef CVIDEODISPLAY_H
#define CVIDEODISPLAY_H

#include "../CDataDisplay.h"
#include "../Image/CImageDisplay.h"

class CVideoDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        enum DataDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            PLAYER_SYNC = 0x00000010,
            RESULTS_BTN = 0x00000020,
            PLAYER = TITLE | MAXIMIZE_BUTTON | PLAYER_SYNC,
            RESULTS = TITLE | MAXIMIZE_BUTTON | RESULTS_BTN
        };

        CVideoDisplay(QWidget* parent = nullptr, const QString &name = "", int flags = TITLE);
        ~CVideoDisplay();

        void            initLayout();
        void            initConnections();
        void            initPlayer();

        CImageDisplay*  getImageDisplay() const;

        void            setStreamOptions(bool bEnable);
        void            setImage(CImageScene *pScene, QImage image, QString name, bool bZoomFit);
        virtual void    setViewSpaceShared(bool bShared) override;
        virtual void    setSelected(bool bSelect) override;

        bool            hasStreamOptions() const;
        bool            isPlaying() const;

        void            emitVideoInfo();

        bool            eventFilter(QObject* obj, QEvent* event) override;
        virtual void    applyViewProperty() override;

    signals:

        void            doPlayVideo();

        void            doStopVideo();
        void            doStopPlayer();

        void            doUpdateVideoPos(int pos);

        void            doStartPlayer(int msec);

        void            doSaveVideo();
        void            doSaveCurrentFrame();

        void            doExportVideo(const QString& filename);
        void            doExportCurrentFrame(const QString& path, bool bWithGraphics);

        void            doNotifyVideoStart();

        void            doRecordVideo(bool bRecord);

        void            doImageDoubleClicked();

        void            doToggleGraphicsToolbar();

        // Sync signals
        void            doSyncPlay();
        void            doSyncStop();
        void            doSetTotalTime(int totalTime);
        void            doSetCurrentTime(int currentTime);
        void            doSetSliderLength(int length);
        void            doSetSliderPos(int pos);
        void            doSetFPS(double fps);
        void            doSetPlayVideo(bool bPlay);

    public slots:

        void            onSetSliderLength(int length);
        void            onSetSliderPos(int pos);
        void            onSetFPS(double fps);
        void            onSetTotalTime(int totalTime);
        void            onSetCurrentTime(int currentTime);
        void            onSetSliderPosSync(int pos);

        void            onUpdateVideoPos(int pos);

        void            onStartPlayer(int msec);
        void            onStopPlayer();

        void            onSyncPlayVideo();
        void            onSyncSetPlayVideo(bool bPlay);
        void            onSyncStopVideo();

        void            onPlayVideo();
        void            onStopVideo();

        void            onRecordVideo();

        void            onSaveVideo();
        void            onExportVideo();

    private slots:

        void            onLoadNextFrame();

    private:

        void            playVideo();
        void            pauseVideo();
        void            stopVideo();

    private:

        CImageDisplay*  m_pImgDisplay = nullptr;
        QSlider*        m_pSliderPos = nullptr;
        QTimer*         m_pTimer = nullptr;
        QLabel*         m_pCurrentTime = nullptr;
        QLabel*         m_pTotalTime = nullptr;
        QPushButton*    m_pPlayBtn = nullptr;
        QPushButton*    m_pStopBtn = nullptr;
        QPushButton*    m_pRecordBtn = nullptr;
        QPushButton*    m_pSaveBtn = nullptr;
        QPushButton*    m_pExportBtn = nullptr;
        QPushButton*    m_pMaximizeBtn = nullptr;
        bool            m_bIsPaused = true;
        bool            m_bIsRecording = false;
        bool            m_bIsStream = false;
        bool            m_bLastFrame = false;
        double          m_fps = 0;
        int             m_currentTime = 0;
        int             m_totalTime = 0;
        int             m_length = 0;
        int             m_currentPos = 0;
};

#endif // CVIDEODISPLAY_H
