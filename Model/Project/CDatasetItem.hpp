#ifndef CDATASETITEM_HPP
#define CDATASETITEM_HPP

#include "../CItem.hpp"
#include "Data/CDataset.hpp"

template <typename T>
class CDatasetItem : public CItem
{
    public:

        using CItem::CItem;

        CDatasetItem(const std::string& name, IODataType dataType) : CItem(name)
        {
            m_pDataset = std::make_shared<CDataset<T>>(dataType);
        }
        CDatasetItem(const CDatasetItem& other) : CItem(other)
        {
            m_pDataset = other.m_pDataset->clone();
        }
        CDatasetItem(CDatasetItem&& other) noexcept : CItem(other)
        {
            m_pDataset = other.m_pDataset->clone();
        }

        ~CDatasetItem()
        {
        }

        std::shared_ptr<CDatasetItem<T>>   clone() const
        {
            return std::make_shared<CDatasetItem<T>>(*this);
        }

        //Operators
        CDatasetItem&   operator=(CDatasetItem other) noexcept
        {
            CItem::operator=(other);
            m_pDataset = other.m_pDataset->clone();
            return *this;
        }

        //Getters
        auto            getDataset()
        {
            return m_pDataset;
        }

        TreeItemType    getTypeId() const override
        {
            return TreeItemType::DATASET;
        }

    private:

        std::shared_ptr<CDataset<T>>    m_pDataset = nullptr;
};

#endif // CDATASETITEM_HPP
