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

#include "CSvgButton.h"
#include <QtSvg>

CSvgButton::CSvgButton(const QString& filePath, bool isCircular, QWidget* parent) : QToolButton(parent)
{
    m_filePath = filePath;
    m_bIsCircular = isCircular;
    initLayout();
}

CSvgButton::~CSvgButton()
{

}

void CSvgButton::setImage(const QString& filePath)
{
    m_filePath = filePath;
    m_pSvgWidget->load(m_filePath);
}

void CSvgButton::setCircular(bool bIsCircular)
{
    m_bIsCircular = bIsCircular;
}

void CSvgButton::initLayout()
{
    m_pSvgWidget = new QSvgWidget(m_filePath);
    m_pSvgWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->addWidget(m_pSvgWidget);
    setLayout(pLayout);

    if(m_bIsCircular)
        this->setStyleSheet("QToolButton {"
                            "background: transparent;"
                            "border: 2px transparent red;"
                            "border-radius: 10px;"
                            "}");
}
