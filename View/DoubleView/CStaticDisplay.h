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

#ifndef CSTATICDISPLAY_H
#define CSTATICDISPLAY_H

#include "CDataDisplay.h"

class CStaticDisplay : public CDataDisplay
{
    public:

        CStaticDisplay(const QString& name, QWidget* parent = nullptr);
        CStaticDisplay(const QString& text, const QString& name, QWidget* parent = nullptr);

        void    setFont(int pointSize=12, int weight=QFont::Normal, bool bItalic=false);
        void    setBackgroundColor(QColor color);
        void    setBackground(const QString &imageUrl);

    private:

        void    initLayout(const QString& text="");

    private:

        QLabel* m_pLabel = nullptr;
        QString m_style;
        QString m_labelStyle;
};

#endif // CSTATICDISPLAY_H
