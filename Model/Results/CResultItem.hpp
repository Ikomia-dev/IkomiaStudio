#ifndef CRESULTITEM_HPP
#define CRESULTITEM_HPP

#include "Model/CItem.hpp"
#include "Process/Core/CBlobMeasure.h"

class CResultItem : public CItem
{
    public:

        using CItem::CItem;

        enum class NodeType : size_t { ROOT, PROTOCOL, TASK };

        CResultItem(const std::string& name, NodeType type) : CItem(name)
        {
            m_nodeType = type;
        }
        CResultItem(const std::string& name, int dbId, NodeType type) : CItem(name)
        {
            m_dbId = dbId;
            m_nodeType = type;
        }
        CResultItem(const CResultItem& other) : CItem(other)
        {
            m_nodeType = other.m_nodeType;
            m_bLoaded = other.m_bLoaded;
            m_measures = other.m_measures;
        }
        CResultItem(CResultItem&& other) : CItem(other)
        {
            m_nodeType = std::move(other.m_nodeType);
            m_bLoaded = std::move(other.m_bLoaded);
            m_measures = std::move(other.m_measures);
        }

        ~CResultItem()
        {
        }

        void            setNodeType(NodeType type)
        {
            m_nodeType = type;
        }
        void            setMeasures(const ObjectsMeasures& measures)
        {
            m_measures = measures;
            m_bLoaded = true;
        }

        TreeItemType    getTypeId() const override
        {
            return TreeItemType::RESULT;
        }
        uint            getHash() const override
        {
            size_t nbMeasures = 0;
            for(size_t i=0; i<m_measures.size(); ++i)
                for(size_t j=0; j<m_measures[i].size(); ++j)
                    nbMeasures++;

            uint dataHash = qHash(QPair<uint, QString>(m_id, QString::fromStdString(m_name)));
            dataHash = qHash(QPair<uint, size_t>(dataHash, nbMeasures));
            return qHash(QPair<size_t, uint>(static_cast<size_t>(getTypeId()), dataHash));
        }
        ObjectsMeasures getMeasures() const
        {
            return m_measures;
        }
        NodeType        getNodeType() const
        {
            return m_nodeType;
        }

        bool            isLoaded() const
        {
            return m_bLoaded;
        }

        void            notifyItemSaved(int dbId)
        {
            m_dbId = dbId;
        }

        std::shared_ptr<CResultItem>    clone() const
        {
            return std::make_shared<CResultItem>(*this);
        }

    private:

        NodeType        m_nodeType = NodeType::ROOT;
        bool            m_bLoaded = false;
        ObjectsMeasures m_measures;
};

using ResultItemPtr = std::shared_ptr<CResultItem>;

#endif // CRESULTITEM_HPP

