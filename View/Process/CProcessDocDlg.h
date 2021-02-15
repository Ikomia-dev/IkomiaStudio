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

#ifndef CPROCESSDOCDLG_H
#define CPROCESSDOCDLG_H

#include "View/Common/CDialog.h"
#include "Core/CProcessInfo.h"
#include "Model/User/CUser.h"

class CProcessDocWidget;

class CProcessDocDlg : public CDialog
{
    public:

        CProcessDocDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
        CProcessDocDlg(int style = DEFAULT_FIXED, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
        ~CProcessDocDlg();

        void                setCurrentUser(const CUser& user);
        void                setProcessInfo(const CProcessInfo& info);

        CProcessDocWidget*  getDocWidget();

    private:

        void                initLayout();

    private:

        CProcessDocWidget*   m_pDocWidget = nullptr;
};

#endif // CPROCESSDOCDLG_H
