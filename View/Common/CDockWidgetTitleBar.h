#ifndef CDOCKWIDGETTITLEBAR_H
#define CDOCKWIDGETTITLEBAR_H

#include <QWidget>
#include "Main/forwards.hpp"

class CDockWidgetTitleBar : public QFrame
{
    Q_OBJECT

    public:

        explicit CDockWidgetTitleBar(QWidget *parent = nullptr);

        void    updateTitle();

    public slots:

        void    onToggleMaximize();
        void    onToggleFloat();

    protected:

        void    mouseMoveEvent(QMouseEvent *event);
        void    enterEvent(QEvent* event);
        void    leaveEvent(QEvent* event);

    private:

        void    initLayout();
        void    initConnections();

    private:

        QLabel*         m_pLabel = nullptr;
        QPushButton*    m_pBtnMaximize = nullptr;
        QPushButton*    m_pBtnFloat = nullptr;
};

#endif // CDOCKWIDGETTITLEBAR_H
