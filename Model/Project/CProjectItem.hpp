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

#ifndef CPROJECTITEM_HPP
#define CPROJECTITEM_HPP

#include "Model/Common/CItem.hpp"

class CProjectItem : public CItem
{
    public:

        using CItem::CItem;

        CProjectItem(const CProjectItem& other) : CItem()
        {
            m_name = other.m_name;
            m_id = other.m_id;
        }
        CProjectItem(CProjectItem&& other) noexcept
        {
            swap(*this, other);
        }

        friend void     swap(CProjectItem& first, CProjectItem& second) noexcept
        {
            using std::swap;
            swap(first.m_name, second.m_name);
            swap(first.m_id, second.m_id);
        }        

        CProjectItem&   operator=(CProjectItem other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        TreeItemType    getTypeId() const override { return TreeItemType::PROJECT; }
};

#endif // CPROJECTITEM_HPP
