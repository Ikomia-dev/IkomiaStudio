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

#include "CGraphicsManager.h"
#include <QDebug>
#include "Main/LogCategory.h"
#include "Main/AppTools.hpp"
#include "Graphics/CGraphicsLayer.h"

CGraphicsManager::CGraphicsManager()
{
    m_contextPtr = std::make_shared<CGraphicsContext>();
}

GraphicsContextPtr CGraphicsManager::getContext()
{
    return m_contextPtr;
}

GraphicsShape CGraphicsManager::getCurrentTool() const
{
    return m_contextPtr->getTool();
}

QModelIndex CGraphicsManager::getRootLayerIndex() const
{
    assert(m_pProjectMgr);
    return m_pProjectMgr->getImageSubTreeRootIndex(TreeItemType::GRAPHICS_LAYER);
}

QModelIndex CGraphicsManager::getCurrentLayerIndex() const
{
    return m_currentLayerIndex;
}

GraphicsLayerItemPtr CGraphicsManager::getLayerItemFromImage(const QModelIndex &imageIndex) const
{
    auto pMultiModel = m_pProjectMgr->getMultiModel();
    int childCount = pMultiModel->rowCount(imageIndex);

    for(int i=0; i<childCount; ++i)
    {
        auto indexTmp = pMultiModel->index(i, 0, imageIndex);
        auto wrapIndex = m_pProjectMgr->wrapIndex(indexTmp);
        auto pChild = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

        if(pChild->getTypeId() == TreeItemType::GRAPHICS_LAYER)
        {
            auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
            if(itemPtr != nullptr)
                return itemPtr->getNode<std::shared_ptr<CGraphicsLayerItem>>();
        }
    }
    return nullptr;
}

size_t CGraphicsManager::getImageLayerCount(const QModelIndex &imageIndex) const
{
    auto pLayerItem = getLayerItemFromImage(imageIndex);
    if(pLayerItem == nullptr)
        return 0;

    auto pRootLayer = pLayerItem->getLayer();
    if(pRootLayer == nullptr)
        return 0;

    return pRootLayer->getChildLayerCount();
}

bool CGraphicsManager::isChildLayer(const QModelIndex &index, const QModelIndex &parentIndex) const
{
    assert(m_pProjectMgr);
    assert(index.isValid() && parentIndex.isValid());

    if(index == parentIndex)
        return false;

    auto wrapIndex = m_pProjectMgr->wrapIndex(index);
    auto wrapParentIndex = m_pProjectMgr->wrapIndex(parentIndex);

    QModelIndex parentTmp = wrapIndex.parent();
    auto pItem = static_cast<ProjectTreeItem*>(parentTmp.internalPointer());
    bool isChild = parentTmp == wrapParentIndex;

    while(!isChild && pItem->getTypeId() == TreeItemType::GRAPHICS_LAYER)
    {
        parentTmp = parentTmp.parent();
        pItem = static_cast<ProjectTreeItem*>(parentTmp.internalPointer());
        isChild = parentTmp == wrapParentIndex;
    }
    return isChild;
}

void CGraphicsManager::setManagers(CProjectManager *pProjectMgr, CWorkflowManager *pWorkflowMgr)
{
    m_pProjectMgr = pProjectMgr;
    m_pWorkflowMgr = pWorkflowMgr;
}

void CGraphicsManager::setLayerVisible(const QModelIndex &index, bool bVisible)
{
    auto pLayer = getLayer(index);
    if(pLayer)
        pLayer->setVisible(bVisible);
}

void CGraphicsManager::setCurrentLayer(const QModelIndex &index, bool bNotifyView)
{
    if(!index.isValid())
    {
        qCDebug(logGraphics) << tr("Current graphics layer not set: invalid index");
        return;
    }

    auto pLayer = getLayer(index);
    if(pLayer == nullptr)
    {
        qCDebug(logGraphics) << tr("Current graphics layer not set: invalid layer");
        return;
    }

    if(index != m_currentLayerIndex)
    {
        setLayerHighlighted(m_currentLayerIndex, false);
        m_currentLayerIndex = index;

        if(pLayer->isRoot() == false)
            setLayerHighlighted(m_currentLayerIndex, true);
    }

    if(bNotifyView)
    {
        // Set view (CImageScene) current layer
        emit doSetCurrentLayer(pLayer);
    }
}

void CGraphicsManager::setCurrentLayer(CGraphicsLayer *pLayer)
{
    if(pLayer == nullptr)
    {
        qCCritical(logGraphics).noquote() << tr("Current graphics layer not set: invalid layer");
        return;
    }

    // Set view (CImageScene) current layer
    emit doSetCurrentLayer(pLayer);
}

void CGraphicsManager::setCurrentTool(GraphicsShape tool)
{
    if(m_contextPtr)
    {
        m_contextPtr->setTool(tool);
        emit doGraphicsContextChanged();
    }
}

void CGraphicsManager::setCurrentCategory(const QString &category)
{
    if(m_contextPtr)
    {
        m_contextPtr->setCurrentCategory(category.toStdString());
        emit doGraphicsContextChanged();
    }
}

QModelIndex CGraphicsManager::addLayer(CGraphicsLayer *pLayer)
{
    assert(m_pProjectMgr && pLayer);

    if(!m_currentImgIndex.isValid())
    {
        qCCritical(logGraphics).noquote() << tr("Error while adding layer: invalid current image");
        return QModelIndex();
    }

    QModelIndex parentLayerIndex = m_currentLayerIndex;
    if(!parentLayerIndex.isValid())
    {
        auto rootLayerIndex = getRootLayerIndex();
        if(!rootLayerIndex.isValid())
            parentLayerIndex = createRootLayer();
    }

    if(pLayer->parentItem() == nullptr)
    {
        auto pParentLayer = getLayer(parentLayerIndex);
        pLayer->setParentItem(pParentLayer);
    }

    QModelIndex newLayerIndex = addLayer(pLayer, parentLayerIndex);
    return newLayerIndex;
}

QModelIndex CGraphicsManager::addLayer(CGraphicsLayer *pLayer, const QModelIndex &parentIndex)
{
    assert(pLayer);

    if(!m_currentImgIndex.isValid())
        return QModelIndex();

    QModelIndex parentIndexTmp = parentIndex;
    if(!parentIndexTmp.isValid())
        parentIndexTmp = m_currentImgIndex;

    auto pMultiProject = m_pProjectMgr->getMultiModel();
    auto index = pMultiProject->addItem(parentIndexTmp, std::make_shared<CGraphicsLayerItem>(pLayer, pLayer->getName().toStdString()));

    if(index.isValid())
        emit doGraphicsSceneChanged();

    return index;
}

void CGraphicsManager::addTemporaryLayer(const CGraphicsLayerInfo& layerInfo)
{
    if(layerInfo.m_pLayer == nullptr)
        return;

    //Add graphics layer to the image view
    //No copy since pLayer is temporary and the ownership returns to caller when this layer is removed from the image scene
    emit doAddTemporaryLayer(layerInfo);
    m_tmpLayersInfo.push_back(layerInfo);
}

QModelIndex CGraphicsManager::findLayerFromName(const QString &name, QModelIndex startIndex) const
{
    if(!startIndex.isValid())
        startIndex = getRootLayerIndex();

    auto pModel = static_cast<const CMultiProjectModel*>(startIndex.model());
    return pModel->findItemFromName(name.toStdString(), startIndex);
}

void CGraphicsManager::onSetGraphicsTool(GraphicsShape tool)
{
    m_contextPtr->setTool(tool);
}

void CGraphicsManager::onAddLayerFromView(CGraphicsLayer* pLayer)
{
    assert(m_pProjectMgr);
    assert(pLayer);

    QModelIndex newLayerIndex = addLayer(pLayer, m_currentLayerIndex);
    setCurrentLayer(newLayerIndex, false);
}

void CGraphicsManager::onAddLayer(const QModelIndex &parentIndex)
{
    assert(m_pProjectMgr);

    auto pParentLayer = getLayer(parentIndex);
    if(pParentLayer == nullptr)
    {
        qCCritical(logGraphics).noquote() << tr("Add graphics layer error: invalid parent layer");
        return;
    }

    auto pLayer = new CGraphicsLayer(tr("New layer"), true, pParentLayer);
    QModelIndex newLayerIndex = addLayer(pLayer, parentIndex);

    //The current layer must be set for the current image only
    if(isLayerFromCurrentImage(newLayerIndex))
        setCurrentLayer(newLayerIndex, true);
}

void CGraphicsManager::onSetCurrentLayer(const QModelIndex &index)
{
    assert(m_pProjectMgr);

    //The current layer must be set for the current image only
    if(isLayerFromCurrentImage(index) == false)
    {
        qCCritical(logGraphics).noquote() << tr("Current graphics layer can only be set for the current image");
        return;
    }

    auto pLayer = getLayer(index);
    if(pLayer == nullptr || pLayer->isRoot())
    {
        qCCritical(logGraphics).noquote() << tr("Current graphics layer not set: invalid layer");
        return;
    }

    setCurrentLayer(index, true);
}

void CGraphicsManager::loadAllGraphics(const QModelIndex &index)
{
    if(index != m_currentImgIndex)
    {
        try
        {
            bool bLoaded = false;
            m_currentImgIndex = index;
            auto wrapIndex = m_pProjectMgr->wrapIndex(index);
            auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

            if(pItem->getTypeId() == TreeItemType::IMAGE)
            {
                auto imgItemPtr = CProjectUtils::getItem<CImageItem>(wrapIndex, TreeItemType::IMAGE);
                bLoaded = loadGraphics(imgItemPtr, index);
            }
            else if(pItem->getTypeId() == TreeItemType::VIDEO)
            {
                auto videoItemPtr = CProjectUtils::getItem<CVideoItem>(wrapIndex, TreeItemType::VIDEO);
                bLoaded = loadGraphics(videoItemPtr, index);
            }
            else if(pItem->getTypeId() == TreeItemType::LIVE_STREAM)
            {
                auto liveStreamItemPtr = CProjectUtils::getItem<CLiveStreamItem>(wrapIndex, TreeItemType::LIVE_STREAM);
                bLoaded = loadGraphics(liveStreamItemPtr, index);
            }

            if(bLoaded == false)
            {
                setLayerHighlighted(m_currentLayerIndex, false);
                m_currentLayerIndex = QPersistentModelIndex();
            }
        }
        catch(std::exception& e)
        {
            qCCritical(logGraphics).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CGraphicsManager::removeLayer(const QModelIndex &itemIndex)
{
    assert(m_pProjectMgr);
    QModelIndex newCurrentIndex;

    //Set the parent layer as current if the layer to remove is the current one
    if(itemIndex == m_currentLayerIndex)
        newCurrentIndex = itemIndex.parent();

    //We need to store ids of removed layers to really remove them if project is saved
    std::vector<int> layerIds;
    fillLayerTreeIds(itemIndex, layerIds);

    auto it = m_removedLayers.find((int)m_pProjectMgr->getProjectIndex(itemIndex));
    if(it != m_removedLayers.end())
        it->second.insert(it->second.end(), layerIds.begin(), layerIds.end());
    else
        m_removedLayers.insert(std::make_pair((int)m_pProjectMgr->getProjectIndex(itemIndex), layerIds));

    //Remove from image scene if the layer is loaded
    auto pLayer = getLayer(itemIndex);
    m_pProjectMgr->getMultiModel()->removeItem(itemIndex);

    if(pLayer)
    {
        QModelIndex refDataIndex = m_pProjectMgr->getCurrentDataItemIndex();
        DisplayType refDataType = Utils::Data::treeItemTypeToDisplayType(m_pProjectMgr->getItemType(refDataIndex));
        m_pWorkflowMgr->beforeGraphicsLayerRemoved(pLayer);
        CGraphicsLayerInfo info(pLayer, 0, refDataType, CGraphicsLayerInfo::SOURCE);
        emit doRemoveGraphicsLayer(info, true);
        emit doGraphicsSceneChanged();
    }

    //Set new current layer if possible
    if(newCurrentIndex.isValid())
        setCurrentLayer(newCurrentIndex, true);
}

void CGraphicsManager::removeTemporaryLayer(const CGraphicsLayerInfo& layerInfo)
{
    if(layerInfo.m_pLayer == nullptr)
        return;

    for (size_t i=m_tmpLayersInfo.size() - 1; i>=0; i--)
    {
        if (m_tmpLayersInfo[i] == layerInfo)
        {
            emit doRemoveGraphicsLayer(m_tmpLayersInfo[i], false);
            if (m_tmpLayersInfo[i].m_displayTarget == CGraphicsLayerInfo::SOURCE)
                setCurrentLayer(m_currentLayerIndex, true);

            m_tmpLayersInfo.erase(m_tmpLayersInfo.begin() + i);
        }
    }
}

void CGraphicsManager::notifyProjectSave(int projectIndex)
{
    auto it = m_removedLayers.find(projectIndex);
    if(it != m_removedLayers.end())
    {
        try
        {
            //Remove orphans graphics items directly in database
            CProjectDbManager projectDB(m_pProjectMgr->getModel(projectIndex));
            CGraphicsDbManager graphicsDB(projectDB.getPath(), projectDB.getConnectionName());
            graphicsDB.remove(it->second);
        }
        catch(std::exception& e)
        {
            qCCritical(logGraphics).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CGraphicsManager::beforeProjectClose(int projectIndex)
{
    auto it = m_removedLayers.find(projectIndex);
    if(it != m_removedLayers.end())
        m_removedLayers.erase(it);
}

void CGraphicsManager::notifyViewShow()
{
    emit doSetGraphicsContext(m_contextPtr);
}

void CGraphicsManager::burnGraphicsToImage(const QModelIndex& imageIndex, CMat &image)
{
    if(imageIndex.isValid() == false)
        return;

    if(!image.data)
        return;

    auto pLayerItem = getLayerItemFromImage(imageIndex);
    if(pLayerItem == nullptr)
        return;

    auto pLayer = pLayerItem->getLayer();
    if(pLayer == nullptr)
        return;

    burnLayerToImage(pLayer, image);
}

void CGraphicsManager::burnLayerToImage(const CGraphicsLayer *pLayer, CMat &image)
{
    if(pLayer->isVisible() == false)
        return;

    CGraphicsConversion graphicsConv((int)image.getNbCols(), (int)image.getNbRows());
    auto items = pLayer->getChildItems();

    //Double dispatch design pattern
    for(auto it : items)
        dynamic_cast<CGraphicsItem*>(it)->insertToImage(image, graphicsConv, false, false);

    auto childLayers = pLayer->getChildLayers();
    for(auto it : childLayers)
        burnLayerToImage(it, image);
}

void CGraphicsManager::onGraphicsChanged()
{
    assert(m_pProjectMgr);
    assert(m_pWorkflowMgr);
    m_pProjectMgr->notifyDataChanged();
    m_pWorkflowMgr->notifyGraphicsChanged();
}

void CGraphicsManager::onGraphicsRemoved(const QSet<CGraphicsLayer *> &layers)
{
    assert(m_pProjectMgr);
    assert(m_pWorkflowMgr);

    //remove empty and non-user layers
    for(auto it: layers)
    {
        QModelIndex layerIndex = findLayerFromName(it->getName());
        removeValidEmptyLayers(layerIndex);
    }

    m_pProjectMgr->notifyDataChanged();
    m_pWorkflowMgr->notifyGraphicsChanged();
}

void CGraphicsManager::onInputDataChanged(const QModelIndex &itemIndex, int inputIndex, bool bNewSequence)
{
    // If temporary layers from plugins are active, we add them in the current view (CImageScene)
    for (size_t i=0; i<m_tmpLayersInfo.size(); ++i)
    {
        if (m_tmpLayersInfo[i].m_pLayer != nullptr)
        {
            emit doAddTemporaryLayer(m_tmpLayersInfo[i]);
            setCurrentLayer(m_tmpLayersInfo[i].m_pLayer);
        }
    }
}

void CGraphicsManager::setLayerHighlighted(const QModelIndex &index, bool bHighlighted)
{
    if(index.isValid())
    {
        auto itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(index).internalPointer());
        if(itemPtr)
            itemPtr->setHighlighted(bHighlighted);
    }
}

CGraphicsLayer *CGraphicsManager::getLayer(const QModelIndex &itemIndex) const
{
    if(!itemIndex.isValid())
        return nullptr;

    auto pItem = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(itemIndex).internalPointer());
    if(!pItem)
        return nullptr;

    if(pItem->getTypeId() != TreeItemType::GRAPHICS_LAYER)
        return nullptr;

    auto pLayertItem = pItem->getNode<std::shared_ptr<CGraphicsLayerItem>>();
    if(!pLayertItem)
        return nullptr;

    return pLayertItem->getLayer();
}

GraphicsLayerItemPtr CGraphicsManager::getLayerItem(const QModelIndex &layerIndex) const
{
    if(!layerIndex.isValid())
        return nullptr;

    auto pItem = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(layerIndex).internalPointer());
    if(!pItem)
        return nullptr;

    if(pItem->getTypeId() != TreeItemType::GRAPHICS_LAYER)
        return nullptr;

    return pItem->getNode<GraphicsLayerItemPtr>();
}

bool CGraphicsManager::isLayerFromCurrentImage(const QModelIndex &layerIndex) const
{
    //Retrieve image QModelIndex
    QModelIndex imageIndex = layerIndex.parent();
    auto itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(layerIndex).parent().internalPointer());

    while(itemPtr != nullptr && isItemCompatible(static_cast<TreeItemType>(itemPtr->getTypeId())) == false)
    {
        imageIndex = imageIndex.parent();
        if(imageIndex.isValid())
            itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(imageIndex).internalPointer());
    }
    return imageIndex == m_currentImgIndex;
}

//Layer is removable if it is empty and if it is not a user layer
bool CGraphicsManager::isLayerRemovable(CGraphicsLayer* pLayer) const
{
    if(pLayer->isUserLayer())
        return false;

    bool bRemovable = (pLayer->getChildItemCount() == 0);
    auto childLayers = pLayer->getChildLayers();

    for(int i=0; i<childLayers.size(); ++i)
        bRemovable &= isLayerRemovable(childLayers[i]);

    return bRemovable;
}

bool CGraphicsManager::isItemCompatible(TreeItemType type) const
{
    return type == TreeItemType::IMAGE || type == TreeItemType::VIDEO || type == TreeItemType::LIVE_STREAM;
}

QModelIndex CGraphicsManager::createRootLayer()
{
    assert(m_currentImgIndex.isValid());

    auto pRootLayer = new CGraphicsLayer(tr("Graphics"));
    pRootLayer->setRoot(true);
    auto layerIndex = addLayer(pRootLayer, m_currentImgIndex);
    emit doAddGraphicsItem(pRootLayer, false);
    emit doSetCurrentLayer(pRootLayer);
    return layerIndex;
}

void CGraphicsManager::fillLayerTreeIds(const QModelIndex &layerIndex, std::vector<int> &ids)
{
    auto pLayerItem = getLayerItem(layerIndex);
    if(pLayerItem)
    {
        ids.push_back(pLayerItem->getDbId());

        //Iterate through all child layers
        auto pMultiProject = m_pProjectMgr->getMultiModel();
        int childCount = pMultiProject->rowCount(layerIndex);

        for(int i=0; i<childCount; ++i)
        {
            auto childIndex = pMultiProject->index(i, 0, layerIndex);
            fillLayerTreeIds(childIndex, ids);
        }
    }
}

void CGraphicsManager::loadLayer(CGraphicsLayer* pParentLayer, const QModelIndex& layerIndex, const GraphicsDbInfo &layersInfo)
{
    //Warning :: layerIndex is from CProjectModel not from CMultiProjectModel
    auto pModel = static_cast<const CProjectModel*>(layerIndex.model());
    auto itemPtr = static_cast<ProjectTreeItem*>(layerIndex.internalPointer());
    auto pLayerItem = itemPtr->getNode<std::shared_ptr<CGraphicsLayerItem>>();

    auto pLayer = new CGraphicsLayer(QString::fromStdString(pLayerItem->getName()), pParentLayer);
    pLayerItem->setLayer(pLayer);

    if(pParentLayer == nullptr)
    {
        pLayer->setRoot(true);
        emit doAddGraphicsItem(pLayer, false);
    }

    //Add all child items (CGraphicsItem)
    CGraphicsJSON jsonMgr;
    auto pFactory = m_registration.getQtBasedFactory();
    auto itRange = layersInfo.equal_range(pLayerItem->getDbId());

    for(auto it=itRange.first; it!=itRange.second; ++it)
    {
        auto pItem = dynamic_cast<CGraphicsItem*>(pFactory.createObject(it->second.first, (QGraphicsItem*)pLayer));
        if(pItem)
            pItem->buildFromJsonData(jsonMgr, it->second.second);
    }

    //Iterate through all child layers
    int childCount = pModel->rowCount(layerIndex);
    for(int i=0; i<childCount; ++i)
    {
        auto childIndex = pModel->index(i, 0, layerIndex);
        auto pChild = static_cast<ProjectTreeItem*>(childIndex.internalPointer());

        if(pChild->getTypeId() == TreeItemType::GRAPHICS_LAYER)
            loadLayer(pLayer, childIndex, layersInfo);
    }
}

void CGraphicsManager::removeValidEmptyLayers(const QModelIndex& layerIndex)
{
    if(layerIndex.isValid() == false)
        return;

    CGraphicsLayer* pLayer = getLayer(layerIndex);
    if(pLayer == nullptr)
        return;

    if(isLayerRemovable(pLayer) == true)
    {
        auto parentLayerIndex = layerIndex.parent();
        removeLayer(layerIndex);
        removeValidEmptyLayers(parentLayerIndex);
    }
}

#include "moc_CGraphicsManager.cpp"
