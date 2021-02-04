#ifndef CGRAPHICSDELETABLEBUTTON_H
#define CGRAPHICSDELETABLEBUTTON_H

#include <QObject>
#include <QGraphicsProxyWidget>

class CGraphicsDeletableButton : public QGraphicsProxyWidget
{
    Q_OBJECT

    public:

        CGraphicsDeletableButton(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());

    signals:

        void    doDelete();

    protected:

        void    hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        void    hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:

        void    showDeleteButton();
        void    hideDeleteButton();

    private:

        QGraphicsProxyWidget*   m_pProxyDeleteBtn = nullptr;
};

#endif // CGRAPHICSDELETABLEBUTTON_H
