#ifndef CPROJECTTREEVIEW_H
#define CPROJECTTREEVIEW_H

#include <QTreeView>

class CProjectTreeView: public QTreeView
{
    Q_OBJECT

    public:

        CProjectTreeView(QWidget *parent = Q_NULLPTR);

    signals:

        void            doDeleteItem(QModelIndex& index);

    protected:

        virtual void    keyPressEvent(QKeyEvent *event) override;
        virtual void    mousePressEvent(QMouseEvent* event) override;

    private:

        void            deleteCurrentItem();
};

#endif // CPROJECTTREEVIEW_H
