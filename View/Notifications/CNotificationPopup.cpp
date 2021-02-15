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

#include "CNotificationPopup.h"
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QPushButton>
#include "View/Common/CRollupBtn.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QTimer>

CNotificationPopup::CNotificationPopup(QWidget* pParent, Qt::WindowFlags f) :
    QDialog(pParent, f | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    initLayout();
    initConnections();
}

CNotificationPopup::~CNotificationPopup()
{
}

void CNotificationPopup::initLayout()
{
    setFixedWidth(200);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // Title
    m_pTitle = new CLabelBtn;
    m_pTitle->setProperty("class", "CNotifStyle");

    // Description message
    m_pDescription = new QLabel;
    m_pDescription->setStyleSheet("font-size: 8pt; background: transparent; border: transparent;");
    m_pDescription->setAlignment(Qt::AlignCenter);
    m_pDescription->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_pDescription->setWordWrap(true);

    // Close button
    m_pCloseBtn = new QToolButton;
    m_pCloseBtn->setIcon(QIcon(":/Images/close.png"));
    m_pCloseBtn->setProperty("class","CBtnNotifStyle");

    // Header Title bar (title + button close)
    QFrame* pTitleBar = new QFrame;
    pTitleBar->setProperty("class", "CHeaderNotifStyle");
    QHBoxLayout* pHBox = new QHBoxLayout;
    pHBox->setContentsMargins(0,0,0,0);
    pHBox->setSpacing(0);
    pHBox->addWidget(m_pTitle);
    pHBox->addWidget(m_pCloseBtn);
    pTitleBar->setLayout(pHBox);

    // Body layout
    QFrame* pBody = new QFrame;
    pBody->setStyleSheet("background: transparent;");
    m_pBodyLayout = new QGridLayout;
    m_pBodyLayout->setContentsMargins(5,5,5,5);
    m_pBodyLayout->setSpacing(5);
    pBody->setLayout(m_pBodyLayout);

    // Main layout
    QFrame* pFrame = new QFrame;
    pFrame->setProperty("class", "CNotifStyle");
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pLayout->addWidget(pTitleBar);
    pLayout->addWidget(pBody);
    pFrame->setLayout(pLayout);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0,0,0,0);
    pMainLayout->addWidget(pFrame);
    setLayout(pMainLayout);
}

void CNotificationPopup::initConnections()
{
    connect(m_pCloseBtn, &QPushButton::clicked, this, &CNotificationPopup::onClosePopup);
}

void CNotificationPopup::showPopup(const QString& title, const QString description, const QIcon& icon)
{
    m_pTitle->setText(title);
    m_pTitle->setIcon(icon);
    m_pDescription->setText(description);

    if(!m_bDescription)
    {
        int cols = m_pBodyLayout->columnCount();
        m_pBodyLayout->addWidget(m_pDescription, 0, cols);
    }

    adjustSize();
    auto pMainView = parentWidget();
    auto parentRect = pMainView->geometry();
    parentRect.setTop(parentRect.top() + (int)m_padding);

    setGeometry(QStyle::alignedRect(Qt::RightToLeft, Qt::AlignTop, size(), parentRect));
    //updatePos();

    QPropertyAnimation* pAnim = new QPropertyAnimation(this, "windowOpacity");
    pAnim->setDuration(200);
    pAnim->setStartValue(0);
    pAnim->setEndValue(1);
    pAnim->setEasingCurve(QEasingCurve::InBack);
    pAnim->start(QPropertyAnimation::DeleteWhenStopped);

    connect(pAnim, &QPropertyAnimation::finished, this, &CNotificationPopup::show);

    QTimer::singleShot(0, pAnim, SLOT(start()));
}

void CNotificationPopup::addWidget(QWidget* pWidget, int x, int y)
{
    if(pWidget == nullptr)
        return;

    m_pBodyLayout->addWidget(pWidget, x, y);
}

void CNotificationPopup::removeWidget(QWidget* pWidget)
{
    if(pWidget == nullptr)
        return;

    m_pBodyLayout->removeWidget(pWidget);
}

void CNotificationPopup::setDescriptionPos(int x, int y)
{
    m_pBodyLayout->addWidget(m_pDescription, x, y);
    m_bDescription = true;
}

void CNotificationPopup::setPadding(size_t padding)
{
    m_padding = padding;
}

void CNotificationPopup::setDescription(const QString& text)
{
    m_pDescription->setText(text);
}

void CNotificationPopup::updatePos()
{
    //Not functionnal and not tested...
    QRect screen = QApplication::desktop()->availableGeometry(this);
    auto size = this->size();
    auto right = this->x();
    auto top = this->y() - size.height();
    auto globalTopRight = QPoint(right, top);

    if(globalTopRight.y() + size.height() > screen.bottom())
        globalTopRight.setY(screen.bottom() - size.height());
    else if(globalTopRight.y() < 0)
        globalTopRight.setY(0);

    if(globalTopRight.x() + size.width() > screen.right())
        globalTopRight.setX(globalTopRight.x() - size.width());

    this->move(globalTopRight);
}

void CNotificationPopup::mousePressEvent(QMouseEvent*)
{
    emit doClicked();
}

void CNotificationPopup::onClosePopup()
{
    disconnect(m_pCloseBtn, &QPushButton::clicked, this, &CNotificationPopup::onClosePopup);

    QPropertyAnimation* pAnim = new QPropertyAnimation(this, "windowOpacity");
    pAnim->setDuration(1000);
    pAnim->setStartValue(1);
    pAnim->setEndValue(0);
    pAnim->setEasingCurve(QEasingCurve::OutBack);
    pAnim->start(QPropertyAnimation::DeleteWhenStopped);

    connect(pAnim, &QPropertyAnimation::finished, [this]
    {
        close();
        emit doClose();
    });
}
