// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CProcessDocDlg.h"
#include "CProcessDocWidget.h"

CProcessDocDlg::CProcessDocDlg(QWidget *parent, int style, int width, int height, Qt::WindowFlags f)
    : CDialog(tr("Documentation"), parent, style, f)
{
    initLayout();
    resize(QSize(width, height));
}

CProcessDocDlg::~CProcessDocDlg()
{
}

void CProcessDocDlg::setCurrentUser(const CUser &user)
{
    if(m_pDocWidget)
        m_pDocWidget->setCurrentUser(user);
}

void CProcessDocDlg::setProcessInfo(const CTaskInfo &info)
{
    if(m_pDocWidget)
        m_pDocWidget->setProcessInfo(info, false);
}

CProcessDocWidget *CProcessDocDlg::getDocWidget()
{
    return m_pDocWidget;
}

void CProcessDocDlg::initLayout()
{
    m_pDocWidget = new CProcessDocWidget(CProcessDocWidget::NONE);
    auto pMainLayout = getContentLayout();
    pMainLayout->addWidget(m_pDocWidget);
}
