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

#include "CProjectExportMgr.h"
#include "CException.h"
#include "Model/Project/CProjectModel.h"
#include "Model/Project/CProjectDbManager.hpp"
#include "Main/LogCategory.h"

CProjectExportMgr::CProjectExportMgr(CProjectModel *pModel, const QString &folder)
{
    m_pSrcModel = pModel;
    m_parentFolder = folder.toStdString();
}

void CProjectExportMgr::run()
{
    if(m_pSrcModel == nullptr)
        throw CException(CoreExCode::NULL_POINTER, "Invalid project", __func__, __FILE__, __LINE__);

    //Get project name
    auto rootIndex = m_pSrcModel->index(0, 0);
    auto projectName = rootIndex.data(Qt::DisplayRole).toString();

    if(projectName.endsWith("*"))
        projectName.chop(1);

    //Create a sub folder of 'folder' with project name
    try
    {
        m_projectFolder = m_parentFolder + "/" + projectName.toStdString();
        boost::filesystem::create_directory(m_projectFolder);
        //Create a data folder in project folder
        m_dataFolder = m_projectFolder + "/Data";
        boost::filesystem::create_directory(m_dataFolder);
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.code().message());
        return;
    }

    //Create new project
    QString newPath = QString::fromStdString(m_projectFolder) + "/" + projectName + ".db";
    m_pNewModel = new CProjectModel;
    m_pNewModel->setExported(true);
    //Copy source model to new one
    copy(rootIndex, QModelIndex());
    //Save it
    CProjectDbManager projectDb(projectName, m_pNewModel);
    projectDb.saveProject(newPath);
}

void CProjectExportMgr::copy(const QModelIndex& srcIndex, const QModelIndex &dstIndex)
{
    ProjectTreeItem* itemSrcPtr = static_cast<ProjectTreeItem*>(srcIndex.internalPointer());
    if(itemSrcPtr == nullptr)
        throw CException(CoreExCode::NULL_POINTER, "Export: invalid source item", __func__, __FILE__, __LINE__);

    QModelIndex newItemIndex;
    if(dstIndex.isValid() == false)
    {
        //Add root item
        m_pNewModel->getRoot()->emplace_back(std::make_shared<CProjectItem>(itemSrcPtr->getName()));
        newItemIndex = m_pNewModel->index(0, 0);
    }
    else
        newItemIndex = addItem(srcIndex, dstIndex);

    int childCount = m_pSrcModel->rowCount(srcIndex);
    for(int i=0; i<childCount; ++i)
        copy(m_pSrcModel->index(i, 0, srcIndex), newItemIndex);
}

QModelIndex CProjectExportMgr::addItem(const QModelIndex& srcIndex, const QModelIndex &dstParent)
{
    assert(srcIndex.isValid());
    assert(dstParent.isValid());

    QModelIndex newItemIndex;
    ProjectTreeItem* itemSrcPtr = static_cast<ProjectTreeItem*>(srcIndex.internalPointer());
    assert(itemSrcPtr);

    auto typeId = static_cast<TreeItemType>(itemSrcPtr->getTypeId());
    switch(typeId)
    {
        case TreeItemType::NONE:
        case TreeItemType::PROJECT:
        case TreeItemType::PROCESS:
            break;

        case TreeItemType::DATASET:
        {
            auto datasetItemPtr = itemSrcPtr->getNode<std::shared_ptr<CDatasetItem<CMat>>>();
            m_pNewModel->emplace_back(dstParent, datasetItemPtr->clone());
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::DIMENSION:
        {
            auto dimItemPtr = itemSrcPtr->getNode<std::shared_ptr<CDimensionItem>>();
            m_pNewModel->emplace_back(dstParent, dimItemPtr->clone());
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::FOLDER:
        {
            auto folderItemPtr = itemSrcPtr->getNode<std::shared_ptr<CFolderItem>>();
            m_pNewModel->emplace_back(dstParent, folderItemPtr->clone());
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::GRAPHICS_LAYER:
        {
            auto graphicsItemPtr = itemSrcPtr->getNode<std::shared_ptr<CGraphicsLayerItem>>();
            m_pNewModel->emplace_back(dstParent, graphicsItemPtr->clone());
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::IMAGE:
        {
            auto imageItemPtr = itemSrcPtr->getNode<std::shared_ptr<CImageItem>>();
            auto newItemPtr = imageItemPtr->clone();
            //Copy image to data folder
            std::string srcPath = imageItemPtr->getFullPath();
            boost::filesystem::path path(srcPath);
            std::string newPath = m_dataFolder + "/" + path.filename().string();
            newPath = Utils::File::getAvailablePath(newPath);

            // Copy file
            try
            {
                boost::filesystem::copy_file(srcPath, newPath, boost::filesystem::copy_options::overwrite_existing);
            }
            catch (const boost::filesystem::filesystem_error& e)
            {
                qCCritical(logProject).noquote() << QString::fromStdString(e.code().message());
                break;
            }

            //Update path for new item
            newItemPtr->setFullPath(newPath);
            m_pNewModel->emplace_back(dstParent, newItemPtr);
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::VIDEO:
        {
            auto videoItemPtr = itemSrcPtr->getNode<std::shared_ptr<CVideoItem>>();
            auto newItemPtr = videoItemPtr->clone();
            //Copy image to data folder
            std::string srcPath = videoItemPtr->getFullPath();
            boost::filesystem::path path(srcPath);
            std::string newPath = m_dataFolder + "/" + path.filename().string();
            newPath = Utils::File::getAvailablePath(newPath);

            // Copy file
            try
            {
                boost::filesystem::copy_file(srcPath, newPath, boost::filesystem::copy_options::overwrite_existing);
            }
            catch (const boost::filesystem::filesystem_error& e)
            {
                qCCritical(logProject).noquote() << QString::fromStdString(e.code().message());
                break;
            }

            //Update path for new item
            newItemPtr->setFullPath(newPath);
            m_pNewModel->emplace_back(dstParent, newItemPtr);
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::LIVE_STREAM:
        {
            auto liveItemPtr = itemSrcPtr->getNode<std::shared_ptr<CLiveStreamItem>>();
            m_pNewModel->emplace_back(dstParent, liveItemPtr->clone());
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }

        case TreeItemType::RESULT:
        {
            auto processItemPtr = itemSrcPtr->getNode<std::shared_ptr<CResultItem>>();
            m_pNewModel->emplace_back(dstParent, processItemPtr->clone());
            newItemIndex = m_pNewModel->index(m_pNewModel->rowCount(dstParent) - 1, 0, dstParent);
            break;
        }
    }
    return newItemIndex;
}
