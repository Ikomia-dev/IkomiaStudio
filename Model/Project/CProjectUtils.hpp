#ifndef CPROJECTUTILS_HPP
#define CPROJECTUTILS_HPP

#include "Model/Project/CProjectModel.h"
#include "CDataIO.hpp"

class CProjectUtils
{
    public:

        static bool                             isLeafDataItem(const QModelIndex &itemIndex)
        {
            if(!itemIndex.isValid())
                return false;

            ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(itemIndex.internalPointer());
            assert(itemPtr);
            return (itemPtr->getTypeId() == TreeItemType::IMAGE ||
                    itemPtr->getTypeId() == TreeItemType::VIDEO ||
                    itemPtr->getTypeId() == TreeItemType::LIVE_STREAM);
        }

        static bool                             isSameImageDimensions(const QStringList &files)
        {
            //Check if all files contain the same data dimensions
            Dimensions refDims = _CDataIO::dimensions(files[0].toStdString());;
            for(int i=1; i<files.size(); ++i)
            {
                Dimensions dims = _CDataIO::dimensions(files[i].toStdString());
                if(dims != refDims)
                    return false;
            }
            return true;
        }

        static bool                             isAscendantItemType(const TreeItemType typeToTest, const TreeItemType typeRef)
        {
            switch(typeToTest)
            {
                case TreeItemType::NONE:
                    return false;

                case TreeItemType::PROJECT:
                    return typeRef != TreeItemType::PROJECT;

                case TreeItemType::FOLDER:
                    return typeRef != TreeItemType::PROJECT && typeRef != TreeItemType::FOLDER;

                case TreeItemType::DATASET:
                    return typeRef != TreeItemType::PROJECT && typeRef != TreeItemType::FOLDER && typeRef != TreeItemType::DATASET;

                case TreeItemType::DIMENSION:
                    return typeRef != TreeItemType::PROJECT && typeRef != TreeItemType::FOLDER &&
                            typeRef != TreeItemType::DATASET && typeRef != TreeItemType::DIMENSION;

                case TreeItemType::IMAGE:
                case TreeItemType::VIDEO:
                case TreeItemType::LIVE_STREAM:
                    return typeRef == TreeItemType::GRAPHICS_LAYER || typeRef == TreeItemType::RESULT;

                case TreeItemType::GRAPHICS_LAYER:
                case TreeItemType::RESULT:
                case TreeItemType::PROCESS:
                    return false;
            }
            return false;
        }

        static DimensionIndices                 getIndicesInDataset(const QModelIndex &itemIndex)
        {
            if(!itemIndex.isValid())
                return DimensionIndices();

            auto pDataset = getDataset<CMat>(itemIndex);
            if(!pDataset)
                return DimensionIndices();

            QModelIndex workIndex = itemIndex;
            if(!isLeafDataItem(workIndex))
            {
                //Get the first leaf item index
                auto pProjectModel = itemIndex.model();
                assert(pProjectModel);
                while(pProjectModel->rowCount(workIndex) != 0)
                    workIndex = pProjectModel->index(pProjectModel->rowCount(workIndex) - 1, 0, workIndex);
            }
            Dimensions dims = pDataset->getDataInfo().dimensions();
            assert(dims.size()>0);
            DimensionIndices indices = Utils::Data::allocateDimensionIndices(dims);
            size_t dimIndex = dims.size() - 1;

            if(!isLeafDataItem(workIndex))
            {
                auto pDimItem = getDimensionItem(workIndex);
                assert(pDimItem);
                while(dims[dimIndex].first != pDimItem->getDimension())
                {
                    Utils::Data::setDimensionIndex(indices, dimIndex, 0);
                    // Potentiel problème lorsque dimIndex = 0 => dimIndex-- devient SIZE_MAX
                    dimIndex--;
                }
            }
            ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(workIndex.internalPointer());
            assert(itemPtr);
            Utils::Data::setDimensionIndex(indices, dimIndex, itemPtr->getRow());
            dimIndex--;

            auto pParent = itemPtr->getParent();
            while(boost::apply_visitor(IsNotNullVisitor(), pParent->getNode()) && pParent->getTypeId() == TreeItemType::DIMENSION)
            {
                Utils::Data::setDimensionIndex(indices, dimIndex, pParent->getRow());
                pParent = pParent->getParent();
                dimIndex--;
            }
            return indices;
        }

        static size_t                           getIndexInDataset(const QModelIndex &itemIndex)
        {
            auto pDataset = getDataset<CMat>(itemIndex);
            if(!pDataset)
                return SIZE_MAX;

            DimensionIndices indices = getIndicesInDataset(itemIndex);
            assert(indices.size()>0);
            return pDataset->getDataInfo().index(indices);
        }

        static std::shared_ptr<CDimensionItem>  getDimensionItem(const QModelIndex &dimensionItemIndex)
        {
            if(dimensionItemIndex.isValid() == false)
                return nullptr;

            ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(dimensionItemIndex.internalPointer());
            assert(itemPtr);
            if(itemPtr->getTypeId() == TreeItemType::DIMENSION)
                return itemPtr->getNode<std::shared_ptr<CDimensionItem>>();
            else
                return nullptr;
        }

        static std::shared_ptr<CImageItem>      getImageItem(const QModelIndex& imageItemIndex)
        {
            if(!imageItemIndex.isValid())
                return nullptr;

            ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(imageItemIndex.internalPointer());
            if(itemPtr == nullptr)
                return nullptr;

            if(itemPtr->getTypeId() == TreeItemType::IMAGE)
                return itemPtr->getNode<std::shared_ptr<CImageItem>>();
            else
                return nullptr;
        }

        static std::shared_ptr<CVideoItem>      getVideoItem(const QModelIndex& videoItemIndex)
        {
            if(!videoItemIndex.isValid())
                return nullptr;

            ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(videoItemIndex.internalPointer());
            if(itemPtr == nullptr)
                return nullptr;

            if(itemPtr->getTypeId() == TreeItemType::VIDEO)
                return itemPtr->getNode<std::shared_ptr<CVideoItem>>();
            else
                return nullptr;
        }

        template<typename T>
        static std::shared_ptr<T>               getItem(const QModelIndex& index, TreeItemType type)
        {
            if(!index.isValid())
                return nullptr;

            ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(index.internalPointer());
            if(itemPtr == nullptr)
                return nullptr;

            if(itemPtr->getTypeId() == static_cast<size_t>(type))
                return itemPtr->getNode<std::shared_ptr<T>>();
            else
                return nullptr;
        }

        template<typename T>
        static std::shared_ptr<CDatasetItem<T>> getDatasetItem(const QModelIndex& datasetItemIndex)
        {
            if(datasetItemIndex.isValid() == false)
                return nullptr;

            auto itemPtr = static_cast<ProjectTreeItem*>(datasetItemIndex.internalPointer());
            if(itemPtr == nullptr)
                return nullptr;

            if(itemPtr->getTypeId() == TreeItemType::DATASET)
                return itemPtr->getNode<std::shared_ptr<CDatasetItem<T>>>();
            else
            {
                auto pParent = itemPtr->getParent();
                while(boost::apply_visitor(IsNotNullVisitor(), pParent->getNode()) && pParent->getTypeId() != TreeItemType::DATASET)
                    pParent = pParent->getParent();

                if(boost::apply_visitor(IsNotNullVisitor(), pParent->getNode()))
                    return pParent->getNode<std::shared_ptr<CDatasetItem<T>>>();
            }
            return nullptr;
        }

        template<typename T>
        static std::shared_ptr<CDataset<T>>     getDataset(const QModelIndex &itemIndex)
        {
            auto pDatasetItem = getDatasetItem<T>(itemIndex);
            if(pDatasetItem)
                return pDatasetItem->getDataset();
            else
                return nullptr;
        }

    private:

        struct IsNotNullVisitor: public boost::static_visitor<bool>
        {
            bool operator()(const std::shared_ptr<CProjectItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CFolderItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CDatasetItem<CMat>>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CDimensionItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CImageItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CGraphicsLayerItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CResultItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CVideoItem>& ptr) const { return ptr != nullptr; }
            bool operator()(const std::shared_ptr<CLiveStreamItem>& ptr) const { return ptr != nullptr; }
        };
};

#endif // CPROJECTUTILS_HPP
