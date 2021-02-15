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

#include "CProjectModel.h"

CProjectModel::CProjectModel(QObject *parent) : ProjectTreeModel(parent)
{
}

Qt::ItemFlags CProjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
    if(item)
    {
        if(item->getTypeId() == TreeItemType::IMAGE)
            return QAbstractItemModel::flags(index);
        else if(item->getTypeId() == TreeItemType::GRAPHICS_LAYER)
            return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
        else
            return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
    else
        return Qt::NoItemFlags;
}

QVariant CProjectModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
    if(item)
    {
        switch(role)
        {
            case Qt::CheckStateRole:
                if(item->getTypeId() == static_cast<size_t>(TreeItemType::GRAPHICS_LAYER))
                {
                    QVariant value;
                    item->isChecked() ? value = Qt::Checked : value = Qt::Unchecked;
                    return value;
                }
                break;
            case Qt::DecorationRole:
                if(item->getTypeId() == static_cast<size_t>(TreeItemType::DIMENSION))
                {
                    auto it = item->getNode<std::shared_ptr<CDimensionItem>>();
                    if(it->getDimension() == DataDimension::TIME)
                        it->setIconPixmap(QPixmap(":/Images/time.png"));
                    else if(it->getDimension() == DataDimension::VOLUME)
                        it->setIconPixmap(QPixmap(":/Images/volume.png"));
                    else if(it->getDimension() == DataDimension::POSITION)
                        it->setIconPixmap(QPixmap(":/Images/position.png"));
                }
                break;
        }
    }
    return CTreeModel::data(index, role);
}

bool CProjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;

    ItemPtr item = static_cast<ItemPtr>(index.internalPointer());
    if(item)
    {
        if(role == Qt::CheckStateRole)
        {
            if(item->getTypeId() == TreeItemType::GRAPHICS_LAYER)
            {
                if(value == Qt::Checked)
                    item->setChecked(true);
                else
                    item->setChecked(false);

                QVector<int> roles = {Qt::CheckStateRole};
                emit dataChanged(index, index, roles);
                return true;
            }
        }
    }
    return CTreeModel::setData(index, value, role);
}

void CProjectModel::setOriginalPath(const QString &path)
{
    m_originalPath = path;
}

void CProjectModel::setPath(const QString& path)
{
    m_path = path;
}

void CProjectModel::setExported(bool bExported)
{
    m_bExported = bExported;
}

QString CProjectModel::getPath() const
{
    return m_path;
}

QString CProjectModel::getOriginalPath() const
{
    return m_originalPath;
}

void CProjectModel::updateHash()
{
    assert(m_root);
    assert(m_root->getChild(0));

    m_projectHash = qHash(m_root->getChild(0).get());
}

bool CProjectModel::isModified() const
{
    if(m_projectHash == 0)
        return false;

    assert(m_root);
    assert(m_root->getChild(0));

    auto newHash = qHash(m_root->getChild(0).get());
    return newHash != m_projectHash;
}

bool CProjectModel::isExported() const
{
    return m_bExported;
}
