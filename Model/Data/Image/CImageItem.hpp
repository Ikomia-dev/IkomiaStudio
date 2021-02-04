#ifndef CIMAGEITEM_HPP
#define CIMAGEITEM_HPP

#include "Model/Project/CDatasetItem.hpp"
#include "View/DoubleView/Image/CImageScene.h"
#include "Graphics/CGraphicsContext.h"

/**
 * @brief
 *
 */
class CImageItem : public CItem
{
    public:

        using CItem::CItem;

        CImageItem(const std::string& name, const std::string& fullPath) : CItem(name)
        {
            m_fullPath = fullPath;
        }
        CImageItem(const CImageItem& other) : CItem(other)
        {
            m_fullPath = other.m_fullPath;
            m_protocolIds = other.m_protocolIds;
        }
        CImageItem(CImageItem&& other) noexcept : CItem(other)
        {
            m_fullPath = std::move(other.m_fullPath);
            m_protocolIds = std::move(other.m_protocolIds);
        }

        virtual ~CImageItem()
        {
            if(m_pScene)
                m_pScene->deleteLater();
        }

        CImageItem&             operator=(CImageItem other) noexcept
        {
            CItem::operator =(other);
            std::swap(m_fullPath, other.m_fullPath);
            std::swap(m_protocolIds, other.m_protocolIds);
            m_pScene = new CImageScene;
            return *this;
        }

        std::shared_ptr<CImageItem> clone() const
        {
            return std::make_shared<CImageItem>(*this);
        }

        void                    setFullPath(const std::string& path)
        {
            m_fullPath = path;
        }
        void                    setGraphicsContext(GraphicsContextPtr& contextPtr)
        {
            m_pScene->setGraphicsContext(contextPtr);
        }

        std::string             getFullPath() const
        {
            return m_fullPath;
        }
        CImageScene*            getScene()
        {
            //Postpone scene creation to ensure to be in main thread
            if(m_pScene == nullptr)
                m_pScene = new CImageScene;

            return m_pScene;
        }
        virtual TreeItemType    getTypeId() const override
        {
            return TreeItemType::IMAGE;
        }
        std::vector<int>        getProtocolDbIds() const
        {
            return std::vector<int>(m_protocolIds.begin(), m_protocolIds.end());
        }

        void                    addProtocolDbId(int id)
        {
            m_protocolIds.insert(id);
        }

    protected:

        std::string     m_fullPath = "";
        std::set<int>   m_protocolIds;
        CImageScene*    m_pScene = nullptr;
};

#endif // CIMAGEITEM_HPP
