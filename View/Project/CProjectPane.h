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

#ifndef CPROJECTPANE_H
#define CPROJECTPANE_H

/**
 * @file      CProjectPane.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CProjectPane definition
 *
 * @details   Details
 */

#include "View/Common/CPane.h"
#include "Model/CMainModel.h"
#include "View/Common/CContextMenu.hpp"
#include "CProjectTreeView.h"

/**
 * @brief
 *
 */
class CProjectPane : public QWidget
{
    Q_OBJECT

    public:

        CProjectPane(QWidget* parent = Q_NULLPTR);

        void                setModel(QAbstractItemModel* pModel);

        CProjectTreeView*   getTreeView();
        DatasetLoadPolicy   getDatasetLoadPolicy(size_t srcType, const QStringList& files);
        QModelIndex         getCurrentIndex();

        void                selectImageFiles(QModelIndex& index);
        void                selectFolder(const QModelIndex& index);
        void                selectDicomFolder(QModelIndex& index);

    private:

        void                initLayout();
        void                initContextMenu();
        void                initConnections();

        void                addDimensionSubMenu(const QModelIndex& index);
        void                addProjectSubDimension(QModelIndex& index, DataDimension dim);

        bool                isValidImageContainerItem(ProjectTreeItem* pItem);
        bool                isSameImageDimensions(const QStringList& files);
        bool                isLeafDimension(ProjectTreeItem* pItemDim);
        bool                isItemHasChildData(ProjectTreeItem* pItemDim);

        bool                hasDimensionItem(ProjectTreeItem* pItem);

        void                saveAs(const QModelIndex& index);
        void                save(const QModelIndex &index);

        void                exportProject(const QModelIndex& index);

        QModelIndex         wrapIndex(const QModelIndex& index);

    signals:

        void                doUpdateIndex(const QModelIndex& index);
        void                doUpdateProjectModelIndex(const QModelIndex& index);

        void                doAddEmptyFolder(const QModelIndex& index);
        void                doAddFolder(const QString& folder, const QModelIndex& index);
        void                doAddDataset(const QModelIndex& index, IODataType dataType);
        void                doAddDimension(const QModelIndex& index, DataDimension dim);
        void                doAddImages(const QModelIndex& index, QStringList& files, const DatasetLoadPolicy& policy);
        void                doAddImagesToDimension(const QModelIndex& index, QStringList& files);
        void                doAddDicomFolder(const QModelIndex& index, const QString& folder);
        void                doAddGraphicsLayer(const QModelIndex& index);

        void                doDisplayData(const QModelIndex& index);

        void                doNewProject();
        void                doOpenProject();

        void                doSaveProjectAs(const QModelIndex& index, const QString& name);
        void                doSaveProject(const QModelIndex& index);

        void                doExportProject(const QModelIndex& index, const QString& folder);

        void                doCloseProject(const QModelIndex& index);

        void                doApplyProcess(const QModelIndex& index, const std::string& processName, const std::shared_ptr<CProtocolTaskParam>& pParam);

        void                doInsertProjectDimension(const QModelIndex& index, const DataDimension dim);

        void                doSetCurrentGraphicsLayer(const QModelIndex& index);

        void                doRemoveItem(const QModelIndex& index);

        void                doShowLocation(const QModelIndex& index);

    public slots:

        void                onTreeViewClicked(const QModelIndex& index);
        void                onUpdateIndex(const QModelIndex& index);
        void                onUpdateProject(CMultiProjectModel* pProject);
        void                onShowContextMenu(const QPoint& pos);
        void                onApplyProcess(const std::string& processName, const std::shared_ptr<CProtocolTaskParam>& pParam);
        void                onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
        void                onUpdateSelectedIndex(const QModelIndex& index);
        void                onSelectionChanged(const QItemSelection& selected, const QItemSelection& /*deselected*/);

    private:

        CProjectTreeView*           m_pTreeView = nullptr;
        CMultiContextMenu<menu_sig> m_contextMenu;
        QModelIndex                 m_currentIndex;
        bool                        m_bTest = true;
};

#endif // CPROJECTPANE_H
