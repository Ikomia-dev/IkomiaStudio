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

#ifndef CGRAPHICSMANAGER_H
#define CGRAPHICSMANAGER_H

#include <QObject>
#include "Main/AppDefine.hpp"
#include "Graphics/CGraphicsContext.h"
#include "Graphics/CGraphicsRegistration.h"
#include "Model/Project/CProjectDbManager.hpp"
#include "Model/Project/CProjectManager.h"
#include "Model/Workflow/CWorkflowManager.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"
#include "CGraphicsDbManager.h"

class CGraphicsLayer;

class CGraphicsManager : public QObject
{
    Q_OBJECT

    public:

        CGraphicsManager();

        GraphicsContextPtr      getContext();
        GraphicsShape           getCurrentTool() const;
        QModelIndex             getRootLayerIndex() const;
        QModelIndex             getCurrentLayerIndex() const;
        GraphicsLayerItemPtr    getLayerItemFromImage(const QModelIndex& imageIndex) const;
        size_t                  getImageLayerCount(const QModelIndex& imageIndex) const;
        CGraphicsLayer*         getLayer(const QModelIndex &itemIndex) const;

        bool                    isChildLayer(const QModelIndex& index, const QModelIndex& parentIndex) const;

        void                    setManagers(CProjectManager* pProjectMgr, CWorkflowManager* pWorkflowMgr);
        void                    setLayerVisible(const QModelIndex& index, bool bVisible);
        void                    setCurrentLayer(const QModelIndex& index, bool bNotifyView);
        void                    setCurrentLayer(CGraphicsLayer* pLayer);
        void                    setCurrentTool(GraphicsShape tool);
        void                    setCurrentCategory(const QString& category);

        QModelIndex             addLayer(CGraphicsLayer* pLayer);
        void                    addTemporaryLayer(const CGraphicsLayerInfo& layerInfo);

        QModelIndex             findLayerFromName(const QString& name, QModelIndex startIndex=QModelIndex()) const;

        template<typename T>
        bool                    loadGraphics(std::shared_ptr<T> itemPtr, const QModelIndex& index)
        {
            if(itemPtr)
            {
                //Set global graphics context
                itemPtr->setGraphicsContext(m_contextPtr);
                //Get graphics root layer
                QModelIndex rootLayerIndex;
                auto pMultiModel = m_pProjectMgr->getMultiModel();
                int childCount = pMultiModel->rowCount(index);

                for(int i=0; i<childCount && !rootLayerIndex.isValid(); ++i)
                {
                    auto indexTmp = pMultiModel->index(i, 0, index);
                    auto pChild = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(indexTmp).internalPointer());

                    if(pChild->getTypeId() == TreeItemType::GRAPHICS_LAYER)
                        rootLayerIndex = indexTmp;
                }

                if(rootLayerIndex.isValid())
                {
                    auto wrapRootLayerIdx = m_pProjectMgr->wrapIndex(rootLayerIndex);
                    auto pItem = static_cast<ProjectTreeItem*>(wrapRootLayerIdx.internalPointer());
                    auto pLayerItem = pItem->getNode<std::shared_ptr<CGraphicsLayerItem>>();

                    if(pLayerItem->isLoaded() == false)
                    {
                        //Recursive loading of all graphics layers
                        CProjectDbManager projectDB(m_pProjectMgr->getModel(index));
                        QString path = QString::fromStdString(itemPtr->getFullPath());
                        CGraphicsDbManager graphicsDB(projectDB.getPath(), projectDB.getConnectionName());
                        GraphicsDbInfo info = graphicsDB.loadGraphics(itemPtr->getTypeId(), path);
                        loadLayer(nullptr, wrapRootLayerIdx, info);
                    }

                    //Set first child layer as current
                    if(pMultiModel->rowCount(rootLayerIndex) > 0)
                    {
                        auto firstIndex = pMultiModel->index(0, 0, rootLayerIndex);
                        setCurrentLayer(firstIndex, true);
                        return true;
                        /*auto pItem = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(firstIndex).internalPointer());
                        assert(pItem);
                        auto pLayerItem = pItem->getNode<std::shared_ptr<CGraphicsLayerItem>>();
                        assert(pLayerItem);
                        pLayerItem->setHighlighted(true);
                        m_currentLayerIndex = firstIndex;
                        emit doSetCurrentLayer(pLayerItem->getLayer());*/
                    }
                }
            }
            return false;
        }
        void                    loadAllGraphics(const QModelIndex& index);

        void                    removeLayer(const QModelIndex &itemIndex);
        void                    removeTemporaryLayer(const CGraphicsLayerInfo &layerInfo);

        void                    notifyProjectSave(int projectIndex);
        void                    beforeProjectClose(int projectIndex);
        void                    notifyViewShow();

        void                    burnGraphicsToImage(const QModelIndex& imageIndex, CMat &image);
        void                    burnLayerToImage(const CGraphicsLayer *pLayer, CMat& image);

    public slots:

        void                    onSetGraphicsTool(GraphicsShape tool);
        void                    onSetCurrentLayer(const QModelIndex& index);
        void                    onAddLayerFromView(CGraphicsLayer* pLayer);
        void                    onAddLayer(const QModelIndex& parentIndex);
        void                    onGraphicsChanged();
        void                    onGraphicsRemoved(const QSet<CGraphicsLayer*>& layers);
        void                    onInputDataChanged(const QModelIndex& itemIndex, int inputIndex, bool bNewSequence);

    signals:

        void                    doSetGraphicsContext(GraphicsContextPtr& context);
        void                    doSetGraphicsTool(GraphicsShape tool);
        void                    doSetCurrentLayer(CGraphicsLayer* pLayer);
        void                    doAddGraphicsItem(QGraphicsItem* pItem, bool bForceParent);
        void                    doAddTemporaryLayer(const CGraphicsLayerInfo& layerInfo);
        void                    doRemoveGraphicsLayer(const CGraphicsLayerInfo& layerInfo, bool bDelete);
        void                    doGraphicsSceneChanged();
        void                    doGraphicsContextChanged();

    private:

        void                    setLayerHighlighted(const QModelIndex& index, bool bHighlighted);

        GraphicsLayerItemPtr    getLayerItem(const QModelIndex& layerIndex) const;

        bool                    isLayerFromCurrentImage(const QModelIndex& layerIndex) const;
        bool                    isLayerRemovable(CGraphicsLayer* pLayer) const;
        bool                    isItemCompatible(TreeItemType type) const;

        QModelIndex             createRootLayer();

        QModelIndex             addLayer(CGraphicsLayer* pLayer, const QModelIndex& parentIndex);

        void                    fillLayerTreeIds(const QModelIndex& layerIndex, std::vector<int>& ids);

        void                    loadLayer(CGraphicsLayer *pParentLayer, const QModelIndex &layerIndex, const GraphicsDbInfo& layersInfo);

        void                    removeValidEmptyLayers(const QModelIndex &layerIndex);

    private:

        CProjectManager*                            m_pProjectMgr = nullptr;
        CWorkflowManager*                           m_pWorkflowMgr = nullptr;
        QPersistentModelIndex                       m_currentLayerIndex = QPersistentModelIndex();
        QPersistentModelIndex                       m_currentImgIndex = QPersistentModelIndex();
        CGraphicsRegistration                       m_registration;
        //Layer database ids per project to be removed
        std::unordered_map<int, std::vector<int>>   m_removedLayers;
        GraphicsContextPtr                          m_contextPtr;
        std::vector<CGraphicsLayerInfo>             m_tmpLayersInfo;
};

#endif // CGRAPHICSMANAGER_H
