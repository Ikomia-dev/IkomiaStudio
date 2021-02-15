/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CMULTIIMAGEMODEL_H
#define CMULTIIMAGEMODEL_H

#include <QAbstractListModel>
#include "Graphics/CGraphicsLayer.h"
#include "DesignPattern/ctpl_stl.h"


class CMultiImageModel: public QAbstractListModel
{
    public:

        CMultiImageModel(QObject *parent = nullptr);

        void            addImage(const QString& path, const QString& overlayPath, const std::vector<ProxyGraphicsItemPtr>& graphics);

        void            setGraphicsContext(const GraphicsContextPtr& contextPtr);

        QString         getPath(int index) const;
        QString         getOverlayPath(int index) const;
        QString         getFileName(int index) const;
        CGraphicsLayer* getGraphicsLayer(int index) const;

    protected:

        int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant        headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    private:

        struct ImageData
        {
            QString         m_path = "";
            QString         m_filename = "";
            QString         m_overlayPath = "";
            CGraphicsLayer* m_pGraphicsLayer = nullptr;
            QPixmap         m_pixmap;
        };

        std::vector<ImageData>  m_data;
        GraphicsContextPtr      m_graphicsContextPtr = nullptr;
        ctpl::thread_pool       m_pool;
};

#endif // CMULTIIMAGEMODEL_H
