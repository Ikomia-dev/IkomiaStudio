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

#include "CRollupBtn.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

CRollupBtn::CRollupBtn(const QIcon& icon, const QString& title, bool bExpandable, QWidget* pParent) : QFrame(pParent)
{
    m_pLabelBtn = new CLabelBtn;
    m_pLabelBtn->setProperty("class", "CRollupStyle");

    m_pLabelBtn->setIcon(icon);
    m_pLabelBtn->setText(title);
    m_pLabelBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    connect(m_pLabelBtn, &CLabelBtn::clicked, this, &CRollupBtn::onFold);

    m_pHBoxLayout = new QHBoxLayout();
    m_pHBoxLayout->setMargin(2);
    m_pHBoxLayout->addWidget(m_pLabelBtn);

    QFrame* pWidget = new QFrame;
    pWidget->setProperty("class", "CRollupStyle");
    pWidget->setLayout(m_pHBoxLayout);

    m_pVBoxLayout = new QVBoxLayout();
    m_pVBoxLayout->setContentsMargins(0,0,0,0);
    m_pVBoxLayout->setSpacing(0);
    m_pVBoxLayout->addWidget(pWidget);
    setLayout(m_pVBoxLayout);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    setExpandable(bExpandable);
}

void CRollupBtn::setExpandable(bool bExpandable)
{
    m_bExpandable = bExpandable;

    if (m_bExpandable)
    {
        if (m_pLabelIcon)
            return;

        m_pLabelIcon = new QLabel;
        m_pLabelIcon->setStyleSheet("background: transparent;");
        m_pLabelIcon->installEventFilter(this);
        m_pLabelIcon->setFixedSize(QSize(16,16));
        m_pHBoxLayout->addWidget(m_pLabelIcon);
        changeIcons();

    }
    else
    {
        if (!m_pLabelIcon)
            return;

        m_pLabelIcon->removeEventFilter(this);
        m_pLabelIcon->setParent(nullptr);
        delete m_pLabelIcon;
        m_pLabelIcon = nullptr;
        changeIcons();
    }
}

bool CRollupBtn::isExpandable()
{
    return m_bExpandable;
}

bool CRollupBtn::isFold()
{
    return m_bFold;
}

void CRollupBtn::setFold(bool bFold)
{
    m_bFold = bFold;
}

void CRollupBtn::setText(const QString& text)
{
    m_pLabelBtn->setText(text);
}

void CRollupBtn::setWidget(QWidget* pWidget)
{
    m_pFrame = pWidget;
    m_pVBoxLayout->addWidget(pWidget);
    connect(this, &CRollupBtn::doClicked, [this]{ toggleWidget(m_pFrame); });
}

void CRollupBtn::removeWidget(QWidget* pWidget)
{
    pWidget->disconnect();
    m_pVBoxLayout->removeWidget(pWidget);
}

void CRollupBtn::replaceWidget(QWidget* pOldWidget, QWidget* pNewWidget)
{
    m_pFrame = pNewWidget;
    removeWidget(pOldWidget);
    m_pVBoxLayout->insertWidget(1, pNewWidget);
}

void CRollupBtn::onFold()
{
    if (m_bExpandable)
    {
        emit doClicked();

        m_bFold = !m_bFold;
        changeIcons();
    }
}

void CRollupBtn::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Down:
        {
            QKeyEvent ke(QEvent::KeyPress, Qt::Key_Tab, 0);
            QApplication::sendEvent(this, &ke);
            return;
        }

        case Qt::Key_Up:
        {
            QKeyEvent ke(QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier);
            QApplication::sendEvent(this, &ke);
            return;
        }

        default:;
    }

    QFrame::keyPressEvent(event);
}

void CRollupBtn::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Down:
        {
            QKeyEvent ke(QEvent::KeyRelease, Qt::Key_Tab, 0);
            QApplication::sendEvent(this, &ke);
            return;
        }

        case Qt::Key_Up:
        {
            QKeyEvent ke(QEvent::KeyRelease, Qt::Key_Tab, Qt::ShiftModifier);
            QApplication::sendEvent(this, &ke);
            return;
        }

        default:;
    }

    QFrame::keyReleaseEvent(event);
}

bool CRollupBtn::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
            if (m_bExpandable)
              onFold();
            return true;

        case QEvent::Enter:
            m_bOverIcon = true;
            changeIcons();
            return true;

        case QEvent::Leave:
            m_bOverIcon = false;
            changeIcons();
            return true;

        default:;
    }

    return QFrame::eventFilter(obj, event);
}

void CRollupBtn::changeIcons()
{
    if(!m_pLabelIcon)
        return;

    if(m_bOverIcon)
    {
        if(m_bFold)
            m_pLabelIcon->setPixmap(QPixmap(":/Images/Fold.png").scaled(16,16));
        else
            m_pLabelIcon->setPixmap(QPixmap(":/Images/Unfold.png").scaled(16,16));
    }
    else
    {
        if(m_bFold)
            m_pLabelIcon->setPixmap(QPixmap(":/Images/Fold.png").scaled(16,16));
        else
            m_pLabelIcon->setPixmap(QPixmap(":/Images/Unfold.png").scaled(16,16));
    }

    m_pLabelIcon->setFixedSize(QSize(16,16));
}

void CRollupBtn::toggleWidget(QWidget *pWidget)
{
    if(pWidget)
    {
        if(pWidget->isHidden())
        {
            processShow(pWidget);
        }
        else
        {
            processHide(pWidget);
        }
    }
}

void CRollupBtn::processShow(QWidget* pWidget)
{
    assert(pWidget != nullptr);

    pWidget->show();

    QPropertyAnimation* pAnimMax = new QPropertyAnimation(pWidget, "maximumHeight");
    pAnimMax->setDuration(100);
    pAnimMax->setStartValue(0);
    pAnimMax->setEndValue(pWidget->sizeHint().height());
    pAnimMax->setEasingCurve(QEasingCurve::OutQuad);

    QPropertyAnimation* pAnimMin = new QPropertyAnimation(pWidget, "minimumHeight");
    pAnimMin->setDuration(100);
    pAnimMin->setStartValue(0);
    pAnimMin->setEndValue(pWidget->sizeHint().height());
    pAnimMin->setEasingCurve(QEasingCurve::OutQuad);

    QParallelAnimationGroup* pParallel = new QParallelAnimationGroup;
    pParallel->addAnimation(pAnimMin);
    pParallel->addAnimation(pAnimMax);
    pParallel->start(QPropertyAnimation::DeleteWhenStopped);
}

void CRollupBtn::processHide(QWidget* pWidget)
{
    assert(pWidget != nullptr);

    QPropertyAnimation* pAnimMax = new QPropertyAnimation(pWidget, "maximumHeight");
    pAnimMax->setDuration(100);
    pAnimMax->setStartValue(pWidget->height());
    pAnimMax->setEndValue(0);
    pAnimMax->setEasingCurve(QEasingCurve::OutQuad);

    QPropertyAnimation* pAnimMin = new QPropertyAnimation(pWidget, "minimumHeight");
    pAnimMin->setDuration(100);
    pAnimMin->setStartValue(pWidget->height());
    pAnimMin->setEndValue(0);
    pAnimMin->setEasingCurve(QEasingCurve::OutQuad);

    QParallelAnimationGroup* pParallel = new QParallelAnimationGroup;
    pParallel->addAnimation(pAnimMin);
    pParallel->addAnimation(pAnimMax);
    pParallel->start(QPropertyAnimation::DeleteWhenStopped);

    connect(pParallel, &QParallelAnimationGroup::finished, pWidget, &QWidget::hide);
}
