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

#include "CGeneralSettingsWidget.h"
#include <QCheckBox>
#include <QVBoxLayout>

CGeneralSettingsWidget::CGeneralSettingsWidget(QWidget* parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

void CGeneralSettingsWidget::onEnableTutorialHelper(bool bEnable)
{
    // Tutorials standby
    m_pCheckTuto->setChecked(bEnable);
}

void CGeneralSettingsWidget::onEnableNativeDialog(bool bEnable)
{
    m_pCheckNative->setChecked(bEnable);
}

void CGeneralSettingsWidget::initLayout()
{
    QVBoxLayout* pVBoxLayout = new QVBoxLayout;
    // Tutorials standby
    m_pCheckTuto = new QCheckBox(tr("Enable tutorial helper"));
    pVBoxLayout->addWidget(m_pCheckTuto);

    m_pCheckNative = new QCheckBox(tr("Enable native file manager"));
    pVBoxLayout->addWidget(m_pCheckNative);

    setLayout(pVBoxLayout);
}

void CGeneralSettingsWidget::initConnections()
{
    // Tutorials standby
    connect(m_pCheckTuto, &QCheckBox::toggled, [&](bool bEnable){ emit doEnableTutorialHelper(bEnable); });
    connect(m_pCheckNative, &QCheckBox::toggled, [&](bool bEnable){ emit doEnableNativeDialog(bEnable); });
}
