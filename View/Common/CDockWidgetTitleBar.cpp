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

#include "CDockWidgetTitleBar.h"
#include <QDockWidget>
#include <QApplication>
#include <QFrame>

CDockWidgetTitleBar::CDockWidgetTitleBar(QWidget *parent) : QFrame(parent)
{
    initLayout();
    initConnections();
    setMouseTracking(true);
}

void CDockWidgetTitleBar::updateTitle()
{
    if(parentWidget())
        m_pLabel->setText(parentWidget()->windowTitle());
}

void CDockWidgetTitleBar::onToggleMaximize()
{
    QDockWidget* pDockWidget = qobject_cast<QDockWidget*>(parentWidget());
    if(pDockWidget)
    {
        if(pDockWidget->isMaximized())
            pDockWidget->showNormal();
        else
            pDockWidget->showMaximized();
    }
}

void CDockWidgetTitleBar::onToggleFloat()
{
    QDockWidget* pDockWidget = qobject_cast<QDockWidget*>(parentWidget());
    if(pDockWidget)
        pDockWidget->setFloating(!pDockWidget->isFloating());
}

void CDockWidgetTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(m_pBtnMaximize->underMouse() || m_pBtnFloat->underMouse())
        setCursor(Qt::ArrowCursor);
    else
        setCursor(Qt::SizeAllCursor);

    QFrame::mouseMoveEvent(event);
}

void CDockWidgetTitleBar::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    setCursor(Qt::SizeAllCursor);
}

void CDockWidgetTitleBar::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    setCursor(Qt::ArrowCursor);
}

void CDockWidgetTitleBar::initLayout()
{
    m_pLabel = new QLabel;
    m_pLabel->setAlignment(Qt::AlignCenter);
    updateTitle();

    auto pal = qApp->palette();
    auto color = pal.highlight().color();

    m_pBtnMaximize = new QPushButton(QIcon(":Images/view-fullscreen.png"), "");
    m_pBtnMaximize->setToolTip(tr("Maximize/Restore"));
    m_pBtnMaximize->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    m_pBtnMaximize->setFlat(true);
    m_pBtnMaximize->setFixedSize(QSize(16,16));
    m_pBtnMaximize->setIconSize(QSize(14,14));
    m_pBtnMaximize->setMouseTracking(true);

    m_pBtnFloat = new QPushButton(QIcon(":Images/skew-arrow.png"), "");
    m_pBtnFloat->setToolTip(tr("Float/Dock"));
    m_pBtnFloat->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    m_pBtnFloat->setFlat(true);
    m_pBtnFloat->setFixedSize(QSize(16,16));
    m_pBtnFloat->setIconSize(QSize(14,14));
    m_pBtnFloat->setMouseTracking(true);

    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameShape(QFrame::HLine);
    pLine->setStyleSheet(QString("color:%1;").arg(pal.window().color().name()));

    auto pHLayout = new QHBoxLayout;
    pHLayout->setSpacing(1);
    pHLayout->addWidget(m_pLabel);
    pHLayout->addWidget(m_pBtnMaximize);
    pHLayout->addWidget(m_pBtnFloat);

    auto pVLayout = new QVBoxLayout;
    // Space between splitter separator and dockwidget
    pVLayout->setContentsMargins(0,5,0,0);
    pVLayout->addWidget(pLine);
    pVLayout->addLayout(pHLayout);

    //setStyleSheet("background: rgb(40,40,40);");
    setLayout(pVLayout);
}

void CDockWidgetTitleBar::initConnections()
{
    connect(m_pBtnFloat, &QPushButton::clicked, this, &CDockWidgetTitleBar::onToggleFloat);
    connect(m_pBtnMaximize, &QPushButton::clicked, this, &CDockWidgetTitleBar::onToggleMaximize);
}

#include "moc_CDockWidgetTitleBar.cpp"
