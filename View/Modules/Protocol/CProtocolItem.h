#ifndef CPROTOCOLITEM_H
#define CPROTOCOLITEM_H

#include <QGraphicsPathItem>
#include "Main/AppDefine.hpp"
#include "Core/CProtocol.h"
#include "View/Common/CGraphicsItemTools.hpp"

class QGraphicsProxyWidget;
class CProtocolPortItem;
class CProtocolConnection;

class CProtocolItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

    public:

        enum { Type = UserType + ProtocolGraphicsItem::TASK };

        CProtocolItem(QGraphicsItem* parent = Q_NULLPTR);
        CProtocolItem(QString name, ProtocolVertex getId, QGraphicsItem* parent = Q_NULLPTR);
        virtual ~CProtocolItem();

        // Setters
        void                            setHeaderColor(QColor bgColor, QColor textColor);
        void                            setBodyColor(QColor color);
        void                            setLineColor(QColor lineColor, QColor lineSelectedColor);
        void                            setStatus(CProtocolTask::State status);
        void                            setStatusMsg(const QString& msg);
        void                            setInputPortAssigned(int index, bool bAssigned);
        void                            setSize(QSize itemSize);
        void                            setActionButtonSize(int size);
        void                            setPortRadius(float radius);
        void                            setIOInfo(const CDataInfoPtr &info, int index, bool bInput);

        // Getters
        ProtocolVertex                  getId() const;
        int                             type() const override;
        int                             getInputPortCount() const;
        int                             getOutputPortCount() const;
        int                             getInputConnectionCount() const;
        int                             getOutputConnectionCount() const;
        CProtocolPortItem*              getInputPort(int index);
        CProtocolPortItem*              getOutputPort(int index);
        QVector<CProtocolConnection*>   getConnections() const;

        bool                            isConnectedTo(const ProtocolVertex& id) const;

        void                            removeInputPort(int index);
        void                            removeOutputPort(int index);

        void                            paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        void                            updateConnections();
        void                            updatePorts(const ProtocolTaskPtr &pTask);
        void                            updateActions(const std::map<CProtocolTask::ActionFlag,bool>& flags);
        void                            updateItem();

        void                            prepareRemove();

    signals:

        void                            doGetIOInfo(const ProtocolVertex& taskId, int index, bool bInput);

    protected:

        void                            hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        void                            hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    private:

        void                            initHeader();
        void                            initBody();

        void                            paintStatus(QPainter* painter, QRect rect);

        void                            addInputPort(IODataType type, const QColor &color);
        void                            addOutputPort(IODataType type, const QColor& color);
        QPushButton *                   addActionButton(CProtocolTask::ActionFlag flag, const QString &iconUncheckedPath, const QString &iconCheckedPath, const QString &text);

        void                            updateInputPorts(const ProtocolTaskPtr &pTask);
        void                            updateOutputPorts(const ProtocolTaskPtr &pTask);
        void                            updateActionButtonsPos();

    private:

        using ActionWidgets = QMap<CProtocolTask::ActionFlag, QGraphicsProxyWidget*>;

        QString                         m_name;
        ProtocolVertex                  m_id;
        CGraphicsInactiveTextItem*      m_pHeaderTextItem = nullptr;
        QPainterPath                    m_headerPath;
        QPainterPath                    m_bodyPath;
        QVector<CProtocolPortItem*>     m_inputPorts;
        QVector<CProtocolPortItem*>     m_outputPorts;
        ActionWidgets                   m_actionWidgets;
        int                             m_width = 90;
        int                             m_height = 90;
        int                             m_headerHeight = 26;
        int                             m_topMargin = 1;
        int                             m_actionButtonSize = 16;
        float                           m_lineSize = 2;
        float                           m_portRadius = 4;
        QColor                          m_headerBckColor = Qt::darkCyan;
        QColor                          m_headerTextColor = Qt::white;
        QColor                          m_bodyBckColor = Qt::lightGray;
        QColor                          m_lineColor = Qt::darkCyan;
        QColor                          m_lineSelectedColor = Qt::darkRed;
        QLinearGradient                 m_bodyGradient;
        QLinearGradient                 m_bodyGradientLight;
        QLinearGradient                 m_headerGradient;
        CProtocolTask::State            m_status = CProtocolTask::State::UNDONE;
        QRect                           m_statusRect;
        QString                         m_statusMsg = QObject::tr("Idle");
};

#endif // CPROTOCOLITEM_H
