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

#include "CMultiImageModel.h"
#include <QIcon>
#include "Main/AppTools.hpp"


CMultiImageModel::CMultiImageModel(QObject *parent) : QAbstractListModel(parent)
{
    m_pool.resize(std::thread::hardware_concurrency());
}

void CMultiImageModel::addImage(const QString &path, const QString &overlayPath, const std::vector<ProxyGraphicsItemPtr> &graphics)
{
    ImageData imgData;
    imgData.m_path = path;
    imgData.m_filename = QString::fromStdString(Utils::File::getFileName(path.toStdString()));
    imgData.m_overlayPath = overlayPath;
    imgData.m_pGraphicsLayer = Utils::Graphics::createLayer(imgData.m_filename, graphics, m_graphicsContextPtr);
    imgData.m_pixmap = QPixmap(":/Images/image.png");
    size_t index = m_data.size();
    m_data.push_back(imgData);

    //Generate thumbnail in a thread pool
    m_pool.push([this, index](int id){
        Q_UNUSED(id);
        auto pixmap = Utils::Image::loadPixmap(this->m_data[index].m_path, QSize(150,150));
        this->m_data[index].m_pixmap = Utils::Image::createRoundedPixmap(pixmap);
    });
}

void CMultiImageModel::setGraphicsContext(const GraphicsContextPtr &contextPtr)
{
    m_graphicsContextPtr = contextPtr;
}

QString CMultiImageModel::getPath(int index) const
{
    assert(index >= 0 && index < (int)m_data.size());
    return m_data[index].m_path;
}

QString CMultiImageModel::getOverlayPath(int index) const
{
    assert(index >= 0 && index < (int)m_data.size());
    return m_data[index].m_overlayPath;
}

QString CMultiImageModel::getFileName(int index) const
{
    assert(index >= 0 && index < (int)m_data.size());
    return m_data[index].m_filename;
}

CGraphicsLayer *CMultiImageModel::getGraphicsLayer(int index) const
{
    assert(index >= 0 && index < (int)m_data.size());
    return m_data[index].m_pGraphicsLayer;
}

int CMultiImageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int)m_data.size();
}

QVariant CMultiImageModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QVariant();
    int itemIndex = index.row();

    if(itemIndex >= (int)m_data.size())
        return value;

    switch(role)
    {
        case Qt::DisplayRole:
            value = m_data[itemIndex].m_filename;
            break;

        case Qt::ToolTipRole:
            value = m_data[itemIndex].m_path;
            break;

        case Qt::DecorationRole:
            return QIcon(m_data[itemIndex].m_pixmap);
            break;
    }
    return value;
}

QVariant CMultiImageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractListModel::headerData(section, orientation, role);
}
