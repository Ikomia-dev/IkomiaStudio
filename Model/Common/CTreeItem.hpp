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

#ifndef CTREEITEM_H
#define CTREEITEM_H

#include <memory>
#include <vector>
#include <boost/variant.hpp>
#include <typeindex>
#include <QString>
#include <QHash>

//Generic visitor pattern based on lambda c++ 14
template<class Result, class Func>
struct forwarding_visitor : boost::static_visitor<Result>
{
    Func func;
    forwarding_visitor(const Func& f):func(f){}
    forwarding_visitor(Func&& f):func(std::move(f)){}

    template<class Arg>
    Result operator()(Arg && arg) const
    {
        return func(std::forward<Arg>(arg));
    }
};

template<class Result, class Func>
forwarding_visitor<Result, std::decay_t<Func> > make_forwarding_visitor(Func && func)
{
    return { std::forward<Func>(func) };
}

//***********************************************
// Class CTreeItem
// for managing treeItem without Qt
// using std and boost
//***********************************************
/**
 * @brief
 *
 */
template<class ...types>
class CTreeItem : public std::enable_shared_from_this< CTreeItem<types...> >
{
    private:

        using self = CTreeItem<types...>;
        using item_t = std::shared_ptr<self>;
        using const_item_t = std::shared_ptr<const self>;
        using weak_item_t = std::weak_ptr<self>;

    public:

        using variant = boost::variant<types...>;

        CTreeItem(weak_item_t p = weak_item_t()) : m_parent(p)
        {}
        CTreeItem(weak_item_t p, variant value ) : m_parent(p)
        {
            m_node = value;
        }

        int             getRow() const
        {
            if(m_parent.expired())
                return 0;
            return m_parent.lock()->getChildPos( self::shared_from_this());
        }
        int             getChildCount() const
        {
            return static_cast<int>(m_children.size());
        }
        item_t          getParent() const
        {
            return m_parent.lock();
        }      
        item_t          getChild(int row) const
        {
            if(row < 0 || row > (int)m_children.size())
                return item_t();

            return m_children[row];
        }
        size_t          getTypeId() const
        {
            auto visitor = make_forwarding_visitor<size_t>([](const auto& t){ return t->getTypeId(); });
            return boost::apply_visitor(visitor, m_node);
        }
        unsigned int    getId() const
        {
            auto visitor = make_forwarding_visitor<unsigned int>([](const auto& t){ return t->getId();});
            return boost::apply_visitor(visitor, m_node);
        }
        int             getDbId() const
        {
            auto visitor = make_forwarding_visitor<int>([](const auto& t){ return t->getDbId();});
            return boost::apply_visitor(visitor, m_node);
        }
        std::string     getName() const
        {
            auto visitor = make_forwarding_visitor<std::string>([](const auto& t){ return t->getName();});
            return boost::apply_visitor(visitor, m_node);
        }
        QPixmap         getIconPixmap() const
        {
            auto visitor = make_forwarding_visitor<QPixmap>([](const auto& t){ return t->getIconPixmap(); });
            return boost::apply_visitor(visitor, m_node);
        }
        item_t          getLastChild()
        {
            return m_children.back();
        }

        template<typename T>
        T               getNode()
        {
            return boost::get<T>(m_node);
        }

        variant         getNode() const
        {
            return m_node;
        }

        bool            isChecked()
        {
            auto visitor = make_forwarding_visitor<bool>([](const auto& t){ return t->isChecked();});
            return boost::apply_visitor(visitor, m_node);
        }
        bool            isHighlighted()
        {
            auto visitor = make_forwarding_visitor<bool>([](const auto& t){ return t->isHighlighted();});
            return boost::apply_visitor(visitor, m_node);
        }

        void            setName(const std::string& name)
        {
            auto visitor = make_forwarding_visitor<void>([name](const auto& t){ t->setName(name); });
            return boost::apply_visitor(visitor, m_node);
        }
        void            setChecked(bool bChecked)
        {
            auto visitor = make_forwarding_visitor<void>([bChecked](const auto& t){ t->setChecked(bChecked);});
            return boost::apply_visitor(visitor, m_node);
        }
        void            setHighlighted(bool bHighlighted)
        {
            auto visitor = make_forwarding_visitor<void>([bHighlighted](const auto& t){ t->setHighlighted(bHighlighted);});
            return boost::apply_visitor(visitor, m_node);
        }

        void            clearChildren()
        {
            for(auto&& it : m_children)
                it->clearChildren();
            m_children.clear();
        }

        void            erase(int pos)
        {
            m_children.erase(m_children.begin()+pos);
        }

        template<typename T>
        item_t          findItem(T& t)
        {
            for(auto&& it : m_children)
            {
                if(it->getId() == t->getId())
                    return it;
            }
            return nullptr;
        }

        template<typename T>
        int             findItemPos(T& t)
        {
            return getChildPos(findItem(t));
        }

        template<class T>
        void            emplace_back(T &&t)
        {
            m_children.emplace_back(std::make_shared<self>(self::shared_from_this(), std::forward<T>(t)));
        }

        template<class T>
        void            insert(T& t, size_t pos)
        {
            if(pos < m_children.size())
                m_children.insert(m_children.begin()+pos, std::make_shared<self>(self::shared_from_this(), t));
            else
                m_children.push_back(std::make_shared<self>(self::shared_from_this(), t));
        }

        void            insert(item_t item, size_t pos)
        {
            if(pos < m_children.size())
                m_children.insert(m_children.begin()+pos, item);
            else
                m_children.push_back(item);
        }

        void            move(item_t newParent)
        {
            m_parent = newParent;
        }

        item_t          findItemByName(std::string name)
        {
            for(auto&& it : m_children)
            {
                if(it->getName() == name)
                    return it;
            }
            return nullptr;
        }

        uint            hash() const
        {
            auto visitor = make_forwarding_visitor<uint>([](const auto& t){ return t->getHash();});
            return boost::apply_visitor(visitor, m_node);
        }

    private:

        int             getChildPos(const const_item_t& item) const
        {
            auto it = std::find(std::begin(m_children), std::end(m_children), item);
            if(it != m_children.end())
                return it - m_children.begin();
            return -1;
        }

    private:

        variant             m_node;
        std::vector<item_t> m_children;
        weak_item_t         m_parent;
};

#endif // CTREEITEM_H
