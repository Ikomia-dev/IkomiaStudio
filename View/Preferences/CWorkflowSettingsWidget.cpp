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

#include "CWorkflowSettingsWidget.h"
#include "Widgets/CBrowseFileWidget.h"

CWorkflowSettingsWidget::CWorkflowSettingsWidget(QWidget *parent): QWidget(parent)
{
    initLayout();
    initConnections();
}

void CWorkflowSettingsWidget::onSetSaveFolder(const QString &path)
{
    m_pBrowseWidget->setPath(path);
}

void CWorkflowSettingsWidget::initLayout()
{
    auto pLabel = new QLabel(tr("Auto-save folder"));

    m_pBrowseWidget = new CBrowseFileWidget();
    m_pBrowseWidget->setMode(QFileDialog::FileMode::Directory);

    auto pLayout = new QGridLayout;
    pLayout->addWidget(pLabel, 0, 0);
    pLayout->addWidget(m_pBrowseWidget, 0, 1);
    setLayout(pLayout);
}

void CWorkflowSettingsWidget::initConnections()
{
    connect(m_pBrowseWidget, &CBrowseFileWidget::selectedFileChanged, [&](const QString& path){ emit doSetSaveFolder(path);});
}
