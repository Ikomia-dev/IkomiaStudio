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

#ifndef CPROCESSITEM_HPP
#define CPROCESSITEM_HPP

#include "Model/Common/CItem.hpp"

/**
 * @brief
 *
 */
class CProcessItem : public CItem
{
    public:

        using CItem::CItem;

        CProcessItem(const CProcessItem& other) : CItem(other)
        {
        }
        CProcessItem(CProcessItem&& other) noexcept : CItem(other)
        {
        }

        CProcessItem&   operator=(CProcessItem other) noexcept
        {
            CItem::operator =(other);
            return *this;
        }

        TreeItemType    getTypeId() const override { return TreeItemType::PROCESS; }
};

#endif // CPROCESSITEM_HPP
