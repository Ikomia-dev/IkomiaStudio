// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CWorkflowInput.h"

CWorkflowInput::CWorkflowInput()
{ 
}

CWorkflowInput::CWorkflowInput(WorkflowInputMode mode, TreeItemType type)
{
    m_mode = mode;
    m_type = type;
}

WorkflowInputMode CWorkflowInput::getMode() const
{
    return m_mode;
}

TreeItemType CWorkflowInput::getType() const
{
    return m_type;
}

size_t CWorkflowInput::getModelIndicesCount() const
{
    return m_indices.size();
}

QModelIndex CWorkflowInput::getModelIndex(size_t pos) const
{
    if(pos < m_indices.size())
        return m_indices[pos];
    else
        return QModelIndex();
}

std::vector<DataDimension> CWorkflowInput::getDataFilters() const
{
    return m_dataFilters;
}

size_t CWorkflowInput::getContainerIndex(size_t dataIndex) const
{
    //Return first container if sizes are not already computed
    if(m_sizes.size() == 0)
        return 0;

    size_t count = 0;
    for(size_t i=0; i<m_sizes.size(); ++i)
    {
        count += m_sizes[i];
        if(dataIndex < count)
            return i;
    }

    if(m_mode == CURRENT_DATA)
        return 0;

    return SIZE_MAX;
}

size_t CWorkflowInput::getDataIndexInContainer(size_t containerIndex, size_t dataIndex)
{
    //Return first data if sizes are not already computed
    if(m_sizes.size() == 0)
        return 0;

    if(containerIndex >= m_sizes.size())
        return SIZE_MAX;

    for(size_t i=0; i<containerIndex; ++i)
        dataIndex -= m_sizes[i];

    return dataIndex;
}

size_t CWorkflowInput::getSize(size_t pos) const
{
    if(pos < m_sizes.size())
        return m_sizes[pos];
    else
        return 0;
}

bool CWorkflowInput::isValid() const
{
    if(m_indices.empty())
        return false;

    for(size_t i=0; i<m_indices.size(); ++i)
        if(m_indices[i].isValid() == false)
            return false;

    return true;
}

void CWorkflowInput::setMode(WorkflowInputMode mode)
{
    m_mode = mode;
}

void CWorkflowInput::setType(TreeItemType type)
{
    m_type = type;
}

void CWorkflowInput::setModelIndex(const std::vector<QModelIndex> &indices)
{
    m_indices = indices;
}

void CWorkflowInput::setModelIndex(const QModelIndex &index, size_t pos)
{
    if(pos >= m_indices.size())
        m_indices.push_back(index);
    else
        m_indices[pos] = index;
}

void CWorkflowInput::setSize(size_t index, size_t size)
{
    if(index < m_sizes.size())
        m_sizes[index] = size;
    else
        m_sizes.push_back(size);
}

void CWorkflowInput::addDataFilters(DataDimension dim)
{
    m_dataFilters.push_back(dim);
}

void CWorkflowInput::appendModelIndex(const QModelIndex &index)
{
    m_indices.push_back(index);
}

void CWorkflowInput::clear()
{
    m_mode = WorkflowInputMode::CURRENT_DATA;
    m_type = TreeItemType::IMAGE;
    m_indices.clear();
}

void CWorkflowInput::clearModelIndex()
{
    m_indices.clear();
}

bool CWorkflowInput::contains(const QModelIndex &index)
{
    return std::find(m_indices.begin(), m_indices.end(), index) != m_indices.end();
}
