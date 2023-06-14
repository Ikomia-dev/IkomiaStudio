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

#ifndef CWORKFLOWINPUTTYPEDLG_H
#define CWORKFLOWINPUTTYPEDLG_H

#include <QObject>
#include "View/Common/CDialog.h"
#include "Main/AppTools.hpp"
#include "Model/Workflow/CWorkflowManager.h"

class QTreeView;
class CProjectDataProxyModel;

class CWorkflowInputTypeDlg : public CDialog
{
    Q_OBJECT

    public:

        CWorkflowInputTypeDlg(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void                setModel(CProjectDataProxyModel* pModel);

        CWorkflowInput      getInput() const;

    public slots:

        int                 exec();


    private slots:

        void                onInputTypeSelected(QListWidgetItem *current, QListWidgetItem *previous);
        void                onBack();
        void                onValidate();

    signals:

        void                doQueryProjectDataProxyModel(const TreeItemType& inputType, const std::vector<DataDimension>& filters);

    private:

        void                initLayout();
        void                initInputTypesWidget();
        void                initInputOptionWidget();
        void                initProjectViewWidget();
        void                initConnections();

        void                showInputTypes();
        void                showInputOptions();
        void                showProjectView();

        void                validateFilters();
        void                validateProjectItems();

    private:

        QStackedWidget*         m_pStackedWidget = nullptr;
        QListWidget*            m_pInputTypesWidget = nullptr;
        QWidget*                m_pInputOptionWidget = nullptr;
        QWidget*                m_pProjectViewWidget = nullptr;
        QTreeView*              m_pTreeView = nullptr;

        QPushButton*            m_pBackBtn = nullptr;
        QPushButton*            m_pNextBtn = nullptr;
        QPushButton*            m_pCancelBtn = nullptr;
        CProjectDataProxyModel* m_pModel = nullptr;

        QCheckBox*              m_pCheckSimpleDataset = nullptr;
        QCheckBox*              m_pCheckVolumeDataset = nullptr;
        QCheckBox*              m_pCheckPositionDataset = nullptr;
        QCheckBox*              m_pCheckTimeDataset = nullptr;

        CWorkflowInput          m_input;
};

#endif // CWORKFLOWINPUTTYPEDLG_H
