/**
 * @file      CDataListViewProxyModel.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CDataListViewProxyModel
 *
 * @details   Details
 */

#include "CProjectViewProxyModel.h"
#include <QtConcurrent>
#include "Main/LogCategory.h"
#include "Main/AppTools.hpp"
#include "Model/Project/CMultiProjectModel.h"
#include "Model/Project/CProjectModel.h"
#include "Model/Project/CProjectUtils.hpp"


static void createIcon(const QPersistentModelIndex& itemIndex, const QSize& size)
{
    assert(itemIndex.isValid());
    auto pModel = static_cast<const CMultiProjectModel*>(itemIndex.model());
    assert(pModel);
    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(itemIndex.internalPointer());
    assert(pTreeItem);
    auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, itemIndex);

    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    if(pItem && pItem->getTypeId() == TreeItemType::IMAGE)
    {
        auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex);
        if(pDataset)
        {
            size_t index = CProjectUtils::getIndexInDataset(wrapIndex);
            QString path = QString::fromStdString(pDataset->at(index)->getFileName());
            QPixmap pixmap = Utils::Image::loadPixmap(path, size);

            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);

            auto pImageItem = pItem->getNode<std::shared_ptr<CImageItem>>();
            if(pImageItem)
                pImageItem->setIconPixmap(rounded);
        }
    }
}

CProjectViewProxyModel::CProjectViewProxyModel(QObject* parent) : QSortFilterProxyModel{parent}
{
    connect(this, &CProjectViewProxyModel::doCreateIcon, this, &CProjectViewProxyModel::onCreateIcon);
}

QVariant CProjectViewProxyModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DecorationRole)
    {
        auto srcIndex = mapToSource(index);
        auto pModel = static_cast<const CMultiProjectModel*>(srcIndex.model());
        assert(pModel);
        auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(srcIndex.internalPointer());
        assert(pTreeItem);
        auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, srcIndex);
        auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

        if(pItem)
        {
            auto ic = QIcon(pItem->getIconPixmap());
            if(ic.isNull())
            {
                QPersistentModelIndex pIndex{srcIndex};
                emit doCreateIcon(pIndex, QSize(150,150));
            }
            else
                return ic;
        }
    }
    return QSortFilterProxyModel::data(index, role);
}

void CProjectViewProxyModel::onCreateIcon(const QPersistentModelIndex& index, QSize size)
{
    if(!index.isValid())
        return;

    auto pModel = static_cast<const CMultiProjectModel*>(index.model());
    assert(pModel);
    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
    assert(pTreeItem);
    auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, index);
    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

    // To avoid reentrant thread, we set the imageItem default icon with the chosen default icon
    switch(pItem->getTypeId())
    {
        case TreeItemType::IMAGE:
        {
            auto pImageItem = pItem->getNode<std::shared_ptr<CImageItem>>();
            assert(pImageItem != nullptr);
            QIcon icon = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));

            if(!icon.isNull())
            {
                QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
                QPixmap pixmap =  ic.pixmap(size);
                // creating a new transparent pixmap with equal sides
                auto rounded = Utils::Image::createRoundedPixmap(pixmap);
                pImageItem->setIconPixmap(rounded);
            }
            break;
        }
        case TreeItemType::PROJECT:
        {
            auto pProjectItem = pItem->getNode<std::shared_ptr<CProjectItem>>();
            QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
            QPixmap pixmap =  ic.pixmap(size);
            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);
            pProjectItem->setIconPixmap(rounded);
            break;
        }
        case TreeItemType::FOLDER:
        {
            auto pFolderItem = pItem->getNode<std::shared_ptr<CFolderItem>>();
            QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
            QPixmap pixmap =  ic.pixmap(size);
            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);
            pFolderItem->setIconPixmap(rounded);
            break;
        }
        case TreeItemType::DATASET:
        {
            auto pDatasetItem = pItem->getNode<std::shared_ptr<CDatasetItem<CMat>>>();
            QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
            QPixmap pixmap =  ic.pixmap(size);
            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);
            pDatasetItem->setIconPixmap(rounded);
            break;
        }
        case TreeItemType::DIMENSION:
        {
            auto pDimensionItem = pItem->getNode<std::shared_ptr<CDimensionItem>>();
            QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
            QPixmap pixmap =  ic.pixmap(size);
            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);
            pDimensionItem->setIconPixmap(rounded);
            break;
        }
        case TreeItemType::VIDEO:
        {
            auto pDimensionItem = pItem->getNode<std::shared_ptr<CVideoItem>>();
            QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
            QPixmap pixmap =  ic.pixmap(size);
            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);
            pDimensionItem->setIconPixmap(rounded);
            break;
        }
        case TreeItemType::LIVE_STREAM:
        {
            auto pDimensionItem = pItem->getNode<std::shared_ptr<CLiveStreamItem>>();
            QIcon ic = qvariant_cast<QIcon>(wrapIndex.data(Qt::DecorationRole));
            QPixmap pixmap =  ic.pixmap(size);
            // creating a new transparent pixmap with equal sides
            auto rounded = Utils::Image::createRoundedPixmap(pixmap);
            pDimensionItem->setIconPixmap(rounded);
            break;
        }
    }
    QtConcurrent::run([this, index, size]
    //Utils::async([this, index, size]
    {
        try
        {
            createIcon(index, size);
            emit dataChanged(index, index, QVector<int>{Qt::DecorationRole});
        }
        catch(std::exception& e)
        {
            //Nothing has to be done if icon is not created
            qCWarning(logProject).noquote() << QString::fromStdString(e.what());
        }
    });
}

#include "moc_CProjectViewProxyModel.cpp"
