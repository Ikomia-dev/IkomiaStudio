#ifndef CWIDGETDATADISPLAY_H
#define CWIDGETDATADISPLAY_H

#include "CDataDisplay.h"

class CWidgetDataDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        CWidgetDataDisplay(QWidget* pWidget, QWidget* pParent = nullptr, bool bDeleteWidget = true, int flags=CDataDisplay::MAXIMIZE_BUTTON);
        ~CWidgetDataDisplay();

    protected:
        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout();

        void            removeWidget();

    public slots:

        virtual void    onClose();

    private:

        QWidget*    m_pWidget = nullptr;
        bool        m_bDeleteWidget = true;
};

#endif // CWIDGETDATADISPLAY_H
