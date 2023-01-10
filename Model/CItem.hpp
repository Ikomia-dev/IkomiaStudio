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

#ifndef CITEM_HPP
#define CITEM_HPP

#include <QHash>
#include <QPixmap>
#include "Main/AppDefine.hpp"

/**
 * @brief
 *
 */
class CItem
{
    public:

        //Constructors
        CItem()
        {
            m_id = _getId()++;
        }
        CItem(const std::string& name) : m_name(name)
        {
            m_id = _getId()++;
        }
        CItem(const CItem& item)
        {
            m_name = item.m_name;
            m_id = _getId()++;
            m_dbId = -1;
            m_pix = item.m_pix;
            m_bChecked = item.m_bChecked;
            m_bHighlighted = item.m_bHighlighted;
        }
        CItem(CItem&& item)
        {
            m_name = std::move(item.m_name);
            m_id = _getId()++;
            m_dbId = -1;
            m_pix = std::move(item.m_pix);
            m_bChecked = std::move(item.m_bChecked);
            m_bHighlighted = std::move(item.m_bHighlighted);
        }

        //Destructors
        virtual ~CItem() {}

        //Operators
        CItem&                  operator=(CItem item) noexcept //Copy and swap
        {
            swap(*this, item);
            return *this;
        }

        friend void             swap(CItem& first, CItem& second) noexcept
        {
            std::swap(first.m_name, second.m_name);
            std::swap(first.m_id, second.m_id);
            std::swap(first.m_dbId, second.m_dbId);
            std::swap(first.m_pix, second.m_pix);
            std::swap(first.m_bChecked, second.m_bChecked);
            std::swap(first.m_bHighlighted, second.m_bHighlighted);
        }

        //Setters
        virtual void            setName(const std::string& name) { m_name = name; }
        void                    setIconPixmap(QPixmap pix) { m_pix = pix; }
        void                    setChecked(bool bChecked)
        {
            m_bChecked = bChecked;
        }
        void                    setHighlighted(bool bHighlighted)
        {
            m_bHighlighted = bHighlighted;
        }
        void                    setDbId(int id) { m_dbId = id; }

        //Getters
        virtual TreeItemType    getTypeId() const { return TreeItemType::NONE; }
        const std::string&      getName() const { return m_name; }
        const unsigned int&     getId() const { return m_id; }
        int                     getDbId() const { return m_dbId; }
        QPixmap                 getIconPixmap() const { return m_pix; }
        virtual uint            getHash() const
        {
            const uint dataHash = qHash(QPair<uint, QString>(m_id, QString::fromStdString(m_name)));
            return qHash(QPair<size_t, uint>(static_cast<size_t>(getTypeId()), dataHash));
        }

        bool                    isChecked() const
        {
            return m_bChecked;
        }
        bool                    isHighlighted() const
        {
            return m_bHighlighted;
        }

    protected:

        static unsigned int&    _getId()
        {
            static unsigned int count = 0;
            return count;
        }

    protected:

        std::string         m_name = "";
        unsigned int        m_id = 0;
        int                 m_dbId = -1;
        QPixmap             m_pix;
        bool                m_bChecked = true;
        bool                m_bHighlighted = false;
};

#endif // CITEM_HPP
