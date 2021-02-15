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

#ifndef CLIVESTREAMITEM_HPP
#define CLIVESTREAMITEM_HPP

#include "Model/Data/Image/CImageItem.hpp"

class CLiveStreamItem : public CImageItem
{
    public:

        using CImageItem::CImageItem;

        CLiveStreamItem(const std::string& name, const std::string& fullpath) : CImageItem(name, fullpath)
        {
        }
        CLiveStreamItem(const CLiveStreamItem& other) : CImageItem(other)
        {
        }
        CLiveStreamItem(CLiveStreamItem&& other) noexcept : CImageItem(other)
        {
        }

        ~CLiveStreamItem()
        {
            if(m_pScene)
                m_pScene->deleteLater();
        }

        CLiveStreamItem&    operator=(CLiveStreamItem other) noexcept
        {
            CImageItem::operator =(other);
            return *this;
        }

        std::shared_ptr<CLiveStreamItem> clone() const
        {
            return std::make_shared<CLiveStreamItem>(*this);
        }

        TreeItemType              getTypeId() const override
        {
            return TreeItemType::LIVE_STREAM;
        }
};

#endif // CLIVESTREAMITEM_HPP
