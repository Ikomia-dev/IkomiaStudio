// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CProtocolItem.h"
#include <QGraphicsProxyWidget>
#include <QTextOption>
#include <QGraphicsDropShadowEffect>
#include "CProtocolPortItem.h"
#include "CProtocolScene.h"
#include "Main/AppTools.hpp"

CProtocolItem::CProtocolItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setZValue(1.0);
}

CProtocolItem::CProtocolItem(QString name, ProtocolVertex id, QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    m_name = name;
    m_id = id;
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setZValue(1.0);
}

CProtocolItem::~CProtocolItem()
{
    delete m_pHeaderTextItem;
}

void CProtocolItem::setHeaderColor(QColor bgColor, QColor textColor)
{
    m_headerBckColor = bgColor;
    m_headerTextColor = textColor;
}

void CProtocolItem::setBodyColor(QColor color)
{
    m_bodyBckColor = color;
}

void CProtocolItem::setLineColor(QColor lineColor, QColor lineSelectedColor)
{
    m_lineColor = lineColor;
    m_lineSelectedColor = lineSelectedColor;
}

void CProtocolItem::setStatus(CProtocolTask::State status)
{
    m_status = status;
    if(m_status == CProtocolTask::State::VALIDATE)
        m_statusMsg = QObject::tr("Success");
    else if(m_status == CProtocolTask::State::UNDONE)
        m_statusMsg = QObject::tr("Idle");
    else
        m_statusMsg = QObject::tr("Error");

    update();
}

void CProtocolItem::setStatusMsg(const QString& msg)
{
    m_statusMsg = msg;
}

void CProtocolItem::setInputPortAssigned(int index, bool bAssigned)
{
    if(index < m_inputPorts.size())
    {
        m_inputPorts[index]->setAssigned(bAssigned);
        m_inputPorts[index]->update();
    }
}

void CProtocolItem::setSize(QSize itemSize)
{
    assert(itemSize.width() > 0 && itemSize.height() > 0);
    m_width = itemSize.width();
    m_height = itemSize.height();
}

void CProtocolItem::setActionButtonSize(int size)
{
    m_actionButtonSize = size;
}

void CProtocolItem::setPortRadius(float radius)
{
    m_portRadius = radius;
}

void CProtocolItem::setIOInfo(const CDataInfoPtr &info, int index, bool bInput)
{
    if(bInput)
    {
        if(index >= 0 && index < m_inputPorts.size())
            m_inputPorts[index]->setInfo(info);
    }
    else
    {
        if(index >= 0 && index < m_outputPorts.size())
            m_outputPorts[index]->setInfo(info);
    }
}

ProtocolVertex CProtocolItem::getId() const
{
    return m_id;
}

int CProtocolItem::type() const
{
    return Type;
}

int CProtocolItem::getInputPortCount() const
{
    return m_inputPorts.size();
}

int CProtocolItem::getOutputPortCount() const
{
    return m_outputPorts.size();
}

int CProtocolItem::getInputConnectionCount() const
{
    int nb = 0;
    for(int i=0; i<m_inputPorts.size(); ++i)
        nb += m_inputPorts[i]->getConnectionCount();

    return nb;
}

int CProtocolItem::getOutputConnectionCount() const
{
    int nb = 0;
    for(int i=0; i<m_outputPorts.size(); ++i)
        nb += m_outputPorts[i]->getConnectionCount();

    return nb;
}

CProtocolPortItem *CProtocolItem::getInputPort(int index)
{
    if(index >= 0 && index < m_inputPorts.size())
        return m_inputPorts[index];
    else
        return nullptr;
}

CProtocolPortItem *CProtocolItem::getOutputPort(int index)
{
    if(index >= 0 && index < m_outputPorts.size())
        return m_outputPorts[index];
    else
        return nullptr;
}

QVector<CProtocolConnection*> CProtocolItem::getConnections() const
{
    QVector<CProtocolConnection*> connections;

    //Input connections
    for(int i=0; i<m_inputPorts.size(); ++i)
    {
        auto portConnections = m_inputPorts[i]->getConnections();
        connections.append(portConnections);
    }

    //Output connections
    for(int i=0; i<m_outputPorts.size(); ++i)
    {
        auto portConnections = m_outputPorts[i]->getConnections();
        connections.append(portConnections);
    }
    return connections;
}

bool CProtocolItem::isConnectedTo(const ProtocolVertex &id) const
{
    for(int i=0; i < m_inputPorts.size(); ++i)
    {
        if(m_inputPorts[i]->isConnectedTo(id))
            return true;
    }

    for(int i=0; i < m_outputPorts.size(); ++i)
    {
        if(m_outputPorts[i]->isConnectedTo(id))
            return true;
    }
    return false;
}

void CProtocolItem::addInputPort(IODataType type, const QColor& color)
{
    auto pPort = new CProtocolPortItem(m_inputPorts.size(), true, m_id, type, color, this);
    pPort->setRadius(m_portRadius);
    float radius = pPort->getRadius();
    qreal top = m_headerHeight + (2 * radius);
    qreal dy = m_inputPorts.size() * 3 * radius;
    pPort->setPos(0, top + dy);
    m_inputPorts.push_back(pPort);

    if(top+dy>m_height)
    {
         m_height += 3 * radius;
         QPainterPath p;
         p.setFillRule(Qt::WindingFill);
         p.addRoundedRect(0, 0, m_width, m_height, m_width*0.1, m_width*0.1, Qt::AbsoluteSize);
         m_bodyPath = p;
         setPath(p);
    }
}

void CProtocolItem::addOutputPort(IODataType type, const QColor& color)
{
    auto pPort = new CProtocolPortItem(m_outputPorts.size(), false, m_id, type, color, this);
    pPort->setRadius(m_portRadius);
    float radius = pPort->getRadius();
    qreal top = m_headerHeight + (2 * radius);
    qreal dy = m_outputPorts.size() * 3 * radius;
    pPort->setPos(m_width, top + dy);
    m_outputPorts.push_back(pPort);

    if(top+dy>m_height)
    {
         m_height += 3 * radius;
         QPainterPath p;
         p.setFillRule(Qt::WindingFill);
         p.addRoundedRect(0, 0, m_width, m_height, m_width*0.1, m_width*0.1, Qt::AbsoluteSize);
         m_bodyPath = p;
         setPath(p);
    }
}

QPushButton* CProtocolItem::addActionButton(CProtocolTask::ActionFlag flag, const QString& iconUncheckedPath, const QString& iconCheckedPath, const QString& text)
{
    //Style sheet
    auto pal = qApp->palette();
    auto color = pal.highlight().color();
    auto strStyleSheet = QString("QPushButton { width:%1px; height:%1px; background:transparent; border:none; image:url(%2); } \
                                  QPushButton:hover { border:1px solid%3; } \
                                  QPushButton::checked { image:url(%4); }")
                                 .arg(m_actionButtonSize)
                                 .arg(iconUncheckedPath)
                                 .arg(color.name())
                                 .arg(iconCheckedPath);

    //Create button
    auto pBtn = new QPushButton;
    pBtn->setToolTip(text);
    pBtn->setCheckable(true);
    pBtn->setStyleSheet(strStyleSheet);

    //Create proxy QGraphicsItem
    auto pProxy = new QGraphicsProxyWidget(this);
    pProxy->setWidget(pBtn);
    m_actionWidgets.insert(flag, pProxy);

    return pBtn;
}

void CProtocolItem::removeInputPort(int index)
{
    if(index < m_inputPorts.size())
    {
        delete m_inputPorts[index];
        m_inputPorts.erase(m_inputPorts.begin() + index);
    }
}

void CProtocolItem::removeOutputPort(int index)
{
    if(index < m_outputPorts.size())
    {
        delete m_outputPorts[index];
        m_outputPorts.erase(m_outputPorts.begin() + index);
    }
}

void CProtocolItem::initHeader()
{
    m_headerGradient.setStart(QPointF(m_width/2, 0));
    m_headerGradient.setFinalStop(QPointF(m_width/2, m_headerHeight/2));

    m_headerGradient.setColorAt(0.0, QColor(64,64,64));
    m_headerGradient.setColorAt(0.03, QColor(50,50,50));
    m_headerGradient.setColorAt(0.5, QColor(32,32,32));
    m_headerGradient.setColorAt(1.0, QColor(24,24,24));

    if(m_pHeaderTextItem)
        m_pHeaderTextItem->deleteLater();

    m_pHeaderTextItem = new CGraphicsInactiveTextItem(this);
    //Font
    QFont font = m_pHeaderTextItem->font();
    font.setPointSize(7);
    m_pHeaderTextItem->setFont(font);
    //Text width
    m_pHeaderTextItem->setTextWidth(m_width);
    //Set text
    QFontMetrics fontM(font);
    QString shortTitle = fontM.elidedText(m_name, Qt::TextElideMode::ElideRight, m_width);
    m_pHeaderTextItem->setPlainText(shortTitle);
    //Color
    m_pHeaderTextItem->setDefaultTextColor(m_headerTextColor);
    //Alignement
    QTextOption option = m_pHeaderTextItem->document()->defaultTextOption();
    option.setAlignment(Qt::AlignCenter);
    m_pHeaderTextItem->document()->setDefaultTextOption(option);
    //Position
    QRectF rcText = m_pHeaderTextItem->boundingRect();
    m_pHeaderTextItem->setPos(m_width/2 - rcText.width()/2, m_headerHeight/2 - rcText.height()/2 + m_topMargin);

    //Round rectangle area
    QPainterPath p;
    p.setFillRule(Qt::WindingFill);
    p.addRoundedRect(0, 0, m_width, m_headerHeight, m_width*0.1, m_width*0.1, Qt::AbsoluteSize);
    p.addRect(0, m_headerHeight/2, m_width, m_headerHeight/2 + 1);
    m_headerPath = p.simplified();
}

void CProtocolItem::initBody()
{
    m_bodyGradient.setStart(QPointF(m_width/2, 0));
    m_bodyGradient.setFinalStop(QPointF(m_width/2, m_height));

    int grad1 = 160;
    int grad2 = 100;
    int grad3 = 70;
    int grad4 = 24;

    m_bodyGradient.setColorAt(0.0, QColor(grad1,grad1,grad1,204));
    m_bodyGradient.setColorAt(0.80, QColor(grad2,grad2,grad2,204));
    m_bodyGradient.setColorAt(0.97, QColor(grad3,grad3,grad3,204));
    m_bodyGradient.setColorAt(1.0, QColor(grad4,grad4,grad4,204));

    grad1 *= 1.5;
    grad2 *= 1.5;
    grad3 *= 1.5;
    grad4 *= 1.5;

    m_bodyGradientLight.setStart(QPointF(m_width/2, 0));
    m_bodyGradientLight.setFinalStop(QPointF(m_width/2, m_height));

    m_bodyGradientLight.setColorAt(0.0, QColor(grad1,grad1,grad1,204));
    m_bodyGradientLight.setColorAt(0.80, QColor(grad2,grad2,grad2,204));
    m_bodyGradientLight.setColorAt(0.97, QColor(grad3,grad3,grad3,204));
    m_bodyGradientLight.setColorAt(1.0, QColor(grad4,grad4,grad4,204));

    m_bodyPath.setFillRule(Qt::WindingFill);
    m_bodyPath.addRoundedRect(0, 0, m_width, m_height, m_width*0.1, m_width*0.1, Qt::AbsoluteSize);
    setPath(m_bodyPath);

    auto pGraphicsEffect = graphicsEffect();
    if(pGraphicsEffect)
        pGraphicsEffect->deleteLater();

    auto pBodyShadow = new QGraphicsDropShadowEffect;
    pBodyShadow->setBlurRadius(9.0);
    pBodyShadow->setColor(QColor(0, 0, 0, 160));
    pBodyShadow->setOffset(4.0);
    setGraphicsEffect(pBodyShadow);
}

void CProtocolItem::paintStatus(QPainter* painter, QRect rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath outerEllipse;
    outerEllipse.addEllipse(rect);
    auto green = QColor(155,219,58);
    auto red = QColor(255,100,100);

    if(m_status == CProtocolTask::State::UNDONE)
    {
        painter->setPen(QPen(m_lineColor, 3));
        painter->setBrush(Qt::transparent);
    }
    else if(m_status == CProtocolTask::State::VALIDATE)
    {
        painter->setPen(QPen(m_lineColor, 3));
        painter->setBrush(green);
    }
    else
    {
        painter->setPen(QPen(m_lineColor, 3));
        painter->setBrush(red);
    }
    painter->drawPath(outerEllipse);
    painter->restore();
}

void CProtocolItem::updateInputPorts(const ProtocolTaskPtr &pTask)
{
    int itemPortNb = m_inputPorts.size();
    int taskPortNb = (int)pTask->getInputCount();

    for(int i=0; i<std::min(itemPortNb, taskPortNb); ++i)
    {
        IODataType type = pTask->getInputDataType(i);
        m_inputPorts[i]->setDataType(type);
        m_inputPorts[i]->setColor(Utils::Protocol::getPortColor(type));
    }

    if(itemPortNb < taskPortNb)
    {
        for(int i=itemPortNb; i<taskPortNb; ++i)
        {
            IODataType type = pTask->getInputDataType(i);
            auto color = Utils::Protocol::getPortColor(type);
            addInputPort(type, color);
        }
    }
    else if(itemPortNb > taskPortNb)
    {
        auto pScene = static_cast<CProtocolScene*>(scene());
        for(int i=itemPortNb-1; i>=taskPortNb; --i)
        {
            auto connections = m_inputPorts[i]->getConnections();
            for(int j=0; j<connections.size(); ++j)
                pScene->deleteConnection(connections[j], false, false, false);

            pScene->removeItem(m_inputPorts[i]);
            delete m_inputPorts[i];
            m_inputPorts.pop_back();
        }
    }

    //Check graphics inputs assignement
    for(int i=0; i<taskPortNb; ++i)
    {
        if(pTask->getInputDataType(i) == IODataType::INPUT_GRAPHICS)
        {
            auto pGraphicsInput = pTask->getInput(i);
            m_inputPorts[i]->setAssigned(pGraphicsInput->isDataAvailable());
        }
    }
}

void CProtocolItem::updateOutputPorts(const ProtocolTaskPtr &pTask)
{
    int itemPortNb = m_outputPorts.size();
    int taskPortNb = (int)pTask->getOutputCount();

    for(int i=0; i<std::min(itemPortNb, taskPortNb); ++i)
    {
        m_outputPorts[i]->setColor(Utils::Protocol::getPortColor(pTask->getOutputDataType(i)));
        m_outputPorts[i]->updateConnections();
    }

    if(itemPortNb < taskPortNb)
    {
        for(int i=itemPortNb; i<taskPortNb; ++i)
        {
            IODataType type = pTask->getOutputDataType(i);
            auto color = Utils::Protocol::getPortColor(type);
            addOutputPort(type, color);
        }
    }
    else if(itemPortNb > taskPortNb)
    {
        auto pScene = static_cast<CProtocolScene*>(scene());
        for(int i=itemPortNb-1; i>=taskPortNb; --i)
        {
            auto connections = m_outputPorts[i]->getConnections();
            for(int j=0; j<connections.size(); ++j)
                pScene->deleteConnection(connections[j], false, false, false);

            pScene->removeItem(m_outputPorts[i]);
            delete m_outputPorts[i];
            m_outputPorts.pop_back();
        }
    }
}

void CProtocolItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    //----- Draw body -----//
    QPen bodyPen;
    QBrush bodyBrush;
    QRect r = option->rect;

    bodyPen.setWidth(m_lineSize);
    bodyPen.setStyle(Qt::NoPen);

    if(isSelected())
    {
        bodyPen.setColor(m_lineSelectedColor);
        bodyBrush = m_bodyGradientLight;
    }
    else
    {
        bodyPen.setColor(m_lineColor);
        bodyBrush = m_bodyGradient;
    }

    painter->setPen(bodyPen);
    painter->setBrush(bodyBrush);
    painter->drawPath(m_bodyPath);

    // Square size for ellipse
    auto minSize = std::min(r.width(), r.height());
    // Put the ellipse bottom right (works as long as width do not change)
    m_statusRect = QRect(0.75*r.width(), r.height() - 0.20*minSize, 0.15*minSize, 0.15*minSize);
    paintStatus(painter, m_statusRect);

    //----- Draw header -----//
    QBrush headerBrush;

    //Background header
    headerBrush.setStyle(Qt::SolidPattern);
    if(isSelected())
        headerBrush.setColor(m_headerBckColor.lighter());
    else
        headerBrush.setColor(m_headerBckColor);

    painter->setBrush(m_headerGradient);
    painter->drawPath(m_headerPath);

    // Manage drawing external contour
    bodyPen.setStyle(Qt::SolidLine);
    painter->setPen(bodyPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(0, 0, m_width, m_height, m_width*0.1, m_width*0.1, Qt::AbsoluteSize);
}

void CProtocolItem::updateConnections()
{
    for(int i=0; i < m_inputPorts.size(); ++i)
        m_inputPorts[i]->updateConnections();

    for(int i=0; i < m_outputPorts.size(); ++i)
        m_outputPorts[i]->updateConnections();
}

void CProtocolItem::updatePorts(const ProtocolTaskPtr &pTask)
{
    assert(pTask);
    //Update inputs
    updateInputPorts(pTask);
    //Update outputs
    updateOutputPorts(pTask);
}

void CProtocolItem::updateActions(const std::map<CProtocolTask::ActionFlag, bool> &flags)
{
    //Remove widgets not wanted anymore
    for(auto it=m_actionWidgets.begin(); it!=m_actionWidgets.end();)
    {
        auto itFlag = flags.find(static_cast<CProtocolTask::ActionFlag>(it.key()));
        if(itFlag == flags.end())
        {
            it.value()->deleteLater();
            it = m_actionWidgets.erase(it);
        }
        else
            ++it;
    }

    for(auto it=flags.begin(); it!=flags.end(); ++it)
    {
        auto flag = it->first;
        auto itWidgets = m_actionWidgets.find(flag);

        if(itWidgets != m_actionWidgets.end())
        {
            //Modification du bouton existant
            QPushButton* pBtn = static_cast<QPushButton*>(itWidgets.value()->widget());
            if(pBtn)
                pBtn->setChecked(it->second);
        }
        else
        {
            //Creation du bouton en fonction de l'action voulue
            QPushButton* pBtn = nullptr;
            switch(flag)
            {
                case CProtocolTask::ActionFlag::APPLY_VOLUME:
                    pBtn = addActionButton(flag,
                                           ":/Images/draw-layers-current.png",
                                           ":/Images/draw-layers.png",
                                           QObject::tr("Apply on volume"));
                    break;

                case CProtocolTask::ActionFlag::OUTPUT_AUTO_EXPORT:
                    pBtn = addActionButton(flag,
                                           ":/Images/save-as-disable.png",
                                           ":/Images/save-as.png",
                                           QObject::tr("Outputs auto-export"));
                    break;
            }
            if(pBtn)
            {
                pBtn->setChecked(it->second);
                connect(pBtn, &QPushButton::clicked, [this, pBtn, flag]()
                {
                    auto pScene = static_cast<CProtocolScene*>(scene());
                    pScene->setTaskActionFlag(m_id, flag, pBtn->isChecked());
                });
            }
        }
    }
    updateActionButtonsPos();
}

void CProtocolItem::updateActionButtonsPos()
{
    if(m_actionWidgets.size() == 0)
        return;

    const int space = 0;
    const int btnCount = m_actionWidgets.size();
    const QRectF btnRect = m_actionWidgets.begin().value()->rect();
    const int lineW = (btnCount * btnRect.width()) + ((btnCount - 1) * space);
    const int xmin = (m_width - lineW) / 2;
    const int y = m_headerHeight + ((m_height - m_headerHeight - btnRect.height()) / 2);
    int index = 0;

    for(auto it=m_actionWidgets.begin(); it!=m_actionWidgets.end(); ++it)
    {
        it.value()->setPos(QPointF(xmin + (index * (btnRect.width() + space)), y));
        index++;
    }
}

void CProtocolItem::updateItem()
{
    initHeader();
    initBody();
}

void CProtocolItem::prepareRemove()
{
    //Workaround to prevent QT bug when removing from the scene a QGraphicsItem with QGraphicsDropShadowEffect
    prepareGeometryChange();
}

void CProtocolItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverEnterEvent(event);
}

void CProtocolItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    auto currentPt = event->pos().toPoint();
    if(m_statusRect.contains(currentPt))
        setToolTip(m_statusMsg);
    else
        setToolTip("");

    QGraphicsItem::hoverMoveEvent(event);
}

#include "moc_CProtocolItem.cpp"
