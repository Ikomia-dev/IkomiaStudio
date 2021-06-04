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

#ifndef CGRAPHICSDELETABLEBUTTON_H
#define CGRAPHICSDELETABLEBUTTON_H

#include <QObject>
#include <QGraphicsProxyWidget>

class CGraphicsDeletableButton : public QGraphicsProxyWidget
{
    Q_OBJECT

    public:

        CGraphicsDeletableButton(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());

    signals:

        void    doDelete();

    protected:

        void    hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        void    hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:

        void    showDeleteButton();
        void    hideDeleteButton();

    private:

        QGraphicsProxyWidget*   m_pProxyDeleteBtn = nullptr;
};

#endif // CGRAPHICSDELETABLEBUTTON_H
