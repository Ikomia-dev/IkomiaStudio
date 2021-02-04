#ifndef CVIDEOITEM_HPP
#define CVIDEOITEM_HPP

#include "Model/Data/Image/CImageItem.hpp"

class CVideoItem : public CImageItem
{
    public:

        using CImageItem::CImageItem;

        CVideoItem(const std::string& name, const std::string& fullPath) : CImageItem(name, fullPath)
        {
        }
        CVideoItem(const CVideoItem& other) : CImageItem(other)
        {
        }
        CVideoItem(CVideoItem&& other) noexcept : CImageItem(other)
        {
        }

        ~CVideoItem()
        {
            if(m_pScene)
                m_pScene->deleteLater();
        }

        CVideoItem&                 operator=(CVideoItem other) noexcept
        {
            CImageItem::operator =(other);
            return *this;
        }

        std::shared_ptr<CVideoItem> clone() const
        {
            return std::make_shared<CVideoItem>(*this);
        }

        TreeItemType                getTypeId() const override
        {
            return TreeItemType::VIDEO;
        }
};

#endif // CVIDEOITEM_HPP
