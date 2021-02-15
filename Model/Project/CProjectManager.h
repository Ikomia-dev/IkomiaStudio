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

#ifndef CPROJECTMANAGER_H
#define CPROJECTMANAGER_H

#include <QObject>
#include "CImageDataManager.h"
#include "CProjectUtils.hpp"
#include "CProjectDbManager.hpp"
#include "CProjectModel.h"
#include "CMultiProjectModel.h"
#include "CProgressSignalHandler.h"

class CGraphicsLayer;
class CGraphicsManager;
class CRenderManager;
class CResultManager;
class CProtocolManager;
class CVideoManager;
class CProgressBarManager;
class CMainDataManager;
class CProjectGraphicsProxyModel;
class CProjectDataProxyModel;

using CImageManagerPtr = std::shared_ptr<CImageDataManager>;

class CProjectManager : public QObject
{
    Q_OBJECT

    public:

        CProjectManager();
        ~CProjectManager();

        CProjectModel *     createProject(bool bWithRoot=true);

        QModelIndex         addFolder(const QModelIndex &itemIndex, const std::string& name, const std::string& path="");
        void                addFolderContent(const QString& folder, const QModelIndex& index);
        QModelIndex         addDataset(const QModelIndex &itemIndex, std::string name, IODataType type);
        QModelIndex         addDimension(const QModelIndex &itemIndex, const DataDimension dim);
        QModelIndex         addImages(const QModelIndex& itemIndex, QStringList &files, const DatasetLoadPolicy &policy);
        QModelIndex         addImagesToDataset(const QModelIndex &datasetItemIndex, const QStringList &files);
        QModelIndex         addGraphicsLayer(CGraphicsLayer* pLayer, const QModelIndex& parentIndex=QModelIndex());
        QModelIndex         addVideos(const QModelIndex& itemIndex, const QStringList& files);
        QModelIndex         addStream(const QModelIndex& itemIndex, const QString& cameraId, const QString& cameraName);

        void                setManagers(CGraphicsManager* pGraphicsMgr, CRenderManager* pRenderMgr,
                                        CResultManager* pResultsMgr, CProtocolManager* pProtocolMgr,
                                        CProgressBarManager *pProgressMgr, CMainDataManager* pDataMgr);

        QModelIndex         getDatasetIndex(const QModelIndex &index) const;
        QModelIndex         getCurrentDataItemIndex() const;
        QModelIndex         getCurrentVideoItemIndex() const;
        CProjectModel*      getModel(const QModelIndex& itemIndex);
        CProjectModel*      getModel(size_t index);
        CMultiProjectModel* getMultiModel();
        size_t              getProjectIndex(const QModelIndex& itemIndex);
        QModelIndex         getImageSubTreeRootIndex(const TreeItemType& type) const;
        CProjectGraphicsProxyModel* getGraphicsProxyModel(const std::vector<QModelIndex> &indicesFrom);
        CProjectDataProxyModel*     getDataProxyModel(const QModelIndex& from, const std::vector<TreeItemType> &dataTypes, const std::vector<DataDimension> &filters);
        std::string         getItemPath(const QModelIndex& index) const;
        TreeItemType        getItemType(const QModelIndex& index) const;
        ProjectTreeItem*    getItem(const QModelIndex& index) const;
        size_t              getFolderDataItemCount(const QModelIndex& index) const;
        QModelIndex         getDatasetDataIndex(const QModelIndex& datasetIndex, size_t dataIndex) const;
        QModelIndex         getFolderDataIndex(const QModelIndex& folderIndex, size_t &dataIndex) const;

        bool                isTimeDataItem(const QModelIndex& index) const;

        QModelIndex         wrapIndex(const QModelIndex& index) const;

        void                notifyDataChanged();

    signals:

        void                doSetModel(CMultiProjectModel* pMultiProject);
        void                doSetFPS(int fps);
        void                doSetVideoLength(int length);
        void                doSetTotalTime(int totalTime);
        void                doSetCurrentTime(int currentTime);

        void                doUpdateImgZ(const QModelIndex& index);
        void                doUpdateIndex(const QModelIndex& index);
        void                doUpdateSelectedIndex(const QModelIndex& index);
        void                doUpdateVideoPos(int pos);

        void                doDisplayVideo(const QModelIndex &modelIndex, int displayIndex, CImageScene *pScene, QImage image, QString name, bool bVideoChanged, CViewPropertyIO *pViewProperty);
        void                doDisplayResults(QWidget* pWidget);

        void                doSwitchView();

        void                doCloseProject(bool bCurrentImgClosed);

        void                doAfterCurrentImageDeleted();

        void                doNotifyVideoStart(int frameCount);

    public slots:

        void                onNewProject();

        void                onLoadProject(QStringList fileNames);
        void                onLoadImages(QStringList fileNames, const DatasetLoadPolicy &policy, const QModelIndex& currentIndex);
        void                onLoadVideos(QStringList fileNames, const QModelIndex& currentIndex);
        void                onLoadStream(const QString& cameraId, const QString& cameraName, const QModelIndex& currentIndex);
        void                onLoadFolder(const QString& folder, const QModelIndex& currentIndex);

        void                onSaveProjectAs(const QModelIndex& index, const QString& fileName);
        void                onSaveProject(const QModelIndex& index);

        void                onCloseProject(const QModelIndex& index);

        void                onAddFolder(const QModelIndex& index);
        void                onAddDataset(const QModelIndex& index, IODataType type);
        void                onAddDimension(const QModelIndex& index, DataDimension dim);
        void                onAddImage(const QModelIndex& index, QStringList &files, const DatasetLoadPolicy& policy);
        void                onAddImageToDimension(const QModelIndex& index, const QStringList& files);
        void                onAddDicomFolder(const QModelIndex& index, const QString& folder);
        void                onAddRecordVideo(const QString& path);

        void                onDeleteItem(const QModelIndex &index);

        void                onDisplayData(const QModelIndex &index);
        void                onDisplayVideoImage(const QModelIndex &modelIndex, int index, QImage image, QString imgName);

        void                onCurrentImgChangedInZ(int index);

        void                onInsertDimension(const QModelIndex& parentIndex, const DataDimension dim);

        void                onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

        void                onUpdateVideoPos(const QModelIndex& modelIndex, int index, int pos);
        void                onUpdateIndex(const QPersistentModelIndex& index);

        void                onRecordVideo(const QModelIndex &modelIndex, bool bRecord);
        void                onVideoStopped();

        void                onExportProject(const QModelIndex& index, const QString& folder);

        void                onShowLocation(const QModelIndex& index);

    private:

        void                initConnections();

        QModelIndex         getImageIndexFromSubItem(const QModelIndex& index);

        QModelIndex         addImagesToDimension(const QModelIndex &itemIndex, const QStringList &files);

        void                displayImage(const QModelIndex &itemIndex);
        void                displayVideo(const QModelIndex& itemIndex);

        void                removeImage(const QModelIndex& itemIndex);
        void                removeVideo(const QModelIndex& itemIndex);
        void                removeDimension(const QModelIndex& itemIndex);
        void                removeDataset(const QModelIndex& itemIndex);
        void                removeFolder(const QModelIndex& itemIndex);
        void                deleteItem(const QModelIndex& index);

        void                createIcon(const QPersistentModelIndex &itemIndex, QSize size);

        void                fillChildDataItem(const QModelIndex &parentIndex, std::vector<QModelIndex>& indexes) const;

        bool                isAlreadyLoadedInMemory(const QModelIndex& itemIndex) const;
        bool                isContainerItem(const QModelIndex& wrapIndex) const;
        bool                isImageItem(const QModelIndex& wrapIndex) const;
        bool                isImageSubItem(const QModelIndex& wrapIndex) const;

        bool                sequenceHasChanged(const QModelIndex& itemIndex) const;

        void                moveChildData(const QModelIndex& source, const QModelIndex& target, const int childCount);

        void                manageCheckStateChanged(const QModelIndex& index);

        void                notifyBeforeProjectClose(const QModelIndex& projectIndex, bool bWithCurrentImg);
        void                notifyBeforeDataDeleted(const QModelIndex& index);
        void                notifyBeforeDataDeleted(const std::vector<QModelIndex>& indexes);

        void                checkSequenceFileNames(DataDimension dim, QStringList& files);
        void                checkTimeSequenceFileName(QStringList& files);
        QModelIndex         findRootProjectIndex(const QModelIndex& index);

        void                updateImageSequenceIndex();

    private:

        CMultiProjectModel                  m_multiProject;
        std::vector<CProjectModel*>         m_projectList;
        CProjectGraphicsProxyModel*         m_pGraphicsProxyModel = nullptr;
        CProjectDataProxyModel*             m_pDataProxyModel = nullptr;
        const QAbstractItemModel*           m_pCurrentModel = nullptr;
        QPersistentModelIndex               m_currentDataItemIndex;
        size_t                              m_currentDatasetId = 0;
        DimensionIndices                    m_currentIndices;
        CGraphicsManager*                   m_pGraphicsMgr = nullptr;
        CRenderManager*                     m_pRenderMgr = nullptr;
        CResultManager*                     m_pResultsMgr = nullptr;
        CProtocolManager*                   m_pProtocolMgr = nullptr;
        CProgressBarManager*                m_pProgressMgr = nullptr;
        CMainDataManager*                   m_pDataMgr = nullptr;
        CProgressSignalHandler              m_progressSignal;
        int                                 m_loadWatcherCount = 0;
        bool                                m_bVideoChanged = true;
};

#endif // CPROJECTMANAGER_H
