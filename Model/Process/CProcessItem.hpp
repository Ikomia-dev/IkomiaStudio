#ifndef CPROCESSITEM_HPP
#define CPROCESSITEM_HPP

#include "../CItem.hpp"

/**
 * @brief
 *
 */
class CProcessItem : public CItem
{
    public:

        using CItem::CItem;

        CProcessItem(const CProcessItem& other) : CItem(other)
        {
        }
        CProcessItem(CProcessItem&& other) noexcept : CItem(other)
        {
        }

        CProcessItem&   operator=(CProcessItem other) noexcept
        {
            CItem::operator =(other);
            return *this;
        }

        TreeItemType    getTypeId() const override { return TreeItemType::PROCESS; }
};

#endif // CPROCESSITEM_HPP
