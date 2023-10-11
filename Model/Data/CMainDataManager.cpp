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

#include "CMainDataManager.h"
#include "Main/LogCategory.h"
#include "Model/Project/CProjectUtils.hpp"
#include "Model/Workflow/CWorkflowManager.h"

CMainDataManager::CMainDataManager()
{
}

CMainDataManager::~CMainDataManager()
{
}

CImgManager* CMainDataManager::getImgMgr()
{
    return &m_imgMgr;
}

CVideoManager* CMainDataManager::getVideoMgr()
{
    return &m_videoMgr;
}

QString CMainDataManager::getDataPath(const QModelIndex &wrapIndex)
{
    if(!wrapIndex.isValid())
        return nullptr;

    DimensionIndices indices = CProjectUtils::getIndicesInDataset(wrapIndex);
    if(indices.size() == 0)
    {
        qCCritical(logProject).noquote() << tr("Invalid image index");
        return nullptr;
    }

    auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex);
    if(!pDataset)
    {
        qCCritical(logProject).noquote() << tr("Invalid dataset");
        return nullptr;
    }

    auto currentImgIndex = Utils::Data::getDimensionSize(indices, DataDimension::IMAGE);
    auto pDataInfo = pDataset->getDataInfo()[currentImgIndex];
    if(pDataInfo == nullptr)
        return QString();
    else
        return QString::fromStdString(pDataInfo->getFileName());
}

CDataInfoPtr CMainDataManager::getDataInfoPtr(const QModelIndex& wrapIndex)
{
    QString fileName = getDataPath(wrapIndex);
    if(fileName.isEmpty())
        return nullptr;

    ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    if(itemPtr->getTypeId() == static_cast<size_t>(TreeItemType::IMAGE))
    {
        CImageDataIO io(fileName.toStdString());
        return io.dataInfo();
    }
    else if(itemPtr->getTypeId() == static_cast<size_t>(TreeItemType::VIDEO))
    {
        CVideoDataIO io(fileName.toStdString());
        return io.dataInfo();
    }
    return nullptr;
}

int CMainDataManager::getSelectedDisplayCategory() const
{
    return m_selectedDisplayCategory;
}

int CMainDataManager::getSelectedDisplayIndex() const
{
    return m_selectedDisplayIndex;
}

void CMainDataManager::setManagers(CProjectManager *pProjectMgr, CWorkflowManager *pWorkflowMgr, CGraphicsManager *pGraphicsMgr,
                                   CResultManager* pResultMgr, CRenderManager *pRenderMgr, CProgressBarManager *pProgressMgr)
{
    m_pProjectMgr = pProjectMgr;
    m_pWorkflowMgr = pWorkflowMgr;

    m_imgMgr.setManagers(pProjectMgr, pGraphicsMgr, pResultMgr, pRenderMgr, pProgressMgr);
    m_videoMgr.setManagers(pProjectMgr, pWorkflowMgr, pGraphicsMgr, pResultMgr, pProgressMgr);
}

void CMainDataManager::setProgressSignalHandler(CProgressSignalHandler* pHandler)
{
    m_imgMgr.setProgressSignalHandler(pHandler);
    m_videoMgr.setProgressSignalHandler(pHandler);
}

void CMainDataManager::closeData(const QModelIndex& index)
{
    m_videoMgr.closeData(index);
}

void CMainDataManager::displaySimpleImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, size_t inputIndex, bool bNewSequence)
{
    try
    {
        m_imgMgr.displaySimpleImage(pScene, index, wrapIndex, inputIndex, bNewSequence);
    }
    catch(std::exception& e)
    {
        qCCritical(logImage).noquote() << QString::fromStdString(e.what());
    }
}

void CMainDataManager::displayVolumeImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence)
{
    try
    {
        m_imgMgr.displayVolumeImage(pScene, index, wrapIndex, bNewSequence);
    }
    catch(std::exception& e)
    {
        qCCritical(logImage).noquote() << QString::fromStdString(e.what());
    }
}

void CMainDataManager::displayPositionImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence)
{
    try
    {
        m_imgMgr.displayPositionImage(pScene, index, wrapIndex, bNewSequence);
    }
    catch(std::exception& e)
    {
        qCCritical(logImage).noquote() << QString::fromStdString(e.what());
    }
}

void CMainDataManager::displayVideoImage(const QModelIndex& index, size_t inputIndex, bool bNewSequence)
{
    try
    {
        m_videoMgr.displayVideoImage(index, inputIndex, bNewSequence);
    }
    catch(std::exception& e)
    {
        qCCritical(logVideo).noquote() << QString::fromStdString(e.what());
    }
}

void CMainDataManager::displayImageSequence(const QModelIndex& index, size_t inputIndex, bool bNewSequence)
{
    assert(m_pProjectMgr);
    try
    {
        m_videoMgr.displayImageSequence(m_pProjectMgr->getCurrentVideoItemIndex(), index.row(), inputIndex, bNewSequence);
    }
    catch(std::exception& e)
    {
        qCCritical(logImage).noquote() << QString::fromStdString(e.what());
    }
}

void CMainDataManager::beforeProjectClose(int projectIndex, bool bWithCurrentImage)
{
    m_videoMgr.beforeProjectClose(projectIndex, bWithCurrentImage);
}

void CMainDataManager::reloadCurrent()
{
    assert(m_pProjectMgr);
    QModelIndex currentIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(currentIndex.isValid() == false)
        return;

    QModelIndex currentWrapIndex = m_pProjectMgr->wrapIndex(currentIndex);
    ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(currentWrapIndex.internalPointer());

    if(itemPtr->getTypeId() == TreeItemType::IMAGE)
    {
        //Get the corresponding image item
        auto imgItemPtr = CProjectUtils::getImageItem(currentWrapIndex);
        assert(imgItemPtr);
        auto pDataset = CProjectUtils::getDataset<CMat>(currentWrapIndex);

        if(pDataset->hasDimension(DataDimension::VOLUME)) // 3D image
            displayVolumeImage(imgItemPtr->getScene(), currentIndex, currentWrapIndex, true);
        else if(pDataset->hasDimension(DataDimension::TIME)) // Video image sequence
            displayImageSequence(currentIndex, 0, true);
        else if(pDataset->hasDimension(DataDimension::POSITION)) // Position image sequence
            displayPositionImage(imgItemPtr->getScene(), currentIndex, currentWrapIndex, true);
        else if(pDataset->hasDimension(DataDimension::IMAGE)) // Simple image
            displaySimpleImage(imgItemPtr->getScene(), currentIndex, currentWrapIndex, 0, true);
    }
    else if(itemPtr->getTypeId() == TreeItemType::VIDEO ||
            itemPtr->getTypeId() == TreeItemType::LIVE_STREAM)
    {
        displayVideoImage(currentIndex, 0, true);
    }
}

void CMainDataManager::onSetSelectedDisplay(DisplayCategory category, int index)
{
    assert(m_pWorkflowMgr);

    if(category == m_selectedDisplayCategory && index == m_selectedDisplayIndex)
        return;

    m_selectedDisplayCategory = category;
    m_selectedDisplayIndex = index;
}

void CMainDataManager::onSaveCurrentVideoFrame(const QModelIndex &modelIndex, int index)
{
    assert(m_pWorkflowMgr);

    if(m_pWorkflowMgr->isWorkflowExists())
        m_pWorkflowMgr->saveCurrentInputImage((size_t)index);
    else
        m_videoMgr.saveCurrentFrame(modelIndex);
}

void CMainDataManager::onExportCurrentImage(int index, const QString &path, bool bWithGraphics)
{
    assert(m_pProjectMgr && m_pWorkflowMgr);

    if(m_pWorkflowMgr->isWorkflowExists())
        m_pWorkflowMgr->exportCurrentInputImage(index, path, bWithGraphics);
    else
    {
        QModelIndex currentIndex = m_pProjectMgr->getCurrentDataItemIndex();
        m_imgMgr.exportImage(currentIndex, path, bWithGraphics);
    }
}

void CMainDataManager::onExportCurrentVideoFrame(const QModelIndex& index, int inputIndex, const QString &path, bool bWithGraphics)
{
    assert(m_pWorkflowMgr);

    if(m_pWorkflowMgr->isWorkflowExists())
        m_pWorkflowMgr->exportCurrentInputImage(inputIndex, path, bWithGraphics);
    else
        m_videoMgr.exportCurrentFrame(index, path, bWithGraphics);
}

void CMainDataManager::onEnableInfo(bool bEnable)
{
    assert(m_pWorkflowMgr);

    if(m_pWorkflowMgr->isWorkflowExists())
        m_pWorkflowMgr->updateDataInfo();
    else
        m_imgMgr.enableInfoUpdate(bEnable);

    m_videoMgr.enableInfoUpdate(m_pProjectMgr->getCurrentDataItemIndex(), bEnable);
}

void CMainDataManager::onPlayVideo(int index)
{
    assert(m_pWorkflowMgr);
    assert(m_pProjectMgr);

    if(m_pWorkflowMgr->isWorkflowExists())
        m_pWorkflowMgr->playVideoInput(index);
    else
        m_videoMgr.play(m_pProjectMgr->getCurrentVideoItemIndex(), index);
}

void CMainDataManager::onStopVideo(const QModelIndex& index)
{
    try
    {
        m_videoMgr.stopPlay(index);
        m_pProjectMgr->onVideoStopped();
    }
    catch(std::exception& e)
    {
        qCCritical(logVideo).noquote() << QString::fromStdString(e.what());
    }
}
