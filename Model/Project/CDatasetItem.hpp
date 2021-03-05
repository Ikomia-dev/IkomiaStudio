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

#ifndef CDATASETITEM_HPP
#define CDATASETITEM_HPP

#include "../CItem.hpp"
#include "Data/CDataset.hpp"

template <typename T>
class CDatasetItem : public CItem
{
    public:

        using CItem::CItem;

        CDatasetItem(const std::string& name, IODataType dataType) : CItem(name)
        {
            m_pDataset = std::make_shared<CDataset<T>>(dataType, name);
        }
        CDatasetItem(const CDatasetItem& other) : CItem(other)
        {
            m_pDataset = other.m_pDataset->clone();
        }
        CDatasetItem(CDatasetItem&& other) noexcept : CItem(other)
        {
            m_pDataset = other.m_pDataset->clone();
        }

        ~CDatasetItem()
        {
        }

        std::shared_ptr<CDatasetItem<T>>   clone() const
        {
            return std::make_shared<CDatasetItem<T>>(*this);
        }

        //Operators
        CDatasetItem&   operator=(CDatasetItem other) noexcept
        {
            CItem::operator=(other);
            m_pDataset = other.m_pDataset->clone();
            return *this;
        }

        //Getters
        auto            getDataset()
        {
            return m_pDataset;
        }

        TreeItemType    getTypeId() const override
        {
            return TreeItemType::DATASET;
        }

    private:

        std::shared_ptr<CDataset<T>>    m_pDataset = nullptr;
};

#endif // CDATASETITEM_HPP
