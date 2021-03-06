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

#ifndef CMULTIIMAGESDISPLAY_H
#define CMULTIIMAGESDISPLAY_H

#include "View/DoubleView/CDataDisplay.h"
#include "View/Common/CImageListView.h"
#include "Data/CMat.hpp"

class CMultiImageModel;
class CGraphicsLayer;


class CMultiImageDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        CMultiImageDisplay(QWidget* pParent = nullptr);

        void    setModel(CMultiImageModel* pModel);

    signals:

        void    doExportImage(const QString& path, CMat& img, CGraphicsLayer* pLayer);

    private slots:

        void    onShowImage(const QModelIndex &index);
        void    onExportImage(const QString& path, bool bWithGraphics);

    protected:

        void    keyPressEvent(QKeyEvent* event) override;

    private:

        void    initLayout();
        void    initConnections();
        void    initOverlayColors();

        void    loadImage(const QModelIndex &index);
        void    loadPreviousImage();
        void    loadNextImage();

        CMat    createDisplayOverlay(const CMat& img);

    private:

        CImageDisplay*          m_pImageDisplay = nullptr;
        CImageListView*         m_pListView = nullptr;
        QStackedWidget*         m_pStackedWidget = nullptr;
        CGraphicsLayer*         m_pCurrentLayer = nullptr;
        QPersistentModelIndex   m_currentIndex;
        CMat                    m_overlayColormap;
};

#endif // CMULTIIMAGESDISPLAY_H
