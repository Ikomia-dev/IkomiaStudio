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

#include "CListViewDelegate.h"
#include "CListViewDelegate.h"
#include "Main/AppTools.hpp"
#include "Model/Project/CMultiProjectModel.h"
#include "Model/Process/CProcessManager.h"
#include <QFileSystemModel>
#include <QStaticText>
#include <QPainter>
#include <QStylePainter>

CListViewDelegate::CListViewDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
    initBrush();
}

void CListViewDelegate::setSizeHint(QSize sizeHint)
{
    m_sizeHint = sizeHint;
}

void CListViewDelegate::setIconSize(QSize size)
{
    m_iconSize = size;
}

bool CListViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model)
    if(!(option.state & QStyle::State_MouseOver))
        m_mapBtnHover[index] = NONE;

    if(event->type() == QEvent::MouseMove)
    {
        auto ev = static_cast<QMouseEvent*>(event);
        for(int i=0; i<m_actionBtnCount; ++i)
        {
            if(getBtnRect(i, option).contains(ev->pos()))
            {
                m_mapBtnHover[index] = getBtnAction(i);
                showTooltip(index, ev->globalPosition().toPoint(), i);
                break;
            }
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        auto ev = static_cast<QMouseEvent*>(event);
        for(int i=0; i<m_actionBtnCount; ++i)
        {
            if(getBtnRect(i, option).contains(ev->pos()))
            {
                executeAction(getBtnAction(i), index);
                break;
            }
        }
    }

    return false;
}

QSize CListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return m_sizeHint;
}

void CListViewDelegate::initBrush()
{
    m_headerGradient.setColorAt(0.00, QColor(150,150,150));
    m_headerGradient.setColorAt(0.03, QColor(130,130,130));
    m_headerGradient.setColorAt(0.10, QColor(80,80,80));
    m_headerGradient.setColorAt(1.00, QColor(24,24,24));

    int grad1 = 100;
    int grad2 = 85;
    int grad3 = 40;
    int grad4 = 24;

    m_bodyGradient.setColorAt(0.0, QColor(grad1,grad1,grad1));
    m_bodyGradient.setColorAt(0.93, QColor(grad2,grad2,grad2));
    m_bodyGradient.setColorAt(0.98, QColor(grad3,grad3,grad3));
    m_bodyGradient.setColorAt(1.0, QColor(grad4,grad4,grad4));

    m_bodyGradientLight = m_bodyGradient;

    /*grad1 *= 0.7;
    grad2 *= 0.7;
    grad3 *= 0.7;
    grad4 *= 0.7;

    m_bodyGradientLight.setColorAt(0.0, QColor(grad1,grad1,grad1));
    m_bodyGradientLight.setColorAt(0.90, QColor(grad2,grad2,grad2));
    m_bodyGradientLight.setColorAt(0.97, QColor(grad3,grad3,grad3));
    m_bodyGradientLight.setColorAt(1.0, QColor(grad4,grad4,grad4));*/
}

void CListViewDelegate::paintBoundingRect(QPainter* painter, const QStyleOptionViewItem& option) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path, p, p1, p2, headerPath;
    int w = option.rect.width();
    int h = option.rect.height();
    int headerHeight = m_headerHeight+m_contentMargins.top()+m_spacings.height();
    int left = option.rect.left();
    int top = option.rect.top();
    int r = m_headerHeight; // rect radius must fit header height

    // Create main rounded rectangle
    path.addRoundedRect(left, top, w, h, r, r);

    // Create header rectangle with rounded corner on top and square corner on bottom
    p1.addRoundedRect(left, top, w, h, r, r);
    p2.addRect(QRect(left, top+headerHeight, w, h-headerHeight));
    p = p1.subtracted(p2);
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
    painter->drawRoundedRect(left, top, w, h, r, r, Qt::AbsoluteSize);

    painter->restore();
}

void CListViewDelegate::paintIcon(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    QIcon icon = getDataIcon(index);
    if (!icon.isNull())
    {
        int headerHeight = m_headerHeight+m_contentMargins.top()+m_spacings.height();

        QRect iconRect(option.rect.left() + m_contentMargins.left(),
                       option.rect.top() + m_contentMargins.top() + headerHeight,
                       m_iconSize.width(),
                       option.rect.height()*0.5);

        // Get rounded icon if too large
        QIcon roundedIcon = getRoundedIcon(icon);

        //Paint icon in path item
        roundedIcon.paint(painter, iconRect, Qt::AlignCenter);
    }
}

void CListViewDelegate::paintStars(QPainter* painter, QRect& rect, const QModelIndex& index) const
{
    auto pProxy = static_cast<const CProcessTableProxyModel*>(index.model());
    auto srcIndex = pProxy->mapToSource(index);
    auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
    // Get rating
    int nbStars = pTable->record(index.row()).value(5).toInt();
    for(int i=0; i<nbStars; ++i)
    {
        painter->drawPixmap(rect, QPixmap(":/Images/star-selected.png").scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        rect.adjust(22, 0 , 22, 0);
    }
    for(int i=nbStars; i<5; ++i)
    {
        painter->drawPixmap(rect, QPixmap(":/Images/star.png").scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        rect.adjust(22, 0 , 22, 0);
    }
}

void CListViewDelegate::paintActionButtons(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    for(int i=0; i<m_actionBtnCount; ++i)
    {
        int action = getBtnAction(i);
        QRect btnRect = getBtnRect(i, option);
        QString iconPath = getActionIconPath(action);
        paintBtn(painter, btnRect, iconPath, isBtnEnabled(index, i), m_mapBtnHover[index] == action);
    }
}

void CListViewDelegate::paintBtn(QPainter* painter, const QRect& rect, const QString& pixmapPath, bool bEnabled, bool bHovered) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPixmap pixmap = QPixmap(pixmapPath).scaled(m_btnSize.width(), m_btnSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if(bEnabled == true)
    {
        if(bHovered == true)
        {
            QPainterPath p;
            p.setFillRule(Qt::WindingFill);
            p.addEllipse(rect);
            painter->setPen(QPen(qApp->palette().text().color(), 2));
            painter->drawPath(p);
        }
        painter->drawPixmap(rect, pixmap);
        painter->restore();
    }
    else
    {
        pixmap = Utils::Image::colorToMono(pixmap);
        painter->drawPixmap(rect, pixmap);
        painter->restore();
    }
}

QSize CListViewDelegate::paintStaticText(QPainter *painter, int x, int y, int w, const QString &text, QFont font, const QColor &color) const
{
    painter->save();

    QStaticText staticText;
    // Fix text width if needed
    if(w>0)
        staticText.setTextWidth(w);

    staticText.setText(text);
    painter->setFont(font);
    painter->setPen(color);
    painter->drawStaticText(x, y, staticText);

    painter->restore();

    return staticText.size().toSize();
}

QRect CListViewDelegate::getBtnRect(int index, const QStyleOptionViewItem &option) const
{
    if(index >= m_actionBtnCount)
        return QRect();

    return QRect(   option.rect.right() - m_contentMargins.right() - (m_actionBtnCount - index)*m_btnSize.width() - (m_actionBtnCount - index - 1)*m_spacings.width(),
                    option.rect.bottom() - m_contentMargins.bottom() - m_btnSize.height(),
                    m_btnSize.width(),
                    m_btnSize.height());
}

QIcon CListViewDelegate::getDataIcon(const QModelIndex& index) const
{
    QIcon ic = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    return ic;
}

QString CListViewDelegate::getDataName(const QModelIndex& index) const
{
    QString title = index.data(Qt::DisplayRole).toString();
    return title;
}

QString CListViewDelegate::getShortDescription(const QModelIndex &index) const
{
    auto pModel = static_cast<const CProcessTableProxyModel*>(index.model());
    auto srcIndex = pModel->mapToSource(index);
    auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
    // Get name from table process (id, name, description, folder_id)
    QString shortDescription = pTable->record(index.row()).value("shortDescription").toString();
    return shortDescription;
}

QString CListViewDelegate::getDescription(const QModelIndex& index) const
{
    auto pModel = static_cast<const CProcessTableProxyModel*>(index.model());
    auto srcIndex = pModel->mapToSource(index);
    auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
    // Get name from table process (id, name, description, folder_id)
    QString description = pTable->record(index.row()).value("description").toString();
    return description;
}

QIcon CListViewDelegate::getRoundedIcon(QIcon icon) const
{
    QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(128,128)));
    // creating a new transparent pixmap with equal sides
    QPixmap rounded = QPixmap(pixmap.width(), pixmap.height());
    rounded.fill(Qt::transparent);

    // creating circle clip area
    QPainterPath painterPath;
    painterPath.addRoundedRect(rounded.rect(), pixmap.width()*0.1, pixmap.height()*0.1);

    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setClipPath(painterPath);

    p.drawPixmap(0, 0, pixmap.width(), pixmap.height(), pixmap);

    return QIcon(rounded);
}

void CListViewDelegate::showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const
{
    Q_UNUSED(modelIndex)
    Q_UNUSED(index)
    Q_UNUSED(pos)
}

