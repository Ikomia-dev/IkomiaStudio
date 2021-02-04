#ifndef CFOLDERITEM_HPP
#define CFOLDERITEM_HPP

#include "Model/CItem.hpp"

/**
 * @brief
 *
 */
class CFolderItem : public CItem
{
    public:

        using CItem::CItem;

        CFolderItem(const std::string& name) : CItem(name)
        {
        }
        CFolderItem(const std::string& name, const std::string& path) : CItem(name)
        {
            m_path = path;
        }
        CFolderItem(const CFolderItem& other) : CItem(other)
        {
            m_path = other.m_path;
        }
        CFolderItem(CFolderItem&& other) noexcept : CItem(other)
        {
            m_path = std::move(other.m_path);
        }

        CFolderItem&    operator=(CFolderItem other) noexcept
        {
            CItem::operator =(other);
            m_path = other.m_path;
            return *this;
        }

        std::shared_ptr<CFolderItem> clone() const
        {
            return std::make_shared<CFolderItem>(*this);
        }

        TreeItemType    getTypeId() const override { return TreeItemType::FOLDER; }
        std::string     getPath() const
        {
            return m_path;
        }

    protected:

        std::string m_path = "";
};

#endif // CFOLDERITEM_HPP
