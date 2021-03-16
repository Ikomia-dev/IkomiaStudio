/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CVIDEOMANAGER_H
#define CVIDEOMANAGER_H

#include <QObject>
#include "CVideoDataManager.h"

class CProjectManager;
class CImageScene;
class CProgressBarManager;
class CProtocolManager;
class CGraphicsManager;
class CResultManager;

using CVideoDataMgrPtr = std::shared_ptr<CVideoDataManager>;

//------------------------//
//----- CVideoPlayer -----//
//------------------------//
class CVideoPlayer : public QObject
{
    Q_OBJECT

    public:

        CVideoPlayer();
        CVideoPlayer(const QModelIndex& wrapIndex);
        ~CVideoPlayer();

        void                setStream(bool bStream);
        void                setCurrentImage(const CMat& image);
        void                setCurrentProcessedImage();

        CVideoDataMgrPtr    getManager() const;
        QModelIndex         getWrapIndex() const;
        CMat                getCurrentImage() const;
        CMat                getImage();
        CMat                getImage(int pos);
        CMat                getSequenceImage(const QModelIndex& wrapIndex);
        std::string         getRecordPath() const;
        CDataVideoBuffer::Type  getSourceType() const;

        bool                isStream() const;
        bool                isPlaying() const;
        bool                isRecording() const;

        void                play(const QModelIndex& modelIndex, int index);

        void                startRecord(const std::string& path);

        void                stop();
        void                stopRecord();

        CMat                readImage();
        CMat                readImage(int index);

        void                recordImage(const CMat &image);

        void                releaseProcessedImage();

    signals:

        void                doImageIsLoaded(const QModelIndex& modelIndex, const CMat& image, int index, bool bNewSequence);
        void                doPlayError(const QModelIndex& modelIndex, const QString& error);

    private:

        CVideoDataMgrPtr        m_mgrPtr = nullptr;
        QPersistentModelIndex   m_wrapIndex;
        bool                    m_bStream = false;
        bool                    m_bRecord = false;
        std::atomic_bool        m_bStop{true};
        CMat                    m_currentImage;
        CMat                    m_currentProcessedImage;
        std::string             m_recordPath = "";
        std::mutex              m_playMutex;
        std::mutex              m_imgMutex;
        QFutureWatcher<void>    m_readWatcher;
        std::condition_variable m_threadCond;
};

//-------------------------//
//----- CVideoManager -----//
//-------------------------//
class CVideoManager : public QObject
{
    Q_OBJECT

    public:

        CVideoManager();
        ~CVideoManager();

        void                setManagers(CProjectManager* pProjectMgr, CProtocolManager* pProtocolMgr, CGraphicsManager *pGraphicsMgr,
                                        CResultManager *pResultMgr, CProgressBarManager* pProgressMgr);
        void                setVideoRecord(const QModelIndex &modelIndex, bool bEnable);
        void                setProgressSignalHandler(CProgressSignalHandler* pHandler);
        void                setSelectedImageFromWorkflow(const CMat& image);

        int                 getCurrentPos(const QModelIndex &index);
        CMat                getVideoImage(const QModelIndex& index);
        CMat                getVideoImage(const QModelIndex& index, int pos);
        CMat                getSequenceImage(const QModelIndex& datasetIndex, const QModelIndex &imgWrapIndex);
        CMat                getCurrentImage(const QModelIndex& index);
        CDataVideoInfoPtr   getVideoInfo(const QModelIndex& index);
        CDataVideoBuffer::Type  getSourceType(const QModelIndex& index);

        void                play(const QModelIndex& modelIndex, size_t inputIndex);

        void                displayVideoImage(const QModelIndex& modelIndex, size_t index, bool bNewSequence);
        void                displayImageSequence(const QModelIndex& modelIndex, size_t imageIndex, size_t index, bool bNewSequence);
        void                displayCurrentVideoImage(const QModelIndex& modelIndex, int index);

        void                updateVideoPos(const QModelIndex& modelIndex, size_t index, int pos);
        void                updateInfo(const QModelIndex& modelIndex, int index);

        bool                isPlaying(const QModelIndex& index);
        bool                isStream(const QModelIndex& index);

        void                stopPlay(const QModelIndex& index);

        void                closeData(const QModelIndex& index);

        void                beforeProjectClose(int projectIndex, bool bWithCurrentImage);

        void                saveCurrentFrame(const QModelIndex& index);

        void                exportCurrentFrame(const QModelIndex& index, const QString &path, bool bWithGraphics);

        void                enableInfoUpdate(const QModelIndex& index, bool bEnable);

    public slots:

        void                onNotifyVideoStart(const QModelIndex &index);
        void                onDisplayCurrentVideoImage(const QModelIndex& modelIndex, int index);
        void                onCloseProtocol();
        void                onImageIsLoaded(const QModelIndex &modelIndex, const CMat& image, int index, bool bNewSequence);
        void                onInitInfo(const QModelIndex &modelIndex, int index);

    signals:

        void                doUpdateVideoPos(int index, int pos);
        void                doUpdateImgSequenceIndex(const QModelIndex& index);

        void                doSetFPS(int index, int fps);
        void                doSetVideoLength(int index, int length);
        void                doSetTotalTime(int index, int totalTime);
        void                doSetCurrentTime(int index, int currentTime);
        void                doSetSourceType(int index, CDataVideoBuffer::Type type);

        void                doDisplayVideoImage(const QModelIndex& index, int inputIndex, QImage image, QString imgName);
        void                doDisplayVideoInfo(const VectorPairString& infoMap);

        void                doAddRecordVideo(const QString& path);

        void                doVideoDataChanged(const QModelIndex& index, int inputIndex, bool bNewSequence);

        void                doCurrentDataChanged(const QModelIndex& index, bool bNewSequence);

        void                doNotifyVideoStart(int frameCount);

        void                doStopProtocolThread();
        void                doStopRecording(const QModelIndex& index);
        void                doStopVideoPlayerView(const QModelIndex& index);

    private:

        void                initInfo(const QModelIndex &modelIndex, int index);

        CVideoPlayer*       getPlayer(const QModelIndex& index);

        void                clearPlayers();

        void                displayVideoInfo(const QModelIndex& index);

    private:

        std::map<QPersistentModelIndex, CVideoPlayer*> m_players;

        CProjectManager*        m_pProjectMgr = nullptr;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CProgressSignalHandler* m_pProgressSignal = nullptr;
        CGraphicsManager*       m_pGraphicsMgr = nullptr;
        CProtocolManager*       m_pProtocolMgr = nullptr;
        CResultManager*         m_pResultMgr = nullptr;
        CMat                    m_selectedWorkflowImage;
        bool                    m_bInfoUpdate = false;
};

#endif // CVIDEOMANAGER_H
