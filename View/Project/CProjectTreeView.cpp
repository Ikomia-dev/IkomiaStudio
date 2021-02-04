#include "CProjectTreeView.h"

CProjectTreeView::CProjectTreeView(QWidget *parent) : QTreeView(parent)
{
}

void CProjectTreeView::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Delete:
            deleteCurrentItem();
            break;
    }
    QTreeView::keyPressEvent(event);
}

void CProjectTreeView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
        return;

    QTreeView::mousePressEvent(event);
}

void CProjectTreeView::deleteCurrentItem()
{
    QModelIndex index = currentIndex();
    if(index.isValid())
        emit doDeleteItem(index);
}
