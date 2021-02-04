#include "CResizeStackedWidget.h"

CResizeStackedWidget::CResizeStackedWidget(QWidget *parent) : QStackedWidget(parent)
{
}

QSize CResizeStackedWidget::sizeHint() const
{
    if(currentWidget())
        return currentWidget()->sizeHint();
    else
        return QStackedWidget::sizeHint();
}

QSize CResizeStackedWidget::minimumSizeHint() const
{
    if(currentWidget())
        return currentWidget()->minimumSizeHint();
    else
        return QStackedWidget::minimumSizeHint();
}
