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

#include "CVideoManager.h"
#include "Main/LogCategory.h"
#include "Model/Project/CProjectManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "Model/Workflow/CWorkflowManager.h"
#include "Model/Graphics/CGraphicsManager.h"
#include "Model/Results/CResultManager.h"
#include "Data/CDataConversion.h"

//------------------------//
//----- CVideoPlayer -----//
//------------------------//
CVideoPlayer::CVideoPlayer()
{
    m_mgrPtr = std::make_shared<CVideoDataManager>();
}

CVideoPlayer::CVideoPlayer(const QModelIndex &wrapIndex)
{
    m_mgrPtr = std::make_shared<CVideoDataManager>();
    m_wrapIndex = wrapIndex;
}

CVideoPlayer::~CVideoPlayer()
{
    stop();
    m_mgrPtr->close();
}

void CVideoPlayer::setStream(bool bStream)
{
    m_bStream = bStream;
    m_mgrPtr->setLiveMode(bStream);
}

void CVideoPlayer::setCurrentImage(const CMat &image)
{
    m_currentImage = image;
}

void CVideoPlayer::setCurrentProcessedImage()
{
    m_currentProcessedImage = m_currentImage;
}

CVideoDataMgrPtr CVideoPlayer::getManager() const
{
    return m_mgrPtr;
}

QModelIndex CVideoPlayer::getWrapIndex() const
{
    return m_wrapIndex;
}

CMat CVideoPlayer::getCurrentImage() const
{
    // Use current image when no protocol but use processed image otherwise
    if(m_currentProcessedImage.empty())
        return m_currentImage;
    else
        return m_currentProcessedImage;
}

CMat CVideoPlayer::getImage()
{
    // if real video, play video => go one image forward and get it
    auto pDataset = CProjectUtils::getDataset<CMat>(m_wrapIndex);
    if(!pDataset)
        return CMat();

    std::lock_guard<std::mutex> lock(m_imgMutex);
    m_currentImage = m_mgrPtr->playVideo(*pDataset, m_readTimeout);
    return m_currentImage;
}

CMat CVideoPlayer::getImage(size_t pos)
{
    // If real video, set subset bound with new position image
    auto pDataset = CProjectUtils::getDataset<CMat>(m_wrapIndex);
    auto bounds = pDataset->whole();
    Utils::Data::setSubsetBounds(bounds, DataDimension::IMAGE, pos, pos);
    // Play video at this position and get it
    std::lock_guard<std::mutex> lock(m_imgMutex);
    m_currentImage = m_mgrPtr->playVideo(*pDataset, bounds, m_readTimeout);
    return m_currentImage;
}

CMat CVideoPlayer::getSequenceImage(const QModelIndex &wrapIndex)
{
    DimensionIndices indices = CProjectUtils::getIndicesInDataset(wrapIndex);
    if(indices.size() == 0)
        return CMat();

    auto pDataset = CProjectUtils::getDataset<CMat>(m_wrapIndex);
    if(!pDataset)
        return CMat();

    size_t imageIndex = pDataset->getDataInfo().index(indices);
    auto bounds = pDataset->subsetBounds(imageIndex);
    // Load data
    m_mgrPtr->loadData(*pDataset, bounds);
    m_currentImage = pDataset->dataAt(imageIndex);
    return m_currentImage;
}

std::string CVideoPlayer::getRecordPath() const
{
    return m_recordPath;
}

CDataVideoBuffer::Type CVideoPlayer::getSourceType() const
{
    return m_mgrPtr->getSourceType();
}

bool CVideoPlayer::isStream() const
{
    return m_bStream;
}

bool CVideoPlayer::isPlaying() const
{
    return !m_bStop;
}

bool CVideoPlayer::isRecording() const
{
    return m_bRecord;
}

void CVideoPlayer::play(const QModelIndex& modelIndex, int index)
{
    m_bStop = false;

    if(m_readWatcher.isFinished())
    {
        auto future = QtConcurrent::run([&, modelIndex, index]
        {
            // Use unique_lock for synchronization purpose
            // It is used with std::conditional_variable
            std::unique_lock<std::mutex> lock(m_playMutex);
            while(!m_bStop)
            {
                try
                {
                    // Wait for new image
                    m_threadCond.wait(lock);

                    // Manage if we stop acquisition
                    if(m_bStop)
                        break;

                    // Read new image
                    // If read failed we exit the thread
                    // and send signal to report error (the play loop of the view has to be stopped)
                    auto image = getImage();
                    if(image.empty())
                    {
                        emit doPlayError(modelIndex, "Video player error: invalid frame buffer.");
                        break;
                    }

                    // Manage if we stop acquisition
                    if(m_bStop)
                        break;

                    // Notify that image changed
                    emit doImageIsLoaded(modelIndex, image, index, false);
                }
                catch(std::exception& e)
                {
                    emit doPlayError(modelIndex, QString::fromStdString(e.what()));
                    break;
                }
            }
        });
        m_readWatcher.setFuture(future);
    }
    else
    {
        // Notify thread that there is a new image
        m_threadCond.notify_one();
    }
}

void CVideoPlayer::startRecord(const std::string &path)
{
    m_bRecord = true;
    m_recordPath = path;
}

void CVideoPlayer::stop()
{
    m_bStop = true;

    // Notify threads that the user stopped
    m_threadCond.notify_all();

    // Ensure read is stop
    if(m_readWatcher.isRunning())
        m_readWatcher.waitForFinished();

    m_mgrPtr->stopReadVideo();
}

void CVideoPlayer::stopRecord()
{
    m_mgrPtr->waitWriteFinished(m_writeTimeout);
    m_bRecord = false;
    m_recordPath.clear();
}

void CVideoPlayer::recordImage(const CMat& image)
{
    m_mgrPtr->writeVideo(image, m_recordPath);
}

void CVideoPlayer::releaseProcessedImage()
{
    m_currentProcessedImage.release();
}

//-------------------------//
//----- CVideoManager -----//
//-------------------------//
CVideoManager::CVideoManager()
{
}

CVideoManager::~CVideoManager()
{
    // Ensure stopping reader thread when we shut down app
    clearPlayers();
}

void CVideoManager::setManagers(CProjectManager *pProjectMgr, CWorkflowManager* pWorkflowMgr, CGraphicsManager* pGraphicsMgr,
                                CResultManager* pResultMgr, CProgressBarManager* pProgressMgr)
{
    m_pProjectMgr = pProjectMgr;
    m_pWorkflowMgr = pWorkflowMgr;
    m_pGraphicsMgr = pGraphicsMgr;
    m_pResultMgr = pResultMgr;
    m_pProgressMgr = pProgressMgr;
}

void CVideoManager::play(const QModelIndex& modelIndex, size_t index)
{
    auto pPlayer = getPlayer(modelIndex);
    if(pPlayer)
        pPlayer->play(modelIndex, index);
}

void CVideoManager::displayVideoImage(const QModelIndex& modelIndex, size_t index, bool bNewSequence)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    // Set video type: live, video or image sequence
    ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(pPlayer->getWrapIndex().internalPointer());
    pPlayer->setStream(itemPtr->getTypeId() == TreeItemType::LIVE_STREAM);

    // Create and set current video IO according to video type
    auto pDataset = CProjectUtils::getDataset<CMat>(pPlayer->getWrapIndex());
    pPlayer->getManager()->setCurrentVideoIO(*pDataset);

    //Get image
    auto image = pPlayer->getImage();

    // Notify that image changed
    onImageIsLoaded(modelIndex, image, index, bNewSequence);

    //Stream read thread should be stopped
    pPlayer->stop();

    if(bNewSequence == true)
        initInfo(modelIndex, index);
}

void CVideoManager::displayImageSequence(const QModelIndex& modelIndex, size_t imageIndex, size_t index, bool bNewSequence)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    // Set video type: image sequence
    pPlayer->setStream(false);

    // Create and set current video IO according to video type
    auto pDataset = CProjectUtils::getDataset<CMat>(pPlayer->getWrapIndex());
    pPlayer->getManager()->setCurrentVideoIO(*pDataset);

    // Read image at position corresponding to index row
    auto image = pPlayer->getImage(imageIndex);

    // Notify that image changed
    onImageIsLoaded(modelIndex, image, index, bNewSequence);

    if(bNewSequence == true)
        initInfo(modelIndex, index);
}

void CVideoManager::displayCurrentVideoImage(const QModelIndex& modelIndex, int index)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    CMat currentImage = pPlayer->getCurrentImage();
    if(currentImage.empty())
    {
        qCWarning(logVideo).noquote() << "Current image is empty.";
        return;
    }

    if(pPlayer->isRecording())
    {
        try
        {
            pPlayer->recordImage(currentImage);
        }
        catch (std::exception& e)
        {
            qCCritical(logVideo).noquote() << QString::fromStdString(e.what());
            pPlayer->stopRecord();
            emit doStopRecording(modelIndex);
        }
    }

    // Notify project manager to display image
    QString name = pPlayer->getWrapIndex().data(Qt::DisplayRole).toString();
    emit doDisplayVideoImage(modelIndex, index, CDataConversion::CMatToQImage(currentImage), name);

    // Notify view to update video widgets
    updateInfo(modelIndex, index);

    // Display video info -> time consuming so we do it only if necessary
    if(m_bInfoUpdate)
        displayVideoInfo(modelIndex);
}

void CVideoManager::displayVideoInfo(const QModelIndex& index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return;

    auto pDataInfoPtr = pPlayer->getManager()->getDataVideoInfoPtr();
    if(pDataInfoPtr == nullptr)
        return;

    // When workflow exists, displayed video frames are managed by CWorkflowManager
    // and video player can only retrieve source image information.
    // We must override image information by those given by CWorkflowManager
    if(!m_selectedWorkflowImage.empty())
        pDataInfoPtr->updateImage(m_selectedWorkflowImage);

    VectorPairString infoList = pDataInfoPtr->getStringList();
    auto pDataset = CProjectUtils::getDataset<CMat>(pPlayer->getWrapIndex());

    if(pDataset->size() > 1)
    {
        auto it = std::find_if(infoList.begin(), infoList.end(), [](const PairString& elt){ return elt.first == tr("Total frame number").toStdString(); });
        if(it != infoList.end())
        {
            if(std::stoi(it->second) == 0)
                it->second = std::to_string(pDataset->size());
        }
    }
    emit doDisplayVideoInfo(infoList);
}

void CVideoManager::updateVideoPos(const QModelIndex& modelIndex, size_t index, size_t pos)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    auto image = pPlayer->getImage(pos);

    // Notify that image changed
    onImageIsLoaded(modelIndex, image, index, false);
}

void CVideoManager::updateInfo(const QModelIndex &modelIndex, int index)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    // Get info from new image
    auto pInfo = pPlayer->getManager()->getDataVideoInfoPtr();
    if(!pInfo)
        return;

    size_t fps = pInfo->m_fps;
    if(fps == 0)
        fps = 1;

    // Update current elapsed time
    emit doSetCurrentTime(index, getCurrentPos(modelIndex) / fps);
    // Update scroll bar
    emit doUpdateVideoPos(index, getCurrentPos(modelIndex));
}

void CVideoManager::initInfo(const QModelIndex& modelIndex, int index)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    // Get info generated by getImage()
    auto pInfo = pPlayer->getManager()->getDataVideoInfoPtr();
    if(pInfo == nullptr)
        return;

    emit doSetSourceType(index, pPlayer->getSourceType());
    // Set fps rate in video display view
    emit doSetFPS(index, pInfo->m_fps);
    // Set number of frames in display view (for scroll bar)
    emit doSetVideoLength(index, pInfo->m_frameCount);

    size_t fps = pInfo->m_fps;
    if(fps == 0)
        fps = 1;

    // Set video total time in second
    emit doSetTotalTime(index, pInfo->m_frameCount / fps);
    // Update current time in second
    emit doSetCurrentTime(index, getCurrentPos(modelIndex) / fps);
    // Update scroll bar
    emit doUpdateVideoPos(index, getCurrentPos(modelIndex));
}

CVideoPlayer* CVideoManager::getPlayer(const QModelIndex &modelIndex)
{
    assert(m_pProjectMgr);

    auto it = m_players.find(modelIndex);
    if(it == m_players.end())
    {
        auto wrapInd = m_pProjectMgr->wrapIndex(modelIndex);
        auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());

        if(!pItem)
        {
            qCCritical(logVideo).noquote() << tr("No video player available: invalid project item.");
            return nullptr;
        }

        auto type = pItem->getTypeId();
        if(type != TreeItemType::VIDEO && type != TreeItemType::LIVE_STREAM && type != TreeItemType::IMAGE)
        {
            qCWarning(logVideo).noquote() << tr("No video player available: project item is not a video, stream or image sequence.");
            return nullptr;
        }

        if(type == TreeItemType::IMAGE)
        {
            auto pDataset = CProjectUtils::getDataset<CMat>(wrapInd);
            if(!pDataset)
            {
                qCWarning(logVideo).noquote() << tr("No video player available: project item is not an image sequence.");
                return nullptr;
            }

            if(!pDataset->hasDimension(DataDimension::TIME))
            {
                qCWarning(logVideo).noquote() << tr("No video player available: project item is not an image sequence.");
                return nullptr;
            }
        }

        auto pPlayer = new CVideoPlayer(wrapInd);
        connect(pPlayer, &CVideoPlayer::doImageIsLoaded, this, &CVideoManager::onImageIsLoaded);
        connect(pPlayer, &CVideoPlayer::doPlayError, [&](const QModelIndex& index, const QString& msg)
        {
            qCCritical(logVideo).noquote() << msg;
            emit doStopVideoPlayerView(index);
        });
        auto ret = m_players.insert(std::make_pair(QPersistentModelIndex(modelIndex), pPlayer));
        return ret.first->second;
    }
    else
        return it->second;
}

void CVideoManager::clearPlayers()
{
    for(auto it=m_players.begin(); it!=m_players.end(); ++it)
        delete it->second;

    m_players.clear();
}

bool CVideoManager::isPlaying(const QModelIndex& index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return false;
    else
        return pPlayer->isPlaying();
}

bool CVideoManager::isStream(const QModelIndex &index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return false;
    else
        return pPlayer->isStream();
}

void CVideoManager::stopPlay(const QModelIndex& index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return;

    pPlayer->stop();
    // Ensure all signals have been executed (in particular doImageIsLoaded)
    qApp->processEvents();
}

void CVideoManager::closeData(const QModelIndex& index)
{
    emit doStopWorkflowThread();

    auto it = m_players.find(index);
    if(it != m_players.end())
    {
        delete it->second;
        m_players.erase(it);
    }
}

void CVideoManager::beforeProjectClose(int projectIndex, bool bWithCurrentImage)
{
    Q_UNUSED(projectIndex);
    if(bWithCurrentImage == true)
        clearPlayers();
}

void CVideoManager::saveCurrentFrame(const QModelIndex& index)
{
    assert(m_pProjectMgr);

    std::string destPath;
    std::string originalPath = m_pProjectMgr->getItemPath(index);
    std::string newItemName = index.data(Qt::DisplayRole).toString().toStdString();

    if(originalPath.empty())
        destPath = Utils::IkomiaApp::getIkomiaFolder() + "/SavedData/" + newItemName;
    else
    {
        boost::filesystem::path boostOriginalPath(originalPath);
        destPath = boostOriginalPath.parent_path().string() + "/" + newItemName;
    }

    try
    {
        //Création d'un sous-dossier du même nom que l'item original
        Utils::File::createDirectory(destPath);
    }
    catch(const CException& e)
    {
        qCCritical(logProject).noquote() << tr("Error while saving video frame:") << QString::fromStdString(e.what());
        return;
    }

    // Use current image when no protocol but use processed image otherwise
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return;

    CMat currentImage = pPlayer->getCurrentImage();

    //Sauvegarde de l'image
    try
    {
        destPath += "/frame.png";
        destPath = Utils::File::getAvailablePath(destPath);
        CImageDataIO io(destPath);
        io.write(currentImage);
    }
    catch(std::exception& e)
    {
       qCCritical(logProject).noquote() << QString::fromStdString(e.what());
       return;
    }

    //Création d'un dataset dans le projet
    auto currentDatasetIndex = m_pProjectMgr->getDatasetIndex(index);
    auto datasetIndex = m_pProjectMgr->addDataset(currentDatasetIndex.parent(), newItemName, IODataType::IMAGE);

    //Ajout de l'image
    QStringList files;
    files.push_back(QString::fromStdString(destPath));
    m_pProjectMgr->addImagesToDataset(datasetIndex, files);
}

void CVideoManager::exportCurrentFrame(const QModelIndex& index, const QString &path, bool bWithGraphics)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return;

    CMat imgSaved;
    CMat currentImage = pPlayer->getCurrentImage();

    if(bWithGraphics == true)
    {
        assert(m_pProjectMgr);
        assert(m_pGraphicsMgr);
        imgSaved = currentImage.clone();
        m_pGraphicsMgr->burnGraphicsToImage(index, imgSaved);
    }
    else
        imgSaved = currentImage;

    try
    {
        CImageDataIO io(path.toStdString());
        io.write(imgSaved);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.what());
    }
}

void CVideoManager::enableInfoUpdate(const QModelIndex& index, bool bEnable)
{
    m_bInfoUpdate = bEnable;
    if(m_bInfoUpdate == true)
    {
        // Update current video information
        displayVideoInfo(index);
    }
}

void CVideoManager::onNotifyVideoStart(const QModelIndex& index)
{
    auto pPlayer = getPlayer(index);
    if(pPlayer)
        emit doNotifyVideoStart(pPlayer->getManager()->getDataVideoInfoPtr()->m_frameCount);
}

void CVideoManager::onDisplayCurrentVideoImage(const QModelIndex& modelIndex, int index)
{
    displayCurrentVideoImage(modelIndex, index);
}

void CVideoManager::onCloseWorkflow()
{
    // Clear processed image
    for(auto it=m_players.begin(); it!=m_players.end(); ++it)
        it->second->releaseProcessedImage();
}

void CVideoManager::onImageIsLoaded(const QModelIndex& modelIndex, const CMat& image, int index, bool bNewSequence)
{
    assert(m_pProjectMgr);
    assert(m_pWorkflowMgr);

    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    pPlayer->setCurrentImage(image);

    if(m_pWorkflowMgr->isWorkflowExists() == false)
        displayCurrentVideoImage(modelIndex, index);
    else
    {
        pPlayer->setCurrentProcessedImage();
        if(bNewSequence == true)            
            displayCurrentVideoImage(modelIndex, index);

        // Notify protocol that image changed
        emit doVideoDataChanged(modelIndex, index, bNewSequence);
    }
    // Notify view that image changed
    emit doCurrentDataChanged(modelIndex, bNewSequence);
}

void CVideoManager::onInitInfo(const QModelIndex &modelIndex, int index)
{
    initInfo(modelIndex, index);
}

void CVideoManager::setVideoRecord(const QModelIndex& modelIndex, bool bEnable)
{
    auto pPlayer = getPlayer(modelIndex);
    if(!pPlayer)
        return;

    if(bEnable == false)
    {
        if(pPlayer->isRecording())
        {
            std::string path = pPlayer->getRecordPath();
            emit doAddRecordVideo(QString::fromStdString(path));
            pPlayer->stopRecord();
        }
    }
    else
    {
        try
        {
            //Création d'un sous-dossier du même nom
            std::string folder = Utils::IkomiaApp::getIkomiaFolder() + "/VideoRecord/";
            Utils::File::createDirectory(folder);
            std::string path = folder + "liveCamera.avi";
            path = Utils::File::getAvailablePath(path);
            pPlayer->startRecord(path);
        }
        catch (std::exception& e)
        {
            qCCritical(logVideo).noquote() << QString::fromStdString(e.what());
            emit doStopRecording(modelIndex);
        }
    }
}

void CVideoManager::setProgressSignalHandler(CProgressSignalHandler* pHandler)
{
    m_pProgressSignal = pHandler;
}

void CVideoManager::setSelectedImageFromWorkflow(const CMat &image)
{
    m_selectedWorkflowImage = image;
}

size_t CVideoManager::getCurrentPos(const QModelIndex& index)
{
    // Get info generated by getImage()
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return 0;

    auto pInfo = pPlayer->getManager()->getDataVideoInfoPtr();
    if(pInfo == nullptr)
        return 0;

    if (pInfo->m_currentPos == 0)
        return 0;
    else
        return pInfo->m_currentPos - 1;
}

CMat CVideoManager::getVideoImage(const QModelIndex& index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return CMat();
    else
        return pPlayer->getImage();
}

CMat CVideoManager::getVideoImage(const QModelIndex& index, size_t pos)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return CMat();
    else
        return pPlayer->getImage(pos);
}

CMat CVideoManager::getSequenceImage(const QModelIndex& datasetIndex, const QModelIndex& imgWrapIndex)
{
    auto pPlayer = getPlayer(datasetIndex);
    if(!pPlayer)
        return CMat();
    else
        return pPlayer->getSequenceImage(imgWrapIndex);
}

CMat CVideoManager::getCurrentImage(const QModelIndex& index)
{
    // Store image currently processed by protocol in order to show it at the same time than the result image
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return CMat();

    auto image = pPlayer->getCurrentImage();
    if(image.empty())
        image = pPlayer->getImage();

    return image;
}

CDataVideoInfoPtr CVideoManager::getVideoInfo(const QModelIndex &index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return nullptr;
    else
        return pPlayer->getManager()->getDataVideoInfoPtr();
}

CDataVideoBuffer::Type CVideoManager::getSourceType(const QModelIndex &index)
{
    auto pPlayer = getPlayer(index);
    if(!pPlayer)
        return CDataVideoBuffer::NONE;
    else
        return pPlayer->getSourceType();
}

//#include "moc_CVideoManager.cpp"


