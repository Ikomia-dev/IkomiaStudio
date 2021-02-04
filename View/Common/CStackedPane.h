#ifndef CSTACKEDPANE_H
#define CSTACKEDPANE_H

#include "Main/forwards.hpp"
#include "CPane.h"

class CStackedPane : public CPane
{
    public:

        CStackedPane(QWidget* parent = Q_NULLPTR);

        int     addPane(QWidget* pane);

        void    togglePane(QWidget* pane);

        void    showPane(QWidget* pPane);

    private:

        QStackedWidget* m_pWidgets;
};

#endif // CSTACKEDPANE_H
