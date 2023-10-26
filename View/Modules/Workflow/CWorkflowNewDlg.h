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

#ifndef CWORKFLOWNEWDLG_H
#define CWORKFLOWNEWDLG_H

#include "View/Common/CDialog.h"
#include "Main/forwards.hpp"

class CWorkflowNewDlg : public CDialog
{
    public:

        CWorkflowNewDlg(const QString &name, const QString& description="", const QString& keywords="", const QStringList& names=QStringList(), QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

        QString         getName() const;
        QString         getKeywords() const;
        QString         getDescription() const;

    private slots:

        void            onDescriptionTabChanged(int index);

    private:

        void            initLayout();
        void            initConnections();

        void            validate();

    private:

        QLineEdit*      m_pEditName = nullptr;
        QTabWidget*     m_pTabDescription = nullptr;
        QTextEdit*      m_pPreviewDescription = nullptr;
        QTextEdit*      m_pEditDescription = nullptr;
        QLineEdit*      m_pEditKeywords = nullptr;
        QPushButton*    m_pBtnOk = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        QString         m_name;
        QString         m_keywords;
        QString         m_description;
        QStringList     m_workflowNames;
};

#endif // CWORKFLOWNEWDLG_H
