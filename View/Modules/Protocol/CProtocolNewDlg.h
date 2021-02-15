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

#ifndef CPROTOCOLNEWDLG_H
#define CPROTOCOLNEWDLG_H

#include "View/Common/CDialog.h"
#include "Main/forwards.hpp"

class CProtocolNewDlg : public CDialog
{
    public:

        CProtocolNewDlg(const QString &name, const QStringList& names, QWidget * parent = 0, Qt::WindowFlags f = 0);

        QString         getName() const;
        QString         getKeywords() const;
        QString         getDescription() const;

    private:

        void            initLayout();
        void            initConnections();

        void            validate();

    private:

        QLineEdit*      m_pEditName = nullptr;
        QPlainTextEdit* m_pEditKeywords = nullptr;
        QPlainTextEdit* m_pEditDescription = nullptr;
        QPushButton*    m_pBtnOk = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        QString         m_name;
        QString         m_keywords;
        QString         m_description;
        QStringList     m_protocolNames;
};

#endif // CPROTOCOLNEWDLG_H
