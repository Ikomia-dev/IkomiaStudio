#ifndef CLIVESTREAMITEM_HPP
#define CLIVESTREAMITEM_HPP

#include "Model/Data/Image/CImageItem.hpp"

class CLiveStreamItem : public CImageItem
{
    public:

        using CImageItem::CImageItem;

        CLiveStreamItem(const std::string& name, const std::string& fullpath) : CImageItem(name, fullpath)
        {
        }
        CLiveStreamItem(const CLiveStreamItem& other) : CImageItem(other)
        {
        }
        CLiveStreamItem(CLiveStreamItem&& other) noexcept : CImageItem(other)
        {
        }

        ~CLiveStreamItem()
        {
            if(m_pScene)
                m_pScene->deleteLater();
        }

        CLiveStreamItem&    operator=(CLiveStreamItem other) noexcept
        {
            CImageItem::operator =(other);
            return *this;
        }

        std::shared_ptr<CLiveStreamItem> clone() const
        {
            return std::make_shared<CLiveStreamItem>(*this);
        }

        TreeItemType              getTypeId() const override
        {
            return TreeItemType::LIVE_STREAM;
        }
};

#endif // CLIVESTREAMITEM_HPP
