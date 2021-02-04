#ifndef CRESIZESTACKEDWIDGET_H
#define CRESIZESTACKEDWIDGET_H

#include <QStackedWidget>

class CResizeStackedWidget : public QStackedWidget
{
    public:

        CResizeStackedWidget(QWidget *parent = Q_NULLPTR);

    protected:

        QSize   sizeHint() const override;
        QSize   minimumSizeHint() const override;
};

#endif // CRESIZESTACKEDWIDGET_H
