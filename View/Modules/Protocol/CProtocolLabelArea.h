#ifndef CPROTOCOLLABELAREA_H
#define CPROTOCOLLABELAREA_H

#include <QGraphicsPathItem>
#include <QLinearGradient>
#include "Main/AppDefine.hpp"

class CProtocolLabelArea : public QGraphicsPathItem
{
    public:

        enum { Type = UserType + ProtocolGraphicsItem::LABEL_AREA };

        CProtocolLabelArea(QString label, QGraphicsItem* parent = Q_NULLPTR);

        //Getters
        int     type() const override;
        QSize   getSize() const;

        //Setters
        void    setSize(QSize size);
        void    setTextColor(QColor color);
        void    setColorBg(QColor color);
        void    setLineColor(QColor color);

        void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

        void    update();

    private:

        void    initText();
        void    initArea();

    protected:

        QString         m_label;
        QSize           m_size = QSize(80,120);
        int             m_topMargin = 1;
        int             m_topContent = 1;
        int             m_lineSize = 2;
        QPainterPath    m_areaPath;
        QPainterPath    m_textPath;
        QColor          m_textColor = Qt::white;
        QColor          m_bckColor = Qt::darkBlue;
        QColor          m_lineColor = Qt::blue;
        QColor          m_lineSelected;
        QLinearGradient m_areaGradient;
};

#endif // CPROTOCOLLABELAREA_H
