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

#ifndef CDATASETLOADPOLICYDLG_H
#define CDATASETLOADPOLICYDLG_H

#include "View/Common/CDialog.h"
#include "Main/CoreDefine.hpp"
#include "Main/forwards.hpp"

class CDatasetLoadPolicyDlg : public CDialog
{
    public:

        CDatasetLoadPolicyDlg(TreeItemType srcType, QWidget * parent = 0, Qt::WindowFlags f = 0);

        DatasetLoadPolicy   getLoadPolicy();

    private:

        void                initLayout();
        void                initPolicyWidget();
        void                initDimensionWidget();
        void                initConnections();

    private slots:

        void                onToggledMulti(bool bChecked);
        void                onToggledSingle(bool bChecked);
        void                onToggledStructured(bool bChecked);
        void                onToggledSimpleList(bool bChecked);
        void                onToggledVolume(bool bChecked);
        void                onToggledPosition(bool bChecked);
        void                onToggledTime(bool bChecked);
        void                onToggledModality(bool bChecked);

    private:

        QWidget*            m_pPolicyWidget = nullptr;
        QRadioButton*       m_pRadioMulti = nullptr;
        QRadioButton*       m_pRadioSingle = nullptr;
        QRadioButton*       m_pRadioStructured = nullptr;
        QWidget*            m_pDimensionWidget = nullptr;
        QRadioButton*       m_pRadioList = nullptr;
        QRadioButton*       m_pRadioVolume = nullptr;
        QRadioButton*       m_pRadioPosition = nullptr;
        QRadioButton*       m_pRadioTime = nullptr;
        QRadioButton*       m_pRadioModality = nullptr;
        QPushButton*        m_pOkBtn = nullptr;
        QPushButton*        m_pCancelBtn = nullptr;
        Relationship        m_relationship = Relationship::MANY_TO_ONE;
        DataDimension       m_extraDim = DataDimension::NONE;
        TreeItemType        m_srcType = TreeItemType::NONE;
};

#endif // CDATASETLOADPOLICYDLG_H
