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

#include "CImgManager.h"
#include "Main/LogCategory.h"
#include "DataProcessTools.hpp"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "Model/Project/CProjectManager.h"
#include "Model/Results/CResultManager.h"
#include "Model/Render/CRenderManager.h"
#include "Model/Graphics/CGraphicsManager.h"
#include "Data/CDataConversion.h"

CImgManager::CImgManager()
{
    m_pImgMgr = std::make_shared<CImageDataManager>();
}

void CImgManager::setManagers(CProjectManager *pProjectMgr, CGraphicsManager* pGraphicsMgr, CResultManager* pResultMgr,
                              CRenderManager* pRenderMgr, CProgressBarManager* pProgressMgr)
{
    m_pProjectMgr = pProjectMgr;
    m_pGraphicsMgr = pGraphicsMgr;
    m_pResultMgr = pResultMgr;
    m_pRenderMgr = pRenderMgr;
    m_pProgressMgr = pProgressMgr;
}

void CImgManager::setProgressSignalHandler(CProgressSignalHandler* pHandler)
{
    m_pProgressSignal = pHandler;
    m_pImgMgr->setProgressSignalHandler(pHandler);
}

CImageDataMgrPtr CImgManager::getImageMgrPtr()
{
    return m_pImgMgr;
}

CMat CImgManager::getImage(const QModelIndex& wrapIndex)
{
    if(wrapIndex.isValid() == false)
        return CMat();

    DimensionIndices indices = CProjectUtils::getIndicesInDataset(wrapIndex);
    if(indices.size() == 0)
    {
        //qCCritical(logProject).noquote() << tr("Invalid image index");
        return CMat();
    }

    auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex);
    if(!pDataset)
    {
        //qCCritical(logProject).noquote() << tr("Invalid dataset");
        return CMat();
    }

    size_t imageIndex = pDataset->getDataInfo().index(indices);
    auto bounds = pDataset->subsetBounds(imageIndex);

    if(pDataset->hasDimension(DataDimension::VOLUME))
        Utils::Data::setSubsetBounds(bounds, DataDimension::IMAGE, 0, pDataset->size(DataDimension::IMAGE)-1);

    //Progress bar
    if(Utils::Data::getSubsetBoundsSize(bounds) > 1 && pDataset->subset().contains(bounds) == false)
        m_pProgressMgr->launchProgress(m_pProgressSignal, tr("Load images..."), true);

    // Get and store image info
    auto currentImgIndex = Utils::Data::getDimensionSize(indices, DataDimension::IMAGE);
    auto pDataInfo = pDataset->getDataInfo()[currentImgIndex];
    assert(pDataInfo != nullptr);

    CImageDataIO io(pDataInfo->getFileName());
    m_pCurrentDataInfo = io.dataInfo();

    // Load data
    m_pImgMgr->loadData(*pDataset, bounds);

    return pDataset->dataAt(imageIndex);
}

CMat CImgManager::getImage(const QModelIndex &datasetWrapIndex, int imgIndex)
{
    if(datasetWrapIndex.isValid() == false)
        return CMat();

    auto pDataset = CProjectUtils::getDataset<CMat>(datasetWrapIndex);
    if(!pDataset)
        return CMat();

    auto bounds = pDataset->subsetBounds(imgIndex);

    if(pDataset->hasDimension(DataDimension::VOLUME))
        Utils::Data::setSubsetBounds(bounds, DataDimension::IMAGE, 0, pDataset->size(DataDimension::IMAGE)-1);

    // Progress bar
    if(Utils::Data::getSubsetBoundsSize(bounds) > 1 && pDataset->subset().contains(bounds) == false)
        m_pProgressMgr->launchProgress(m_pProgressSignal, tr("Load images..."), true);

    // Get and store image info
    auto pDataInfo = pDataset->getDataInfo()[imgIndex];
    assert(pDataInfo != nullptr);

    CImageDataIO io(pDataInfo->getFileName());
    m_pCurrentDataInfo = io.dataInfo();

    // Load data
    m_pImgMgr->loadData(*pDataset, bounds);
    return pDataset->dataAt(imgIndex);
}

void CImgManager::displaySimpleImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, size_t inputIndex, bool bNewSequence)
{
    assert(m_pResultMgr);
    assert(m_pGraphicsMgr);

    // Get image
    CMat image = getImage(wrapIndex);
    if(!image.data)
    {
        qCritical().noquote() << tr("Display image failed : invalid image");
        return;
    }

    // Notify view to display image
    emit doDisplayImage(0, pScene, CDataConversion::CMatToQImage(image), index.data(Qt::DisplayRole).toString(), nullptr);
    // Load results
    m_pResultMgr->loadImageResults(index);
    //Load associated graphics and protocols
    m_pGraphicsMgr->loadAllGraphics(index);
    // Notify protocol that image changed
    emit doInputDataChanged(index, inputIndex, bNewSequence);
    // Notify view that image changed
    emit doCurrentDataChanged(index, bNewSequence);
    // Display image info
    if(m_bInfoUpdate)
        displayImageInfo(image, wrapIndex);
}

void CImgManager::displayVolumeImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence)
{
    assert(m_pProjectMgr);
    assert(m_pResultMgr);

    // Get image
    auto image = getImage(wrapIndex);
    if(!image.data)
    {
        //qCCritical(logProject).noquote() << tr("Display image failed : invalid image");
        return;
    }
    // Get indices
    DimensionIndices indices = CProjectUtils::getIndicesInDataset(wrapIndex);
    if(indices.size() == 0)
    {
        //qCCritical(logProject).noquote() << tr("Invalid image index");
        return;
    }
    // Image is a volume, load the appropriate slice/plane
    auto currentImgIndex = Utils::Data::getDimensionSize(indices, DataDimension::IMAGE);
    CMat plane = image.getPlane(currentImgIndex);

    // Notify view to display new slice/plane
    emit doDisplayVolume(pScene, CDataConversion::CMatToQImage(plane), index.data(Qt::DisplayRole).toString(), bNewSequence, nullptr);

    // Check if we must load a new 3D volume
    if(bNewSequence)
    {        
        // Stack has changed, we must update the number of slices/planes
        emit doUpdateNbImg(image.getNbStacks());
        // Notify view that data changed
        emit doCurrentDataChanged(index, bNewSequence);
        // Notify 3D render to update volume
        m_pRenderMgr->updateVolumeRenderInput(image);
        // Load results
        m_pResultMgr->loadImageResults(index);
    }
    else
    {
        bool bTypeHasChanged = false;
        QModelIndex currentDataItemIndex = m_pProjectMgr->getCurrentDataItemIndex();

        // Check if item type has changed
        if(currentDataItemIndex.isValid())
        {
            auto wrapCurrentInd = m_pProjectMgr->wrapIndex(currentDataItemIndex);
            auto wrapNewInd = wrapIndex;
            auto pCurrentItem = static_cast<ProjectTreeItem*>(wrapCurrentInd.internalPointer());
            auto pNewItem = static_cast<ProjectTreeItem*>(wrapNewInd.internalPointer());
            bTypeHasChanged = pCurrentItem->getTypeId() != pNewItem->getTypeId();
        }
        // Notify view that data changed -> affichage d'éventuels résultats
        if(bTypeHasChanged)
            emit doCurrentDataChanged(index, bNewSequence);

        // Notify view to display image result associated with current image
        m_pResultMgr->setCurrentOutputImage(index);
    }

    //Load associated graphics and protocols
    m_pGraphicsMgr->loadAllGraphics(index);
    // Notify view that index changed
    emit doUpdateCurrentImgIndex(currentImgIndex);
    // Notify protocol that image changed
    emit doInputDataChanged(index, 0, bNewSequence);

    // Display image info
    if(m_bInfoUpdate)
        displayImageInfo(image, wrapIndex);
}

void CImgManager::displayImageInfo(const CMat &image, const QModelIndex& wrapIndex)
{
    auto pDataInfoPtr = std::static_pointer_cast<CDataImageInfo>(m_pCurrentDataInfo);
    if(pDataInfoPtr == nullptr)
        return;

    if(image.empty() == false)
        pDataInfoPtr->updateImage(image);

    VectorPairString infoList = pDataInfoPtr->getStringList();

    auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex);
    if(pDataset->size() > 1)
        infoList.push_back(std::make_pair(tr("# Images").toStdString(), std::to_string(pDataset->size())));

    emit doDisplayImageInfo(infoList);
}

void CImgManager::exportImage(const QModelIndex& index, const QString &path, bool bWithGraphics)
{
    assert(m_pProjectMgr);

    if(index.isValid() == false)
    {
        qCCritical(logProject).noquote() << tr("Invalid image index");
        return;
    }

    CMat image = getImage(m_pProjectMgr->wrapIndex(index));
    if(!image.data)
    {
        qCCritical(logProject).noquote() << tr("Invalid image");
        return;
    }

    CMat imgToSave;
    if(bWithGraphics == true)
    {
        assert(m_pGraphicsMgr);
        imgToSave = image.clone();
        m_pGraphicsMgr->burnGraphicsToImage(index, imgToSave);
    }
    else
        imgToSave = image;

    try
    {
        CImageDataIO io(Utils::File::getAvailablePath(path.toStdString()));
        io.write(imgToSave);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.what());
    }
}

void CImgManager::exportImage(const CMat &image, const std::vector<ProxyGraphicsItemPtr> &graphics, const QString &path)
{
    CMat imgToSave;
    if(graphics.size() > 0)
    {
        assert(m_pGraphicsMgr);
        imgToSave = image.clone();
        Utils::Image::burnGraphics(imgToSave, graphics);
    }
    else
        imgToSave = image;

    try
    {
        CImageDataIO io(Utils::File::getAvailablePath(path.toStdString()));
        io.write(imgToSave);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.what());
    }
}

void CImgManager::enableInfoUpdate(bool bEnable)
{
    m_bInfoUpdate = bEnable;

    if(m_bInfoUpdate == true)
    {
        // Update current image information
        auto currentIndex = m_pProjectMgr->wrapIndex(m_pProjectMgr->getCurrentDataItemIndex());
        displayImageInfo(CMat(), currentIndex);
    }
}

void CImgManager::onCloseWorkflow()
{
    assert(m_pProjectMgr);
    assert(m_pRenderMgr);

    QModelIndex itemIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(itemIndex.isValid())
    {
        auto wrapInd = m_pProjectMgr->wrapIndex(itemIndex);
        // Test if current index is an image and not video/stream
        ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());
        if(pItem->getTypeId() == static_cast<size_t>(TreeItemType::IMAGE))
        {
            auto image = getImage(wrapInd);
            if(image.dims == 3)
                m_pRenderMgr->updateVolumeRenderInput(image);
        }
    }
}
