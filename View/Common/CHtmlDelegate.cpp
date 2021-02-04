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
