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

#ifndef CDATAPOSITIONVIEW_H
#define CDATAPOSITIONVIEW_H

#include <QWidget>
#include "../CDataDisplay.h"
#include "Main/forwards.hpp"


class CPositionDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        CPositionDisplay(QWidget* pParent = nullptr);

        void            setImage(CImageScene* pScene, QImage image, const QString& name);
        virtual void    setViewSpaceShared(bool bShared) override;
        virtual void    setSelected(bool bSelect) override;

        CImageDisplay*  getImageDisplay();

    protected:

        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

    private:

        CImageDisplay*  m_pView2D = nullptr;
};

#endif // CDATAPOSITIONVIEW_H
