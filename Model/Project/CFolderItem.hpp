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

#ifndef CFOLDERITEM_HPP
#define CFOLDERITEM_HPP

#include "Model/Common/CItem.hpp"

/**
 * @brief
 *
 */
class CFolderItem : public CItem
{
    public:

        using CItem::CItem;

        CFolderItem(const std::string& name) : CItem(name)
        {
        }
        CFolderItem(const std::string& name, const std::string& path) : CItem(name)
        {
            m_path = path;
        }
        CFolderItem(const CFolderItem& other) : CItem(other)
        {
            m_path = other.m_path;
        }
        CFolderItem(CFolderItem&& other) noexcept : CItem(other)
        {
            m_path = std::move(other.m_path);
        }

        CFolderItem&    operator=(CFolderItem other) noexcept
        {
            CItem::operator =(other);
            m_path = other.m_path;
            return *this;
        }

        std::shared_ptr<CFolderItem> clone() const
        {
            return std::make_shared<CFolderItem>(*this);
        }

        TreeItemType    getTypeId() const override { return TreeItemType::FOLDER; }
        std::string     getPath() const
        {
            return m_path;
        }

    protected:

        std::string m_path = "";
};

#endif // CFOLDERITEM_HPP
