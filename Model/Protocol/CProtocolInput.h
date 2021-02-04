#ifndef CPROTOCOLINPUT_H
#define CPROTOCOLINPUT_H

#include "Main/AppDefine.hpp"

class CProtocolInput
{
    public:

        CProtocolInput();
        CProtocolInput(ProtocolInputMode mode, TreeItemType type);

        ProtocolInputMode           getMode() const;
        TreeItemType                getType() const;
        size_t                      getModelIndicesCount() const;
        QModelIndex                 getModelIndex(size_t pos) const;
        std::vector<DataDimension>  getDataFilters() const;
        size_t                      getContainerIndex(size_t dataIndex) const;
        size_t                      getDataIndexInContainer(size_t containerIndex, size_t dataIndex);

        bool                        isValid() const;

        void                        setMode(ProtocolInputMode mode);
        void                        setType(TreeItemType type);
        void                        setModelIndex(const std::vector<QModelIndex>& indices);
        void                        setModelIndex(const QModelIndex& index, size_t pos);
        void                        setSize(size_t index, size_t size);

        void                        addDataFilters(DataDimension dim);

        void                        appendModelIndex(const QModelIndex& index);

        void                        clear();
        void                        clearModelIndex();

        bool                        contains(const QModelIndex& index);

    private:

        ProtocolInputMode           m_mode = ProtocolInputMode::CURRENT_DATA;
        TreeItemType                m_type = TreeItemType::NONE;
        std::vector<QModelIndex>    m_indices;
        std::vector<DataDimension>  m_dataFilters;
        std::vector<size_t>         m_sizes;
};

using CProtocolInputs = std::vector<CProtocolInput>;

#endif // CPROTOCOLINPUT_H
