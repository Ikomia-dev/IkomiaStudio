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

#include "CWizardStepListViewDelegate.h"
#include "Model/Wizard/CWizardStepModel.h"
#include <QPainter>

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

CWizardStepListViewDelegate::CWizardStepListViewDelegate(QObject* parent) : QStyledItemDelegate(parent)
{

}

void CWizardStepListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    paintBorders(painter, option, index);
    paintInfo(painter, option, index);
    //QStyledItemDelegate::paint(painter, option, index);
}

QSize CWizardStepListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return m_size;
}

void CWizardStepListViewDelegate::paintBorders(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath pathCircle;
    QRect rcCircle(option.rect.topLeft(), QSize(option.rect.width()/3, option.rect.height()));
    int h = option.rect.height();

    // Create main rounded rectangle
    pathCircle.addEllipse(rcCircle.center(), h/2-2, h/2-2);

    // Contour color
    QBrush brushHighlight = option.palette.highlight();
    QColor colorHighlight = brushHighlight.color();
    QBrush brushBase = option.palette.toolTipBase();
    QColor colorBase = brushBase.color();
    QBrush brushText = option.palette.text();
    QColor colorText = brushText.color();

    // Draw connection rect
    painter->setPen(QPen(colorBase, 2));

    auto pModel = static_cast<const CWizardStepModel*>(index.model());
    QStandardItem* pItem = pModel->itemFromIndex(index);
    if(pItem->type() != QStandardItem::UserType)
    {
        QPainterPath pathRect;
        pathRect.addRect(QRect(rcCircle.center(), QSize(2,h)));
        painter->drawPath(pathRect);
    }

    // No pen while drawing background
    painter->setPen(Qt::NoPen);
    painter->setBrush(brushBase);
    painter->drawPath(pathCircle);

    // Draw contour according to app color theme
    if(pItem->checkState() == Qt::Checked)
        painter->setPen(QPen(colorHighlight, 2));
    else
        painter->setPen(QPen(colorText, 2));

    // Manage drawing external contour
    painter->setBrush(Qt::transparent);
    painter->drawPath(pathCircle);

    painter->restore();
}

void CWizardStepListViewDelegate::paintInfo(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    assert(index.isValid());

    auto name = index.data().toString();
    auto row = index.row();

    QRect rcCircle(option.rect.topLeft(), QSize(option.rect.width()/3, option.rect.height()));
    int h = option.rect.height();

    QFont fontName = qApp->font();
    fontName.setBold(true);
    fontName.setPointSize(8);
    QFontMetrics fontMetricsName(fontName);
    QRect rcStep(rcCircle.center().x() - h/2,
                 rcCircle.center().y() - h/2,
                 h,
                 h);
    painter->setFont(fontName);

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if(icon.isNull())
        painter->drawText(rcStep, Qt::AlignCenter, QString::number(row+1));
    else
        icon.paint(painter, rcStep, Qt::AlignCenter);

    QRect rcName(rcCircle.topRight(), QSize(option.rect.width()*2/3, option.rect.height()));
    auto displayName = fontMetricsName.elidedText(name, Qt::TextElideMode::ElideRight, rcName.width());
    painter->drawText(rcName, Qt::AlignCenter, displayName);
}
