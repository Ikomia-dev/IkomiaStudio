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

#ifndef CWORKFLOWINFODLG_H
#define CWORKFLOWINFODLG_H

#include "View/Common/CDialog.h"

class CWorkflowInfoDlg : public CDialog
{
    Q_OBJECT

    public:

        CWorkflowInfoDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void        setName(const QString& name);
        void        setDescription(const QString& text);

        QString     getName() const;

    signals:

        void        doLoadWorkflow();

    private:

        void        initLayout();
        void        initConnections();

        QFrame*     createLine();

    private:

        QLabel*         m_pLabel = nullptr;
        QLabel*         m_pLabelDescription = nullptr;
        QPushButton*    m_pLoadBtn = nullptr;
        const int       m_textMaxWidth = 150;
};

#endif // CWORKFLOWINFODLG_H
