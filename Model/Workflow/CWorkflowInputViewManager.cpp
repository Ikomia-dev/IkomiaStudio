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

#include "CWorkflowInputViewManager.h"
#include "Main/LogCategory.h"
#include "IO/CImageIO.h"
#include "IO/CVideoIO.h"
#include "Data/CDataConversion.h"
#include "CWorkflowInput.h"
#include "Model/Project/CProjectManager.h"

CWorkflowInputViewManager::CWorkflowInputViewManager(CWorkflowInputs *pInputs, WorkflowInputViewMode mode)
{
    m_pInputs = pInputs;
    m_viewMode = mode;
}

void CWorkflowInputViewManager::setManagers(CProjectManager *pProjectMgr)
{
    m_pProjectMgr = pProjectMgr;
}

void CWorkflowInputViewManager::setViewMode(WorkflowInputViewMode mode)
{
    m_viewMode = mode;
}

void CWorkflowInputViewManager::setLiveInputIndex(size_t index)
{
    m_videoInputIndex = index;
}

void CWorkflowInputViewManager::manageOriginVideoInput(size_t index)
{
    assert(m_pProjectMgr);

    if(index >= m_pInputs->size())
        return;

    bool bUpdateVideo = false;
    TreeItemType type = m_pInputs->at(index).getType();
    //Get source model index
    QModelIndex modelIndex = getSrcModelIndex(index);

    if(type == TreeItemType::VIDEO || type == TreeItemType::LIVE_STREAM)
        bUpdateVideo = true;
    else if(type == TreeItemType::IMAGE)
    {
        auto wrapInd = m_pProjectMgr->wrapIndex(modelIndex);
        auto pDataset = CProjectUtils::getDataset<CMat>(wrapInd);
        bUpdateVideo = pDataset != nullptr && pDataset->hasDimension(DataDimension::TIME);
    }

    if(bUpdateVideo)
    {
        // Notify video manager to display source image so that it displays at the same time than the result image
        emit doDisplayVideoImage(modelIndex, index);
    }
}

void CWorkflowInputViewManager::manageInputs(const WorkflowTaskPtr &pTask)
{
    assert(pTask);

    const std::string name = pTask->getName();
    size_t inputCount = pTask->getInputCount();

    if(inputCount == 0)
        return;

    auto types = getInputTypes(pTask);
    emit doInitDisplay(types);

    int displayIndex = 0;
    for(size_t i=0; i<inputCount; ++i)
    {
        auto inputPtr = pTask->getInput(i);
        if(inputPtr && inputPtr->isDataAvailable())
        {
            switch(inputPtr->getDataType())
            {
                case IODataType::IMAGE:
                case IODataType::IMAGE_BINARY:
                case IODataType::IMAGE_LABEL:
                    manageImageInput(displayIndex++, inputPtr, name, pTask->getInputViewProperty(i));
                    break;

                case IODataType::VIDEO:
                case IODataType::VIDEO_BINARY:
                case IODataType::VIDEO_LABEL:
                case IODataType::LIVE_STREAM:
                case IODataType::LIVE_STREAM_BINARY:
                case IODataType::LIVE_STREAM_LABEL:
                    manageVideoInput(displayIndex++, inputPtr, name, pTask->getInputViewProperty(i));
                    break;

                case IODataType::VOLUME:
                case IODataType::VOLUME_BINARY:
                    manageVolumeInput(displayIndex++, inputPtr, name, pTask->getInputViewProperty(i));
                    break;

                case IODataType::INPUT_GRAPHICS:
                    manageGraphicsInput(inputPtr);
                    break;

                default: break;
            }
        }
    }
    //Apply the view properties for all inputs (maximized status, zoom...)
    emit doApplyViewProperty();
}

void CWorkflowInputViewManager::clear(const WorkflowTaskPtr &pTask)
{
    auto types = getInputType(pTask, 0);
    emit doInitDisplay(types);
}

std::map<DisplayType,int> CWorkflowInputViewManager::getInputTypes(const WorkflowTaskPtr &pTask) const
{
    std::map<DisplayType,int> mapTypeCount;
    for(size_t i=0; i<pTask->getInputCount(); ++i)
    {
        auto inputPtr = pTask->getInput(i);
        if(inputPtr && inputPtr->isDataAvailable())
        {
            auto type = getDataViewerType(inputPtr->getDataType());
            auto it = mapTypeCount.find(type);

            if(it == mapTypeCount.end())
                mapTypeCount.insert(std::make_pair(type, 1));
            else
                it->second++;
        }
    }
    return mapTypeCount;
}

std::map<DisplayType, int> CWorkflowInputViewManager::getInputType(const WorkflowTaskPtr &pTask, int index) const
{
    std::map<DisplayType,int> mapTypeCount;
    auto inputPtr = pTask->getInput(index);

    if(inputPtr && inputPtr->isDataAvailable())
    {
        auto type = getDataViewerType(inputPtr->getDataType());
        mapTypeCount.insert(std::make_pair(type, 1));
    }
    return mapTypeCount;
}

DisplayType CWorkflowInputViewManager::getDataViewerType(const IODataType& dataType) const
{
    DisplayType type;
    switch(dataType)
    {
        case IODataType::IMAGE:
        case IODataType::IMAGE_BINARY:
        case IODataType::IMAGE_LABEL:
            type = DisplayType::IMAGE_DISPLAY;
            break;

        case IODataType::VIDEO:
        case IODataType::VIDEO_BINARY:
        case IODataType::VIDEO_LABEL:
        case IODataType::LIVE_STREAM:
        case IODataType::LIVE_STREAM_BINARY:
        case IODataType::LIVE_STREAM_LABEL:
            type = DisplayType::VIDEO_DISPLAY;
            break;

        case IODataType::VOLUME:
        case IODataType::VOLUME_BINARY:
            type = DisplayType::VOLUME_DISPLAY;
            break;

        default:
            type = DisplayType::EMPTY_DISPLAY;
            break;
    }
    return type;
}

QModelIndex CWorkflowInputViewManager::getSrcModelIndex(size_t index) const
{
    if(m_viewMode == WorkflowInputViewMode::ORIGIN)
        return m_pInputs->at(index).getModelIndex(0);
    else
        return m_pInputs->at(m_videoInputIndex).getModelIndex(0);
}

void CWorkflowInputViewManager::manageImageInput(int index, const WorkflowTaskIOPtr &inputPtr, const std::string &taskName, CViewPropertyIO* pViewProp)
{
    auto inPtr = std::dynamic_pointer_cast<CImageIO>(inputPtr);
    if(!inPtr)
    {
        qCCritical(logWorkflow).noquote() << tr("Process input management : invalid image");
        return;
    }

    CMat image;
    if(inPtr->getDataType() == IODataType::IMAGE_LABEL)
        CDataConversion::grayscaleToAlpha(inPtr->getImage(), image);
    else
        image = inPtr->getImage();

    //Emit signal to display input
    emit doDisplayImage(index, nullptr, CDataConversion::CMatToQImage(image), QString::fromStdString(taskName), pViewProp);
}

void CWorkflowInputViewManager::manageVideoInput(int index, const WorkflowTaskIOPtr &inputPtr, const std::string &taskName, CViewPropertyIO* pViewProp)
{
    auto inPtr = std::dynamic_pointer_cast<CVideoIO>(inputPtr);
    if(!inPtr)
    {
        qCCritical(logWorkflow).noquote() << tr("Process input management: invalid image");
        return;
    }

    CMat image;
    if(inPtr->getDataType() == IODataType::VIDEO_LABEL || inPtr->getDataType() == IODataType::LIVE_STREAM_LABEL)
        CDataConversion::grayscaleToAlpha(inPtr->getImage(), image);
    else
        image = inPtr->getImage();

    //Get source model index
    QModelIndex modelIndex = getSrcModelIndex(index);
    auto wrapInd = m_pProjectMgr->wrapIndex(modelIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());

    if(pItem->getTypeId() == TreeItemType::DATASET)
        modelIndex = m_pProjectMgr->getDatasetDataIndex(modelIndex, 0);

    // Emit signal to display input
    emit doDisplayVideo(modelIndex, index, nullptr, CDataConversion::CMatToQImage(image), QString::fromStdString(taskName), false, pViewProp);
    //Emit signal to initialize video info (fps...)
    emit doInitVideoInfo(modelIndex, index);
}

void CWorkflowInputViewManager::manageVolumeInput(int index, const WorkflowTaskIOPtr &inputPtr, const std::string &taskName, CViewPropertyIO* pViewProp)
{
    auto inPtr = std::dynamic_pointer_cast<CImageIO>(inputPtr);
    if(!inPtr)
    {
        qCCritical(logWorkflow).noquote() << tr("Process input management : invalid volume");
        return;
    }

    //Emit signal to update image input only. The 3D scene keeps displaying the result volume.
    emit doUpdateVolumeImage(index, CDataConversion::CMatToQImage(inPtr->getImage()), QString::fromStdString(taskName), pViewProp);
}

void CWorkflowInputViewManager::manageGraphicsInput(const WorkflowTaskIOPtr &inputPtr)
{
    Q_UNUSED(inputPtr);
}
