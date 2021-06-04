/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CWORKFLOWINPUT_H
#define CWORKFLOWINPUT_H

#include "Main/AppDefine.hpp"

class CWorkflowInput
{
    public:

        CWorkflowInput();
        CWorkflowInput(WorkflowInputMode mode, TreeItemType type);

        WorkflowInputMode           getMode() const;
        TreeItemType                getType() const;
        size_t                      getModelIndicesCount() const;
        QModelIndex                 getModelIndex(size_t pos) const;
        std::vector<DataDimension>  getDataFilters() const;
        size_t                      getContainerIndex(size_t dataIndex) const;
        size_t                      getDataIndexInContainer(size_t containerIndex, size_t dataIndex);
        size_t                      getSize(size_t pos) const;

        bool                        isValid() const;

        void                        setMode(WorkflowInputMode mode);
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

        WorkflowInputMode           m_mode = WorkflowInputMode::CURRENT_DATA;
        TreeItemType                m_type = TreeItemType::NONE;
        std::vector<QModelIndex>    m_indices;
        std::vector<DataDimension>  m_dataFilters;
        std::vector<size_t>         m_sizes;
};

using CWorkflowInputs = std::vector<CWorkflowInput>;

#endif // CWORKFLOWINPUT_H
