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

#include "CWizardTutoListViewDelegate.h"
#include <QStaticText>
#include "Model/Wizard/CWizardQueryModel.h"
#include "UtilsTools.hpp"
#include <QToolTip>

CWizardTutoListViewDelegate::CWizardTutoListViewDelegate(QObject* parent) : CListViewDelegate(parent)
{
    initBrush();

    m_iconSize = QSize(32, 32);
}

void CWizardTutoListViewDelegate::initBrush()
{
    QLinearGradient headerGradient, bodyGradient, bodyGradientLight;
    headerGradient.setColorAt(0.00, QColor(120,120,120));
    headerGradient.setColorAt(0.03, QColor(80,80,80));
    headerGradient.setColorAt(0.10, QColor(32,32,32));
    headerGradient.setColorAt(1.00, QColor(24,24,24));

    int grad1 = 60;
    int grad2 = 40;
    int grad3 = 35;
    int grad4 = 28;

    bodyGradient.setColorAt(0.0, QColor(grad1,grad1,grad1));
    bodyGradient.setColorAt(0.95, QColor(grad2,grad2,grad2));
    bodyGradient.setColorAt(0.98, QColor(grad3,grad3,grad3));
    bodyGradient.setColorAt(1.0, QColor(grad4,grad4,grad4));

    grad1 *= 1.5;
    grad2 *= 1.5;
    grad3 *= 1.5;
    grad4 *= 1.5;

    bodyGradientLight.setColorAt(0.0, QColor(grad1,grad1,grad1));
    bodyGradientLight.setColorAt(0.95, QColor(grad2,grad2,grad2));
    bodyGradientLight.setColorAt(0.98, QColor(grad3,grad3,grad3));
    bodyGradientLight.setColorAt(1.0, QColor(grad4,grad4,grad4));

    m_headerGradient = headerGradient;
    m_bodyGradient = bodyGradient;
    m_bodyGradientLight = bodyGradientLight;
}

void CWizardTutoListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    paintBorders(painter, option);
    paintIcon(painter, option, index);
    paintTutoInfo(painter, option, index);
    paintActionButtons(painter, option, index);
}

QString CWizardTutoListViewDelegate::getActionIconPath(int action) const
{
    QString path;
    switch(action)
    {
        case NONE: break;
        case INFO: path = ":/Images/info-color2.png"; break;
        case PLAY: path = ":/Images/playBtn.png"; break;
    }
    return path;
}

void CWizardTutoListViewDelegate::executeAction(int action, const QModelIndex &index)
{
    switch(action)
    {
        case NONE: break;
        case INFO: emit doShowInfo(index); break;
        case PLAY: emit doPlayTuto(index); break;
    }
}

void CWizardTutoListViewDelegate::paintBorders(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path, p, headerPath;
    int w = option.rect.width();
    int h = option.rect.height();
    int headerHeight = m_iconSize.height()+m_contentMargins.top()+m_spacings.height();
    int left = option.rect.left();
    int top = option.rect.top();

    // Create main rounded rectangle
    path.addRoundedRect(left, top, w, h, w*0.1, w*0.1);

    // Create header rectangle with rounded corner on top and square corner on bottom
    p.setFillRule(Qt::WindingFill);
    p.addRoundedRect(QRect(left, top, w, headerHeight), w*0.1, w*0.1);
    p.addRect(left, top+headerHeight/2, w, headerHeight/2 + 1);
    headerPath = p.simplified();

    // Set background color gradient direction
    QLinearGradient headerGradient = m_headerGradient;
    QLinearGradient bodyGradient = m_bodyGradient;
    QLinearGradient bodyGradientLight = m_bodyGradientLight;

    headerGradient.setStart(QPointF(left+w/2, top));
    headerGradient.setFinalStop(QPointF(left+w/2, top+headerHeight));

    bodyGradient.setStart(QPointF(left+w/2, top+headerHeight));
    bodyGradient.setFinalStop(QPointF(left+w/2, top+h));

    bodyGradientLight.setStart(QPointF(left+w/2, top+headerHeight));
    bodyGradientLight.setFinalStop(QPointF(left+w/2, top+h));

    // Contour color
    QBrush brushHighlight = option.palette.highlight();
    QColor colorHighlight = brushHighlight.color();
    QBrush brushDark = option.palette.dark();
    QColor colorDark = brushDark.color();

    // No pen while drawing background
    painter->setPen(Qt::NoPen);

    // Paint body and header background color (gradient)
    if(option.state & QStyle::State_Selected)
    {
        painter->setBrush(bodyGradientLight);
        painter->drawPath(path);
        painter->setBrush(headerGradient);
        painter->drawPath(headerPath);
    }
    else
    {
        painter->setBrush(bodyGradient);
        painter->drawPath(path);
        painter->setBrush(headerGradient);
        painter->drawPath(headerPath);
    }

    // Draw contour according to app color theme
    if(option.state & QStyle::State_MouseOver)
        painter->setPen(QPen(colorHighlight, 2));
    else
        painter->setPen(QPen(colorDark, 2));

    // Manage drawing external contour
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(left, top, w, h, w*0.1, w*0.1, Qt::AbsoluteSize);

    painter->restore();
}

void CWizardTutoListViewDelegate::paintIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect iconRect(option.rect.left() + m_contentMargins.left(),
                   option.rect.top() + m_contentMargins.top(),
                   m_iconSize.width(),
                   m_iconSize.height());

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    icon.paint(painter, iconRect, Qt::AlignCenter);
}

void CWizardTutoListViewDelegate::paintTutoInfo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    assert(index.isValid());
    auto pModel = static_cast<const CWizardQueryModel*>(index.model());
    assert(pModel);

    //Plugin name
    QFont font = qApp->font();
    font.setBold(true);
    font.setPointSize(8);
    QFontMetrics fontMetricsName(font);
    QRect rcName(option.rect.left() + m_iconSize.width() + m_contentMargins.left() + m_spacings.width(),
                 option.rect.top() + m_contentMargins.top(),
                 option.rect.width() - m_iconSize.width() - m_contentMargins.left() - m_spacings.width() - m_contentMargins.right(),
                 fontMetricsName.height());

    auto name = pModel->record(index.row()).value("name").toString();
    auto displayName = fontMetricsName.elidedText(name, Qt::TextElideMode::ElideRight, rcName.width());
    QStaticText staticTextName;
    staticTextName.setTextWidth(rcName.width());
    staticTextName.setText(displayName);
    painter->setFont(font);
    painter->drawStaticText(rcName.left(), rcName.top(), staticTextName);

    //Tuto authors
    font.setPointSize(6);
    QFontMetrics fontMetricsAuthor(font);
    QRect rcAuthor(rcName.left(), rcName.bottom() + m_spacings.height(), rcName.width(), fontMetricsAuthor.height());

    auto user = pModel->record(index.row()).value("authors").toString();
    if(user.isEmpty())
        user = "Unknown authors";

    auto displayAuthor = fontMetricsName.elidedText(user, Qt::TextElideMode::ElideRight, rcAuthor.width());
    QStaticText staticTextAuthor;
    staticTextAuthor.setTextWidth(rcAuthor.width());
    staticTextAuthor.setText(displayAuthor);
    painter->setFont(font);
    painter->drawStaticText(rcAuthor.left(), rcAuthor.top(), staticTextAuthor);

    //Tuto description
    font.setBold(false);
    font.setPointSize(8);
    QRect rcDescription(option.rect.left() + m_contentMargins.left(),
                        rcAuthor.bottom() + m_spacings.height() + 10,
                        option.rect.width() - m_contentMargins.left() - m_contentMargins.right(),
                        fontMetricsAuthor.height());

    auto description = pModel->record(index.row()).value("description").toString();
    QString displayDescription = Utils::String::getElidedString(description, font, rcDescription.width(), 3);
    QStaticText staticTextDescription;
    staticTextDescription.setTextWidth(rcDescription.width());
    staticTextDescription.setText(displayDescription);
    painter->setFont(font);
    painter->drawStaticText(rcDescription.left(), rcDescription.top(), staticTextDescription);
}

void CWizardTutoListViewDelegate::paintActionButtons(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    for(int i=0; i<m_actionBtnCount; ++i)
    {
        int action = getBtnAction(i);
        paintButton(painter, getBtnRect(i, option), getActionIconPath(action), m_mapBtnHover[index] == action);
    }
}

void CWizardTutoListViewDelegate::paintButton(QPainter *painter, const QRect &rc, const QString& pixmapPath, bool bHovered) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    if(bHovered == true)
    {
        QPainterPath p;
        p.setFillRule(Qt::WindingFill);
        p.addEllipse(rc);
        painter->setPen(QPen(qApp->palette().text().color(), 2));
        painter->drawPath(p);
    }

    QPixmap pixmap = QPixmap(pixmapPath).scaled(m_btnSize.width(), m_btnSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(rc, pixmap);
    painter->restore();
}

int CWizardTutoListViewDelegate::getBtnAction(int index) const
{
    int action = NONE;
    switch(index)
    {
        case 0: action = INFO; break;
        case 1: action = PLAY; break;
    }
    return action;
}

bool CWizardTutoListViewDelegate::isBtnEnabled(const QModelIndex &itemIndex, int index) const
{
    Q_UNUSED(itemIndex);
    Q_UNUSED(index);
    return true;
}

void CWizardTutoListViewDelegate::showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const
{
    Q_UNUSED(modelIndex);
    switch(index)
    {
        case 0:
            QToolTip::showText(pos, tr("Info"));
            break;
        case 1:
            QToolTip::showText(pos, tr("Play"));
            break;
    }
}
