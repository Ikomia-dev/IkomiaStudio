#include "CImageListView.h"
#include "View/DoubleView/CDataListViewDelegate.h"


CImageListView::CImageListView(QWidget *parent) : QListView(parent)
{
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setSpacing(5);
    setIconSize(QSize(200,200));
    setResizeMode(QListView::Adjust);
    setMouseTracking(true);
    setItemDelegate(new CDataListViewDelegate(this));
}

void CImageListView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    //Avoid hover visual artefact
    update();
}
