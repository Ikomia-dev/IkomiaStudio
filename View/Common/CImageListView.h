#ifndef CIMAGELISTVIEW_H
#define CIMAGELISTVIEW_H

#include <QListView>


class CImageListView : public QListView
{
    public:

        CImageListView(QWidget *parent = nullptr);

    protected:

        void mouseMoveEvent(QMouseEvent* event) override;
};

#endif // CIMAGELISTVIEW_H
