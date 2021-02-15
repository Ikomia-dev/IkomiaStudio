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

#ifndef CINFOPANE_H
#define CINFOPANE_H

#include <QWidget>
#include <QToolBox>
#include <Widgets/QtPropertyBrowser/qttreepropertybrowser.h>
#include <Widgets/QtPropertyBrowser/qtbuttonpropertybrowser.h>
#include <Widgets/QtPropertyBrowser/qtvariantproperty.h>

#include "Main/AppDefine.hpp"

class CInfoPane : public QWidget
{
    Q_OBJECT

    public:

        CInfoPane();

    signals:

        void    doEnableInfo(bool bEnable);

    public slots:

        void    onDisplayImageInfo(const VectorPairString &infoList);
        void    onDisplayVideoInfo(const VectorPairString& infoList);

    protected:

        void    showEvent(QShowEvent* event);
        void    hideEvent(QHideEvent* event);

    private:

        void    initLayout();

        void    fillProperties(const VectorPairString &infoList, const QString &title);
        void    fillPropertySize(QtProperty* pItem);

    private:

        QToolBox*                   m_pInfoToolBox = nullptr;
        QtTreePropertyBrowser*      m_pPropertyBrowser = nullptr;
        QtVariantPropertyManager*   m_pVariantManager = nullptr;
        VectorPairString            m_infoList;
};

#endif // CINFOPANE_H
