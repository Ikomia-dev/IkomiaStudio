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

#ifndef CPROCESSEDITDOCFRAME_H
#define CPROCESSEDITDOCFRAME_H

#include <QFrame>
#include "Task/CTaskInfo.h"
#include "Model/User/CUser.h"

class CProcessEditDocFrame : public QFrame
{
    Q_OBJECT

    public:

        CProcessEditDocFrame(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void        setCurrentUser(const CUser& user);
        void        setProcessInfo(const CTaskInfo& info);

    signals:

        void        doSave(bool bFullEdit, const CTaskInfo& info);
        void        doCancel();

    private:

        void        initLayout();

        QWidget*    createFullEditWidget();
        QWidget*    createPartialEditWidget();

        void        fillForm();

    private slots:

        void        onSave();
        void        onBrowseIconPath();

    private:

        bool            m_bFullEdit = false;
        CUser           m_currentUser;
        CTaskInfo    m_originalInfo;
        QStackedWidget* m_pStackWidget = nullptr;
        QTextEdit*      m_pTextEditDescription = nullptr;
        QTextEdit*      m_pTextEditKeywords = nullptr;
        QTextEdit*      m_pTextEditKeywords2 = nullptr;
        QTextEdit*      m_pTextEditAuthors = nullptr;
        QTextEdit*      m_pTextEditArticle = nullptr;
        QTextEdit*      m_pTextEditJournal = nullptr;
        QSpinBox*       m_pSpinYear = nullptr;
        QLineEdit*      m_pEditDocLink = nullptr;
        QLineEdit*      m_pEditVersion = nullptr;
        QLineEdit*      m_pEditIconPath = nullptr;
        QRadioButton*   m_pRadioLinux = nullptr;
        QRadioButton*   m_pRadioMac = nullptr;
        QRadioButton*   m_pRadioWin = nullptr;
};

#endif // CPROCESSEDITDOCFRAME_H
