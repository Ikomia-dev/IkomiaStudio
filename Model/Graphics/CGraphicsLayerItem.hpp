#ifndef CGRAPHICSLAYERITEM_HPP
#define CGRAPHICSLAYERITEM_HPP

#include "Model/CItem.hpp"
#include "Graphics/CGraphicsLayer.h"
#include "Graphics/CGraphicsItem.hpp"

class CGraphicsLayerItem : public CItem
{
    public:

        using CItem::CItem;

        CGraphicsLayerItem(const std::string& name, int dbId, int childCount) : CItem(name)
        {
            m_dbId = dbId;
            m_childItemCount = childCount;
        }
        CGraphicsLayerItem(CGraphicsLayer* pLayer, const std::string& name) : CItem(name)
        {
            m_pLayer = pLayer;
        }
        CGraphicsLayerItem(const CGraphicsLayerItem& item) : CItem(item)
        {
            m_childItemCount = item.m_childItemCount;
            m_pLayer = item.m_pLayer;
        }

        ~CGraphicsLayerItem()
        {
        }

        void            setName(const std::string& name) override
        {
            m_name = name;
            if(m_pLayer)
                m_pLayer->setName(QString::fromStdString(name));
        }
        void            setLayer(CGraphicsLayer* pLayer)
        {
            m_pLayer = pLayer;
        }

        TreeItemType    getTypeId() const override
        {
            return TreeItemType::GRAPHICS_LAYER;
        }
        CGraphicsLayer* getLayer() const
        {
            return m_pLayer;
        }
        uint            getHash() const override
        {
            int nbGraphicsItem = -1;
            if(m_pLayer)
            {
                nbGraphicsItem = 0;
                auto childs = m_pLayer->childItems();

                for(int i=0; i<childs.size(); ++i)
                {
                    auto pItem = dynamic_cast<CGraphicsItem*>(childs[i]);
                    if(pItem)
                        nbGraphicsItem++;
                }
            }

            if(nbGraphicsItem == -1)
                nbGraphicsItem = m_childItemCount;

            uint dataHash = qHash(QPair<uint, QString>(m_id, QString::fromStdString(m_name)));
            dataHash = qHash(QPair<uint, int>(dataHash, nbGraphicsItem));
            return qHash(QPair<size_t, uint>(static_cast<size_t>(getTypeId()), dataHash));
        }

        bool            isLoaded() const
        {
            return m_pLayer != nullptr;
        }

        void            notifyProjectSaved(int databaseId)
        {
            m_dbId = databaseId;

            if(m_pLayer)
                m_childItemCount = m_pLayer->childItems().size();
            else
                m_childItemCount = 0;
        }

        std::shared_ptr<CGraphicsLayerItem> clone() const
        {
            return std::make_shared<CGraphicsLayerItem>(*this);
        }

    private:

        int             m_childItemCount = 0;

        //This pointer has to be seen as weak pointer, it is owned by image scene
        //Use of std::weak_ptr is not possible since Qt can't handle such type
        CGraphicsLayer* m_pLayer = nullptr;
};

using GraphicsLayerItemPtr = std::shared_ptr<CGraphicsLayerItem>;

#endif // CGRAPHICSLAYERITEM_HPP
