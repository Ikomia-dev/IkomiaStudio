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

#include "CGraphicsTextPropWidget.h"
#include <QPushButton>
#include <QSpinBox>
#include "Graphics/CGraphicsText.h"
#include "Main/CoreTools.hpp"

CGraphicsTextPropWidget::CGraphicsTextPropWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

void CGraphicsTextPropWidget::setProperties(CGraphicsTextProperty *pProp)
{
    m_pProperty = pProp;
    updateUI();
}

void CGraphicsTextPropWidget::propertyChanged()
{
    updateUI();
}

void CGraphicsTextPropWidget::initLayout()
{
    QLabel* pLabelColor = new QLabel(tr("Color"));
    m_pColorBtn = new CColorPushButton(tr("Color"), QColor(255,0,0));

    QLabel* pLabelFont = new QLabel(tr("Font"));
    m_pComboFont = new QFontComboBox();

    QLabel* pLabelFontSize = new QLabel(tr("Font size"));
    m_pSpinFontSize = new QSpinBox();
    m_pSpinFontSize->setRange(6, 99);
    m_pSpinFontSize->setValue(12);

    m_pBoldBtn = createToolButton(tr("Bold"), QIcon(":Images/format-text-bold"), true);
    m_pItalicBtn = createToolButton(tr("Italic"), QIcon(":Images/format-text-italic"), true);
    m_pUnderlineBtn = createToolButton(tr("Underline"), QIcon(":Images/format-text-underline"), true);
    m_pStrikeOutBtn = createToolButton(tr("Strike out"), QIcon(":Images/format-text-strikethrough"), true);

    QHBoxLayout* pStyleLayout = new QHBoxLayout;
    pStyleLayout->addWidget(m_pBoldBtn);
    pStyleLayout->addWidget(m_pItalicBtn);
    pStyleLayout->addWidget(m_pUnderlineBtn);
    pStyleLayout->addWidget(m_pStrikeOutBtn);

    QWidget* pEmpty = new QWidget();
    pEmpty->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QGridLayout* pLayout = new QGridLayout();
    pLayout->addWidget(pLabelColor, 0, 0);
    pLayout->addWidget(m_pColorBtn, 0, 1);
    pLayout->addWidget(pLabelFont, 1, 0);
    pLayout->addWidget(m_pComboFont, 1, 1);
    pLayout->addWidget(pLabelFontSize, 2, 0);
    pLayout->addWidget(m_pSpinFontSize, 2, 1);
    pLayout->addLayout(pStyleLayout, 3, 0, 1, 2);
    pLayout->addWidget(pEmpty, 5, 0);

    setLayout(pLayout);
}

void CGraphicsTextPropWidget::initConnections()
{
    connect(m_pColorBtn, &CColorPushButton::colorChanged, [&](const QColor& color){ m_pProperty->m_color = Utils::Graphics::toCColor(color); });
    connect(m_pComboFont, &QFontComboBox::currentFontChanged, [&](const QFont &font){ m_pProperty->m_fontName = font.family().toStdString(); });
    connect(m_pSpinFontSize, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value){ m_pProperty->m_fontSize = value; });
    connect(m_pBoldBtn, &QPushButton::toggled, [&](bool checked){ m_pProperty->m_bBold = checked; });
    connect(m_pItalicBtn, &QPushButton::toggled, [&](bool checked){ m_pProperty->m_bItalic = checked; });
    connect(m_pUnderlineBtn, &QPushButton::toggled, [&](bool checked){ m_pProperty->m_bUnderline = checked; });
    connect(m_pStrikeOutBtn, &QPushButton::toggled, [&](bool checked){ m_pProperty->m_bStrikeOut = checked; });
}

QPushButton *CGraphicsTextPropWidget::createToolButton(const QString& title, const QIcon& icon, bool bCheckable)
{
    auto pal = qApp->palette();
    auto hoverBorderColor = pal.light().color();
    auto highLightColor = pal.highlight().color();
    QColor checkedColor(highLightColor.red(), highLightColor.green(), highLightColor.blue(), 128);

    auto pBtn = new QPushButton(icon, "");
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none; } QPushButton:hover {border: 1px solid %1;} QPushButton:checked { background-color: rgba(%2,%3,%4,%5); }")
                        .arg(hoverBorderColor.name())
                        .arg(checkedColor.red()).arg(checkedColor.green()).arg(checkedColor.blue()).arg(checkedColor.alpha()));
    pBtn->setFlat(true);
    pBtn->setToolTip(title);
    pBtn->setCheckable(bCheckable);
    return pBtn;
}

void CGraphicsTextPropWidget::updateUI()
{
    if(m_pProperty)
    {
        m_pColorBtn->setColor(Utils::Graphics::toQColor(m_pProperty->m_color));

        QFont font;
        font.setFamily(QString::fromStdString(m_pProperty->m_fontName));
        m_pComboFont->setFont(font);

        m_pSpinFontSize->setValue(m_pProperty->m_fontSize);
        m_pBoldBtn->setChecked(m_pProperty->m_bBold);
        m_pItalicBtn->setChecked(m_pProperty->m_bItalic);
        m_pUnderlineBtn->setChecked(m_pProperty->m_bUnderline);
        m_pStrikeOutBtn->setChecked(m_pProperty->m_bStrikeOut);
    }
}


