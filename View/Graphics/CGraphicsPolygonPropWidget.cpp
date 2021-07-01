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

#include "CGraphicsPolygonPropWidget.h"
#include "Graphics/CGraphicsPolygon.h"
#include "Main/CoreTools.hpp"

CGraphicsPolygonPropWidget::CGraphicsPolygonPropWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

void CGraphicsPolygonPropWidget::setProperties(CGraphicsPolygonProperty *pProp)
{
    m_pProperty = pProp;
    updateUI();
}

void CGraphicsPolygonPropWidget::propertyChanged()
{
    updateUI();
}

void CGraphicsPolygonPropWidget::initLayout()
{
    QLabel* pLabelPenColor = new QLabel(tr("Line color"));
    m_pPenColorBtn = new CColorPushButton(tr("Line color"), QColor(255,0,0));

    QLabel* pLabelSize = new QLabel(tr("Line size"));
    m_pSpinSize = new QSpinBox();
    m_pSpinSize->setRange(1, 5);
    m_pSpinSize->setValue(1);

    QLabel* pLabelBrushColor = new QLabel(tr("Fill color"));
    m_pBrushColorBtn = new CColorPushButton(tr("Fill color"), QColor(255,0,0,0));

    QLabel* pLabelCategory = new QLabel(tr("Category"));
    m_pEditCategory = new QLineEdit("Default");

    QLabel* pLabelOpacity = new QLabel(tr("Opacity(%)"));
    m_pSpinOpacity = new QSpinBox();
    m_pSpinOpacity->setRange(0, 100);
    m_pSpinOpacity->setValue(100);

    QWidget* pEmpty = new QWidget();
    pEmpty->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QGridLayout* pLayout = new QGridLayout();
    pLayout->addWidget(pLabelPenColor, 0, 0);
    pLayout->addWidget(m_pPenColorBtn, 0, 1);
    pLayout->addWidget(pLabelSize, 1, 0);
    pLayout->addWidget(m_pSpinSize, 1, 1);
    pLayout->addWidget(pLabelBrushColor, 2, 0);
    pLayout->addWidget(m_pBrushColorBtn, 2, 1);
    pLayout->addWidget(pLabelCategory, 3, 0);
    pLayout->addWidget(m_pEditCategory, 3, 1);
    pLayout->addWidget(pLabelOpacity, 4, 0);
    pLayout->addWidget(m_pSpinOpacity, 4, 1);
    pLayout->addWidget(pEmpty, 5, 0);

    setLayout(pLayout);
}

void CGraphicsPolygonPropWidget::initConnections()
{
    connect(m_pPenColorBtn, &CColorPushButton::colorChanged, [&](const QColor& color)
    {
        m_pProperty->m_penColor = Utils::Graphics::toCColor(color);
    });
    connect(m_pBrushColorBtn, &CColorPushButton::colorChanged, [&](const QColor& color)
    {
        m_pProperty->m_brushColor = Utils::Graphics::toCColor(color);
        m_pProperty->m_brushColor[3] = 255*m_pSpinOpacity->value()/100;
    });
    connect(m_pSpinSize, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value)
    {
        m_pProperty->m_lineSize = value;
    });
    connect(m_pEditCategory, &QLineEdit::editingFinished, [&]
    {
        m_pProperty->m_category = m_pEditCategory->text().toStdString();
    });
    connect(m_pSpinOpacity, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value)
    {
        m_pProperty->m_brushColor[3] = 255*value/100;
    });
}

void CGraphicsPolygonPropWidget::updateUI()
{
    if(m_pProperty)
    {
        m_pPenColorBtn->setColor(Utils::Graphics::toQColor(m_pProperty->m_penColor));
        m_pBrushColorBtn->setColor(Utils::Graphics::toQColor(m_pProperty->m_brushColor));
        m_pSpinSize->setValue(m_pProperty->m_lineSize);
        m_pEditCategory->setText(QString::fromStdString(m_pProperty->m_category));
        m_pSpinOpacity->setValue(m_pProperty->m_brushColor[3]*100/255);
    }
}
