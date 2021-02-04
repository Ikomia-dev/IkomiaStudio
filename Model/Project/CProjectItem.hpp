#ifndef CPROJECTITEM_HPP
#define CPROJECTITEM_HPP

#include "Model/CItem.hpp"

class CProjectItem : public CItem
{
    public:

        using CItem::CItem;

        CProjectItem(const CProjectItem& other) : CItem()
        {
            m_name = other.m_name;
            m_id = other.m_id;
        }
        CProjectItem(CProjectItem&& other) noexcept
        {
            swap(*this, other);
        }

        friend void     swap(CProjectItem& first, CProjectItem& second) noexcept
        {
            using std::swap;
            swap(first.m_name, second.m_name);
            swap(first.m_id, second.m_id);
        }        

        CProjectItem&   operator=(CProjectItem other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        TreeItemType    getTypeId() const override { return TreeItemType::PROJECT; }
};

#endif // CPROJECTITEM_HPP
