#ifndef CTREEMODEL_H
#define CTREEMODEL_H

#include <QAbstractItemModel>
#include <QApplication>
#include <QIcon>
#include <memory>
#include <boost/container/flat_map.hpp>
#include "CTreeItem.hpp"
#include "CItem.hpp"

//***********************************************
// Class CQtTreeModel
// for binding QT Model
// with generic tree using std and boost
//***********************************************
template<class ...types>
class CTreeModel : public QAbstractItemModel
{
    public:

        using TreeItem = CTreeItem<types...>;
        using ItemPtr = TreeItem*;
        using ConstItemPtr = const TreeItem*;

        explicit CTreeModel(QObject *parent = 0) : QAbstractItemModel(parent)
        {
            m_root = std::make_shared<TreeItem>();
        }

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
        {
            if(!hasIndex(row, column, parent))
                return QModelIndex();

            ItemPtr item = m_root.get();
            if(parent.isValid())
                item = static_cast<ItemPtr>(parent.internalPointer());

            auto child = item->getChild(row);
            if(child)
                return createIndex(row,column,(void*)child.get());

            return QModelIndex();
        }

        QModelIndex parent(const QModelIndex &child) const
        {
            if(!child.isValid())
                return QModelIndex();

            ItemPtr c = static_cast<ItemPtr>(child.internalPointer());
            auto p = c->getParent().get();
            if(p == m_root.get())
                return QModelIndex();

            return createIndex(p->getRow(), 0, (void*)p);
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const
        {
            if(!parent.isValid())
                return m_root->getChildCount();
            if(parent.column()>0)
                return 0;

            ItemPtr p =static_cast<ItemPtr>(parent.internalPointer());
            return p->getChildCount();
        }

        int columnCount(const QModelIndex &parent = QModelIndex()) const
        {
            Q_UNUSED(parent)
            return 1;
        }

        QVariant data(const QModelIndex &index, int role) const
        {
            QVariant value = QVariant();

            if(!index.isValid())
                return value;

            ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
            if(item)
            {
                switch(role)
                {
                    case Qt::EditRole:
                        value = QString::fromStdString(item->getName());
                        break;

                    case Qt::DisplayRole:
                        value = QString::fromStdString(item->getName());
                        break;

                    case Qt::ToolTipRole:
                        value = QString::fromStdString(item->getName());
                        break;

                    case Qt::DecorationRole:
                        if(item->getIconPixmap().isNull())
                        {
                            auto it = m_typeToIcon.find(item->getTypeId());
                            if(it != m_typeToIcon.end())
                                value = it->second;
                        }
                        else
                            value = QIcon(item->getIconPixmap());
                        break;

                    case Qt::FontRole:
                        if(item->isHighlighted())
                        {
                            QFont font;
                            font.setBold(true);
                            value = font;
                        }
                        break;

                    case Qt::ForegroundRole:
                        if(item->isHighlighted())
                        {
                            auto pal = qApp->palette();
                            value = pal.highlight();
                        }
                        break;
                }
            }
            return value;
        }

        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
        {
            bool bDone = false;

            if(!index.isValid())
                return bDone;

            ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
            if(item)
            {
                switch(role)
                {
                    case Qt::EditRole:
                        if(value.canConvert<QString>())
                        {
                            item->setName(value.toString().toStdString());
                            QVector<int> roles = {Qt::EditRole};
                            emit dataChanged(index, index, roles);
                            bDone = true;
                        }
                        break;

                    case Qt::DisplayRole:
                        break;

                    case Qt::DecorationRole:
                        break;
                }
            }
            return bDone;
        }

        void setName(const QModelIndex& index, const QString& name)
        {
            if(!index.isValid())
                return;

            ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
            if(!item)
                return;

            item->setName(name.toStdString());
        }

        std::shared_ptr<TreeItem> getRoot() const
        {
            return m_root;
        }

        template<typename T>
        QModelIndex getIndexFrom(std::shared_ptr<TreeItem> src, T&& t)
        {
            QModelIndex index;
            auto p = src->findItem(std::forward<T>(t));
            if(p == nullptr)
            {
                for(int i=0; i<src->getChildCount(); i++)
                {
                    auto child = src->getChild(i);
                    index = getIndexFrom(child, t);
                    if(index != QModelIndex())
                        return index;
                }
                return QModelIndex();
            }
            else
                return createIndex(p->getRow(), 0, (void*)p.get());
        }

        QModelIndex getIndexFrom(std::shared_ptr<TreeItem> src, std::string name)
        {
            QModelIndex index;
            auto p = src->findItemByName(name);
            if(p == nullptr)
            {
                for(int i=0; i<src->getChildCount(); i++)
                {
                    auto child = src->getChild(i);
                    index = getIndexFrom(child, name);
                    if(index != QModelIndex())
                        return index;
                }
                return QModelIndex();
            }
            else
                return createIndex(p->getRow(), 0, (void*)p.get());
        }

        template<class T>
        void emplace_back(const QModelIndex &index, T&& t)
        {
            if(!index.isValid())
                return;

            ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
            if(!item)
                return;

            // Add item at the end
            beginInsertRows(index, item->getChildCount(), item->getChildCount());
            item->emplace_back(std::forward<T>(t));
            endInsertRows();
        }

        void removeItem(QModelIndex& index)
        {
            if(!index.isValid())
                return;

            ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
            if(!item)
                return;

            // Remove all children from current item
            if(item->getChildCount()>0)
            {
                beginRemoveRows(index, 0, 0);
                item->clearChildren();
                endRemoveRows();
            }

            ItemPtr itemParent = static_cast<ItemPtr>(parent(index).internalPointer());
            if(!itemParent)
                return;

            // Remove current item from parent item
            beginRemoveRows(parent(index),item->getRow(),item->getRow());
            itemParent->erase(item->getRow());
            endRemoveRows();
        }

        void insertIcon(size_t type, QIcon icon)
        {
            m_typeToIcon[type] = icon;
        }

        template<class T>
        bool moveRow(const QModelIndex &rowIndex, const QModelIndex &targetIndex, int targetPosition)
        {
            if(!rowIndex.isValid() || !targetIndex.isValid())
                return false;

            ItemPtr pParentItem = static_cast<ItemPtr>(rowIndex.parent().internalPointer());
            if(!pParentItem)
                return false;

            ItemPtr pTargetItem = static_cast<ItemPtr>(targetIndex.internalPointer());
            if(!pTargetItem)
                return false;

            ItemPtr pItem = static_cast<ItemPtr>(rowIndex.internalPointer());
            if(!pItem)
                return false;

            int position = rowIndex.row();
            beginMoveRows(rowIndex.parent(), position, position, targetIndex, targetPosition);
                pTargetItem->insert(pItem->shared_from_this(), targetPosition);
                pItem->move(pTargetItem->shared_from_this());
                pParentItem->erase(position);
            endMoveRows();
            return true;
        }

    public:

        std::shared_ptr<TreeItem>                   m_root = nullptr;
        boost::container::flat_map<size_t, QIcon>   m_typeToIcon;
};

#endif // CTREEMODEL_H
