#ifndef CDIMENSIONITEM_HPP
#define CDIMENSIONITEM_HPP

#include "../CItem.hpp"

class CDimensionItem : public CItem
{
    public:

        using CItem::CItem;

        CDimensionItem(DataDimension dim, const std::string& name) : CItem(name)
        {
            m_dim = dim;
        }
        CDimensionItem(const CDimensionItem& other) : CItem(other)
        {
            m_dim = other.m_dim;
        }
        CDimensionItem(CDimensionItem&& other) noexcept : CItem(other)
        {
            m_dim = std::move(other.m_dim);
        }

        CDimensionItem& operator=(CDimensionItem other) noexcept
        {
            CItem::operator =(other);
            std::swap(m_dim, other.m_dim);
            return *this;
        }

        std::shared_ptr<CDimensionItem> clone() const
        {
            return std::make_shared<CDimensionItem>(*this);
        }

        DataDimension   getDimension() const
        {
            return m_dim;
        }

        TreeItemType    getTypeId() const override { return TreeItemType::DIMENSION; }

    private:

        DataDimension   m_dim = DataDimension::NONE;
};

#endif // CDIMENSIONITEM_HPP
