/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CWORKFLOWITEM_H
#define CWORKFLOWITEM_H

#include <QGraphicsPathItem>
#include "Main/AppDefine.hpp"
#include "Core/CWorkflow.h"
#include "View/Common/CGraphicsItemTools.hpp"

class QGraphicsProxyWidget;
class CWorkflowPortItem;
class CWorkflowConnection;

class CWorkflowItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

    public:

        enum { Type = UserType + WorkflowGraphicsItem::TASK };

        CWorkflowItem(QGraphicsItem* parent = Q_NULLPTR);
        CWorkflowItem(QString name, WorkflowVertex getId, QGraphicsItem* parent = Q_NULLPTR);
        virtual ~CWorkflowItem();

        // Setters
        void                            setHeaderColor(QColor bgColor, QColor textColor);
        void                            setBodyColor(QColor color);
        void                            setLineColor(QColor lineColor, QColor lineSelectedColor);
        void                            setStatus(CWorkflowTask::State status);
        void                            setStatusMsg(const QString& msg);
        void                            setInputPortAssigned(int index, bool bAssigned);
        void                            setSize(QSize itemSize);
        void                            setActionButtonSize(int size);
        void                            setPortRadius(float radius);
        void                            setIOInfo(const CDataInfoPtr &info, int index, bool bInput);

        // Getters
        WorkflowVertex                  getId() const;
        int                             type() const override;
        int                             getInputPortCount() const;
        int                             getOutputPortCount() const;
        int                             getInputConnectionCount() const;
        int                             getOutputConnectionCount() const;
        CWorkflowPortItem*              getInputPort(int index);
        CWorkflowPortItem*              getOutputPort(int index);
        QVector<CWorkflowConnection*>   getConnections() const;

        bool                            isConnectedTo(const WorkflowVertex& id) const;

        void                            removeInputPort(int index);
        void                            removeOutputPort(int index);

        void                            paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        void                            updateConnections();
        void                            updatePorts(const WorkflowTaskPtr &pTask);
        void                            updateActions(const std::map<CWorkflowTask::ActionFlag,bool>& flags);
        void                            updateItem();

        void                            prepareRemove();

    signals:

        void                            doGetIOInfo(const WorkflowVertex& taskId, int index, bool bInput);

    protected:

        void                            hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        void                            hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    private:

        void                            initHeader();
        void                            initBody();

        void                            paintStatus(QPainter* painter, QRect rect);

        void                            addInputPort(IODataType type, const QColor &color);
        void                            addOutputPort(IODataType type, const QColor& color);
        QPushButton *                   addActionButton(CWorkflowTask::ActionFlag flag, const QString &iconUncheckedPath, const QString &iconCheckedPath, const QString &text);

        void                            updateInputPorts(const WorkflowTaskPtr &pTask);
        void                            updateOutputPorts(const WorkflowTaskPtr &pTask);
        void                            updateActionButtonsPos();

    private:

        using ActionWidgets = QMap<CWorkflowTask::ActionFlag, QGraphicsProxyWidget*>;

        QString                         m_name;
        WorkflowVertex                  m_id;
        CGraphicsInactiveTextItem*      m_pHeaderTextItem = nullptr;
        QPainterPath                    m_headerPath;
        QPainterPath                    m_bodyPath;
        QVector<CWorkflowPortItem*>     m_inputPorts;
        QVector<CWorkflowPortItem*>     m_outputPorts;
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
        CWorkflowTask::State            m_status = CWorkflowTask::State::UNDONE;
        QRect                           m_statusRect;
        QString                         m_statusMsg = QObject::tr("Idle");
};

#endif // CWORKFLOWITEM_H
