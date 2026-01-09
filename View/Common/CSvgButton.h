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

#ifndef CSVGBUTTON_H
#define CSVGBUTTON_H

#include <QToolButton>

class QSvgWidget;

class CSvgButton : public QToolButton
{
    public:
        CSvgButton(const QString& filePath, bool isCircular = false, QWidget* parent = nullptr);
        ~CSvgButton();

        void        setImage(const QString& filePath);
        void        setCircular(bool bIsCircular);

    private:
        void        initLayout();

    private:
        QSvgWidget* m_pSvgWidget = nullptr;
        QString     m_filePath = "";
        bool        m_bIsCircular = false;
};

#endif // CSVGBUTTON_H
