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

#include "CProtocolInput.h"

CProtocolInput::CProtocolInput()
{ 
}

CProtocolInput::CProtocolInput(ProtocolInputMode mode, TreeItemType type)
{
    m_mode = mode;
    m_type = type;
}

ProtocolInputMode CProtocolInput::getMode() const
{
    return m_mode;
}

TreeItemType CProtocolInput::getType() const
{
    return m_type;
}

size_t CProtocolInput::getModelIndicesCount() const
{
    return m_indices.size();
}

QModelIndex CProtocolInput::getModelIndex(size_t pos) const
{
    if(pos < m_indices.size())
        return m_indices[pos];
    else
        return QModelIndex();
}

std::vector<DataDimension> CProtocolInput::getDataFilters() const
{
    return m_dataFilters;
}

size_t CProtocolInput::getContainerIndex(size_t dataIndex) const
{
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

size_t CProtocolInput::getDataIndexInContainer(size_t containerIndex, size_t dataIndex)
{
    if(m_sizes.size() == 0 && m_mode == CURRENT_DATA)
        return 0;

    if(containerIndex >= m_sizes.size())
        return SIZE_MAX;

    for(size_t i=0; i<containerIndex; ++i)
        dataIndex -= m_sizes[i];

    return dataIndex;
}

size_t CProtocolInput::getSize(size_t pos) const
{
    if(pos < m_sizes.size())
        return m_sizes[pos];
    else
        return 0;
}

bool CProtocolInput::isValid() const
{
    if(m_indices.empty())
        return false;

    for(size_t i=0; i<m_indices.size(); ++i)
        if(m_indices[i].isValid() == false)
            return false;

    return true;
}

void CProtocolInput::setMode(ProtocolInputMode mode)
{
    m_mode = mode;
}

void CProtocolInput::setType(TreeItemType type)
{
    m_type = type;
}

void CProtocolInput::setModelIndex(const std::vector<QModelIndex> &indices)
{
    m_indices = indices;
}

void CProtocolInput::setModelIndex(const QModelIndex &index, size_t pos)
{
    if(pos >= m_indices.size())
        m_indices.push_back(index);
    else
        m_indices[pos] = index;
}

void CProtocolInput::setSize(size_t index, size_t size)
{
    if(index < m_sizes.size())
        m_sizes[index] = size;
    else
        m_sizes.push_back(size);
}

void CProtocolInput::addDataFilters(DataDimension dim)
{
    m_dataFilters.push_back(dim);
}

void CProtocolInput::appendModelIndex(const QModelIndex &index)
{
    m_indices.push_back(index);
}

void CProtocolInput::clear()
{
    m_mode = ProtocolInputMode::CURRENT_DATA;
    m_type = TreeItemType::IMAGE;
    m_indices.clear();
}

void CProtocolInput::clearModelIndex()
{
    m_indices.clear();
}

bool CProtocolInput::contains(const QModelIndex &index)
{
    return std::find(m_indices.begin(), m_indices.end(), index) != m_indices.end();
}
