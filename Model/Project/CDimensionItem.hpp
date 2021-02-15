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

#ifndef CDIMENSIONITEM_HPP
#define CDIMENSIONITEM_HPP

#include "../CItem.hpp"

class CDimensionItem : public CItem
{
    public:

        using CItem::CItem;

        CDimensionItem(DataDimension dim, const std::string& name) : CItem(name)
        {
            m_dim = dim;
        }
        CDimensionItem(const CDimensionItem& other) : CItem(other)
        {
            m_dim = other.m_dim;
        }
        CDimensionItem(CDimensionItem&& other) noexcept : CItem(other)
        {
            m_dim = std::move(other.m_dim);
        }

        CDimensionItem& operator=(CDimensionItem other) noexcept
        {
            CItem::operator =(other);
            std::swap(m_dim, other.m_dim);
            return *this;
        }

        std::shared_ptr<CDimensionItem> clone() const
        {
            return std::make_shared<CDimensionItem>(*this);
        }

        DataDimension   getDimension() const
        {
            return m_dim;
        }

        TreeItemType    getTypeId() const override { return TreeItemType::DIMENSION; }

    private:

        DataDimension   m_dim = DataDimension::NONE;
};

#endif // CDIMENSIONITEM_HPP
