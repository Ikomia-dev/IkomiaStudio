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

#include "CHtmlDelegate.h"
#include <QTextDocument>
#include <QTextLine>
#include <QTextBlock>

int CHtmlDelegate::m_padding = 5;

void CHtmlDelegate::setMutex(std::mutex* pMutex)
{
    m_pMutex = pMutex;
}

void CHtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid())
            return;

    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    QTextOption textOption(doc.defaultTextOption());
    textOption.setWrapMode(QTextOption::WordWrap);
    doc.setDefaultTextOption(textOption);
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    painter->translate(options.rect.left(), options.rect.top());

    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

QSize CHtmlDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid())
        return QSize();

    const QAbstractItemModel* model = index.model();
    assert(model != nullptr);
    QString text = model->data(index, Qt::DisplayRole).toString();

    QTextDocument doc;
    QTextOption textOption(doc.defaultTextOption());
    textOption.setWrapMode(QTextOption::WordWrap);
    doc.setDefaultTextOption(textOption);
    doc.setHtml(text);
    doc.setTextWidth(option.rect.width());

    int h = doc.documentLayout()->documentSize().height();

    QSize size(option.rect.width(), h);

    return size;
}
